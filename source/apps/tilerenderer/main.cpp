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
#                           robert.wueest@fhnw.ch                              #
********************************************************************************
*     Licensed under MIT License. Read the file LICENSE for more information   *
*******************************************************************************/
// This is the version without mpi intended for regular 
// workstations. Multi cores are supported (OpenMP) and highly recommended.
//------------------------------------------------------------------------------
// Some code adapted from: generate_tiles.py
// Found at: http://trac.openstreetmap.org/browser/applications/rendering/mapnik
//------------------------------------------------------------------------------
#include <mapnik/map.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/filter_factory.hpp>
#include <mapnik/color_factory.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/config_error.hpp>
#include <mapnik/load_map.hpp>
#include <mapnik/envelope.hpp>
#include <mapnik/proj_transform.hpp>
#include <iostream>
#include <string>
#include <boost/filesystem.hpp>
#include "render_tile.h"
#include <string/FilenameUtils.h>
#include <string/StringUtils.h>
#include <io/FileSystem.h>
#include <math/mathutils.h>
#include "ogprocess.h"
#include "app/ProcessingSettings.h"
#include "errors.h"
#include <boost/program_options.hpp>
#include <omp.h>
#include "functions.h"

namespace po = boost::program_options;

//------------------------------------------------------------------------------------
int main ( int argc , char** argv)
{    
   po::options_description desc("Program-Options");
   desc.add_options()
      ("mapnik_dir", po::value<std::string>(), "mapnik path")
      ("map_file", po::value<std::string>(), "map configurations file")
      ("output_path", po::value<std::string>(), "output path")
      ("numthreads", po::value<int>(), "force number of threads")
      ("min_zoom", po::value<int>(), "[optional] min zoom level")
      ("max_zoom", po::value<int>(), "[optional] max zoom level")
      ("expired_list", po::value<std::string>(), "[optional] list of expired tiles for update rendering (global rendering will be disabled)")
      ("bounds", po::value<std::vector<double>>(), "[optional] boundaries (default: -180.0 -90.0 180.0 90.0)")
      ;
   
   po::positional_options_description p;
   p.add("bounds", -1);
   po::variables_map vm;
   po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
   po::notify(vm);

   //---------------------------------------------------------------------------
   // init options:

   boost::shared_ptr<ProcessingSettings> qSettings =  ProcessingUtils::LoadAppSettings();

   if (!qSettings)
   {
      std::cout << "Error in configuration! Check setup.xml\n";
      return ERROR_CONFIG;
   }

   //---------------------------------------------------------------------------
   // create logger
   boost::shared_ptr<Logger> qLogger =  ProcessingUtils::CreateLogger("tile_renderer", qSettings);

   if (!qLogger)
   {
      std::cout << "Error in configuration! Check setup.xml\n";
      return ERROR_CONFIG;
   }

   bool bError = false;
   //----------------------------------------
   // Read commandline
   try
   {
      po::store(po::parse_command_line(argc, argv, desc), vm);
      po::notify(vm);
   }
   catch (std::exception&)
   {
      bError = true;
   }
   std::string mapnik_dir;
   if(vm.count("mapnik_dir"))
   {
     mapnik_dir = vm["mapnik_dir"].as<std::string>();
     if(!(mapnik_dir.at(mapnik_dir.length()-1) == '\\' || mapnik_dir.at(mapnik_dir.length()-1) == '/'))
        mapnik_dir = mapnik_dir + "/";
   }
   else
      bError = true;

   std::string map_file;
   if(vm.count("map_file"))
      map_file = vm["map_file"].as<std::string>();
   else
      bError = true;

   std::string output_path;
   if(vm.count("output_path"))
   {
      output_path = vm["output_path"].as<std::string>();
      if(!(output_path.at(output_path.length()-1) == '\\' || output_path.at(output_path.length()-1) == '/'))
        output_path = output_path + "/";
   }
   else
      bError = true;

   int minZoom = 1;
   if(vm.count("min_zoom"))
      minZoom = vm["min_zoom"].as<int>();

   int maxZoom = 18;
   if(vm.count("max_zoom"))
      maxZoom = vm["max_zoom"].as<int>();

   bool bUpdateMode = false;
   std::string expire_list;
   if(vm.count("expire_list"))
      {
         expire_list = vm["expire_list"].as<std::string>();
         bUpdateMode = true;
      }

   // CH Bounds  double bounds[4] = {5.955870,46.818020,10.492030,47.808380};
   double bounds[4] = {-180.0,-90.0,180.0,90.0};
   if(vm.count("bounds"))
   {
      std::vector<double> dv = vm["bounds"].as<std::vector<double>>();
      if(dv.size() != 4)
         bError = true;
      else
      {
         bounds[0] = dv[0]; bounds[1] = dv[1]; bounds[2] =dv [2]; bounds[3] = dv[3];
      }
   }

   std::ostringstream oss; 
   oss << "Render boundaries: " << bounds[0] << ", " << bounds[1] << ", " << bounds[2] << ", " << bounds[3] << "\n";
   oss << "Render Map File: " << map_file << "\n";
   oss << "Min-Zoom: " << minZoom << "\n";
   oss << "Max-Zoom: " << maxZoom << "\n";
   qLogger->Info(oss.str());
  
   if (vm.count("numthreads"))
   {
      int n = vm["numthreads"].as<int>();
      if (n>0 && n<65)
      {
         std::ostringstream oss; 
         oss << "Forcing number of threads to " << n;
         qLogger->Info(oss.str());
         omp_set_num_threads(n);
      }
   }

   //---------------------------------------------------------------------------
   if (bError)
   {
      std::ostringstream oss; 
      qLogger->Error("Wrong parameters!");
      std::ostringstream sstr;
      sstr << desc;
      qLogger->Info("\n" + sstr.str());

      return ERROR_PARAMS;
   }
   bool tsmScheme = false;
   
    
   using namespace mapnik;
   try
   {
      {
         std::ostringstream oss;
         oss << "Generating map ...... \n";
         qLogger->Info(oss.str());
      }
      GoogleProjection gProj = GoogleProjection(maxZoom); // maxlevel 12
      //projection merc = projection("+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +units=m +nadgrids=@null +no_defs +over");
      //projection longlat = projection("+proj=latlong +datum=WGS84");
      double dummy = 0.0;
   
      //proj_transform transform = proj_transform(longlat,merc);
      std::ostringstream oss;
      #ifdef _DEBUG
      std::string plugin_path = mapnik_dir + "input/debug/";
      oss << "..load plugins from "<<plugin_path<<"\n";
      #else
      std::string plugin_path = mapnik_dir + "input/release/";   
      oss << "..load plugins from "<<plugin_path<<"\n";
      #endif
      qLogger->Info(oss.str());
      datasource_cache::instance()->register_datasources(plugin_path.c_str()); 
      std::string font_dir = mapnik_dir + "fonts/dejavu-fonts-ttf-2.30/ttf/";
      {
         std::stringstream oss;
         oss << "..looking for DejaVuSans fonts in... " << font_dir << "\n";
         qLogger->Info(oss.str());
      }
      if (boost::filesystem3::exists( font_dir ) )
      {
         boost::filesystem3::directory_iterator end_itr; // default construction yields past-the-end
         for ( boost::filesystem3::directory_iterator itr( font_dir );
            itr != end_itr;
            ++itr )
         {
            if (!boost::filesystem3::is_directory(itr->status()) )
            {
               freetype_engine::register_font(itr->path().string());
            }
         }
      }
      // Generate map container
      Map m(256,256);
         m.set_background(color_factory::from_string("white"));
      load_map(m,map_file);
      projection mapnikProj = projection(m.srs());

      if(!FileSystem::DirExists(output_path))
         FileSystem::makedir(output_path);
      
      if(!bUpdateMode)
      {
         std::stringstream oss;
         oss << "[Rendermode: Normal] Start rendering tiles..\n";
         qLogger->Info(oss.str());
         double avtps = 0.0;
         int avtps_it = 0;
         for(int z = minZoom; z < maxZoom + 1; z++)
         {
            ituple px0 = gProj.geoCoord2Pixel(dtuple(bounds[0], bounds[3]),z);
            ituple px1 = gProj.geoCoord2Pixel(dtuple(bounds[2], bounds[1]),z);

            // check if we have directories in place
            std::string szoom = StringUtils::IntegerToString(z, 10);
            if(!FileSystem::DirExists(output_path + szoom))
               FileSystem::makedir(output_path + szoom);

            for(int x = int(px0.a/256.0); x <= int(px1.a/256.0) +1; x++)
            {
               // Validate x co-ordinate
               if((x < 0) || (x >= math::Pow2(z)))
                  continue;
               // check if we have directories in place
               std::string str_x = StringUtils::IntegerToString(x,10);
               if(!FileSystem::DirExists(output_path + szoom + "/" + str_x))
                  FileSystem::makedir(output_path + szoom + "/" + str_x);
               clock_t t0,t1;
               t0 = clock();
               int tileCount = 0;
               int low = int(px0.b/256.0);
               int high = int(px1.b/256.0)+1;
               #pragma omp parallel shared(low,high,x,z,m,gProj,mapnikProj,tsmScheme, output_path, szoom,str_x,tileCount)
               {
                  #pragma omp for 
                  for(int y = low; y <= high; y++)
                  {
                     // Validate x co-ordinate
                     if((y < 0) || (y >= math::Pow2(z)))
                        continue;
                     // flip y to match OSGEO TMS spec
                     std::string str_y;
                     std::stringstream ss;
                     if(tsmScheme)
                     {
                        ss << math::Pow2(z-1);
                        str_y = ss.str();
                     }
                     else
                     {
                        ss << y;
                        str_y = ss.str();
                     }

                     std::string tile_uri = output_path + szoom + '/' + str_x + '/' + str_y + ".png";
                     // Submit tile to be rendered
                     _renderTile(tile_uri,m,x,y,z,gProj,mapnikProj);
                     tileCount++;
                  }
               }
               t1=clock();
               double tilesPerSecond = tileCount/(double(t1-t0)/double(CLOCKS_PER_SEC));
               //std::cout << " average tiles per second: " << tilesPerSecond << "\n";
               {
                  std::stringstream oss;
                  oss << "..average tiles per second: " << tilesPerSecond << "\n";
                  qLogger->Info(oss.str());
                  avtps += tilesPerSecond;
                  avtps_it++;
               }
            }
         }
         {
         std::stringstream oss;
         oss << "Finished rendering with total average " <<  (avtps/avtps_it)  << " Tiles per second!\n";
         qLogger->Info(oss.str());
         }
      }
      else
      {
         std::stringstream oss;
         oss << "[Rendermode: Update] Start rendering tiles..\n reading expire list...\n";
         qLogger->Info(oss.str());
         std::vector<Tile> vExpireList = _readExpireList(expire_list);
         clock_t t0,t1;
            t0 = clock();
            int tileCount = 0;
         #pragma omp parallel shared(qLogger,vExpireList,m,gProj,mapnikProj,tsmScheme, output_path,tileCount)
         {
            #pragma omp for 
            for(int i = 0; i < vExpireList.size(); i++)
            {
               std::stringstream ss;
               Tile t = vExpireList[i];
              

               std::string szoom = StringUtils::IntegerToString(t.zoom, 10);
               if(!FileSystem::DirExists(output_path + szoom))
               FileSystem::makedir(output_path + szoom);
               std::string str_x = StringUtils::IntegerToString(t.x,10);
               if(!FileSystem::DirExists(output_path + szoom + "/" + str_x))
                  FileSystem::makedir(output_path + szoom + "/" + str_x);

               ss << output_path << t.zoom << "/" << t.x << "/" << t.y << ".png";
               std::string tile_uri = ss.str();
               _renderTile(tile_uri,m,t.x,t.y,t.zoom,gProj,mapnikProj);
               if(tileCount % 10000)
               {
                  std::stringstream oss;
                  oss << ".." << tileCount << " processed!\n";
                  qLogger->Info(oss.str());
               }
            }
         }
         double tilesPerSecond = tileCount/(double(t1-t0)/double(CLOCKS_PER_SEC));
         {
         std::stringstream oss;
         oss << "Finished rendering with total average " <<  tilesPerSecond  << " Tiles per second!\n";
         qLogger->Info(oss.str());
         }
      }
   }
   catch ( const mapnik::config_error & ex )
   {
      std::cerr << "### Configuration error: " << ex.what() << std::endl;
      return ERROR_CONFIG;
   }
   catch ( const std::exception & ex )
   {
      std::cerr << "### std::exception: " << ex.what() << std::endl;
      return ERROR_MAPNIK;
   }
   catch ( ... )
   {
      std::cerr << "### Unknown exception." << std::endl;
      return EXIT_FAILURE;
   }
   return EXIT_SUCCESS;
}
