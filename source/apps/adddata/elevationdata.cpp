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
#include "math/ElevationPoint.h"
#include "geo/ElevationReader.h"
#include <sstream>
#include <fstream>
#include <ctime>
#include <omp.h>


namespace ElevationData
{
   struct SElevationCell
   {
      std::vector<ElevationPoint*> vecPts;
   };

   //---------------------------------------------------------------------------

   int process( boost::shared_ptr<Logger> qLogger, boost::shared_ptr<ProcessingSettings> qSettings, std::string sLayer, bool bVerbose, bool bLock, int epsg, std::string sElevationFile, bool bFill, int& out_lod, int64& out_x0, int64& out_y0, int64& out_x1, int64& out_y1)
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
      std::string sTileDir = FilenameUtils::DelimitPath(FilenameUtils::DelimitPath(sElevationLayerDir) + "temp/tiles");

      boost::shared_ptr<ElevationLayerSettings> qElevationLayerSettings = ElevationLayerSettings::Load(sElevationLayerDir);
      if (!qElevationLayerSettings)
      {
         qLogger->Error("Failed retrieving elevation layer settings! Make sure to create it using 'createlayer'.");
         ProcessingUtils::exit_gdal();
         return ERROR_ELVLAYERSETTINGS;
      }

      int lod = qElevationLayerSettings->GetMaxLod();
      out_lod = lod;
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

      out_x0 = elvTileX0;
      out_y0 = elvTileY0;
      out_x1 = elvTileX1;
      out_y1 = elvTileY1;

      // total number of tiles affected by this dataset:
      int64 total = (elvTileX1-elvTileX0+1)*(elvTileY1-elvTileY0+1);
      int tilewidth_i = int(elvTileX1-elvTileX0+1);
      int tileheight_i = int(elvTileY1-elvTileY0+1);
      double tilewidth = double(elvTileX1-elvTileX0+1);
      double tileheight = double(elvTileY1-elvTileY0+1);


      // retrieve min/max mercator coodinates of dataset:
      std::string qc0 = qQuadtree->TileCoordToQuadkey(elvTileX0, elvTileY0, lod);
      std::string qc1 = qQuadtree->TileCoordToQuadkey(elvTileX1, elvTileY1, lod);

      double x00, y00, x10, y10;
      double x01, y01, x11, y11;
      qQuadtree->QuadKeyToMercatorCoord(qc0, x00,y00,x10,y10);
      qQuadtree->QuadKeyToMercatorCoord(qc1, x01,y01,x11,y11);

      double x0,y0,x1,y1;

      x0 = x00;
      y0 = y11;
      x1 = x11;
      y1 = y00;

      if (bVerbose)
      {
         oss << "\nMercator coords of dataset:";
         oss << "   (" << x0 << ", " << y0 << ")-(" << x1 << ", " << y1 << ")\n";
         qLogger->Info(oss.str());
         oss.str("");
      }

      double width_merc = fabs(x1 - x0);
      double height_merc = fabs(y1 - y0);
      tilewidth = width_merc / (tilewidth);
      tileheight =  height_merc / (tileheight);

      int max_threads = omp_get_max_threads();

      if (bVerbose)
      {
         oss << "\nMemory required for matrix:";
         oss << "  " << max_threads*tilewidth_i*tileheight_i*sizeof(SElevationCell) << " Bytes\n";
         qLogger->Info(oss.str());
         oss.str("");
      }

      SElevationCell* matrix = new SElevationCell[max_threads*tilewidth_i*tileheight_i]; 

      if (!matrix)
      {
         oss << "\nOut of memory!";
         qLogger->Error(oss.str());
         oss.str("");
         return ERROR_OUTOFMEMORY;
      }

      // parallel sorting points:
#ifndef _DEBUG
#     pragma omp parallel for
#endif
      for (int i=0;i<(int)vPoints.size();i++)
      {
         // calculate tile coordinate of current point:
         
         int64 ttx = int64((vPoints[i].x - x0) / tilewidth);
         int64 tty = int64((vPoints[i].y - y0) / tileheight);
         int64 tileX = ttx+elvTileX0;
         int64 tileY = tty+elvTileY0;



         if (tileX >= elvTileX0 && tileX<=elvTileX1 &&
             tileY >= elvTileY0 &&  tileY<=elvTileY1)
         {
            int arraynum = omp_get_thread_num();
            SElevationCell& s = matrix[arraynum*total + tty*tilewidth_i+ttx];
            s.vecPts.push_back(&vPoints[i]);
         }
      }

      // write tiles
#ifndef _DEBUG
#     pragma omp parallel for
#endif
      for (int64 xx = elvTileX0; xx <= elvTileX1; ++xx)
      {
         for (int64 yy = elvTileY0; yy <= elvTileY1; ++yy)
         {
            int64 ttx = xx - elvTileX0;
            int64 tty = elvTileY1 - yy;

            std::string sQuadcode = qQuadtree->TileCoordToQuadkey(xx,yy,lod);
            std::string sTilefile = ProcessingUtils::GetTilePath(sTileDir, ".pts" , lod, xx, yy);

            double px0,py0,px1,py1;
            qQuadtree->QuadKeyToMercatorCoord(sQuadcode, px0, py1, px1, py0);

            // LOCK this tile. If this tile is currently locked then wait until the lock is removed.
            int lockhandle = bLock ? FileSystem::Lock(sTilefile) : -1;

            std::ofstream fout;

            if (FileSystem::FileExists(sTilefile))
            {
               fout.open(sTilefile.c_str(), std::ios::binary | std::ios::app); // open in append mode
            }
            else
            {
               fout.open(sTilefile.c_str(), std::ios::binary); // open in append mode
            }

            if (fout.good())
            {
               for (int i=0;i<max_threads;i++)
               {
                   SElevationCell& s = matrix[i*total + tty*tilewidth_i+ttx];

                   for (size_t k=0;k<s.vecPts.size();k++)
                   {
                        ElevationPoint* pt = s.vecPts[k];
                        fout.write((const char*)&(pt->x), sizeof(double));
                        fout.write((const char*)&(pt->y), sizeof(double));
                        fout.write((const char*)&(pt->elevation), sizeof(double));
                        fout.write((const char*)&(pt->weight), sizeof(double));
                   }
               }

               fout.close();
            }
            else
            {
               std::cout << "FILE ERROR!\n";
            }

            // unlock file. Other computers/processes/threads can access it again.
            FileSystem::Unlock(sTilefile, lockhandle);
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