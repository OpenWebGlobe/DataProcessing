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

#include "ogprocess.h"

#include "gdal.h"
#include "gdal_priv.h"
#include "ogr_api.h"
#include "cpl_conv.h"

#include "io/FileSystem.h"
#include <iostream>

namespace ProcessingUtils
{
   //---------------------------------------------------------------------------
   OPENGLOBE_API bool init_gdal()
   {
      // find gdal-data directory
      bool has_data_dir = FileSystem::DirExists("gdal-data");

      GDALAllRegister();
      CPLSetConfigOption("GDAL_CACHEMAX", "0"); // don't need a gdal cache
      if (has_data_dir) 
      {
         CPLSetConfigOption("GDAL_DATA", "gdal-data");
      }
      OGRRegisterAll();

#ifdef OS_WINDOWS
      return has_data_dir;
#else
      return true;
#endif
   }

   //---------------------------------------------------------------------------

   OPENGLOBE_API void exit_gdal()
   {
      GDALDestroyDriverManager();
      OGRCleanupAll();
   }

   //---------------------------------------------------------------------------

   OPENGLOBE_API void RetrieveDatasetInfo(const std::string& filename, CoordinateTransformation* pCT, DataSetInfo* pDataset, bool bVerbose)
   {
      pDataset->bGood=false;
      pDataset->sFilename = filename;

      GDALDataset* s_fh = (GDALDataset*)GDALOpen(filename.c_str(), GA_ReadOnly);
      if(s_fh)
      {
         pDataset->bGood = true;
         s_fh->GetGeoTransform(pDataset->affineTransformation);
         pDataset->nBands = s_fh->GetRasterCount();
         pDataset->nSizeX = s_fh->GetRasterXSize();
         pDataset->nSizeY = s_fh->GetRasterYSize();

         if (bVerbose)
         {
            std::cout << "[OK]   " << filename.c_str() << "\n";
         }

         GDALClose(s_fh);
      }
      else
      {
         if (bVerbose)
         {
            std::cout << "[FAILED]   " << filename.c_str() << "\n";
         }
      }
 
      //------------------------------------------------------------------------
      if (pDataset->bGood)
      {
         if (!ProcessingUtils::InvertGeoMatrix(pDataset->affineTransformation, pDataset->affineTransformation_inverse))
         {
            std::cout << "**FAILED calculating invert of affine transformation of " << filename.c_str() << "\n";
            pDataset->bGood = false;
         }
   
         double dPixelWidth  = pDataset->affineTransformation[1];
         double dPixelHeight = pDataset->affineTransformation[5];

         double ulx = pDataset->affineTransformation[0];
         double uly = pDataset->affineTransformation[3];
         double lrx = ulx + pDataset->affineTransformation[1] * pDataset->nSizeX;
         double lry = uly + pDataset->affineTransformation[5] * pDataset->nSizeY;

         pDataset->dest_ulx = 1e20;
         pDataset->dest_lry = 1e20;
         pDataset->dest_lrx = -1e20;
         pDataset->dest_uly = -1e20;

         //Transform every pixel along border
         for (int p=0;p<=pDataset->nSizeX;p++)
         {
            unsigned long x,y;
            double lng,lat;  // (this is actually in mercator projection)
            x = p;
            y = 0;
            lat = pDataset->affineTransformation[3] + double(x)*pDataset->affineTransformation[4] + double(y)*pDataset->affineTransformation[5];
            lng = pDataset->affineTransformation[0] + double(x)*pDataset->affineTransformation[1] + double(y)*pDataset->affineTransformation[2];
            pCT->Transform(&lng, &lat);
            pDataset->dest_ulx = math::Min<double>(lng, pDataset->dest_ulx);
            pDataset->dest_lry = math::Min<double>(lat, pDataset->dest_lry);
            pDataset->dest_lrx = math::Max<double>(lng, pDataset->dest_lrx);
            pDataset->dest_uly = math::Max<double>(lat, pDataset->dest_uly);
            x = p;
            y = pDataset->nSizeY;
            lat = pDataset->affineTransformation[3] + double(x)*pDataset->affineTransformation[4] + double(y)*pDataset->affineTransformation[5];
            lng = pDataset->affineTransformation[0] + double(x)*pDataset->affineTransformation[1] + double(y)*pDataset->affineTransformation[2];
            pCT->Transform(&lng, &lat);
            pDataset->dest_ulx = math::Min<double>(lng, pDataset->dest_ulx);
            pDataset->dest_lry = math::Min<double>(lat, pDataset->dest_lry);
            pDataset->dest_lrx = math::Max<double>(lng, pDataset->dest_lrx);
            pDataset->dest_uly = math::Max<double>(lat, pDataset->dest_uly);
         }
         for (int p=0;p<=pDataset->nSizeY;p++)
         {
            unsigned long x,y;
            double lng,lat; // (this is actually in mercator projection)
            x = 0;
            y = p;
            lat = pDataset->affineTransformation[3] + double(x)*pDataset->affineTransformation[4] + double(y)*pDataset->affineTransformation[5];
            lng = pDataset->affineTransformation[0] + double(x)*pDataset->affineTransformation[1] + double(y)*pDataset->affineTransformation[2];
            pCT->Transform(&lng, &lat);
            pDataset->dest_ulx = math::Min<double>(lng, pDataset->dest_ulx);
            pDataset->dest_lry = math::Min<double>(lat, pDataset->dest_lry);
            pDataset->dest_lrx = math::Max<double>(lng, pDataset->dest_lrx);
            pDataset->dest_uly = math::Max<double>(lat, pDataset->dest_uly);
            x = pDataset->nSizeX;
            y = p;
            lat = pDataset->affineTransformation[3] + double(x)*pDataset->affineTransformation[4] + double(y)*pDataset->affineTransformation[5];
            lng = pDataset->affineTransformation[0] + double(x)*pDataset->affineTransformation[1] + double(y)*pDataset->affineTransformation[2];
            pCT->Transform(&lng, &lat);
            pDataset->dest_ulx = math::Min<double>(lng, pDataset->dest_ulx);
            pDataset->dest_lry = math::Min<double>(lat, pDataset->dest_lry);
            pDataset->dest_lrx = math::Max<double>(lng, pDataset->dest_lrx);
            pDataset->dest_uly = math::Max<double>(lat, pDataset->dest_uly);
         }


         double x0 = 0.5*(lrx + ulx);
         double x1 = 0.5*(lrx + ulx) + dPixelWidth;
         double y0 = 0.5*(lry + uly);
         double y1 = 0.5*(lry + uly) + dPixelHeight; 

         pCT->Transform(&x0, &y0);
         pCT->Transform(&x1, &y1);

         double dest_dPixelWidth = 2*fabs(x1-x0);
         double dest_dPixelHeight = 2*fabs(y1-y0);
         pDataset->pixelsize = math::Min<double>(dest_dPixelWidth, dest_dPixelHeight);

      }
   }

   //---------------------------------------------------------------------------

   OPENGLOBE_API boost::shared_ptr<ProcessingSettings> LoadAppSettings()
   {
      boost::shared_ptr<ProcessingSettings> qSettings = ProcessingSettings::Load();

      if (qSettings)
      {
         bool bGood = true;
         if (!FileSystem::DirExists(qSettings->GetLogPath()))
         {
            std::cout << "ERROR: log path doesn't exist!\n";
            bGood = false;
         }

         if (!FileSystem::DirExists(qSettings->GetPath()))
         {
            std::cout << "ERROR: process path doesn't exist!\n";
            bGood = false;
         }

         if (!bGood)
         {
            return boost::shared_ptr<ProcessingSettings>();
         }

      }

      return qSettings;

   }

   //---------------------------------------------------------------------------

   OPENGLOBE_API boost::shared_ptr<Logger> CreateLogger(const std::string& appname, boost::shared_ptr<ProcessingSettings> qSettings)
   {
      boost::shared_ptr<Logger> qLogger;

      std::string sPath = qSettings->GetPath();
      std::string sLogPath = qSettings->GetLogPath();

      if (!FileSystem::DirExists(sLogPath))
      {
         std::cerr << "ERROR: logging path doesn't exist. Please edit setup.xml.\n"; 
         return qLogger;
      }

      qLogger = boost::shared_ptr<Logger>(new Logger(sLogPath, appname, true));
      qLogger->Info("Logging started");

      return qLogger;
   }

   //---------------------------------------------------------------------------

   OPENGLOBE_API boost::shared_ptr<Logger> CreateLoggerIn(const std::string& appname, const std::string& sLogPath)
   {
      boost::shared_ptr<Logger> qLogger;

      if (!FileSystem::DirExists(sLogPath))
      {
         std::cerr << "ERROR: logging path doesn't exist. Please edit setup.xml.\n"; 
         return qLogger;
      }

      qLogger = boost::shared_ptr<Logger>(new Logger(sLogPath, appname, true));
      qLogger->Info("Logging started");

      return qLogger;
   }

   //---------------------------------------------------------------------------

   OPENGLOBE_API boost::shared_array<unsigned char> ImageToMemoryRGB(const DataSetInfo& oDataset)
   {
      boost::shared_array<unsigned char> vData;

      if (!oDataset.bGood)  // invalid dataset
      {
         return vData;
      }

      // currently only datasets with 3 bands (RGB) are supported
      if (oDataset.nBands != 3)
      {
         return vData;
      } 

      // load Dataset
      GDALDataset* s_fh = (GDALDataset*)GDALOpen(oDataset.sFilename.c_str(), GA_ReadOnly);
      if(!s_fh)
      {
         return vData;
      }


      // allocate memory

      vData = boost::shared_array<unsigned char>(new unsigned char[oDataset.nSizeX * oDataset.nSizeY * 3]);

      if (!vData)
      {
         std::cout << "OUT OF MEMORY\n";
         GDALClose(s_fh);
         return vData;
      }

      // load full image to memory

       CPLErr err = s_fh->RasterIO(
         GF_Read,                      // eRWFlag
         0,                            // nXOff
         0,                            // nYOff
         oDataset.nSizeX,              // nXSize
         oDataset.nSizeY,              // nYSize
         (void*)vData.get(),           // pData
         oDataset.nSizeX,              // nBufXSize
         oDataset.nSizeY,              // nBufYSize
         GDT_Byte,                     // eBufType
         3,                            // nBandCount
         NULL,                         // panBandMap (1,2,3)
         3,                            // nPixelSpace (use 4 if qImageBuffer is RGBA)
         3*oDataset.nSizeX,            // nLineSpace
         1                             // nBandSpace
         );
      
      GDALClose(s_fh);

      return vData;
   }
   //---------------------------------------------------------------------------
   OPENGLOBE_API boost::shared_array<unsigned short> Image16BitToMemoryGreyScale(const DataSetInfo& oDataset)
   {
      boost::shared_array<unsigned short> vData;

      if (!oDataset.bGood)  // invalid dataset
      {
         return vData;
      }

      // currently only datasets with 3 bands (RGB) are supported
      /*if (oDataset.nBands != 3)
      {
         return vData;
      }*/

      // load Dataset
      GDALDataset* s_fh = (GDALDataset*)GDALOpen(oDataset.sFilename.c_str(), GA_ReadOnly);
      if(!s_fh)
      {
         return vData;
      }


      // allocate memory

      vData = boost::shared_array<unsigned short>(new unsigned short[oDataset.nSizeX * oDataset.nSizeY]);

      if (!vData)
      {
         std::cout << "OUT OF MEMORY\n";
         GDALClose(s_fh);
         return vData;
      }

      // load full image to memory

       CPLErr err = s_fh->RasterIO(
         GF_Read,                      // eRWFlag
         0,                            // nXOff
         0,                            // nYOff
         oDataset.nSizeX,              // nXSize
         oDataset.nSizeY,              // nYSize
         (void*)vData.get(),           // pData
         oDataset.nSizeX,              // nBufXSize
         oDataset.nSizeY,              // nBufYSize
         GDT_Byte,                     // eBufType
         1,                            // nBandCount
         NULL,                         // panBandMap (1,2,3)
         1,                            // nPixelSpace (use 4 if qImageBuffer is RGBA)
         1*oDataset.nSizeX,            // nLineSpace
         1                             // nBandSpace
         );
      
      GDALClose(s_fh);

      return vData;
   }

} // namespace

