/*******************************************************************************
#      ____               __          __  _      _____ _       _               #
#     / __ \              \ \        / / | |    / ____| |     | |              #
#    | |  | |_ __   ___ _ __ \  /\  / /__| |__ | |  __| | ___ | |__   ___      #
#    | |  | | '_ \ / _ \ '_ \ \/  \/ / _ \ '_ \| | |_ | |/ _ \| '_ \ / _ \     #
#    | |__| | |_) |  __/ | | \  /\  /  __/ |_) | |__| | | (_) | |_) |  __/     #
#     \____/| .__/ \___|_| |_|\/  \/ \___|_.__/ \_____|_|\___/|_.__/ \___|     #
#           | |                                                                #
#           |_|                                                                #
#                                                                              #
#                                (c) 2011 by                                   #
#           University of Applied Sciences Northwestern Switzerland            #
#                     Institute of Geomatics Engineering                       #
#                           martin.christen@fhnw.ch                            #
********************************************************************************
*     Licensed under MIT License. Read the file LICENSE for more information   *
*******************************************************************************/

#include "ElevationReader.h"

#include "ogprocess.h"
#include "math/mathutils.h"
#include "io/FileSystem.h"
#include "CoordinateTransformation.h"
#include "string/StringUtils.h"
#include "xml/xml.h"

#include <gdal.h>
#include <gdal_priv.h>
#include <cstdlib>
#include <fstream>
#include <cassert>
#include <iostream>
#include <sstream>

#define _pGDALDataset ((GDALDataset*)_pDataset)
#define _pElvBand     ((GDALRasterBand*)_pElv) 



ElevationReader::ElevationReader()
{
   _pDataset = 0;
   _pElv = 0;
   _pBlockElv = 0;
}

//-----------------------------------------------------------------------------

ElevationReader::~ElevationReader()
{
   _Free();
}

//-----------------------------------------------------------------------------

void ElevationReader::_Free()
{
   Close();

   if (_pBlockElv)
   {
      free(_pBlockElv);
      _pBlockElv = 0;
   }

   _pElv = 0;
}

//-----------------------------------------------------------------------------

bool ElevationReader::Open(std::string& sFilename, int nSourceEPSG, int nDestEPSG)
{
   _nSourceEPSG = nSourceEPSG;
   _nDestEPSG = nDestEPSG;
   _sFilename = sFilename;
   _Free();
   _bImportXYZ = false;

   if (sFilename.length()>3)
   {
      std::string sFileType = StringUtils::Right(sFilename,4);

      if (sFileType == std::string(".xyz"))
      {
         _bImportXYZ = true;
      }
   }

   if (_bImportXYZ)
   {
      return FileSystem::FileExists(sFilename);
   }
   else
   {
      bool bOk = false;
      _pDataset = (void*)GDALOpen(sFilename.c_str(), GA_ReadOnly);   
      if(_pGDALDataset)
      {
         int nBands = _pGDALDataset->GetRasterCount();
         _nRasterSizeX = _pGDALDataset->GetRasterXSize();
         _nRasterSizeY = _pGDALDataset->GetRasterYSize();

         if (nBands>=1)
         {   
            _pElv = (void*)_pGDALDataset->GetRasterBand(1);

            if (_pElv )
            {
               int bx1,by1;

               _pElvBand->GetBlockSize(&bx1, &by1);
               _dNoDataValue = _pElvBand->GetNoDataValue();

               /*std::cout << "-------------------------------------\n";
               std::cout << "No Data Value: " << _dNoDataValue << "\n";
               std::cout << "-------------------------------------\n";*/

               GDALDataType rdd = _pElvBand->GetRasterDataType();

               switch (rdd)
               {
               case GDT_UInt16:
                  _datatype_bytes = GDALGetDataTypeSize(GDT_UInt16) / 8;
                  _datatype = 5;
                  break;
               case GDT_Int16:
                  _datatype_bytes = GDALGetDataTypeSize(GDT_Int16) / 8;
                  _datatype = 6;
                  break;
               case GDT_UInt32:
                  _datatype = 1;
                  _datatype_bytes = GDALGetDataTypeSize(GDT_UInt32) / 8;
                  break;
               case GDT_Int32:
                  _datatype = 2;
                  _datatype_bytes = GDALGetDataTypeSize(GDT_Int32) / 8;
                  break;
               case GDT_Float32:
                  _datatype = 3;
                  _datatype_bytes = GDALGetDataTypeSize(GDT_Float32) / 8;
                  break;
               case GDT_Float64:
                  _datatype = 4;
                  _datatype_bytes = GDALGetDataTypeSize(GDT_Float64) / 8;
                  break;
               default:
                  Close();
                  return false;
               }

               _pGDALDataset->GetGeoTransform(_affineTransformation);
               ProcessingUtils::InvertGeoMatrix(_affineTransformation, _affineTransformation_inverse);
               _ulx = _affineTransformation[0];
               _uly = _affineTransformation[3];
               _lrx = _ulx + _affineTransformation[1] * _nRasterSizeX;
               _lry = _uly + _affineTransformation[5] * _nRasterSizeY;

               if (bx1 != 0 && by1 != 0)
               {
                  _nBlockWidth = bx1;
                  _nBlockHeight = by1;

                  _nXBlocks = (_pElvBand->GetXSize() + _nBlockWidth - 1) / _nBlockWidth;
                  _nYBlocks = (_pElvBand->GetYSize() + _nBlockHeight - 1) / _nBlockHeight;

                  _pBlockElv = malloc(_datatype_bytes * _nBlockWidth*_nBlockHeight);

                  bOk = true;
               }
            }
         }

         if (!bOk)
         {
            Close();
            return false;
         }
      }

      return bOk;
   }


   return false;
}

//------------------------------------------------------------------------------

void ElevationReader::Close()
{
   if (_pDataset)
   {
      GDALClose(_pGDALDataset);
      _pDataset = 0;
   }
}

//------------------------------------------------------------------------------

// Transform Elevation points and write points to binary stream. Returns points written and bounding box
bool ElevationReader::Import(std::vector<ElevationPoint>& result, double& inout_xmin, double& inout_ymin, double& inout_xmax, double& inout_ymax)
{
      if (_bImportXYZ)
      {
         return _ImportXYZ(result, inout_xmin, inout_ymin, inout_xmax, inout_ymax);
      }
      else
      {
         return _ImportRaster(result, inout_xmin, inout_ymin, inout_xmax, inout_ymax);
      }

}

//------------------------------------------------------------------------------

void ElevationReader::_ReadBlock(int bx, int by, int& valid_width, int& valid_height)
{
   _pElvBand->ReadBlock( bx, by, _pBlockElv );

   // Compute the portion of the block that is valid
   // for partial edge blocks.
   if( (bx+1) * _nBlockWidth > _pElvBand->GetXSize() )
      valid_width = _pElvBand->GetXSize() - bx * _nBlockWidth;
   else
      valid_width = _nBlockWidth;

   if( (by+1) * _nBlockHeight > _pElvBand->GetYSize() )
   {
      valid_height = _pElvBand->GetYSize() - by * _nBlockHeight;
   }
   else
   {
      valid_height = _nBlockHeight;
   }
}

//------------------------------------------------------------------------------

bool ElevationReader::_ImportXYZ(std::vector<ElevationPoint>& result, double& inout_xmin, double& inout_ymin, double& inout_xmax, double& inout_ymax)
{
   CoordinateTransformation* pCT = 0;

   if (_nSourceEPSG != 0)
   {
      if (_nSourceEPSG != _nDestEPSG)
      {
         pCT = new CoordinateTransformation(_nSourceEPSG, _nDestEPSG);
      }
   }
  
   std::ifstream myfile(_sFilename.c_str(), std::ios::in);
   char line[4096];
   std::string sLine;

   int numColumns = 0;
   ElevationPoint pt;
   int nPointsWritten = 0;

   if (myfile.good())
   {
      while (!myfile.eof())
      {
         std::vector<double> vOut;
         myfile.getline(line, 4095);
         sLine = line;
         if (sLine.size()>0)
         {
            Tokenize(sLine, ' ', vOut);
            if (nPointsWritten == 0)
            {
               numColumns = vOut.size();  // number of columns MUST be constant! otherwise the point is wrong!!
            }

            if (vOut.size() >= 3 && vOut.size() == numColumns)
            {
               double x = vOut[0];
               double y = vOut[1];
               double z = vOut[2];

               if (pCT)
               {
                  pCT->Transform(&x, &y);
               }

               pt.x = x;
               pt.y = y;
               pt.elevation = z;
               pt.weight = 0; 
               result.push_back(pt);

               nPointsWritten++;

               inout_xmin = math::Min<double>(inout_xmin, x);
               inout_ymin = math::Min<double>(inout_ymin, y);
               
               inout_xmax = math::Max<double>(inout_xmax, x);
               inout_ymax = math::Max<double>(inout_ymax, y);
 
            }
         }
      }

   }
   myfile.close();

   if (pCT)
   {
      delete pCT;
   }

   return true;
}

//------------------------------------------------------------------------------

bool ElevationReader::_ImportRaster(std::vector<ElevationPoint>& result, double& inout_xmin, double& inout_ymin, double& inout_xmax, double& inout_ymax)
{
   int valid_width; int valid_height;
   double fx,fy,fz;
   unsigned int ui_value;
   int i_value;
   float f_value;
   double d_value;
   unsigned short us_value;
   short s_value;

   CoordinateTransformation* pCT = 0;

   if (_nSourceEPSG != 0)
   {
      if (_nSourceEPSG != _nDestEPSG)
         pCT = new CoordinateTransformation(_nSourceEPSG, _nDestEPSG);
   }

   //---------------------------------------------------------------------------
   ElevationPoint pt;

   for(int iYBlock = 0; iYBlock < _nYBlocks; iYBlock++ )
   {
      for(int iXBlock = 0; iXBlock < _nXBlocks; iXBlock++ )
      {
         _ReadBlock(iXBlock, iYBlock, valid_width, valid_height);

         int BaseX = _nBlockWidth * iXBlock;
         int BaseY = _nBlockHeight * iYBlock;

         for (int y=0;y<valid_height;y++)
         {
            for (int x=0;x<valid_width;x++)
            {
               bool bNoData = true;
               GetSourceCoord(double(x+BaseX), double(y+BaseY), &fx, &fy);
               switch(_datatype)
               {
               case 1:  // GDT_UInt32
                  ui_value = ((unsigned int*)_pBlockElv)[y*_nBlockWidth+x];
                  if (ui_value!=_dNoDataValue)
                  {
                     fz = (double) ui_value;
                     bNoData = false;
                  }
                  break;
               case 2:  // GDT_Int32
                  i_value = ((int*)_pBlockElv)[y*_nBlockWidth+x];
                  if (i_value!=_dNoDataValue)
                  {
                     fz = (double) i_value;
                     bNoData = false;
                  }
                  break;
               case 3:  // GDT_Float32
                  f_value = ((float*)_pBlockElv)[y*_nBlockWidth+x];
                  if (f_value!=_dNoDataValue)
                  {
                     fz = (double) f_value;
                     bNoData = false;
                  }
                  break;
               case 4:  // GDT_Float64
                  d_value = ((double*)_pBlockElv)[y*_nBlockWidth+x];
                  if (d_value!=_dNoDataValue)
                  {
                     fz = d_value;
                     bNoData = false;
                  }
                  break;
               case 5: 
                  {
                     unsigned short* pElvData = ((unsigned short*)_pBlockElv);
                     us_value = pElvData[y*_nBlockWidth+x];
                     if (us_value!=_dNoDataValue)
                     {
                        fz = (double) us_value;
                        bNoData = false;
                     }
                  }
                  break;
               case 6:  
                  {
                     short* pElvData = ((short*)_pBlockElv);
                     s_value = pElvData[y*_nBlockWidth+x];
                     if (s_value!=_dNoDataValue)
                     {
                        fz = (double) s_value;
                        bNoData = false;
                     }
                  }
                  break;

               default:
                  assert(false);
               }

               // Transform Point:

               if (!bNoData)
               {

                  if (pCT)
                  {  
                     pCT->Transform(&fx, &fy);
                  }

                  pt.x = fx;
                  pt.y = fy;
                  pt.elevation = fz;
                  pt.weight = 0;
                  result.push_back(pt);

                  inout_xmax = math::Max<double>(inout_xmax, fx);
                  inout_ymax = math::Max<double>(inout_ymax, fy);
                  inout_xmin = math::Min<double>(inout_xmin, fx);
                  inout_ymin = math::Min<double>(inout_ymin, fy);
               }
            }
         }
      }
   }

   return true;
}
