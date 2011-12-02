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
#                           robert.wueest@fhnw.ch                              #
********************************************************************************
*     Licensed under MIT License. Read the file LICENSE for more information   *
*******************************************************************************/

#include "ogprocess.h"
#include "errors.h"
#include "app/ProcessingSettings.h"
#include "geo/MercatorQuadtree.h"
#include "geo/CoordinateTransformation.h"
#include "geo/ElevationLayerSettings.h"
#include "string/FilenameUtils.h"
#include "string/StringUtils.h"
#include "geo/ImageLayerSettings.h"
#include "io/FileSystem.h"
#include "image/ImageLoader.h"
#include "image/ImageWriter.h"
#include "app/Logger.h"
#include "math/mathutils.h"
#include <iostream>
#include <boost/program_options.hpp>
#include <sstream>
#include <omp.h>
#include  "hillshading.h"

namespace po = boost::program_options;

int main(int argc, char *argv[])
{
   po::options_description desc("Program-Options");
   desc.add_options()
      ("layer_path", po::value<std::string>(), "path of layer to generate data")
      ("layer_zoom", po::value<int>(), "maximum zoom which has to be generated previously using ogAddData")
      ("algorithm", po::value<std::string>(), "[optional] define suggested processing algorithm")
      ("numthreads", po::value<int>(), "[optional] force number of threads")
      ("verbose", "[optional] verbose output")
      ;

   po::variables_map vm;

   bool bError = false;
   std::string sLayerPath;
   int iLayerMaxZoom;
   std::string sAlgorithm;
   int iNumThreads = 8;
   bool bVerbose = false;
   int inputX = 768;
   int inputY = 768;
   int outputX = 256;
   int outputY = 256;
   

   try
   {
      po::store(po::parse_command_line(argc, argv, desc), vm);
      po::notify(vm);
   }
   catch (std::exception&)
   {
      bError = true;
   }

   if(vm.count("layer_path"))
   {
     sLayerPath = vm["layer_path"].as<std::string>();
     if(!(sLayerPath.at(sLayerPath.length()-1) == '\\' || sLayerPath.at(sLayerPath.length()-1) == '/'))
        sLayerPath = sLayerPath + "/";
   }
   else
      bError = true;
   if(vm.count("layer_zoom"))
      iLayerMaxZoom = vm["layer_zoom"].as<int>();
   else
      bError = true;
   if(vm.count("algorithm"))
      sAlgorithm = vm["algorithm"].as<std::string>();
   if(vm.count("num_threads"))
      iNumThreads = vm["num_threads"].as<int>();
   if(vm.count("verbose"))
      bVerbose = true;

   //---------------------------------------------------------------------------
   // init options:

   boost::shared_ptr<ProcessingSettings> qSettings =  ProcessingUtils::LoadAppSettings();

   if (!qSettings)
   {
      std::cout << "Error in configuration! Check setup.xml\n";
      return ERROR_CONFIG;
   }
   //---------------------------------------------------------------------------
   boost::shared_ptr<Logger> qLogger =  ProcessingUtils::CreateLogger("hillshading", qSettings);
   if (!qLogger)
   {
      std::cout << "Error in configuration! Check setup.xml\n";
      return ERROR_CONFIG;
   }
   if (bError)
   {
      std::ostringstream oss; 
      qLogger->Error("Wrong parameters!");
      std::ostringstream sstr;
      sstr << desc;
      qLogger->Info("\n" + sstr.str());

      return ERROR_PARAMS;
   }
   //---------------------------------------------------------------------------
   // -- Beginn process
   std::string sElevationLayerDir = sLayerPath;
   std::string sTempTileDir = sLayerPath + "temp/tiles/";
   std::string sTileDir = sLayerPath + "tiles/";

   boost::shared_ptr<ImageLayerSettings> qImageLayerSettings = ImageLayerSettings::Load(sLayerPath);
   if (!qImageLayerSettings)
   {
      qLogger->Error("Failed retrieving image layer settings! Make sure to create it using 'createlayer'.");
      return ERROR_IMAGELAYERSETTINGS;
   }
   int lod = qImageLayerSettings->GetMaxLod();
   int64 layerTileX0, layerTileY0, layerTileX1, layerTileY1;
   qImageLayerSettings->GetTileExtent(layerTileX0, layerTileY0, layerTileX1, layerTileY1);
   std::ostringstream oss;
   if (bVerbose)
   {
      oss << "\nRaw Image Layer:\n";
      oss << "   name = " << qImageLayerSettings->GetLayerName() << "\n";
      oss << "   maxlod = " << lod << "\n";
      oss << "   extent = " << layerTileX0 << ", " << layerTileY0 << ", " << layerTileX1 << ", " << layerTileY1 << "\n";
      qLogger->Info(oss.str());
      oss.str("");
   }
   int64 width = layerTileX1-layerTileX0+1;
   int64 height = layerTileY1-layerTileY0+1;
   if (width<3 || height<3)
   {
      qLogger->Error("Extent is too small for hillshading processing");
      return ERROR_ELVLAYERSETTINGS;
   }

   // Retrieve dataset extent in mercator coord:
   boost::shared_ptr<MercatorQuadtree> qQuadtree = boost::shared_ptr<MercatorQuadtree>(new MercatorQuadtree());
   double xmin, ymin, xmax, ymax;
   std::string qc0 = qQuadtree->TileCoordToQuadkey(layerTileX0,layerTileY0,lod);
   std::string qc1 = qQuadtree->TileCoordToQuadkey(layerTileX1,layerTileY1,lod);

   double x00, y00, x10, y10;
   double x01, y01, x11, y11;
   qQuadtree->QuadKeyToMercatorCoord(qc0, x00,y00,x10,y10);
   qQuadtree->QuadKeyToMercatorCoord(qc1, x01,y01,x11,y11);

   xmin = x00;
   ymin = y11;
   xmax = x11;
   ymax = y00;
   if (bVerbose)
   {
      oss << "\nExtent mercator:";
      oss << "   extent = " << xmin << ", " << ymin << ", " << xmax << ", " << ymax << "\n";
      qLogger->Info(oss.str());
      oss.str("");
   }

   if (!ProcessingUtils::init_gdal())
   {
      std::cout << "Warning: gdal-data directory not found. Ouput may be wrong!\n";
      return 1;
   }   

#ifndef _DEBUG
#     pragma omp parallel for
#endif
   for (int64 xx = layerTileX0+1; xx < layerTileX1; ++xx)
   {
      for (int64 yy = layerTileY0+1; yy < layerTileY1; ++yy)
      {
         std::string sCurrentQuadcode = qQuadtree->TileCoordToQuadkey(xx,yy,lod);

         //std::cout << sCurrentQuadcode << "\n";
         HSProcessChunk pData;
         pData.dfXMax = -1e20;
         pData.dfYMax = -1e20;
         pData.dfXMin = 1e20;
         pData.dfYMin = 1e20;
         pData.data.AllocateImage(inputX, inputY);
         for (int ty=-1;ty<=1;ty++)
         {
            for (int tx=-1;tx<=1;tx++)
            {
               std::string sQuadcode = qQuadtree->TileCoordToQuadkey(xx+tx,yy+ty,lod);
               std::string sTilefile = ProcessingUtils::GetTilePath(sTempTileDir, ".raw" , lod, xx+tx, yy+ty);
                  
               double sx0, sy1, sx1, sy0;
               qQuadtree->QuadKeyToMercatorCoord(sQuadcode, sx0, sy1, sx1, sy0);
               
               pData.dfXMax = math::Max<double>(pData.dfXMax, sx1);
               pData.dfYMax = math::Max<double>(pData.dfXMax, sy1);
               pData.dfXMin = math::Min<double>(pData.dfXMin, sx0);
               pData.dfYMin = math::Min<double>(pData.dfXMin, sy0);

               assert(sx0 < sx1);
               assert(sy0 < sy1);

               //std::cout << "   " << sTilefile << "\n";

               std::ifstream fin;
               fin.open(sTilefile.c_str(), std::ios::binary);
               int posX = (tx+1)*(inputX/3);
               int posY = (ty+1)*(inputY/3);
               int offX = 0;
               int offY = 0;
               if (fin.good())
               {
                  while (!fin.eof())
                  {
                     
                     float value;
                     fin.read((char*)&(value), sizeof(float));
                     if (!fin.eof())
                     {
                        pData.data.SetValue(posX+offX, posY+offY, value);
                     }
                     offX++;
                     if(offX % 256 == 0)
                     {
                        offX = 0;
                        offY++;
                     }

                  }
               }
               fin.close();
            }
         }
         // Generate tile
         process_hillshading(sTileDir, pData, xx, yy, lod, 2, outputX, outputY);
      }
   }
   GDALDestroyDriverManager();
   return 0;
}

