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
#include "io/FileSystem.h"
#include <sstream>
#include <fstream>
#include <ctime>
#include <map>
#include <list>
#include <set>
#include <omp.h>



namespace PointData
{
   //------------------------------------------------------------------------
   const size_t membuffer = 100000; // number of points to keep in memory
   //------------------------------------------------------------------------

   class PointMap
   {
   public:
      //------------------------------------------------------------------------
      PointMap(int levelofdetail)
      {
         _numpts = 0;
         _numkeys = 0;
         _lod = levelofdetail;
         _pow = int64(1) << _lod; 
         _dpow = _pow * _pow;
      }
      //------------------------------------------------------------------------
      virtual ~PointMap()
      {
      }
      //------------------------------------------------------------------------
      void Clear()
      {
         _map.clear();
         _numpts = 0;
         _numkeys = 0;
      }
      //------------------------------------------------------------------------
      void AddPoint(int64 i, int64 j, int64 k, const CloudPoint& pt)
      {
         int64 key = _dpow*k + _pow*j + i;
 
         _numpts++;
         
         std::map<int64, std::list<CloudPoint> >::iterator it;
         it = _map.find(key);
         if (it != _map.end())
         {
            // this key already exists, append point
            it->second.push_back(pt);
         }
         else
         {
            // this key doesn't exist yet. create new entry
            std::list<CloudPoint> newList;
            newList.push_back(pt);
            _map.insert(std::pair<int64, std::list<CloudPoint> >(key, newList));
            _numkeys++;
         }
      }
      //------------------------------------------------------------------------
      size_t GetNumPoints()
      {
         return _numpts;
      }
      //------------------------------------------------------------------------
      size_t GetNumKeys()
      {
          return _index.size();
      }
      //------------------------------------------------------------------------

      void ExportData(const std::string& path)
      {
         std::map<int64, std::list<CloudPoint> >::iterator it = _map.begin();

         while (it != _map.end())
         {
            int64 key = it->first;
            _index.insert(key);
            int64 i,j,k;

            k = key / _dpow;
            j = (key - _dpow*k) / _pow;
            i = key - _dpow*k - j*_pow;

            //std::string sOctocode = Octocode::IndexToOctocode(i,j,k,_lod);
            //create or open existing file: path/lod/x/y-z.json

            std::ostringstream oss;
            oss << path << _lod << "/" << i << "/" << j << "-" << k << ".dat";
            std::string sFilename = oss.str();

 
            std::ofstream of;


            if (FileSystem::FileExists(sFilename))
            {
               of.open(sFilename.c_str(), std::ios::app|std::ios::binary);
            }
            else
            {
               FileSystem::makeallsubdirs(sFilename);
               of.open(sFilename.c_str(), std::ios::binary);
            }

            of.precision(17);

            std::list<CloudPoint>& lstCloudPoints = it->second;

            std::list<CloudPoint>::iterator jt = lstCloudPoints.begin();
            while (jt != lstCloudPoints.end())
            {
               of.write((char*)&jt->x, sizeof(double));
               of.write((char*)&jt->y, sizeof(double));
               of.write((char*)&jt->elevation, sizeof(double));
               of.write((char*)&jt->r, sizeof(double));
               of.write((char*)&jt->g, sizeof(double));
               of.write((char*)&jt->b, sizeof(double));
               of.write((char*)&jt->intensity, sizeof(int));
   
               jt++;
            }

            of.close();

            ++it;
         }
      }

      //------------------------------------------------------------------------

      void ExportIndex(const std::string& sFilename)
      {
         std::set<int64>::iterator it = _index.begin();

         std::ofstream of(sFilename.c_str(), std::ios::binary);
        
         if (of.good())
         {
            while (it != _index.end())
            {
               int64 elm = *it;
               of.write((char*)&elm, sizeof(int64));
            
               it++;
            }
         }

         of.close();
      }

      //------------------------------------------------------------------------

      std::set<int64>& GetIndex(){return _index;}


   private:
      PointMap(){}
      std::map<int64, std::list<CloudPoint> > _map;
      std::set<int64> _index;
      size_t _numpts;
      size_t _numkeys;
      int _lod;
      int64 _pow;
      int64 _dpow;
   };

  
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
      std::string sTempDir = FilenameUtils::DelimitPath(FilenameUtils::DelimitPath(sPointLayerDir) + "temp/tiles");
      std::string sIndexFile = FilenameUtils::DelimitPath(sPointLayerDir) + "temp/" + FilenameUtils::ExtractBaseFileName(sPointFile) + ".idx";

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

      double len = 5000; // octree cube size
   
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
      size_t totalpoints = 0;

      CloudPoint pt;
      CloudPoint pt_octree; // point in octree coords
      PointCloudReader pr;
      PointMap pointmap(lod);

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
            pt_octree.r = pt.r;
            pt_octree.g = pt.g;
            pt_octree.b = pt.b;
            pt_octree.a = pt.a;
            pt_octree.intensity = pt.intensity;
            pt_octree.x = out_pt_octree.x;
            pt_octree.y = out_pt_octree.y;
            pt_octree.elevation = out_pt_octree.z;

            int64 octreeX = int64(out_pt_octree.x * lodlen); 
            int64 octreeY = int64(out_pt_octree.y * lodlen); 
            int64 octreeZ = int64(out_pt_octree.z * lodlen); 

            // now we have the octree coordinate (octreeX,Y,Z) of the point
            // -> add the point to pointmap (which is actually a hash map)
            // -> note: don't calculate the octocode for each point, it would be way too slow.
            pointmap.AddPoint(octreeX, octreeY, octreeZ, pt_octree);

            if (pointmap.GetNumPoints()>membuffer)
            {
               totalpoints+=pointmap.GetNumPoints();

               pointmap.ExportData(sTempDir);

               pointmap.Clear();
            }

            numpts++;
         }
      }
      else
      {
         return -1;
      }

      if (pointmap.GetNumPoints()>0)
         pointmap.ExportData(sTempDir);

      totalpoints+=pointmap.GetNumPoints();
    
      // export list of all written tiles (for future processing)
      pointmap.ExportIndex(sIndexFile);


      //------------------------------------------------------------------------

      ProcessingUtils::exit_gdal();


      std::cout << "Pointmap Stats:\n";
      std::cout << " numkeys:   " << pointmap.GetNumKeys() << "\n";
      std::cout << " numpoints: " << totalpoints << "\n";

      t1 = clock();
      std::cout << "calculated in: " << double(t1-t0)/double(CLOCKS_PER_SEC) << " s \n";

      return 0;
   }

}

