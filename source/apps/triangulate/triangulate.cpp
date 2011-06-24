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

#include "triangulate.h"
#include "string/FilenameUtils.h"
#include "io/FileSystem.h"
#include "geo/ElevationLayerSettings.h"
#include "geo/MercatorQuadtree.h"
#include "image/ImageLoader.h"
#include "image/ImageWriter.h"
#include "math/ElevationPoint.h"
#include "geo/ElevationReader.h"
#include <sstream>
#include <fstream>
#include <ctime>
#include <omp.h>

namespace triangulate
{
   int process(boost::shared_ptr<Logger> qLogger, boost::shared_ptr<ProcessingSettings> qSettings, std::string sLayer, bool bVerbose)
   {
      // Retrieve ElevationLayerSettings:
      std::ostringstream oss;

      std::string sElevationLayerDir = FilenameUtils::DelimitPath(qSettings->GetPath()) + sLayer;
      std::string sTileDir = FilenameUtils::DelimitPath(FilenameUtils::DelimitPath(sElevationLayerDir) + "tiles");

      boost::shared_ptr<ElevationLayerSettings> qElevationLayerSettings = ElevationLayerSettings::Load(sElevationLayerDir);
      if (!qElevationLayerSettings)
      {
         qLogger->Error("Failed retrieving elevation layer settings! Make sure to create it using 'createlayer'.");
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
         qLogger->Info(oss.str());
         oss.str("");
      }

      return 0;
   }
}