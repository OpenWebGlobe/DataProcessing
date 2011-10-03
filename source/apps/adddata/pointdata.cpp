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

#include "pointdata.h"
#include "string/FilenameUtils.h"
#include "io/FileSystem.h"
#include "geo/PointLayerSettings.h"
#include "geo/MercatorQuadtree.h"
#include "geo/PointCloudReader.h"
#include "image/ImageLoader.h"
#include "image/ImageWriter.h"
#include "math/ElevationPoint.h"
#include "math/CloudPoint.h"
#include "math/mat4.h"
#include "math/GeoCoord.h"
#include "math/Octocode.h"
#include <sstream>
#include <fstream>
#include <ctime>

#include <omp.h>



namespace PointData
{
   const size_t membuffer = 10000; // number of points to kept in memory

   int process( boost::shared_ptr<Logger> qLogger, boost::shared_ptr<ProcessingSettings> qSettings, std::string sLayer, bool bVerbose, bool bLock, int epsg, std::string sPointFile, bool bFill, int& out_lod, int64& out_x0, int64& out_y0, int64& out_z0, int64& out_x1, int64& out_y1, int64& out_z1)
   {
      clock_t t0,t1;
      t0 = clock();

      if (!ProcessingUtils::init_gdal())
      {
         qLogger->Error("gdal-data directory not found!");
         return ERROR_GDAL;
      }

      boost::shared_ptr<CoordinateTransformation> qCT;
      qCT = boost::shared_ptr<CoordinateTransformation>(new CoordinateTransformation(epsg, 4326));

      //---------------------------------------------------------------------------
      // Retrieve PointLayerSettings:
      std::ostringstream oss;

      std::string sPointLayerDir = FilenameUtils::DelimitPath(qSettings->GetPath()) + sLayer;
      std::string sTileDir = FilenameUtils::DelimitPath(FilenameUtils::DelimitPath(sPointLayerDir) + "tiles");

      boost::shared_ptr<PointLayerSettings> qPointLayerSettings = PointLayerSettings::Load(sPointLayerDir);
      if (!qPointLayerSettings)
      {
         qLogger->Error("Failed retrieving point layer settings! Make sure to create it using 'createlayer'.");
         ProcessingUtils::exit_gdal();
         return ERROR_ELVLAYERSETTINGS;
      }

      int lod = qPointLayerSettings->GetMaxLod();
      double x0, y0, z0, x1, y1, z1;
      qPointLayerSettings->GetBoundary(x0, y0, z0, x1, y1, z1);
      
      if (bVerbose)
      {
         oss << "Point Layer:\n";
         oss << "     name = " << qPointLayerSettings->GetLayerName() << "\n";
         oss << "   maxlod = " << lod << "\n";
         oss << " boundary = (" << x0 << ", " << y0 << ", " << z0 << ")-(" << x1 << ", " << y1 << ", " << z1 << ")\n";
         qLogger->Info(oss.str());
         oss.str("");
      }

      double lodlen = pow(2.0,lod);

      //------------------------------------------------------------------------
      // Calculate matrix for octree voxel data transformation
      
      double xcenter, ycenter, zcenter;
      xcenter = x0 + fabs(x1-x0);
      ycenter = y0 + fabs(y1-y0);
      zcenter = z0 + fabs(z1-z0); // elevation is currently ignored and set to 0

      double len = 24000; // octree cube size (set to 24km length)
   
      mat4<double> L, Linv;
   
      // center to radiant
      double lng = DEG2RAD(xcenter);
      double lat = DEG2RAD(ycenter);

      // center to cartesian coord
      vec3<double> vCenter;
      GeoCoord geoCenter(xcenter, ycenter, 0);
      geoCenter.GetCartesian(vCenter);

      // create orthonormal basis
      // (and create 4x4 matrix with translation to vCenter)
      // scale to normalized geozentric cartesian coordinates (scaled meters)
      double scalelen = CARTESIAN_SCALE_INV * len;

      // translate to center (lng,lat)
      mat4<double> matTrans;
      matTrans.SetTranslation(vCenter);

      mat4<double> matNavigation;
      //matNavigation.CalcNavigationFrame(lng, lat);

      // Navigation frame with z-axis up!
      matNavigation.Set(
         -sin(lng),  -sin(lat)*cos(lng),  cos(lat)*cos(lng), 0,
         cos(lng),   -sin(lat)*sin(lng),  cos(lat)*sin(lng), 0,
         0,          cos(lat),            sin(lat),          0,
         0,          0,                   0,                 1);

      // scale to range [-0.5,0.5] (local coordinates)
      mat4<double> matScale;
      matScale.SetScale(scalelen); 

      // translate to range [0,1]
      mat4<double> matTrans2;
      matTrans2.SetTranslation(-0.5, -0.5, -0.5);

      L = matTrans; // translate to vCenter
      L *= matNavigation; // rotate to align ellipsoid normal
      L *= matScale; // scale to [-0.5, 0.5]
      L *= matTrans2; // translate [0.5, 0.5, 0.5] to have range [0,1]

      Linv = L.Inverse(); // create inverse of this transformation

      //------------------------------------------------------------------------



      size_t numpts = 0;

      CloudPoint pt;
      PointCloudReader pr;

      if (pr.Open(sPointFile))
      {
         GeoCoord in_geopt;          
         vec3<double> in_pt_cart;    // point in geocentric cartesian coordinates (WGS84)
         vec3<double> out_pt_octree; // point in local octree coordinates

         while (pr.ReadPoint(pt))
         {
            qCT->Transform(&pt.x, &pt.y);

            in_geopt.SetLongitude(pt.x);
            in_geopt.SetLatitude(pt.y);
            in_geopt.SetEllipsoidHeight(pt.elevation);
            
            in_geopt.ToCartesian(&in_pt_cart.x, &in_pt_cart.y, &in_pt_cart.z);
            out_pt_octree = Linv.vec3mul(in_pt_cart);

            int64 octreeX = int64(out_pt_octree.x * lodlen); 
            int64 octreeY = int64(out_pt_octree.y * lodlen); 
            int64 octreeZ = int64(out_pt_octree.z * lodlen); 

            // now we have the octree coordinate (octreeX,Y,Z) of the point
            // 
            // -> #todo: create or update voxel brick
            // ->        store on disk (JSON)

            numpts++;
         }
      }
      else
      {
         return -1;
      }

    

      //------------------------------------------------------------------------

      ProcessingUtils::exit_gdal();


      t1 = clock();
      std::cout << "calculated in: " << double(t1-t0)/double(CLOCKS_PER_SEC) << " s \n";

      return 0;
   }

}

