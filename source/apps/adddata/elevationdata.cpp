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

/*
   Version 0.1:
     This is the initial version for elevation processing.
     Raster based elevation datasets are imported.
     Support for "xyz" files (ASCII) will be added soon.
*/


#include "elevationdata.h"
#include "string/FilenameUtils.h"
#include "io/FileSystem.h"
#include "geo/ElevationLayerSettings.h"
#include "geo/MercatorQuadtree.h"
#include "image/ImageLoader.h"
#include "image/ImageWriter.h"
#include "math/delaunay/DelaunayTriangulation.h"
#include "math/ElevationPoint.h"
#include <sstream>
#include <ctime>

namespace ElevationData
{
   int process( boost::shared_ptr<Logger> qLogger, boost::shared_ptr<ProcessingSettings> qSettings, std::string sLayer, bool bVerbose, int epsg, std::string sElevationFile, bool bFill )
   {
      DataSetInfo oInfo;

      if (!ProcessingUtils::init_gdal())
      {
         qLogger->Error("gdal-data directory not found!");
         return ERROR_GDAL;
      }

      //---------------------------------------------------------------------------
      // Retrieve ImageLayerSettings:
      std::ostringstream oss;

      std::string sElevationLayerDir = FilenameUtils::DelimitPath(qSettings->GetPath()) + sLayer;
      std::string sTileDir = FilenameUtils::DelimitPath(FilenameUtils::DelimitPath(sElevationLayerDir) + "tiles");

      boost::shared_ptr<ElevationLayerSettings> qElevationLayerSettings = ElevationLayerSettings::Load(sElevationLayerDir);
      if (!qElevationLayerSettings)
      {
         qLogger->Error("Failed retrieving image layer settings! Make sure to create it using 'createlayer'.");
         ProcessingUtils::exit_gdal();
         return ERROR_ELVLAYERSETTINGS;
      }

      int lod = qElevationLayerSettings->GetMaxLod();
      int64 layerTileX0, layerTileY0, layerTileX1, layerTileY1;
      qElevationLayerSettings->GetTileExtent(layerTileX0, layerTileY0, layerTileX1, layerTileY1);

      if (bVerbose)
      {
         oss << "\nElevation Layer:\n";
         oss << "     name = " << qElevationLayerSettings->GetLayerName() << "\n";
         oss << "   maxlod = " << lod << "\n";
         oss << "   extent = " << layerTileX0 << ", " << layerTileY0 << ", " << layerTileX1 << ", " << layerTileY1 << "\n";
      }

      //---------------------------------------------------------------------------

      boost::shared_ptr<CoordinateTransformation> qCT;
      qCT = boost::shared_ptr<CoordinateTransformation>(new CoordinateTransformation(epsg, 3785));

      clock_t t0,t1;
      t0 = clock();

      ProcessingUtils::RetrieveDatasetInfo(sElevationFile, qCT.get(), &oInfo, bVerbose);

      if (!oInfo.bGood)
      {
         qLogger->Error("Failed retrieving info!");
      }

      if (bVerbose)
      {
         oss << "Loaded elevation info:\n   Elevation Size: w= " << oInfo.nSizeX << ", h= " << oInfo.nSizeY << "\n";
         oss << "   dest: " << oInfo.dest_lrx << ", " << oInfo.dest_lry << ", " << oInfo.dest_ulx << ", " << oInfo.dest_uly << "\n";
         qLogger->Info(oss.str());
         oss.str("");
      }

      boost::shared_ptr<MercatorQuadtree> qQuadtree = boost::shared_ptr<MercatorQuadtree>(new MercatorQuadtree());

      int64 px0, py0, px1, py1;
      qQuadtree->MercatorToPixel(oInfo.dest_ulx, oInfo.dest_uly, lod, px0, py0);
      qQuadtree->MercatorToPixel(oInfo.dest_lrx, oInfo.dest_lry, lod, px1, py1);

      int64 elvTileX0, elvTileY0, elvTileX1, elvTileY1;
      qQuadtree->PixelToTileCoord(px0, py0, elvTileX0, elvTileY0);
      qQuadtree->PixelToTileCoord(px1, py1, elvTileX1, elvTileY1);

      if (bVerbose)
      {
         oss << "\nTile Coords (elevation):";
         oss << "   (" << elvTileX0 << ", " << elvTileY0 << ")-(" << elvTileX1 << ", " << elvTileY1 << ")\n";
         qLogger->Info(oss.str());
         oss.str("");
      }

      // check if image is outside layer
      if (elvTileX0 > layerTileX1 || 
         elvTileY0 > layerTileY1 ||
         elvTileX1 < layerTileX0 ||
         elvTileY1 < layerTileY0)
      {
         qLogger->Info("The dataset is outside of the layer and not being added!");
         ProcessingUtils::exit_gdal();
         return 0;
      }

      // clip tiles to layer extent
      elvTileX0 = math::Max<int64>(elvTileX0, layerTileX0);
      elvTileY0 = math::Max<int64>(elvTileY0, layerTileY0);
      elvTileX1 = math::Min<int64>(elvTileX1, layerTileX1);
      elvTileY1 = math::Min<int64>(elvTileY1, layerTileY1);

      // DelaunayTriangulation

      std::vector<ElevationPoint> vPoints;
      if (!ProcessingUtils::ElevationToMemory(oInfo, vPoints))
      {

         return ERROR_LOADELEVATION;
      }

      
      ProcessingUtils::exit_gdal();
      return 0;
   }

}

