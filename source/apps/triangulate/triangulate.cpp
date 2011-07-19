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
#include "math/delaunay/DelaunayTriangulation.h"
#include "geo/ElevationReader.h"
#include "geo/ElevationTile.h"
#include "errors.h"
#include <sstream>
#include <fstream>
#include <ctime>
#include <algorithm>
#include <omp.h>

// uncomment to generate .obj instead of JSON (in temp directory)
#define GENERATE_JSON

namespace triangulate
{

   //---------------------------------------------------------------------------

   inline void EliminateCloseToCorner(double x0, double y0, double x1, double y1, std::vector<ElevationPoint>& PointList, const double epsilon)
   {
      std::vector<ElevationPoint>::iterator it = PointList.begin();

      bool advance = true;

      while (it!= PointList.end())
      {
         double dx0 = fabs(x0 - (*it).x);
         double dy0 = fabs(y0 - (*it).y);
         double dx1 = fabs(x1 - (*it).x);
         double dy1 = fabs(y1 - (*it).y);

         if (dx0 < epsilon || dy0 < epsilon || dx1 < epsilon || dy1 < epsilon)
         {

            it = PointList.erase(it);
         }
         else
         {
            it++;
         }
      }
   }

   //-----------------------------------------------------------------------------
   // for performance reasons this only works on a sorted list!
   inline void EliminateDoubleEntriesX(std::vector<ElevationPoint>& PointList, const double epsilon)
   {
      std::vector<ElevationPoint>::iterator it = PointList.begin();

      bool advance = true;
      ElevationPoint current;

      while (it!= PointList.end())
      {
         if (advance)
         {
            current = *it;
            advance = false;
            it++;
         }

         if (it!= PointList.end())
         {
            double dx = fabs((*it).x - current.x);

            if (dx < epsilon)
            {
               it = PointList.erase(it);
            }
            else
            {
               advance = true;
            }
         }
      }
   }


   // for performance reasons this only works on a sorted list!
   inline void EliminateDoubleEntriesY(std::vector<ElevationPoint>& PointList, const double epsilon)
   {
      std::vector<ElevationPoint>::iterator it = PointList.begin();

      bool advance = true;
      ElevationPoint current;

      while (it!= PointList.end())
      {
         if (advance)
         {
            current = *it;
            advance = false;
            it++;
         }

         if (it!= PointList.end())
         {
            double dy = fabs((*it).y - current.y);

            if (dy < epsilon)
            {
               it = PointList.erase(it);
            }
            else
            {
               advance = true;
            }
         }
      }
   }

   //---------------------------------------------------------------------------

   int process(boost::shared_ptr<Logger> qLogger, boost::shared_ptr<ProcessingSettings> qSettings, int nMaxPoints, std::string sLayer, bool bVerbose)
   {
      // Retrieve ElevationLayerSettings:
      std::ostringstream oss;

      std::string sElevationLayerDir = FilenameUtils::DelimitPath(qSettings->GetPath()) + sLayer;
      std::string sTempTileDir = FilenameUtils::DelimitPath(FilenameUtils::DelimitPath(sElevationLayerDir) + "temp/tiles");
      std::string sTileDir = FilenameUtils::DelimitPath(FilenameUtils::DelimitPath(sElevationLayerDir) + "tiles");

      boost::shared_ptr<ElevationLayerSettings> qElevationLayerSettings = ElevationLayerSettings::Load(sElevationLayerDir);
      if (!qElevationLayerSettings)
      {
         qLogger->Error("Failed retrieving elevation layer settings! Make sure to create it using 'createlayer'.");
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

      int64 width = layerTileX1-layerTileX0+1;
      int64 height = layerTileY1-layerTileY0+1;

      if (width<3 || height<3)
      {
         qLogger->Error("Extent is too small for elevation processing");
         return ERROR_AREA;
      }

      boost::shared_ptr<MercatorQuadtree> qQuadtree = boost::shared_ptr<MercatorQuadtree>(new MercatorQuadtree());

      // Retrieve dataset extent in mercator coord:
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

#ifndef _DEBUG
#     pragma omp parallel for
#endif
      for (int64 xx = layerTileX0+1; xx < layerTileX1; ++xx)
      {
         for (int64 yy = layerTileY0+1; yy < layerTileY1; ++yy)
         {
            std::string sCurrentQuadcode = qQuadtree->TileCoordToQuadkey(xx,yy,lod);

            //std::cout << sCurrentQuadcode << "\n";
            std::vector<ElevationPoint> vecPts;

            for (int ty=-1;ty<=1;ty++)
            {
               for (int tx=-1;tx<=1;tx++)
               {
                  std::string sQuadcode = qQuadtree->TileCoordToQuadkey(xx+tx,yy+ty,lod);
                  std::string sTilefile = ProcessingUtils::GetTilePath(sTempTileDir, ".pts" , lod, xx+tx, yy+ty);
                  
                  double sx0, sy1, sx1, sy0;
                  qQuadtree->QuadKeyToMercatorCoord(sQuadcode, sx0, sy1, sx1, sy0);


                  //std::cout << "   " << sTilefile << "\n";

                  std::ifstream fin;
                  fin.open(sTilefile.c_str(), std::ios::binary);
                  if (fin.good())
                  {
                     while (!fin.eof())
                     {
                        ElevationPoint pt;
                        fin.read((char*)&(pt.x), sizeof(double));
                        fin.read((char*)&(pt.y), sizeof(double));
                        fin.read((char*)&(pt.elevation), sizeof(double));
                        fin.read((char*)&(pt.weight), sizeof(double));
                        if (!fin.eof())
                        {
                           vecPts.push_back(pt);
                        }


                     }
                  }
                  fin.close();

               }
            }
            
            // all points are in vecPts now -> triangulate and see if coverage is big enough

            double x0,y0,x1,y1;
            qQuadtree->QuadKeyToMercatorCoord(sCurrentQuadcode, x0, y1, x1, y0);
            double len = fabs(y1-y0);
            double xx0 = x0-len;
            double xx1 = x1+len;
            double yy0 = y0-len;
            double yy1 = y1+len;

            int cnt = 0;
            math::DelaunayTriangulation oTriangulation(xx0,yy0,xx1,yy1);
            math::DelaunayTriangulation oFinalTriangulation(xx0,yy0,xx1,yy1);
            for (size_t i=0;i<vecPts.size();i++)
            {
              if (vecPts[i].x > xx0 && vecPts[i].x < xx1 &&
                  vecPts[i].y > yy0 && vecPts[i].y < yy1)
                  {
                     oTriangulation.InsertPoint(vecPts[i]);
                     cnt++;
                  }
            }

            ElevationPoint NW, NE, SE, SW;
            std::vector<ElevationPoint> vNorth;
            std::vector<ElevationPoint> vEast;
            std::vector<ElevationPoint> vSouth;
            std::vector<ElevationPoint> vWest;
            std::vector<ElevationPoint> vMiddle;
            oTriangulation.IntersectRect(x0,y0,x1,y1, NW, NE, SE, SW, vNorth, vEast, vSouth, vWest, vMiddle);

            ElevationTile oElevationTile(x0,y0,x1,y1); // elevation tile for "sCurrentQuadcode"
            oElevationTile.Setup(NW, NE, SE, SW, vNorth, vEast, vSouth, vWest, vMiddle);

            // Thin out tile if there are too many points:
            oElevationTile.Reduce(nMaxPoints);

            std::string datastr;
            std::string sFilename;
            std::string sTempfilename; // for resampling info

#ifdef GENERATE_JSON
            //if (outputformat == JSON)
            datastr = oElevationTile.CreateJSON();
            sFilename = ProcessingUtils::GetTilePath(sTileDir, ".json" , lod, xx, yy);
#else
            //if (outputformat == OBJ) [internal testing only]
            datastr = oTriangulation.CreateOBJ(xmin, ymin, xmax, ymax);
            sFilename = sTempTileDir + sCurrentQuadcode + ".obj";
#endif

            // for binary data (resampling)
            sTempfilename = ProcessingUtils::GetTilePath(sTempTileDir, ".tri", lod, xx, yy);
            oElevationTile.WriteBinary(sTempfilename);

            // write output tile
            std::ofstream fout(sFilename.c_str());
            fout << datastr;
            fout.close();
         }
      }

      return 0;


   }
}