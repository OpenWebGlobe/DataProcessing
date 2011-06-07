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
#include "errors.h"
#include "imagedata.h"
#include "elevationdata.h"
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
#include <iostream>
#include <boost/program_options.hpp>
#include <sstream>
#include <omp.h>


enum ELayerType
{
   IMAGE_LAYER,
   ELEVATION_LAYER,
};


namespace po = boost::program_options;

int main(int argc, char *argv[])
{
   po::options_description desc("Program-Options");
   desc.add_options()
       ("image", po::value<std::string>(), "image file to add")
       ("elevation",  po::value<std::string>(), "elevation file to add")
       ("srs", po::value<std::string>(), "spatial reference system for input file")
       ("layer", po::value<std::string>(), "name of layer to add the data")
       ("fill", "fill empty parts, don't overwrite already existing data")
       ("overwrite", "overwrite existing data")
       ("numthreads", po::value<int>(), "force number of threads")
       ("verbose", "verbose output")
       ;

   // #todo: image -> vector<std::string> : allow multiple images per application

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

   std::string sFile;
   std::string sSRS;
   std::string sLayer;
   bool bFill = false;
   bool bOverwrite = false;
   bool bVerbose = false;
   ELayerType eLayer = IMAGE_LAYER;

   //---------------------------------------------------------------------------
   // init options:

   boost::shared_ptr<ProcessingSettings> qSettings =  ProcessingUtils::LoadAppSettings();

   if (!qSettings)
   {
      std::cout << "Error in configuration! Check setup.xml\n";
      return ERROR_CONFIG;
   }

   //---------------------------------------------------------------------------

   boost::shared_ptr<Logger> qLogger =  ProcessingUtils::CreateLogger("adddata", qSettings);

   if (!qLogger)
   {
      std::cout << "Error in configuration! Check setup.xml\n";
      return ERROR_CONFIG;
   }

   //---------------------------------------------------------------------------

   if (!vm.count("image") && !vm.count("elevation"))
   {
      bError = true;
   }

   if (vm.count("image") && vm.count("elevation"))
   {
      bError = true;
   }

   if (vm.count("image"))
   {
      eLayer = IMAGE_LAYER;
      sFile = vm["image"].as<std::string>();
   }
   else if  (vm.count("elevation"))
   {
      eLayer = ELEVATION_LAYER;
      sFile = vm["elevation"].as<std::string>();
   }

   if (!vm.count("srs") || !vm.count("layer"))
   {
      bError = true;
   }
   else
   {
      sSRS = vm["srs"].as<std::string>();
      sLayer = vm["layer"].as<std::string>();
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

   if (vm.count("fill"))
   {
      bFill = true;
   }

   if (!bFill && !bOverwrite)
   {
      bError = true; // needs atleast one option (fill or overwrite)
   }

   //---------------------------------------------------------------------------
   if (bError)
   {
      qLogger->Error("Wrong parameters!");
      std::ostringstream sstr;
   
      sstr << desc;
      qLogger->Info("\n" + sstr.str());

      return ERROR_PARAMS;
   }

   //---------------------------------------------------------------------------
   if (StringUtils::Left(sSRS, 5) != "EPSG:")
   {
      qLogger->Error("only srs starting with EPSG: are currently supported");
      return 1;
   }

   int epsg = atoi(sSRS.c_str()+5);

   //---------------------------------------------------------------------------

   if (eLayer == IMAGE_LAYER) 
   {
      return ImageData::process(qLogger, qSettings, sLayer, bVerbose, epsg, sFile, bFill);
   }
   else if (eLayer == ELEVATION_LAYER)
   {
      return ElevationData::process(qLogger, qSettings, sLayer, bVerbose, epsg, sFile, bFill);
   }
}



//------------------------------------------------------------------------------

