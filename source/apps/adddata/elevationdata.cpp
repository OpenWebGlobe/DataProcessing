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
#include <map>
#include <omp.h>

#define MAX_POINTS_IN_MEMORY 20000

namespace ElevationData
{
   struct SElevationCell
   {
      std::vector<ElevationPoint*> vecPts;
   };

   int64 points_in_map = 0;

   //---------------------------------------------------------------------------
   // Elevation Map Utils:
   void AddPoint(std::map< int64, std::list<ElevationPoint> >& streamMap, int64 idx, ElevationPoint& pt)
   {
      std::map< int64, std::list<ElevationPoint> >::iterator it;
      it = streamMap.find(idx);
      if (it == streamMap.end()) // not found, create new entry!
      {
         std::list<ElevationPoint> lst;
         lst.push_back(pt);
         streamMap.insert(std::pair< int64, std::list<ElevationPoint> >(idx, lst));
      }
      else // key found, add elevation point to existing list
      {
         it->second.push_back(pt);
      }

      points_in_map++;

   }

   void WriteMap(boost::shared_ptr<MercatorQuadtree> qQuadtree, std::map< int64, std::list<ElevationPoint> >& streamMap, const std::string& sTileDir, int tilewidth_i, int lod, int64 elvTileX0, int64 elvTileY1)
   {
      std::map< int64, std::list<ElevationPoint> >::iterator it = streamMap.begin();

      while (it!=streamMap.end())
      {
         int64 idx = it->first;

         // convert idx to tile coord:
         int tx = idx % tilewidth_i;
         int ty = idx / tilewidth_i;
         int64 tileX = tx + elvTileX0;
         int64 tileY = elvTileY1 - ty;

         std::string sQuadcode = qQuadtree->TileCoordToQuadkey(tileX,tileY,lod);
         std::string sTilefile = ProcessingUtils::GetTilePath(sTileDir, ".pts" , lod, tileX, tileY);

         /*double px0,py0,px1,py1;
         qQuadtree->QuadKeyToMercatorCoord(sQuadcode, px0, py1, px1, py0);*/

         // LOCK this tile. If this tile is currently locked then wait until the lock is removed.
         int lockhandle = FileSystem::Lock(sTilefile);

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
            std::list<ElevationPoint> lst = it->second;
            std::list<ElevationPoint>::iterator jt = lst.begin();

            while (jt != lst.end())
            {        
               fout.write((const char*)&(jt->x), sizeof(double));
               fout.write((const char*)&(jt->y), sizeof(double));
               fout.write((const char*)&(jt->elevation), sizeof(double));
               fout.write((const char*)&(jt->weight), sizeof(double));

               jt++;
            }
         }
         // unlock file. Other computers/processes/threads can access it again.
         FileSystem::Unlock(sTilefile, lockhandle);

         it++;
      }

      streamMap.clear();
      points_in_map = 0;

      //std::string sTilefile = ProcessingUtils::GetTilePath(sTileDir, ".pts" , lod, xx, yy);
   }


   //---------------------------------------------------------------------------

   int process( boost::shared_ptr<Logger> qLogger, boost::shared_ptr<ProcessingSettings> qSettings, std::string sLayer, bool bVerbose, bool bLock, bool bVirtual, int epsg, std::string sElevationFile, bool bFill, int& out_lod, int64& out_x0, int64& out_y0, int64& out_x1, int64& out_y1)
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
      std::string sTempfile = FilenameUtils::DelimitPath(sTileDir) + FilenameUtils::ExtractBaseFileName(sElevationFile) + ".dat";

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
      size_t numpts;

      if (bVerbose)
      {
         oss << "Please wait... Transforming all points to WGS84/Mercator...\n";
         qLogger->Info(oss.str());
         oss.str("");
      }

      if (!oElevationReader.Import(sTempfile, numpts, xmin, ymin, xmax, ymax))
      {
         qLogger->Error("Failed importing elevation.");
         ProcessingUtils::exit_gdal();
         return ERROR_LOADELEVATION;
      }
     
      if (bVerbose)
      {
         oss << "Number of Points: " << numpts << "\n";
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

      std::map< int64, std::list<ElevationPoint> >   streamMap;

      ElevationPoint pt;
      int n = 0;
      while (oElevationReader.GetNextPoint(pt))
      {
         n++;

          // calculate tile coordinate of current point:         
         int64 ttx = int64((pt.x - x0) / tilewidth);
         int64 tty = int64((pt.y - y0) / tileheight);
         int64 tileX = ttx+elvTileX0;
         int64 tileY = tty+elvTileY0;

         int64 idx = tty*tilewidth_i+ttx;

         AddPoint(streamMap, idx, pt);

         if (points_in_map>=MAX_POINTS_IN_MEMORY) // flush
         {
            if (bVerbose)
            {
               oss << "\nWriting (" << points_in_map << " points to disk. (";
               oss << n << " of " << numpts << " points stored)";
               qLogger->Info(oss.str());
               oss.str("");
            }

            WriteMap(qQuadtree, streamMap, sTileDir, tilewidth_i, lod, elvTileX0, elvTileY1);
         }
      }

      //Write remaining points:

      if (bVerbose  && points_in_map>0)
      {
         if (bVerbose)
            {
               oss << "\nWriting (" << streamMap.size() << " points to disk\n";
               oss << "status: " << n << " of " << numpts << " points stored.";
               qLogger->Info(oss.str());
               oss.str("");
            }
      }

      WriteMap(qQuadtree, streamMap, sTileDir, tilewidth_i, lod, elvTileX0, elvTileY1);

      // finished, print stats:
      t1=clock();

      std::ostringstream out;
      out << "calculated in: " << double(t1-t0)/double(CLOCKS_PER_SEC) << " s \n";
      qLogger->Info(out.str());

      oElevationReader.Close();

      ProcessingUtils::exit_gdal();
      return 0; 
   }
   

} // namespace


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