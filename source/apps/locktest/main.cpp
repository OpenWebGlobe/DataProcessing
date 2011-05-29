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

/******************************************************************************/
/* This application tests the file lock mechanism for the HPC Cluster         */
/* You should run this on your cluster before starting data processing to     */
/* ensure your file system supports the lock mechanism                        */
/* you should run this on multiple compute nodes at the same time, pointing   */ 
/* to the same path.                                                          */
/******************************************************************************/

#include "ogprocess.h"
#include "string/FilenameUtils.h"
#include "string/StringUtils.h"
#include "io/FileSystem.h"
#include <iostream>
#include <fstream>
#include <boost/program_options.hpp>
#include <boost/thread.hpp>
#include <sstream>
#include <omp.h>


std::string g_sPath;
std::string g_sLockFile;
int g_numthreads;
int g_iterations;

//-----------------------------------------------------------------------------

// this function is called from another thread
void threadfunc()
{
   boost::thread::id  id = boost::this_thread::get_id();
 
   for (int i=0;i<g_iterations;i++)
   {
      int handle = FileSystem::Lock(g_sLockFile);
      int cnt;

      std::ifstream ifs;
      ifs.open(g_sLockFile.c_str());
      if (ifs.good())
      {
         ifs >> cnt;
         ifs.close();
      }
      else
      {
         cnt = 0; // first file or failure!
      }
      
      // increment counter
      cnt++;

      // overwrite file with incremented value:
      std::ofstream ofs;
      ofs.open(g_sLockFile.c_str());
      ofs << cnt;
      ofs.close();

      FileSystem::Unlock(g_sLockFile, handle);
   }
}

//-----------------------------------------------------------------------------

namespace po = boost::program_options;

int main(int argc, char *argv[])
{
   po::options_description desc("Program-Options");
   desc.add_options()
       ("path", po::value<std::string>(), "where to run test (this path must exist)")
       ("numthreads", po::value<int>(), "number of threads to use for test")
       ("iterations", po::value<int>(), "number of iterations per thread")
       ;

   po::variables_map vm;

   bool bError = false;

   try
   {
      po::store(po::parse_command_line(argc, argv, desc), vm);
      po::notify(vm);
   }
   catch (std::exception&)
   {
      bError = true;
   }

  
   if (!vm.count("path") || !vm.count("numthreads") || !vm.count("iterations"))
   {
      bError = true;
   }
   else
   {
      g_sPath = vm["path"].as<std::string>();
      g_numthreads = vm["numthreads"].as<int>();
      g_iterations = vm["iterations"].as<int>();

      if (g_numthreads<1 || g_iterations < 1)
      {  
         std::cout << "numthreads must be >1 and iterations must be >1\n";
         bError = true;
      }
   }
   
   if (!FileSystem::DirExists(g_sPath))
   {
      std::cout << "path " << g_sPath << " doesn't exist\n";
      bError = true;
   }

  
   //---------------------------------------------------------------------------
   if (bError)
   {
      std::cout << desc << "\n";
      return 1;
   }
   //---------------------------------------------------------------------------

   g_sLockFile = FilenameUtils::DelimitPath(g_sPath) + "locktest.txt";

   std::cout << "Running test\n";
   std::cout << "number of threads     : " << g_numthreads << "\n";
   std::cout << "number of iterations  : " << g_iterations << "\n";
   std::cout << "path                  : " << g_sPath << "\n";

   boost::thread_group  threads;

   for (int i=0;i<g_numthreads;++i)
   {
      threads.create_thread(threadfunc);
   }

   threads.join_all();
   
   std::cout << "OK. All threads finished on this computer.\n";

   return 0;
}


//------------------------------------------------------------------------------

