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
#include "ogr_api.h"
#include "cpl_conv.h"
#include "geo/MercatorQuadtree.h"
#include "geo/CoordinateTransformation.h"
#include "string/FilenameUtils.h"
#include "io/FileSystem.h"
#include <iostream>
#include <boost/program_options.hpp>


bool init_gdal();
void exit_gdal();
int _frominput(const std::vector<std::string>& vecFiles, const std::string& srs);

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

      return 0;
   }
   else if (vm.count("srs") && vm.count("input"))
   {
      std::vector<std::string> vecFiles = vm["input"].as< std::vector<std::string> >();
      std::string srs = vm["srs"].as<std::string>();

      return _frominput(vecFiles, srs);
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
   //CPLSetConfigOption("GDAL_CACHEMAX", "0");
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


int _frominput(const std::vector<std::string>& vecFiles, const std::string& srs)
{
   for (size_t i=0;i<vecFiles.size();i++)
   {
      std::cout << "input file: " << vecFiles[i] << "\n";
   }
   return 0;
}