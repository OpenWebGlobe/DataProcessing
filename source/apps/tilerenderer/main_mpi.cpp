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
#                           martin.christen@fhnw.ch                            #
********************************************************************************
*     Licensed under MIT License. Read the file LICENSE for more information   *
*******************************************************************************/
/*                                                                            */
/*                  MPI Version of resample (for cluster/cloud)               */
/*                      asynchronous workload distribution                    */
/*          this is the recommended version for all clusters / clouds         */
/*                for multicore single computers use resample.cpp             */
/*                                                                            */
/******************************************************************************/
//------------------------------------------------------------------------------
// MPI Version of the tile rendering mechanism
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
#include <mpi.h>
#include "mpi/Utils.h"
#include "functions.h"

namespace po = boost::program_options;

//-------------------------------------------------------------------
// Job-Struct
struct Job
{
   int x, y, zoom;
};

//------------------------------------------------------------------------------
// globals:
mapnik::projection g_mapnikProj;
mapnik::Map g_map;
GoogleProjection g_gProj;
std::string map_file;
std::string mapnik_dir;
std::string output_path;
std::string expire_list;

//------------------------------------------------------------------------------
// MPI Job callback function (called every thread/compute node)
void jobCallback(const Job& job, int rank)
{
   std::stringstream ss;
   ss << output_path << job.zoom << "/" << job.x << "/" << job.y << ".png";
   if(job.x >= 0)
   _renderTile(ss.str(),g_map,job.x,job.y,job.zoom,g_gProj,g_mapnikProj);
   else
   {
      bool blub = false;
   }
}
//------------------------------------------------------------------------------
void BroadcastString(std::string& sStr, int sender)
{
   unsigned int len = sStr.length();
   MPI_Bcast(&len, 1, MPI_UNSIGNED, sender, MPI_COMM_WORLD);
   if (sStr.length() < len ) 
   {
      sStr.insert(sStr.end(),(size_t)(len-sStr.length()), ' ');
   }
   else if (sStr.length() > len ) 
   {
      sStr.erase( len,sStr.length()-len);
   }

   MPI_Bcast(const_cast<char *>(sStr.data()), len, MPI_CHAR, sender, MPI_COMM_WORLD);
}
//------------------------------------------------------------------------------
void BroadcastInt(int& val, int sender)
{
   MPI_Bcast(&val, 1, MPI_INT, sender, MPI_COMM_WORLD);
}
//------------------------------------------------------------------------------
void BroadcastDouble(double& val, int sender)
{
   MPI_Bcast(&val, 1, MPI_DOUBLE, sender, MPI_COMM_WORLD);
}
//------------------------------------------------------------------------------
void BroadcastInt64(int64& val, int sender)
{
   MPI_Bcast(&val, 1, MPI_LONG_LONG, sender, MPI_COMM_WORLD);
}
//------------------------------------------------------------------------------
void BroadcastBool(bool& val, int sender)
{
   MPI_Datatype bool_type;
   if (sizeof(bool) == 1) bool_type= MPI_BYTE;
   else if (sizeof(bool) == 2) bool_type = MPI_SHORT;
   else if (sizeof(bool) == 4) bool_type = MPI_INT;
   else { assert(false); return;}
   MPI_Bcast(&val, 1, bool_type, sender, MPI_COMM_WORLD);
}
//------------------------------------------------------------------------------------
int main ( int argc , char** argv)
{    
   //---------------------------------------------------------------------------
   // MPI Init
   //---------------------------------------------------------------------------

   int rank, totalnodes;

   MPI_Init(&argc, &argv); 
   MPI_Comm_size(MPI_COMM_WORLD, &totalnodes);
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);

   MPIJobManager<Job> jobmgr(4096);

   
   int minZoom = 1;
   int maxZoom = 18;
   double bounds[4] = {-180.0,-90.0,180.0,90.0};

   bool bVerbose = false;
   int queueSize = 10000;
   bool updateMode = false;

   if (rank == 0)
   {
      po::options_description desc("Program-Options");
      desc.add_options()
         ("mapnik_dir", po::value<std::string>(), "mapnik path")
         ("map_file", po::value<std::string>(), "map configurations file")
         ("output_path", po::value<std::string>(), "output path")
         ("numthreads", po::value<int>(), "force number of threads")
         ("min_zoom", po::value<int>(), "[optional] min zoom level")
         ("max_zoom", po::value<int>(), "[optional] max zoom level")
         ("bounds", po::value<std::vector<double>>(), "[optional] boundaries (default: -180.0 -90.0 180.0 90.0)")
         ("mpi_queue_size", po::value<int>(), "[optional] mpi queue size (Default: 10000)")
         ("verbose_mode", po::value<bool>(), "[optional] Verbose mode")
         ("expired_list", po::value<std::string>(), "[optional] list of expired tiles for update rendering (global rendering will be disabled)")
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
         return MPI_Abort(MPI_COMM_WORLD, ERROR_PARAMS);
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
      
      if(vm.count("mapnik_dir"))
      {
        mapnik_dir = vm["mapnik_dir"].as<std::string>();
        if(!(mapnik_dir.at(mapnik_dir.length()-1) == '\\' || mapnik_dir.at(mapnik_dir.length()-1) == '/'))
           mapnik_dir = mapnik_dir + "/";
      }
      else
         bError = true;

      
      if(vm.count("map_file"))
         map_file = vm["map_file"].as<std::string>();
      else
         bError = true;

      
      if(vm.count("output_path"))
      {
         output_path = vm["output_path"].as<std::string>();
         if(!(output_path.at(output_path.length()-1) == '\\' || output_path.at(output_path.length()-1) == '/'))
           output_path = output_path + "/";
      }
      else
         bError = true;
      
      if(vm.count("min_zoom"))
         minZoom = vm["min_zoom"].as<int>();
      
      if(vm.count("max_zoom"))
         maxZoom = vm["max_zoom"].as<int>();

      if(vm.count("mpi_queue_size"))
         queueSize = vm["mpi_queue_size"].as<int>();

      if(vm.count("verbose_mode"))
         bVerbose = vm["verbose_mode"].as<bool>();

      if(vm.count("expire_list"))
      {
         expire_list = vm["expire_list"].as<std::string>();
         updateMode = true;
      }
     

      // CH Bounds  double bounds[4] = {5.955870,46.818020,10.492030,47.808380};
      
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
      std::cout << "Render boundaries: " << bounds[0] << ", " << bounds[1] << ", " << bounds[2] << ", " << bounds[3] << "\n";
      std::cout << "Render Map File: " << map_file << "\n";
      std::cout << "Min-Zoom: " << minZoom << "\n";
      std::cout << "Max-Zoom: " << maxZoom << "\n";
  
      if (vm.count("numthreads"))
      {
         int n = vm["numthreads"].as<int>();
         if (n>0 && n<65)
         {
            std::cout << "Forcing number of threads to " << n << " per node\n";
            omp_set_num_threads(n);
         }
      }

      //---------------------------------------------------------------------------
       //---------------------------------------------------------------------------
      if (bError)
      {
         std::cout << desc;
         return MPI_Abort(MPI_COMM_WORLD, ERROR_PARAMS);
      }
   }
   bool tsmScheme = false;

   BroadcastString(output_path, 0);
   BroadcastString(mapnik_dir, 0);
   BroadcastString(map_file, 0);
   BroadcastDouble(bounds[0], 0);
   BroadcastDouble(bounds[1], 0);
   BroadcastDouble(bounds[2], 0);
   BroadcastDouble(bounds[3], 0);
   BroadcastInt(minZoom,0);
   BroadcastInt(maxZoom,0);
   BroadcastBool(bVerbose,0);
   BroadcastBool(updateMode,0);

    
   using namespace mapnik;
   try
   {
      {
         std::cout << ">>> Generating map on rank " << rank << "...... \n";
      }
      GoogleProjection gProj = GoogleProjection(maxZoom); // maxlevel 12
      //projection merc = projection("+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +units=m +nadgrids=@null +no_defs +over");
      //projection longlat = projection("+proj=latlong +datum=WGS84");
      double dummy = 0.0;
   
      //proj_transform transform = proj_transform(longlat,merc);
      #ifdef _DEBUG
      std::string plugin_path = mapnik_dir + "input/debug/";
      std::cout << "..load plugins from "<<plugin_path<<"\n";
      #else
      std::string plugin_path = mapnik_dir + "input/release/";   
      std::cout << "..load plugins from "<<plugin_path<<"\n";
      #endif

      datasource_cache::instance()->register_datasources(plugin_path.c_str()); 
      std::string font_dir = mapnik_dir + "fonts/dejavu-fonts-ttf-2.30/ttf/";
      {
         std::cout << "..looking for DejaVuSans fonts in... " << font_dir << "\n";
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

      // Create outputpath
      if(!FileSystem::DirExists(output_path))
         FileSystem::makedir(output_path);
      double avtps = 0.0;
      int avtps_it = 0;
      
      if(rank == 0)
      {
         clock_t t0,t1;
         int tileCount = 0;
         t0 = clock();
         bool bJobQueueEmpty = true;
         //--------------------------------
         // Render ALL tiles
         //--------------------------------
         if(!updateMode)
         {
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
               
                  int low = int(px0.b/256.0);
                  int high = int(px1.b/256.0)+1;
                  //#pragma omp parallel shared(low,high,x,z,m,gProj,mapnikProj,tsmScheme, output_path, szoom,str_x,tileCount)
                  //{
                     //#pragma omp for 
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
                        //_renderTile(tile_uri,m,x,y,z,gProj,mapnikProj);
                        Job work;
                        work.x = x; 
                        work.y = y;
                        work.zoom = z;
                        jobmgr.AddJob(work);
                        bJobQueueEmpty = false;
                        tileCount++;
                        if(tileCount >= queueSize)
                        {
                           jobmgr.Process(jobCallback, bVerbose);
                           t1=clock();
                           bJobQueueEmpty = true;
                           double tilesPerSecond = tileCount/(double(t1-t0)/double(CLOCKS_PER_SEC));
                           std::cout << "..generated " << tileCount << " at " << tilesPerSecond << " tiles per second\n";
                           avtps += tilesPerSecond;
                           avtps_it++;
                           t0 = clock();
                           tileCount = 0;
                        }
                     }
                  //}
               }
            }
            // process remaining tiles
            if(!bJobQueueEmpty)
            {
               jobmgr.Process(jobCallback, bVerbose);
               t1=clock();
               bJobQueueEmpty = true;
               double tilesPerSecond = tileCount/(double(t1-t0)/double(CLOCKS_PER_SEC));
               std::cout << "..generated " << tileCount << " at " << tilesPerSecond << " tiles per second\n";
               avtps += tilesPerSecond;
               avtps_it++;
            }
         }
         else
         {
            //--------------------------------
            // Render UPDATED tiles
            //--------------------------------
            std::vector<Tile> vExpireList = _readExpireList(expire_list);
            for(size_t i = 0; i < vExpireList.size(); i++)
            {
               std::stringstream ss;
               Tile t = vExpireList[i];
               ss << output_path << t.zoom << "/" << t.x << "/" << t.y << ".png";
               std::string tile_uri = ss.str();
               // Submit tile to be rendered
               Job work;
               work.x = t.x; 
               work.y = t.y;
               work.zoom = t.zoom;
               jobmgr.AddJob(work);
               bJobQueueEmpty = false;
               tileCount++;
               if(tileCount >= queueSize)
               {
                  jobmgr.Process(jobCallback, bVerbose);
                  t1=clock();
                  bJobQueueEmpty = true;
                  double tilesPerSecond = tileCount/(double(t1-t0)/double(CLOCKS_PER_SEC));
                  std::cout << "..generated " << tileCount << " at " << tilesPerSecond << " tiles per second\n";
                  avtps += tilesPerSecond;
                  avtps_it++;
                  t0 = clock();
                  tileCount = 0;
               }
            }
            // process remaining tiles
            if(!bJobQueueEmpty)
            {
               jobmgr.Process(jobCallback, bVerbose);
               t1=clock();
               bJobQueueEmpty = true;
               double tilesPerSecond = tileCount/(double(t1-t0)/double(CLOCKS_PER_SEC));
               std::cout << "..generated " << tileCount << " at " << tilesPerSecond << " tiles per second\n";
               avtps += tilesPerSecond;
               avtps_it++;
            } 
         }
         MPI_Finalize();
         std::cout << ">>> Finished rendering with total average " <<  (avtps/avtps_it)  << " Tiles per second!n";
      }
      else
      {
         jobmgr.Process(jobCallback, bVerbose);
         MPI_Finalize();
      }
   }
   catch ( const mapnik::config_error & ex )
   {
      std::cerr << "### Configuration ERROR: " << ex.what() << std::endl;
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

