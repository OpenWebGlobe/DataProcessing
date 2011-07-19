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
// This is the triangulate version without mpi intended for regular 
// workstations. Multi cores are supported (OpenMP) and highly recommended.
//------------------------------------------------------------------------------

#include "ogprocess.h"
#include "errors.h"
#include "app/ProcessingSettings.h"
#include "geo/MercatorQuadtree.h"
#include "geo/CoordinateTransformation.h"
#include "string/FilenameUtils.h"
#include "string/StringUtils.h"
#include "geo/ImageLayerSettings.h"
#include "io/FileSystem.h"
#include "image/ImageLoader.h"
#include "image/ImageWriter.h"
#include "app/Logger.h"
#include "math/mathutils.h"
#include "geo/ProcessStatus.h"
#include "triangulate.h"
#include <iostream>
#include <boost/program_options.hpp>
#include <sstream>
#include <omp.h>

namespace po = boost::program_options;

int main(int argc, char *argv[])
{
   po::options_description desc("Program-Options");
   desc.add_options()
      ("layer", po::value<std::string>(), "name of layer to add the data")
      ("triangulate", "triangulate dataset")
      ("maxpoints", po::value<int>(), "[optional] max number of points per tile. Default is 512.")
      ("grid", "create grid [currently unsupported, do not use!]")
      ("numthreads", po::value<int>(), "force number of threads")
      ("verbose", "verbose output")
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

   std::string sLayer;
   bool bTriangulate = false;
   bool bGrid = false;
   bool bVerbose = false;
   int nMaxpoints = 512; // default: max 512 points per tile (including corners and edges)

   //---------------------------------------------------------------------------
   // init options:

   boost::shared_ptr<ProcessingSettings> qSettings =  ProcessingUtils::LoadAppSettings();

   if (!qSettings)
   {
      std::cout << "Error in configuration! Check setup.xml\n";
      return ERROR_CONFIG;
   }

   //---------------------------------------------------------------------------

   boost::shared_ptr<Logger> qLogger =  ProcessingUtils::CreateLogger("triangulate", qSettings);


   if (!qLogger)
   {
      std::cout << "Error in configuration! Check setup.xml\n";
      return ERROR_CONFIG;
   }

   if (!vm.count("triangulate") && !vm.count("grid"))
   {
      bError = true; // atleast one algorithm
   }

   if (vm.count("triangulate") && vm.count("grid"))
   {
      bError = true; // only one algorithm!
   }

   if (vm.count("triangulate"))
   {
      bTriangulate = true;
   }

   if (vm.count("maxpoints"))
   {
      int v = vm["maxpoints"].as<int>();
      if (v>32 && v<2048) // please note: the ideal number is between 300 and 1000. The default value of 512 shouldn't be changed in most cases..
      {
         std::ostringstream oss; 
         oss << "changing maxpoints to " << v;
         qLogger->Info(oss.str());

         nMaxpoints = v;
      }
   }

   if (vm.count("grid"))
   {
      bGrid = true;
   }

   if (vm.count("verbose"))
   {
      bVerbose = true;
   }

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

   if (!vm.count("layer"))
   {
      bError = true;
   }
   else
   {
      sLayer = vm["layer"].as<std::string>();
   }


   if (bError)
   {
      qLogger->Error("Wrong parameters!");
      std::ostringstream sstr;

      sstr << desc;
      qLogger->Info("\n" + sstr.str());

      return ERROR_PARAMS;
   }

   clock_t t0,t1;
   t0 = clock();

   if (bTriangulate)
   {
      triangulate::process(qLogger, qSettings, nMaxpoints, sLayer, bVerbose);
   }
   else if (bGrid)
   {
      // not yet supported
   }

   t1=clock();

   std::ostringstream out;
   out << "calculated in: " << double(t1-t0)/double(CLOCKS_PER_SEC) << " s \n";
   qLogger->Info(out.str());


   return 0;
}

//------------------------------------------------------------------------------
