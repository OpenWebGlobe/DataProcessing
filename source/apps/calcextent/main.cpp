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

//------------------------------------------------------------------------------

#include "og.h"
#include "gdal.h"
#include "gdal_priv.h"
#include "ogr_api.h"
#include "cpl_conv.h"
#include "geo/MercatorQuadtree.h"
#include "geo/CoordinateTransformation.h"
#include "string/FilenameUtils.h"
#include "string/StringUtils.h"
#include "io/FileSystem.h"
#include <iostream>
#include <ctime>
#include <boost/program_options.hpp>
#include "omp.h"


bool init_gdal();
void exit_gdal();
int _frominput(const std::vector<std::string>& vecFiles, const std::string& srs, bool bVerbose);

//------------------------------------------------------------------------------

namespace po = boost::program_options;

int main(int argc, char *argv[])
{
   po::options_description desc("Program-Options");
   desc.add_options()
       ("maxlod", po::value<int>(), "desired level of detail (integer)")
       ("wgs84", po::value< std::vector<double> >()->multitoken(), "wgs84 coordinates lng0 lat0 lng1 lat1")
       ("srs", po::value<std::string>(), "spatial reference system for input files")
       ("input", po::value< std::vector<std::string> >()->multitoken(), "list input files")
       ("verbose", "display additional information")
       ("numthreads", po::value<int>(), "forcing number of threads to use for calculation")
       ;

   po::variables_map vm;
   po::store(po::parse_command_line(argc, argv, desc), vm);
   po::notify(vm);

   if ((vm.count("wgs84") && !vm.count("maxlod")) || (!vm.count("wgs84") && vm.count("maxlod")))
   {
      std::cout << "ERROR: option --wgs84 and --maxlod must be used together\n";
      return 1;
   }

   if (vm.count("wgs84") && vm.count("maxlod"))
   {
      int lod = vm["maxlod"].as<int>();
      if (lod < 0 || lod>23)
      {
         std::cout << "ERROR: lod value in wrong range\n";
         return 1;
      }

      double lng0, lat0, lng1, lat1;

      std::vector<double> vecCoords= vm["wgs84"].as< std::vector<double> >();
      if (vecCoords.size() != 4)
      {
         std::cout << "ERROR: wrong number of wgs84 coordinates\n";
         return 1;
      }
      
      lng0 = vecCoords[0];
      lat0 = vecCoords[1];
      lng1 = vecCoords[2];
      lat1 = vecCoords[3];
      
      if (lng0 >= lng1) { std::cout << "error: lng1 must be greater than lng0\n"; return 0; }
      if (lat0 >= lat1) { std::cout << "error: lat1 must be greater than lat0\n"; return 0; }
      if (lod < 4) { std::cout << "error: level of detail must be atleast 4\n"; return 0; }


      MercatorQuadtree* pQuadtree = new MercatorQuadtree();

      int64 px0, py0, px1, py1, tx0, ty0, tx1, ty1;
      pQuadtree->WGS84ToPixel(lng0, lat0, lod, px0, py1);
      pQuadtree->WGS84ToPixel(lng1, lat1, lod, px1, py0);

      std::cout << "Number of pixels in specified range: " << (px1-px0+1)*(py1-py0+1) << "\n";

      pQuadtree->PixelToTileCoord(px0, py0, tx0, ty0);
      pQuadtree->PixelToTileCoord(px1, py1, tx1, ty1);

      std::cout << "Tile Coords: (" << tx0 << ", " << ty0 << ")-(" << tx1 << ", " << ty1 << ")\n";

      delete pQuadtree;

      return 0;
   }
   else if (vm.count("srs") && vm.count("input"))
   {
      std::vector<std::string> vecFiles = vm["input"].as< std::vector<std::string> >();
      std::string srs = vm["srs"].as<std::string>();
      bool bVerbose = false;

      if (vm.count("verbose"))
      {
         bVerbose = true;
      }

      if (vm.count("numthreads"))
      {
         int nthreads = vm["numthreads"].as<int>();
         if (nthreads>=1)
         {
            std::cout << "forcing number of threads: " << nthreads << "\n";
            omp_set_num_threads(nthreads);
         }
      }

      return _frominput(vecFiles, srs, bVerbose);
   }
   else
   {
      std::cout << desc << "\n";
      std::cout << "From input files: use --srs and --input together\n";
      std::cout << "From wgs84 coord: use --wgs84 and --maxlod together\n";
      return 1;
   }

   return 0;
}


//------------------------------------------------------------------------------
bool init_gdal()
{
   // find gdal-data directory
   bool has_data_dir = FileSystem::DirExists("gdal-data");

   GDALAllRegister();
   CPLSetConfigOption("GDAL_CACHEMAX", "0"); // don't need a gdal cache
   if (has_data_dir) 
   {
      CPLSetConfigOption("GDAL_DATA", "gdal-data");
   }
   OGRRegisterAll();

   return has_data_dir;

   return true;
}
//------------------------------------------------------------------------------

void exit_gdal()
{
   GDALDestroyDriverManager();
   OGRCleanupAll();
}
//------------------------------------------------------------------------------

bool InvertGeoMatrix(double* mGeoMatrix, double* mInvGeoMatrix)
{
   double	det, inv_det;
   det = mGeoMatrix[1] * mGeoMatrix[5] - mGeoMatrix[2] * mGeoMatrix[4];

   if( fabs(det) < DBL_EPSILON )
      return false;

   inv_det = 1.0 / det;

   mInvGeoMatrix[1] =  mGeoMatrix[5] * inv_det;
   mInvGeoMatrix[4] = -mGeoMatrix[4] * inv_det;
   mInvGeoMatrix[2] = -mGeoMatrix[2] * inv_det;
   mInvGeoMatrix[5] =  mGeoMatrix[1] * inv_det;
   mInvGeoMatrix[0] = ( mGeoMatrix[2] * mGeoMatrix[3] - mGeoMatrix[0] * mGeoMatrix[5]) * inv_det;
   mInvGeoMatrix[3] = (-mGeoMatrix[1] * mGeoMatrix[3] + mGeoMatrix[0] * mGeoMatrix[4]) * inv_det;

   return true;
}

//------------------------------------------------------------------------------

struct DataSetInfo
{
   double   dest_ulx;
   double   dest_lry;
   double   dest_lrx;
   double   dest_uly;
   double   affineTransformation[6];
   double   affineTransformation_inverse[6];
   int      nBands;
   int      nSizeX;
   int      nSizeY;
   bool     bGood;     // true if this data is valid, otherwise couldn't load or failed some way
};

//------------------------------------------------------------------------------

int _frominput(const std::vector<std::string>& vecFiles, const std::string& srs, bool bVerbose)
{
   // create an array of Dataset info for parallel access.
   DataSetInfo* pDataset = new DataSetInfo[vecFiles.size()];

  
   if (StringUtils::Left(srs, 5) != "EPSG:")
   {
      std::cout << "Error: only srs starting with EPSG: are currently supported";
      return 1;
   }

   int epsg = atoi(srs.c_str()+5);
   std::cout << "SRS epsg-code: " << epsg << "\n";

   if (!init_gdal())
   {
      std::cout << "Warning: gdal-data directory not found. Ouput may be wrong!\n";
   }   

   boost::shared_ptr<CoordinateTransformation> qCT;
   qCT = boost::shared_ptr<CoordinateTransformation>(new CoordinateTransformation(epsg, 3785));

   if (bVerbose)
   {
      std::cout << "input files:\n";
      for (size_t i=0;i<vecFiles.size();i++)
      {
         std::cout << "   " << vecFiles[i] << "\n";
      }
   }

   clock_t t0,t1;
   t0 = clock();

#pragma omp parallel for
   for (int i=0;i<(int)vecFiles.size();i++)
   {
      pDataset[i].bGood=false;

      GDALDataset* s_fh = (GDALDataset*)GDALOpen(vecFiles[i].c_str(), GA_ReadOnly);   
      if(s_fh)
      {
         pDataset[i].bGood = true;
         s_fh->GetGeoTransform(pDataset[i].affineTransformation);
         pDataset[i].nBands = s_fh->GetRasterCount();
         pDataset[i].nSizeX = s_fh->GetRasterXSize();
         pDataset[i].nSizeY = s_fh->GetRasterYSize();
      }
      GDALClose(s_fh);

      //------------------------------------------------------------------------
      if (pDataset[i].bGood)
      {
         if (!InvertGeoMatrix(pDataset[i].affineTransformation, pDataset[i].affineTransformation_inverse))
         {
            std::cout << "**FAILED calculating invert of affine transformation of " << vecFiles[i] << "\n";
         }
   
         double dPixelWidth  = pDataset[i].affineTransformation[1];
         double dPixelHeight = pDataset[i].affineTransformation[5];

         double ulx = pDataset[i].affineTransformation[0];
         double uly = pDataset[i].affineTransformation[3];
         double lrx = ulx + pDataset[i].affineTransformation[1] * pDataset[i].nSizeX;
         double lry = uly + pDataset[i].affineTransformation[5] * pDataset[i].nSizeY;

         pDataset[i].dest_ulx = 1e20;
         pDataset[i].dest_lry = 1e20;
         pDataset[i].dest_lrx = -1e20;
         pDataset[i].dest_uly = -1e20;

         //Transform every pixel along border
         for (int p=0;p<=pDataset[i].nSizeX;p++)
         {
            unsigned long x,y;
            double lng,lat;  // (this is actually in mercator projection)
            x = p;
            y = 0;
            lat = pDataset[i].affineTransformation[3] + double(x)*pDataset[i].affineTransformation[4] + double(y)*pDataset[i].affineTransformation[5];
            lng = pDataset[i].affineTransformation[0] + double(x)*pDataset[i].affineTransformation[1] + double(y)*pDataset[i].affineTransformation[2];
            qCT->Transform(&lng, &lat);
            pDataset[i].dest_ulx = math::Min<double>(lng, pDataset[i].dest_ulx);
            pDataset[i].dest_lry = math::Min<double>(lat, pDataset[i].dest_lry);
            pDataset[i].dest_lrx = math::Max<double>(lng, pDataset[i].dest_lrx);
            pDataset[i].dest_uly = math::Max<double>(lat, pDataset[i].dest_uly);
            x = p;
            y = pDataset[i].nSizeY;
            lat = pDataset[i].affineTransformation[3] + double(x)*pDataset[i].affineTransformation[4] + double(y)*pDataset[i].affineTransformation[5];
            lng = pDataset[i].affineTransformation[0] + double(x)*pDataset[i].affineTransformation[1] + double(y)*pDataset[i].affineTransformation[2];
            qCT->Transform(&lng, &lat);
            pDataset[i].dest_ulx = math::Min<double>(lng, pDataset[i].dest_ulx);
            pDataset[i].dest_lry = math::Min<double>(lat, pDataset[i].dest_lry);
            pDataset[i].dest_lrx = math::Max<double>(lng, pDataset[i].dest_lrx);
            pDataset[i].dest_uly = math::Max<double>(lat, pDataset[i].dest_uly);
         }
         for (int p=0;p<=pDataset[i].nSizeY;p++)
         {
            unsigned long x,y;
            double lng,lat; // (this is actually in mercator projection)
            x = 0;
            y = p;
            lat = pDataset[i].affineTransformation[3] + double(x)*pDataset[i].affineTransformation[4] + double(y)*pDataset[i].affineTransformation[5];
            lng = pDataset[i].affineTransformation[0] + double(x)*pDataset[i].affineTransformation[1] + double(y)*pDataset[i].affineTransformation[2];
            qCT->Transform(&lng, &lat);
            pDataset[i].dest_ulx = math::Min<double>(lng, pDataset[i].dest_ulx);
            pDataset[i].dest_lry = math::Min<double>(lat, pDataset[i].dest_lry);
            pDataset[i].dest_lrx = math::Max<double>(lng, pDataset[i].dest_lrx);
            pDataset[i].dest_uly = math::Max<double>(lat, pDataset[i].dest_uly);
            x = pDataset[i].nSizeX;
            y = p;
            lat = pDataset[i].affineTransformation[3] + double(x)*pDataset[i].affineTransformation[4] + double(y)*pDataset[i].affineTransformation[5];
            lng = pDataset[i].affineTransformation[0] + double(x)*pDataset[i].affineTransformation[1] + double(y)*pDataset[i].affineTransformation[2];
            qCT->Transform(&lng, &lat);
            pDataset[i].dest_ulx = math::Min<double>(lng, pDataset[i].dest_ulx);
            pDataset[i].dest_lry = math::Min<double>(lat, pDataset[i].dest_lry);
            pDataset[i].dest_lrx = math::Max<double>(lng, pDataset[i].dest_lrx);
            pDataset[i].dest_uly = math::Max<double>(lat, pDataset[i].dest_uly);
         }
      }
   }

   // at this point we finished calculating all the boundaries of all datasets, now
   // calculate the min/max

   double total_dest_ulx = 1e20;
   double total_dest_lry = 1e20;
   double total_dest_lrx = -1e20;
   double total_dest_uly = -1e20;

   for (size_t i=0;i<vecFiles.size();i++)
   {
      if (pDataset[i].bGood)
      {
         total_dest_ulx = math::Min(pDataset[i].dest_ulx, total_dest_ulx);
         total_dest_lry = math::Min(pDataset[i].dest_lry, total_dest_lry);
         total_dest_lrx = math::Max(pDataset[i].dest_lrx, total_dest_lrx);
         total_dest_uly = math::Max(pDataset[i].dest_uly, total_dest_uly);
      }
   }

   t1 = clock();

   std::cout << "GATHERED BOUNDARY (Mercator):\n";
   std::cout.precision(16);
   std::cout << "   ulx: " << total_dest_ulx << "\n";
   std::cout << "   lry: " << total_dest_lry << "\n";
   std::cout << "   lrx: " << total_dest_lrx << "\n";
   std::cout << "   uly: " << total_dest_uly << "\n";
   std::cout << "BOUNDARY in WGS84:\n";


   MercatorQuadtree* pQuadtree = new MercatorQuadtree();
   double x,y;

   x = total_dest_ulx; y = total_dest_lry;
   pQuadtree->MercatorToWGS84(x, y);
   std::cout << "   lng0: " << x << "\n";
   std::cout << "   lat0: " << y << "\n";
   x = total_dest_lrx; y = total_dest_uly;
   pQuadtree->MercatorToWGS84(x, y);
   std::cout << "   lng1: " << x << "\n";
   std::cout << "   lat1: " << y << "\n";

   delete pQuadtree;

   std::cout << "calculated in: " << double(t1-t0)/double(CLOCKS_PER_SEC) << " s \n";

   delete[] pDataset;

   exit_gdal();

   return 0;
}

//------------------------------------------------------------------------------

