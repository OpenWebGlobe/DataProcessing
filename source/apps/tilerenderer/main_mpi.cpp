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
struct SJob
{
   int x, y, zoom;
};

//------------------------------------------------------------------------------
// globals:
mapnik::projection g_mapnikProj;
mapnik::Map g_map(256, 256);
GoogleProjection g_gProj;
std::string map_file;
std::string mapnik_dir;
std::string output_path;
std::string expire_list;
bool bUpdateMode;
bool bVerbose = false;
int queueSize = 4096;
double bounds[4];
int iX = 0;
int iY = 0;
int iZ = 0;
int iN = 0;
int minZoom;
int maxZoom;
std::vector<Tile> vExpireList;
//------------------------------------------------------------------------------
// MPI Job callback function (called every thread/compute node)
void jobCallback(const SJob& job, int rank)
{
   std::stringstream ss;
   ss << output_path << job.zoom << "/" << job.x << "/" << job.y << ".png";
   std::ofstream myfile;
   std::stringstream tile_log;
   tile_log << output_path << "tile_joblist_rank"<<rank<<".log";
   myfile.open (tile_log.str(), std::ios::out | std::ios::app);
   myfile << "Tile: Z: " << job.zoom << "  X: " << job.x << "  Y: "<< job.y << "\n";
   myfile.close();
   //std::cout << "..Render tile " << ss.str() << "on rank: " << rank << "   Tilesize: "<< g_map.getWidth() << " Projection: " << g_mapnikProj.params() << "\n";
   _renderTile(ss.str(),g_map,job.x,job.y,job.zoom,g_gProj,g_mapnikProj, bVerbose);
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
void GenerateRenderJobs(int count, std::vector<SJob> &vJobs)
{
   int idx = 0;
   //----------------------------------
   // Generate jobs to render ALL tiles
   //----------------------------------
   if(!bUpdateMode)
   {
      if(iZ < 0) { iZ = minZoom; }
      if(bVerbose)
         std::cout << " Generating " << count << " jobs (z, x, y) starting from " << "(" << iZ << ", " << iX << ", " << iY << ")\n";
      for(int z = iZ; z <= maxZoom; z++)
      {
         ituple px0 = g_gProj.geoCoord2Pixel(dtuple(bounds[0], bounds[3]),z);
         ituple px1 = g_gProj.geoCoord2Pixel(dtuple(bounds[2], bounds[1]),z);

         // check if we have directories in place
         std::string szoom = StringUtils::IntegerToString(z, 10);
         if(!FileSystem::DirExists(output_path + szoom))
            FileSystem::makedir(output_path + szoom);
         int xlow = iX < 0 ? int(px0.a/256.0): iX;
         int xhigh = int(px1.a/256.0) +1;
         for(int x = xlow; x <= xhigh; x++)
         {
            // Validate x co-ordinate
            if((x < 0) || (x >= math::Pow2(z)))
            {
               continue;
            }
            // check if we have directories in place
            std::string str_x = StringUtils::IntegerToString(x,10);
            if(!FileSystem::DirExists(output_path + szoom + "/" + str_x))
               FileSystem::makedir(output_path + szoom + "/" + str_x);
               
            int ylow = iY < 0 ? int(px0.b/256.0): iY;
            int yhigh = int(px1.b/256.0)+1;

            for(int y = ylow; y <= yhigh; y++)
            {
               // Validate x co-ordinate
               if((y < 0) || (y >= math::Pow2(z)))
               {
                  continue;
               }
               SJob work;
               work.x = x; 
               work.y = y;
               work.zoom = z;
               vJobs.push_back(work);
               iY = (y < yhigh)? y+1 : -1;
               iX = (y < yhigh) ? x: (x < xhigh) ? x+1 : -1;
               iZ = z;
               if(idx >= (count-1))
               {
                  return;
               }
               idx++;
            }
            iY = -1;
         }
         iX = -1;
      }
      if(iZ == maxZoom)
      {
         iZ++;
      }
      return;
   }
   else
   {
      //--------------------------------------
      // Generate jobs to render UPDATED tiles
      //--------------------------------------
      if(iN < 0)
      {
         vExpireList = _readExpireList(expire_list);
         iN = 0;
      }
      for(size_t i = iN; i < vExpireList.size(); i++)
      {
         Tile t = vExpireList[i];
         // generate folder structure
         std::string szoom = StringUtils::IntegerToString(t.zoom, 10);
         if(!FileSystem::DirExists(output_path + szoom))
            {FileSystem::makedir(output_path + szoom);}
         std::string str_x = StringUtils::IntegerToString(t.x,10);
         if(!FileSystem::DirExists(output_path + szoom + "/" + str_x))
            {FileSystem::makedir(output_path + szoom + "/" + str_x);}

         SJob work;
         work.x = t.x; 
         work.y = t.y;
         work.zoom = t.zoom;
         vJobs.push_back(work);
         iN = (i < vExpireList.size()) ? i+1: i;
         if(idx >= (count-1))
         {
            return;
         }
         idx++;
      }

      return;
   }
}
//------------------------------------------------------------------------------

int main ( int argc , char** argv)
{   
   //---------------------------------------------------------------------------
   // MPI Init
   //---------------------------------------------------------------------------
   int rank, totalnodes;

   MPI_Init(&argc, &argv);
   MPI_Comm_size(MPI_COMM_WORLD, &totalnodes); 
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);

   

   //---------------------------------------------------------------------------
   // -- init default values
   minZoom = 1;
   maxZoom = 18;
   bUpdateMode = false;
   iX = -1;
   iY = -1;
   iZ = -1; 
   iN = -1;
   bounds[0] = -180.0;
   bounds[1] = -90.0;
   bounds[2] = 180.0;
   bounds[3] = 90.0;
   

   if (rank == 0)
   {
      std::cout << ">>> Generating map using mapnik <<<\n";
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
         ("verbose", "[optional] Verbose mode")
         ("expired_list", po::value<std::string>(), "[optional] list of expired tiles for update rendering (global rendering will be disabled)")
         ;
   
      po::positional_options_description p;
      p.add("bounds", -1);
      po::variables_map vm;
      po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
      po::notify(vm);

   

      //---------------------------------------------------------------------------
      // -- init options:
      boost::shared_ptr<ProcessingSettings> qSettings =  ProcessingUtils::LoadAppSettings();

      if (!qSettings)
      {
         std::cout << "Error in configuration! Check setup.xml\n";
         return MPI_Abort(MPI_COMM_WORLD, ERROR_PARAMS);
      }


      bool bError = false;
      //---------------------------------------------------------------------------
      // -- Read commandline
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

      if(vm.count("verbose"))
         bVerbose = true;

      if(vm.count("expire_list"))
      {
         expire_list = vm["expire_list"].as<std::string>();
         bUpdateMode = true;
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

      if (bError)
      {
         std::cout << desc;
         return MPI_Abort(MPI_COMM_WORLD, ERROR_PARAMS);
      }
   }

   BroadcastString(output_path, 0);
   BroadcastString(mapnik_dir, 0);
   BroadcastString(map_file, 0);
   BroadcastDouble(bounds[0], 0);
   BroadcastDouble(bounds[1], 0);
   BroadcastDouble(bounds[2], 0);
   BroadcastDouble(bounds[3], 0);
   BroadcastInt(minZoom,0);
   BroadcastInt(maxZoom,0);
   BroadcastInt(iX,0);
   BroadcastInt(iY,0);
   BroadcastInt(iZ,0);
   BroadcastInt(iN,0);
   BroadcastInt(queueSize,0);
   BroadcastBool(bVerbose,0);
   BroadcastBool(bUpdateMode,0);

   MPIJobManager<SJob> jobmgr(queueSize);

   //---------------------------------------------------------------------------
   //-- MAPNIK RENDERING PROCESS --------
   //---------------------------------------------------------------------------
   using namespace mapnik;
   try
   {
      std::cout << "Generating tiles on rank " << rank << "...... \n" << std::flush;
      g_gProj = GoogleProjection(maxZoom);
      double dummy = 0.0;
   
      #ifdef _DEBUG
      std::string plugin_path = mapnik_dir + "input/debug/";
      std::cout << "..set plugin-path to "<<plugin_path<<"\n"<< std::flush;
      #else
      std::string plugin_path = mapnik_dir + "input/release/";   
      std::cout << "..set plugin-path to "<<plugin_path<<"\n"<< std::flush;
      #endif

      datasource_cache::instance()->register_datasources(plugin_path.c_str()); 
      std::string font_dir = mapnik_dir + "fonts/dejavu-fonts-ttf-2.30/ttf/";
      {
         std::cout << "..looking for DejaVuSans fonts in... " << font_dir << "\n"<< std::flush;
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
         std::cout << "....Ok!\n" << std::flush;
      } else { std::cout << "....#Error# Font directory not found!\n" << std::flush; }
      //---------------------------------------------------------------------------
      // -- Generate map container
      g_map.set_background(color_factory::from_string("white"));
      std::cout << "..loading map file \"" << map_file << "\".....";
      load_map(g_map,map_file);
      std::cout << "....Ok!\n" << std::flush;

      g_mapnikProj = projection(g_map.srs());
      //---------------------------------------------------------------------------
      // -- Create outputpath
      if(!FileSystem::DirExists(output_path))
         FileSystem::makedir(output_path);
      bool bDone = false;
      //---------------------------------------------------------------------------
      // -- performance measurement
      int tileCount = 0;
      int currentJobQueueSize = 0;
      clock_t t_0, t_1;
      t_0 = clock();
      while (!bDone)
      {
         if (rank == 0)
         {
            std::vector<SJob> vJobs;
            GenerateRenderJobs(totalnodes*queueSize, vJobs);
            currentJobQueueSize = vJobs.size();
            tileCount += currentJobQueueSize;
            if (vJobs.size() == 0) // no more jobs
            {
               bDone = true;
               t_1 = clock();
               double time=(double(t_1-t_0)/double(CLOCKS_PER_SEC));
               double tps = tileCount/time;
               std::cout << ">>> Finished rendering " << tileCount << " tiles at " << tps << " tiles per second! TOTAL TIME: " << time << "<<<\n" << std::flush;
            }
            else
            {
               jobmgr.AddJobs(vJobs);
            }
         }
 
         MPI_Barrier(MPI_COMM_WORLD);
         BroadcastBool(bDone, 0);
         if (!bDone)
         {  
            clock_t t0,t1;
            t0 = clock();
            jobmgr.Process(jobCallback, bVerbose);
            t1 = clock();
            double tilesPerSecond = currentJobQueueSize/(double(t1-t0)/double(CLOCKS_PER_SEC));
         }
         else
         {
            MPI_Finalize();
         } 
      }
   }
   catch ( const mapnik::config_error & ex )
   {
      std::cout << "### Configuration ERROR: " << ex.what() << std::endl;
      return ERROR_CONFIG;
   }
   catch ( const std::exception & ex )
   {
      std::cout << "### std::exception: " << ex.what() << std::endl;
      return ERROR_MAPNIK;
   }
   catch ( ... )
   {
      std::cout << "### Unknown exception." << std::endl;
      return EXIT_FAILURE;
   }
   return EXIT_SUCCESS;
}

