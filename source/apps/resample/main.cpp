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
#include "app/ProcessingSettings.h"
#include "geo/MercatorQuadtree.h"
#include "string/FilenameUtils.h"
#include "string/StringUtils.h"
#include "io/FileSystem.h"
#include "geo/ImageLayerSettings.h"
#include "image/ImageLoader.h"
#include "image/ImageWriter.h"
#include <iostream>
#include <fstream>
#include <boost/shared_ptr.hpp>
#include <boost/program_options.hpp>
#include <boost/thread.hpp>
#include <sstream>
#include <omp.h>

//------------------------------------------------------------------------------
#define ERROR_CONFIG             3;    // wrong configuration (setup.xml) (processing path or log-path is wrong)
#define ERROR_PARAMS             4;    // wrong parameters
#define ERROR_IMAGELAYERSETTINGS 5;
//------------------------------------------------------------------------------
const int tilesize = 256;
//------------------------------------------------------------------------------

// Holding/managing memory for a tile
class TileBlock
{
public:
   TileBlock()
   {
      tile = new unsigned char[4*tilesize*tilesize];
   }

   virtual ~TileBlock()
   {
      if (tile) delete[] tile;
   }

   inline void Clear()
   {
      memset(tile, 0, tilesize*tilesize*4);
   }

   unsigned char* tile;
};
//------------------------------------------------------------------------------

inline std::string GetTilePath(const std::string& sBaseTilePath, const std::string& sExtension, int lod, int64 tx, int64 ty)
{
   std::ostringstream oss;
   oss << sBaseTilePath << lod << "/" << tx << "/" << ty << sExtension;
   return oss.str();
}

//------------------------------------------------------------------------------

inline void _getInterpolatedColor(const unsigned char* rgbData, const size_t adr0, const size_t adr1, const size_t adr2, const size_t adr3, unsigned char* r, unsigned char* g, unsigned char* b, unsigned char* a)
{
   int red = 0;
   int green = 0;
   int blue = 0;
   int alpha = 0;

   int alpha0, alpha1, alpha2, alpha3;

   alpha0 = rgbData[adr0+3];
   alpha1 = rgbData[adr1+3];
   alpha2 = rgbData[adr2+3];
   alpha3 = rgbData[adr3+3];
  
   int nCount = 0;
   if (alpha0 > 0)
   {
      red   = red + rgbData[adr0];
      green = green + rgbData[adr0+1];
      blue  = blue + rgbData[adr0+2];
      alpha = alpha + alpha0;
      nCount++;      
   }
   
   if (alpha1 > 0)
   {
      red   = red + rgbData[adr1];
      green = green + rgbData[adr1+1];
      blue  = blue + rgbData[adr1+2];
      alpha = alpha + alpha1;
      nCount++;      
   }
   
   if (alpha2 > 0)
   {
      red   = red + rgbData[adr2];
      green = green + rgbData[adr2+1];
      blue  = blue + rgbData[adr2+2];
      alpha = alpha + alpha2;
      nCount++;      
   }
   
   if (alpha3 > 0)
   {
      red   = red + rgbData[adr3];
      green = green + rgbData[adr3+1];
      blue  = blue + rgbData[adr3+2];
      alpha = alpha + alpha3;
      nCount++;      
   }
   
   if (nCount>0)
   {
      red/=nCount;
      green/=nCount;
      blue/=nCount;
      alpha/=nCount;
   }

   if (red>255) red = 255;
   if (green>255) green=255;
   if (blue>255) blue=255;
   if (alpha>255) alpha=255;

   *r = (unsigned char)red;
   *g = (unsigned char)green;
   *b = (unsigned char)blue;
   *a = (unsigned char)alpha;
}

//------------------------------------------------------------------------------

void _resampleFromParent( TileBlock* pTileBlockArray, boost::shared_ptr<MercatorQuadtree> qQuadtree, int64 x, int64 y,int nLevelOfDetail, std::string sTileDir ) 
{
   int curthread = omp_get_thread_num();
   TileBlock& tile = pTileBlockArray[curthread];
   tile.Clear();

   std::string qcCurrent = qQuadtree->TileCoordToQuadkey(x,y,nLevelOfDetail);

   // calculate parent quadkeys:
   std::string qc0,qc1,qc2,qc3;
   qc0 = qcCurrent + '0';
   qc1 = qcCurrent + '1';
   qc2 = qcCurrent + '2';
   qc3 = qcCurrent + '3';

   int64 _tx, _ty;
   int tmp_lod;

   qQuadtree->QuadKeyToTileCoord(qcCurrent, _tx, _ty, tmp_lod);
   std::string sCurrentTile = GetTilePath(sTileDir, ".png" , tmp_lod, _tx, _ty);

   qQuadtree->QuadKeyToTileCoord(qc0, _tx, _ty, tmp_lod);
   std::string sTilefile0 = GetTilePath(sTileDir, ".png" , tmp_lod, _tx, _ty);

   qQuadtree->QuadKeyToTileCoord(qc1, _tx, _ty, tmp_lod);
   std::string sTilefile1 = GetTilePath(sTileDir, ".png" , tmp_lod, _tx, _ty);

   qQuadtree->QuadKeyToTileCoord(qc2, _tx, _ty, tmp_lod);
   std::string sTilefile2 = GetTilePath(sTileDir, ".png" , tmp_lod, _tx, _ty);

   qQuadtree->QuadKeyToTileCoord(qc3, _tx, _ty, tmp_lod);
   std::string sTilefile3 = GetTilePath(sTileDir, ".png" , tmp_lod, _tx, _ty);

   ImageObject IH0, IH1, IH2, IH3;

   bool b0 = ImageLoader::LoadFromDisk(Img::Format_PNG, sTilefile0, Img::PixelFormat_RGBA, IH0);
   bool b1 = ImageLoader::LoadFromDisk(Img::Format_PNG, sTilefile1, Img::PixelFormat_RGBA, IH1);
   bool b2 = ImageLoader::LoadFromDisk(Img::Format_PNG, sTilefile2, Img::PixelFormat_RGBA, IH2);
   bool b3 = ImageLoader::LoadFromDisk(Img::Format_PNG, sTilefile3, Img::PixelFormat_RGBA, IH3);

   unsigned char* p0 = IH0.GetRawData().get();
   unsigned char* p1 = IH1.GetRawData().get();
   unsigned char* p2 = IH2.GetRawData().get();
   unsigned char* p3 = IH3.GetRawData().get();

   unsigned char cr;
   unsigned char cg;
   unsigned char cb;
   unsigned char ca;

   for (int y=0;y<tilesize;y++)
   {
      for (int x=0;x<tilesize;x++)
      {
         size_t adr = 4*y*tilesize+4*x;

         if (y<tilesize/2)
         {
            if (x<tilesize/2)
            {
               // A
               if (p0)
               {
                  int x0 = 2*x;
                  int y0 = 2*y; 
                  int x1 = x0+1;
                  int y1 = y0+1;

                  size_t tileadr0 = 4*y0*tilesize+4*x0;
                  size_t tileadr1 = 4*y0*tilesize+4*x1;
                  size_t tileadr2 = 4*y1*tilesize+4*x0;
                  size_t tileadr3 = 4*y1*tilesize+4*x1;

                  _getInterpolatedColor(p0, tileadr0, tileadr1, tileadr2, tileadr3, &cr, &cg, &cb, &ca);
               }
               else
               {
                  cr = cg = cb = ca = 0;
               }
            }
            else
            {
               // B 
               if (p1)
               {
                  int x0 = 2*(x-tilesize/2);
                  int y0 = 2*y; 
                  int x1 = x0+1;
                  int y1 = y0+1;

                  size_t tileadr0 = 4*y0*tilesize+4*x0;
                  size_t tileadr1 = 4*y0*tilesize+4*x1;
                  size_t tileadr2 = 4*y1*tilesize+4*x0;
                  size_t tileadr3 = 4*y1*tilesize+4*x1;

                  _getInterpolatedColor(p1, tileadr0, tileadr1, tileadr2, tileadr3, &cr, &cg, &cb, &ca);
               }
               else
               {
                  cr = cg = cb = ca = 0;
               }
            }
         }
         else
         {
            if (x<tilesize/2)
            {
               // C
               if (p2)
               {
                  int x0 = 2*x;
                  int y0 = 2*(y-tilesize/2); 
                  int x1 = x0+1;
                  int y1 = y0+1;

                  size_t tileadr0 = 4*y0*tilesize+4*x0;
                  size_t tileadr1 = 4*y0*tilesize+4*x1;
                  size_t tileadr2 = 4*y1*tilesize+4*x0;
                  size_t tileadr3 = 4*y1*tilesize+4*x1;

                  _getInterpolatedColor(p2, tileadr0, tileadr1, tileadr2, tileadr3, &cr, &cg, &cb, &ca);
               }
               else
               {
                  cr = cg = cb = ca = 0;
               }
            }
            else
            {
               // D 
               if (p3)
               {
                  int x0 = 2*(x-tilesize/2); 
                  int y0 = 2*(y-tilesize/2); 
                  int x1 = x0+1;
                  int y1 = y0+1;

                  size_t tileadr0 = 4*y0*tilesize+4*x0;
                  size_t tileadr1 = 4*y0*tilesize+4*x1;
                  size_t tileadr2 = 4*y1*tilesize+4*x0;
                  size_t tileadr3 = 4*y1*tilesize+4*x1;

                  _getInterpolatedColor(p3, tileadr0, tileadr1, tileadr2, tileadr3, &cr, &cg, &cb, &ca);
               }
               else
               {
                  cr = cg = cb = ca = 0;
               }
            }
         }

         tile.tile[adr+0] = cr;
         tile.tile[adr+1] = cg;
         tile.tile[adr+2] = cb;
         tile.tile[adr+3] = ca;
      }
   }

   ImageWriter::WritePNG(sCurrentTile, tile.tile, tilesize, tilesize);
}

//------------------------------------------------------------------------------

namespace po = boost::program_options;
int main(int argc, char *argv[])
{
   po::options_description desc("Program-Options");
   desc.add_options()
       ("layer", po::value<std::string>(), "image layer to resample")
       ("numthreads", po::value<int>(), "force number of threads")
       ("verbose", "optional info")
       ;

   po::variables_map vm;


    //---------------------------------------------------------------------------
   // init options:

   boost::shared_ptr<ProcessingSettings> qSettings =  ProcessingUtils::LoadAppSettings();

   if (!qSettings)
   {
      std::cout << "Error in configuration! Check setup.xml\n";
      return ERROR_CONFIG;
   }

   //---------------------------------------------------------------------------
   // create logger
   boost::shared_ptr<Logger> qLogger =  ProcessingUtils::CreateLogger("resample", qSettings);

   if (!qLogger)
   {
      std::cout << "Error in configuration! Check setup.xml\n";
      return ERROR_CONFIG;
   }

   // --------------------------------------------------------------------------
   std::string sLayer;
   bool bError = false;
   bool bVerbose = false;


   try
   {
      po::store(po::parse_command_line(argc, argv, desc), vm);
      po::notify(vm);
   }
   catch (std::exception&)
   {
      bError = true;
   }

   if (!vm.count("layer"))
   {
      bError = true;
   }
   else
   {
      sLayer = vm["layer"].as<std::string>();
   }

   if (vm.count("verbose"))
   {
      bVerbose = true;
   }

   if (vm.count("numthreads"))
   {
      int n = vm["numthreads"].as<int>();
      if (n>0 && n<65)
      {
         std::ostringstream oss; 
         oss << "Forcing number of threads to " << n;
         qLogger->Info(oss.str());
         omp_set_num_threads(n);
      }
   }

   //---------------------------------------------------------------------------
   if (bError)
   {
      qLogger->Error("Wrong parameters!");
      std::ostringstream sstr;
   
      sstr << desc;
      qLogger->Info("\n" + sstr.str());

      return ERROR_PARAMS;
   }

   //---------------------------------------------------------------------------

   std::string sImageLayerDir = FilenameUtils::DelimitPath(qSettings->GetPath()) + sLayer;
   std::string sTileDir = FilenameUtils::DelimitPath(FilenameUtils::DelimitPath(sImageLayerDir) + "tiles");

   boost::shared_ptr<ImageLayerSettings> qImageLayerSettings = ImageLayerSettings::Load(sImageLayerDir);
   if (!qImageLayerSettings)
   {
      qLogger->Error("Failed retrieving image layer settings!");
      return ERROR_IMAGELAYERSETTINGS;
   }

   clock_t t0,t1;
   t0 = clock();

   //--------------------------------------------------------------------------
   // create tile blocks (for each thread)
   int maxthreads = omp_get_max_threads();
   TileBlock* pTileBlockArray = new TileBlock[maxthreads];

   //---------------------------------------------------------------------------
   int64 tx0,ty0,tx1,ty1;
   qImageLayerSettings->GetTileExtent(tx0,ty0,tx1,ty1);

   int maxlod = qImageLayerSettings->GetMaxLod();
   int64 layerTileX0, layerTileY0, layerTileX1, layerTileY1;
   qImageLayerSettings->GetTileExtent(layerTileX0, layerTileY0, layerTileX1, layerTileY1);

   if (bVerbose)
   {
      std::ostringstream oss;
      oss << "\nResample Setup:\n";
      oss << "     name = " << qImageLayerSettings->GetLayerName() << "\n";
      oss << "   maxlod = " << maxlod << "\n";
      oss << "   extent = " << layerTileX0 << ", " << layerTileY0 << ", " << layerTileX1 << ", " << layerTileY1 << "\n";
      oss << "  threads = " << maxthreads << "\n";
      qLogger->Info(oss.str());
   }

   boost::shared_ptr<MercatorQuadtree> qQuadtree = boost::shared_ptr<MercatorQuadtree>(new MercatorQuadtree());


    std::string qc0 = qQuadtree->TileCoordToQuadkey(tx0, ty0, maxlod);
    std::string qc1 = qQuadtree->TileCoordToQuadkey(tx1, ty1, maxlod);


   for (int nLevelOfDetail = maxlod - 1; nLevelOfDetail>0; nLevelOfDetail--)
   {
      std::ostringstream oss;
      oss << "Processing Level of Detail " << nLevelOfDetail;
      qLogger->Info(oss.str());

      qc0 = StringUtils::Left(qc0, nLevelOfDetail);
      qc1 = StringUtils::Left(qc1, nLevelOfDetail);

      int tmp_lod;
      qQuadtree->QuadKeyToTileCoord(qc0, tx0, ty0, tmp_lod);
      qQuadtree->QuadKeyToTileCoord(qc1, tx1, ty1, tmp_lod);

#     pragma omp parallel for
      for (int64 y=ty0;y<=ty1;y++)
      {
         for (int64 x=tx0;x<=tx1;x++)
         {
            _resampleFromParent(pTileBlockArray, qQuadtree, x, y, nLevelOfDetail, sTileDir);
         }
      }
   }

   // output time to calculate resampling:
   t1=clock();
   std::ostringstream out;
   out << "calculated in: " << double(t1-t0)/double(CLOCKS_PER_SEC) << " s \n";
   qLogger->Info(out.str());

   // clean up
   if (pTileBlockArray) 
   {
      delete[] pTileBlockArray;
      pTileBlockArray = 0;
   }

}