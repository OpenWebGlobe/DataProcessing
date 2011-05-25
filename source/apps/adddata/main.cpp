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


#include "ogprocess.h"
#include "app/ProcessingSettings.h"
#include "geo/MercatorQuadtree.h"
#include "geo/CoordinateTransformation.h"
#include "string/FilenameUtils.h"
#include "geo/ImageLayerSettings.h"
#include "io/FileSystem.h"
#include "app/Logger.h"
#include <iostream>
#include <boost/program_options.hpp>
#include <omp.h>

//-----------------------------------------------------------------------------
// ERROR CODES:

// App Specific:
#define ERROR_GDAL               2;    // gdal-data directory not found
#define ERROR_CONFIG             3;    // wrong configuration (setup.xml) (processing path or log-path is wrong)
#define ERROR_PARAMS             4;    // wrong parameters

// General Errors:
#define ERROR_OUTOFMEMORY        101;  // not enough memory

//------------------------------------------------------------------------------
namespace po = boost::program_options;


int main(int argc, char *argv[])
{
   //omp_set_num_threads(numthreads);
   po::options_description desc("Program-Options");
   desc.add_options()
       ("image", po::value<int>(), "image to add")
       ("srs", po::value<std::string>(), "spatial reference system for input files")
       ("layer", po::value<std::string>(), "name of layer to add the data")
       ("fill", "fill empty parts, don't overwrite already existing data")
       ("overwrite", "overwrite existing data")
       ("verbose", "verbose output")
       ;

   po::variables_map vm;

   try
   {
      po::store(po::parse_command_line(argc, argv, desc), vm);
      po::notify(vm);
   }
   catch (std::exception& e)
   {
      std::cout << desc;
      return ERROR_PARAMS;
   }


   bool bError = false;

   std::string sImagefile;
   std::string sSRS;
   std::string sLayer;
   bool bFill = false;
   bool bOverwrite = false;
   bool bVerbose = false;


   if (!vm.count("image") || !vm.count("srs") || vm.count("layer"))
   {
      bError = true;
   }

   if (vm.count("verbose"))
   {
      bVerbose = true;
   }

   if (vm.count("overwrite") && vm.count("fill"))
   {
      bError = true; // can't overwrite and fill at same time!
   }

   if (vm.count("overwrite"))
   {
      bOverwrite = true;
   }

   if (vm.count("fill"))
   {
      bFill = true;
   }

   if (!bFill && !bOverwrite)
   {
      bError = true; // needs atleast one option (fill or overwrite)
   }


   //---------------------------------------------------------------------------
   // init options:

   boost::shared_ptr<ProcessingSettings> qSettings =  ProcessingUtils::LoadAppSettings();

   if (!qSettings)
   {
      return ERROR_CONFIG;
   }

   //---------------------------------------------------------------------------

   boost::shared_ptr<Logger> qLogger =  ProcessingUtils::CreateLogger("createlayer", qSettings);

   if (!qLogger)
   {
      return ERROR_CONFIG;
   }

   //---------------------------------------------------------------------------

   DataSetInfo oInfo;

   if (!ProcessingUtils::init_gdal())
   {
      qLogger->Error("gdal-data directory not found!");
      return ERROR_GDAL;
   } 

   //---------------------------------------------------------------------------




   ProcessingUtils::exit_gdal();

   return 0;
}



//------------------------------------------------------------------------------

