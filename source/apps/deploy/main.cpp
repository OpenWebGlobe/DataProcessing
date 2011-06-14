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
********************************************************************************
/*******************************************************************************/
// This is the deploy version without mpi (intended for regular workstations)
// OpenMP is required. All threads write archives.
//------------------------------------------------------------------------------

#include "og.h"
#include "ogprocess.h"
#include "io/TarWriter.h"
#include "deploy.h"
#include "app/ProcessingSettings.h"
#include <system/Utils.h>
#include <sstream>
#include <iostream>
#include <omp.h>
#include <boost/program_options.hpp>

//------------------------------------------------------------------------------

namespace po = boost::program_options;

int main(int argc, char *argv[])
{
   po::options_description desc("Program-Options");
   desc.add_options()
      ("layer", po::value<std::string>(), "name of layer to add the data")
      ("outpath", po::value<std::string>(), "where to write the data (path must exist!)")
      ("type", po::value<std::string>(), "[optional] image (default) or elevation.")
      ("archive", "[optional] create deployment in tar archive. (One archive per thread)")
      ("format", po::value<std::string>(), "[optional] elevation: json (default), image: png(default)|jpg")
      ("numthreads", po::value<int>(), "[optional] force number of threads")
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

   ELayerType layertype = IMAGE_LAYER;
   EOuputImageFormat imageformat = OUTFORMAT_PNG;
   EOutputElevationFormat elevationformat = OUTFORMAT_JSON;
   std::string sLayer;
   std::string sPath;
   bool bArchive = false;

   //---------------------------------------------------------------------------
   // init options:

   boost::shared_ptr<ProcessingSettings> qSettings =  ProcessingUtils::LoadAppSettings();

   if (!qSettings)
   {
      std::cout << "Error in configuration! Check setup.xml\n";
      return ERROR_CONFIG;
   }

   //---------------------------------------------------------------------------

   boost::shared_ptr<Logger> qLogger =  ProcessingUtils::CreateLogger("deploy", qSettings);

   if (!qLogger)
   {
      std::cout << "Error in configuration! Check setup.xml\n";
      return ERROR_CONFIG;
   }

   //---------------------------------------------------------------------------
   // parse options

   if (vm.count("type"))
   {
      std::string sType = vm["type"].as<std::string>();

      if (sType == "elevation")
      {
         layertype = ELEVATION_LAYER;
      }
      else if (sType == "image")
      {
         layertype = IMAGE_LAYER;
      }
      else
      {
         bError = true;
      }
   }

   //--------------------------------------------------------------------------
   if (vm.count("layer"))
   {
      sLayer = vm["layer"].as<std::string>();
   }
   else
   {
      bError = true;
   }

   //--------------------------------------------------------------------------
   if (vm.count("archive"))
   {
      bArchive = true;
   }

   //--------------------------------------------------------------------------
   if (vm.count("outpath"))
   {
      sPath = vm["outpath"].as<std::string>();
   }
   else
   {
      bError = true;
   }

   //--------------------------------------------------------------------------
   if (vm.count("format"))
   {
      std::string sFormat = vm["format"].as<std::string>();
      if (sFormat == "jpg")
      {
         imageformat = OUTFORMAT_JPG;
      }
      else if (sFormat == "png")
      {
         imageformat = OUTFORMAT_PNG;
      }
      else if (sFormat == "json")
      {
         elevationformat = OUTFORMAT_JSON;
      }
   }

   //--------------------------------------------------------------------------
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

   if (bError)
   {
      qLogger->Error("Wrong parameters!");
      std::ostringstream sstr;

      sstr << desc;
      qLogger->Info("\n" + sstr.str());

      return ERROR_PARAMS;
   }


   //---------------------------------------------------------------------------

   Deploy::ThreadInfo* pThreadInfo = Deploy::GenerateThreadInfo(); 


   if (layertype == IMAGE_LAYER)
   {
      Deploy::DeployImageLayer(qLogger, qSettings, sLayer, sPath, bArchive, imageformat, pThreadInfo);
   }
   else if (layertype == ELEVATION_LAYER)
   {
      Deploy::DeployElevationLayer(qLogger, qSettings, sLayer, sPath, bArchive, elevationformat, pThreadInfo);
   }

   Deploy::DestroyThreadInfo(pThreadInfo);

   return 0;
}

//------------------------------------------------------------------------------
