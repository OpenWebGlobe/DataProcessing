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

#include "elevationdata.h"
#include "string/FilenameUtils.h"
#include "io/FileSystem.h"
#include "geo/ElevationLayerSettings.h"
#include "geo/MercatorQuadtree.h"
#include "image/ImageLoader.h"
#include "image/ImageWriter.h"
#include "math/delaunay/DelaunayTriangulation.h"
#include "math/ElevationPoint.h"
#include "geo/ElevationReader.h"
#include <sstream>
#include <fstream>
#include <ctime>

namespace ElevationData
{
   int process( boost::shared_ptr<Logger> qLogger, boost::shared_ptr<ProcessingSettings> qSettings, std::string sLayer, bool bVerbose, int epsg, std::string sElevationFile, bool bFill )
   {
      DataSetInfo oInfo;

      clock_t t0,t1;
      t0 = clock();

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
         qLogger->Info(oss.str());
         oss.str("");
      }

      //---------------------------------------------------------------------------

      ElevationReader oElevationReader;
      std::vector<ElevationPoint> vPoints;

      if (!oElevationReader.Open(sElevationFile, epsg))
      {
         qLogger->Error("Failed opening elevation file.");
         ProcessingUtils::exit_gdal();
         return ERROR_LOADELEVATION;
      }

      double xmin=1e20;
      double ymin=1e20;
      double xmax=-1e20;
      double ymax=-1e20;

      if (!oElevationReader.Import(vPoints, xmin, ymin, xmax, ymax))
      {
         qLogger->Error("Failed importing elevation.");
         ProcessingUtils::exit_gdal();
         return ERROR_LOADELEVATION;
      }


      if (bVerbose)
      {
         oss << "Number of Points: " << vPoints.size() << "\n";
         oss << "Elevation Boundary:" << "(" << xmin << ", " << ymin << ")-(" << xmax << ", " << ymax << ")\n";
         qLogger->Info(oss.str());
         oss.str("");
      }

      boost::shared_ptr<MercatorQuadtree> qQuadtree = boost::shared_ptr<MercatorQuadtree>(new MercatorQuadtree());

      int64 px0, py0, px1, py1;
      qQuadtree->MercatorToPixel(xmin, ymax, lod, px0, py0);
      qQuadtree->MercatorToPixel(xmax, ymin, lod, px1, py1);

      int64 elvTileX0, elvTileY0, elvTileX1, elvTileY1;
      qQuadtree->PixelToTileCoord(px0, py0, elvTileX0, elvTileY0);
      qQuadtree->PixelToTileCoord(px1, py1, elvTileX1, elvTileY1);

      if (bVerbose)
      {
         oss << "\nTile Coords (elevation, unclipped):";
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

      // tatal number of tiles affected by this dataset:
      int64 total = (elvTileX1-elvTileX0+1)*(elvTileY1-elvTileY0+1);

#     pragma omp parallel for      
      for (int64 xx = elvTileX0; xx <= elvTileX1; ++xx)
      {
         for (int64 yy = elvTileY0; yy <= elvTileY1; ++yy)
         {
            std::string sQuadcode = qQuadtree->TileCoordToQuadkey(xx,yy,lod);
            std::string sTilefile = ProcessingUtils::GetTilePath(sTileDir, ".obj" , lod, xx, yy);
            
            double x0, y0, x1, y1;
            qQuadtree->QuadKeyToMercatorCoord(sQuadcode, x0, y1, x1, y0);
            double len = y1-y0;

            double xx0 = x0-0.10*len;
            double xx1 = x1+0.10*len;
            double yy0 = y0-0.10*len;
            double yy1 = y1+0.10*len;

            math::DelaunayTriangulation oTriangulation(x0-2.0*len, y0-2.0*len, x1+2.0*len, y1+2.0*len);

            int cnt=0;
            for (size_t i=0;i<vPoints.size();i++)
            {
               if (vPoints[i].x > xx0 && vPoints[i].x < xx1 &&
                   vPoints[i].y > yy0 && vPoints[i].y < yy1)
                {
                  oTriangulation.InsertPoint(vPoints[i]);
                  cnt++;
                }
            }

             oTriangulation.Reduce(cnt/2); // thin out 50%

            // cut!
            oTriangulation.InsertLine(x0,y0,x1,y0);
            oTriangulation.InsertLine(x1,y0,x1,y1);
            oTriangulation.InsertLine(x1,y1,x0,y1);
            oTriangulation.InsertLine(x0,y1,x0,y0);
            oTriangulation.InvalidateVertices(x0,y0,x1,y1);

            

            std::string str = oTriangulation.CreateOBJ(xmin, ymin, xmax, ymax);
            std::ofstream fout(sTilefile);
            fout << str;
            fout.close();
          
         }
      }  


      // finished, print stats:
      t1=clock();

      std::ostringstream out;
      out << "calculated in: " << double(t1-t0)/double(CLOCKS_PER_SEC) << " s \n";
      qLogger->Info(out.str());

      ProcessingUtils::exit_gdal();
      return 0;
   }

}




/*
// Demo Triangulation Code
// Triangulate a Dataset and write Wavefront OBJ file.

math::DelaunayTriangulation oTriangulation(xmin-0.000001, ymin-0.000001, xmax+0.000001, ymax+000001);
oTriangulation.SetEpsilon(DBL_EPSILON);

for (size_t i=0;i<vPoints.size();i++)
{
oTriangulation.InsertPoint(vPoints[i]);
}

std::string str = oTriangulation.CreateOBJ(xmin, ymin, xmax, ymax);
std::ofstream fout("c:/out.obj");
fout << str;
fout.close();
*/