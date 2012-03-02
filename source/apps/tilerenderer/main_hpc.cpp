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

//------------------------------------------------------------------------------
// HPC Version of the tile rendering mechanism
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
#include "app/QueueManager.h"
#include <boost/asio.hpp>

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
bool bGenerateJobs = false;
bool bOverrideQueue;
bool bOverrideTiles = true;
int iAmount = 256;
bool bLockEnabled = false;
double bounds[4];
int minZoom;
int maxZoom;
std::vector<Tile> vExpireList;
std::string sJobQueueFile;
std::string sProcessHostName;
QueueManager _QueueManager = QueueManager();

//------------------------------------------------------------------------------

void ProcessJob(const SJob& job)
{
   std::stringstream ss;
   ss << output_path << job.zoom << "/" << job.x << "/" << job.y << ".png";
   //std::cout << "..Render tile " << ss.str() << "on rank: " << rank << "   Tilesize: "<< g_map.getWidth() << " Projection: " << g_mapnikProj.params() << "\n";
   try
   {
   _renderTile(ss.str(),g_map,job.x,job.y,job.zoom,g_gProj,g_mapnikProj, bVerbose, bOverrideTiles, bLockEnabled);
   }catch(std::exception ex)
   {
      std::cout << std::cout << "[" << sProcessHostName<< "] ### RENDER ERROR @ z: "<< job.zoom<< "x: "<< job.x<< "y: "<< job.y << "\n";
      std::cout << std::cout << "[" << sProcessHostName<< "] ### -- Details " << ex.what() << "\n";
   }
}

//------------------------------------------------------------------------------------

void ConvertJobs(std::vector<QJob>& input, std::vector<SJob>& output)
{
   output.clear();
   for (size_t i=0;i<input.size();i++)
   {  
      SJob job2;
      memcpy(&job2, input[i].data.get(), sizeof(SJob));
      output.push_back(job2);
   }
}

//------------------------------------------------------------------------------------

void GenerateRenderJobs()
{
   int idx = 0;
   //----------------------------------
   // Generate jobs to render ALL tiles
   //----------------------------------
   if(!bUpdateMode)
   {
      std::cout << "[" << sProcessHostName<< "] " << " Generating jobs!\n"<< std::flush;
      if(bVerbose)
      {
         std::cout << "[" << sProcessHostName<< "] MinZoom: "<< minZoom << "\n";
         std::cout << "[" << sProcessHostName<< "] MaxZoom: "<< maxZoom << "\n" << std::flush;
      }
      for(int z = minZoom; z <= maxZoom; z++)
      {
         ituple px0 = g_gProj.geoCoord2Pixel(dtuple(bounds[0], bounds[3]),z);
         ituple px1 = g_gProj.geoCoord2Pixel(dtuple(bounds[2], bounds[1]),z);

         // check if we have directories in place
         std::string szoom = StringUtils::IntegerToString(z, 10);
         if(!FileSystem::DirExists(output_path + szoom))
            FileSystem::makedir(output_path + szoom);
         int xlow = int(px0.a/256.0);
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
               
            int ylow = int(px0.b/256.0);
            int yhigh = int(px1.b/256.0)+1;

            for(int y = ylow; y <= yhigh; y++)
            {
               // Validate x co-ordinate
               if((y < 0) || (y >= math::Pow2(z)))
               {
                  continue;
               }
               QJob job;
               SJob work;
               work.x = x; 
               work.y = y;
               work.zoom = z;
               job.data = boost::shared_array<char>(new char[sizeof(SJob)]);
               memcpy(job.data.get(), &work, sizeof(SJob));
               job.size = sizeof(SJob);
               _QueueManager.AddToJobQueue(sJobQueueFile, job, (bOverrideQueue && idx == 0)? false : true);
               idx++;
            }
         }
      }
      _QueueManager.CommitJobQueue(sJobQueueFile);
      std::cout << "[" << sProcessHostName<< "] " << "..generated " << idx << " jobs!\n" << std::flush;
      return;
   }
   else
   {
      //--------------------------------------
      // Generate jobs to render UPDATED tiles
      //--------------------------------------
      vExpireList = _readExpireList(expire_list);
      std::cout << "[" << sProcessHostName<< "] " << " Generating expired list jobs (z, x, y) starting from " << "(" << vExpireList[0].zoom << ", " << vExpireList[0].x << ", " << vExpireList[0].y << ")\n"<< std::flush;
      for(size_t i = 0; i < vExpireList.size(); i++)
      {
         Tile t = vExpireList[i];
         // generate folder structure
         std::string szoom = StringUtils::IntegerToString(t.zoom, 10);
         if(!FileSystem::DirExists(output_path + szoom))
            {FileSystem::makedir(output_path + szoom);}
         std::string str_x = StringUtils::IntegerToString(t.x,10);
         if(!FileSystem::DirExists(output_path + szoom + "/" + str_x))
            {FileSystem::makedir(output_path + szoom + "/" + str_x);}
         QJob job;
         SJob work;
         work.x = t.x; 
         work.y = t.y;
         work.zoom = t.zoom;
         job.data = boost::shared_array<char>(new char[sizeof(SJob)]);
         memcpy(job.data.get(), &work, sizeof(SJob));
         job.size = sizeof(SJob);
         _QueueManager.AddToJobQueue(sJobQueueFile, job, (bOverrideQueue && idx == 0)? false : true);
         idx++;
      }
      _QueueManager.CommitJobQueue(sJobQueueFile);
      std::cout << "[" << sProcessHostName<< "] " << "..generated " << idx << " jobs!\n" << std::flush;
      return;
   }
}
//------------------------------------------------------------------------------

int main ( int argc , char** argv)
{  
   sProcessHostName = boost::asio::ip::host_name();
   //---------------------------------------------------------------------------
   // -- init default values
   minZoom = 1;
   maxZoom = 18;
   bUpdateMode = false;
   bounds[0] = -180.0;
   bounds[1] = -90.0;
   bounds[2] = 180.0;
   bounds[3] = 90.0;
   
   po::options_description desc("Program-Options");
   desc.add_options()
      ("mapnik_dir", po::value<std::string>(), "mapnik path")
      ("map_file", po::value<std::string>(), "map configurations file")
      ("output_path", po::value<std::string>(), "output path")
      ("numthreads", po::value<int>(), "force number of threads")
      ("min_zoom", po::value<int>(), "[optional] min zoom level")
      ("max_zoom", po::value<int>(), "[optional] max zoom level")
      ("bounds", po::value<std::vector<double>>(), "[optional] boundaries (default: -180.0 -90.0 180.0 90.0)")
      ("verbose", "[optional] Verbose mode")
      ("generatejobs","[optional] create a jobqueue which can be used in every process")
      ("overridejobqueue","[optional] overrides existing queue file if exist (only when generatejobs is set!)")
      ("amount", po::value<int>(), "[opional] define amount of jobs to be read for one process at the time")
      ("no_override", "[opional] overriding existing tiles disabled")
      ("enable_locking", "[opional] lock files to prevent concurrency on parallel processes")
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
      std::cout << "[" << sProcessHostName<< "] " << "Error in configuration! Check setup.xml\n";
      return ERROR_PARAMS;
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

   if(vm.count("verbose"))
      bVerbose = true;

   if(vm.count("no_override"))
      bOverrideTiles = false;

   if(vm.count("expire_list"))
   {
      std::cout << "[" << sProcessHostName<< "] " << "Using expirelist source " << expire_list << "\n";
      expire_list = vm["expire_list"].as<std::string>();
      bUpdateMode = true;
   }
   if(vm.count("amount"))
      iAmount = vm["amount"].as<int>();

   if(vm.count("generatejobs"))
      bGenerateJobs = true;

   if(vm.count("enable_locking"))
      bLockEnabled = true;

   if(vm.count("overridejobqueue"))
   {
      bOverrideQueue = true;
      std::cout << "overriding job queue\n";
   }else
      bOverrideQueue = false;

     
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
   int numThreads = 1;
   if (vm.count("numthreads"))
   {
      int n = vm["numthreads"].as<int>();
      if (n>0 && n<65)
      {
         std::cout << "[" << sProcessHostName<< "] " << "Forcing number of threads to " << n << " per node\n";
         omp_set_num_threads(n);
         numThreads = n;
      }
   }

   if (bError)
   {
      std::cout << "[" << sProcessHostName<< "] " << desc;
      return  ERROR_PARAMS;
   }

   sJobQueueFile = output_path + "jobqueue.jobs";
   if(bGenerateJobs)
   {
      GenerateRenderJobs();
   }
   else
   {
      std::cout << "[" << sProcessHostName<< "] " << "Render boundaries: " << bounds[0] << ", " << bounds[1] << ", " << bounds[2] << ", " << bounds[3] << "\n";
      std::cout << "[" << sProcessHostName<< "] " << "Render Map File: " << map_file << "\n";
      std::cout << "[" << sProcessHostName<< "] " << "Min-Zoom: " << minZoom << "\n";
      std::cout << "[" << sProcessHostName<< "] " << "Max-Zoom: " << maxZoom << "\n" << std::flush;

      //---------------------------------------------------------------------------
      //-- MAPNIK RENDERING PROCESS --------
      //---------------------------------------------------------------------------
      using namespace mapnik;
      try
      {
         g_gProj = GoogleProjection(maxZoom);
         double dummy = 0.0;
   
         #ifdef _DEBUG
         std::string plugin_path = mapnik_dir + "input/debug/";
         std::cout << "[" << sProcessHostName<< "] " << "..set plugin-path to "<<plugin_path<<"\n"<< std::flush;
         #else
         std::string plugin_path = mapnik_dir + "input/release/";   
         std::cout << "[" << sProcessHostName<< "] " << "..set plugin-path to "<<plugin_path<<"\n"<< std::flush;
         #endif

         datasource_cache::instance()->register_datasources(plugin_path.c_str()); 
         std::string font_dir = mapnik_dir + "fonts/dejavu-fonts-ttf-2.30/ttf/";
         {
            std::cout << "[" << sProcessHostName<< "] " << "..looking for DejaVuSans fonts in... " << font_dir << "\n"<< std::flush;
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
            std::cout << "[" << sProcessHostName<< "] " << "....Ok!\n" << std::flush;
         } else { std::cout << "[" << sProcessHostName<< "] " << "....#Error# Font directory not found!\n" << std::flush; }
         //---------------------------------------------------------------------------
         // -- Generate map container
         g_map.set_background(color_factory::from_string("white"));
         std::cout << "[" << sProcessHostName<< "] " << "..loading map file \"" << map_file << "\".....";
         load_map(g_map,map_file);
         std::cout << "[" << sProcessHostName<< "] " << "....Ok!\n" << std::flush;

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

         if(!FileSystem::FileExists(sJobQueueFile))
         {
            std::cout << "[" << sProcessHostName<< "] " << "ERROR: Jobqueue file not found: " << sJobQueueFile << " use --generatejobs first...\n"<< std::flush;
            return ERROR_PARAMS;
         }
         std::vector<SJob> vecConverted;
         std::vector<QJob> jobs;
         std::cout << "[" << sProcessHostName<< "] >>>" << "start processing...\n"<< std::flush;
         do
         {
            jobs.clear();
            vecConverted.clear();
            jobs = _QueueManager.FetchJobList(sJobQueueFile, sizeof(SJob), iAmount, bVerbose);
            if(jobs.size() > 0)
            {
               ConvertJobs(jobs, vecConverted);
               SJob first, last;
               first = vecConverted[0];
               last = vecConverted[vecConverted.size()-1];
               clock_t subT0 = clock();
               clock_t subT1;
               std::cout << "--[" << sProcessHostName<< "] " << "  processing " << vecConverted.size() << " jobs\n       starting from (z, x, y) " << "(" << first.zoom << ", " << first.x << ", " << first.y << ")\n"<< std::flush;
#ifndef _DEBUG
               std::cout << "..Processing parallel using " << numThreads << "\n";
               #pragma omp parallel shared(vecConverted, output_path, g_map,g_gProj,g_mapnikProj, bVerbose)
               {
                  #pragma omp for 
#endif
                  for(int index = 0; index < vecConverted.size(); index++)
                  {
                     ProcessJob(vecConverted[index]);
                     tileCount++;
                  }
#ifndef _DEBUG
               }
#endif
               subT1 = clock();
               double subTime=(double(subT1-subT0)/double(CLOCKS_PER_SEC));
               double subTps = vecConverted.size()/subTime;
               std::cout << "--[" << sProcessHostName<< "] " << "  processing average " << subTps << " tiles per second.\n";
               std::cout << "--[" << sProcessHostName<< "] " << "  processed " << vecConverted.size() << " jobs\n       terminating with (z, x, y) " << "(" << last.zoom << ", " << last.x << ", " << last.y << ")\n"<< std::flush;
            }
         }while(jobs.size() >= iAmount);
         t_1 = clock();
         double time=(double(t_1-t_0)/double(CLOCKS_PER_SEC));
         double tps = tileCount/time;
         std::cout << "[" << sProcessHostName<< "] <<<" << "finished processing "<< tileCount << " jobs at " << tps << " tiles pers second working for " << time << " seconds.\n"<< std::flush;
      }
      catch ( const mapnik::config_error & ex )
      {
         std::cout << "[" << sProcessHostName<< "] " << "### Configuration ERROR: " << ex.what() << std::flush;
         return ERROR_CONFIG;
      }
      catch ( const std::exception & ex )
      {
         std::cout << "[" << sProcessHostName<< "] " << "### std::exception: " << ex.what() << std::flush;
         return ERROR_MAPNIK;
      }
      catch ( ... )
      {
         std::cout << "[" << sProcessHostName<< "] " << "### Unknown exception." << std::flush;
         return EXIT_FAILURE;
      }
   }
   return EXIT_SUCCESS;
}

