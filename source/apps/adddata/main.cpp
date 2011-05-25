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
#include "string/StringUtils.h"
#include "geo/ImageLayerSettings.h"
#include "io/FileSystem.h"
#include "app/Logger.h"
#include <iostream>
#include <boost/program_options.hpp>
#include <sstream>
#include <omp.h>

//-----------------------------------------------------------------------------
// ERROR CODES:

// App Specific:
#define ERROR_GDAL               2;    // gdal-data directory not found
#define ERROR_CONFIG             3;    // wrong configuration (setup.xml) (processing path or log-path is wrong)
#define ERROR_PARAMS             4;    // wrong parameters
#define ERROR_IMAGELAYERSETTINGS 5;    // can't load imagelayersettings. (image layer probably doesn't exist)

// General Errors:
#define ERROR_OUTOFMEMORY        101;  // not enough memory

//------------------------------------------------------------------------------
namespace po = boost::program_options;


int main(int argc, char *argv[])
{
   po::options_description desc("Program-Options");
   desc.add_options()
       ("image", po::value<std::string>(), "image to add")
       ("srs", po::value<std::string>(), "spatial reference system for input files")
       ("layer", po::value<std::string>(), "name of layer to add the data")
       ("fill", "fill empty parts, don't overwrite already existing data")
       ("overwrite", "overwrite existing data")
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

   std::string sImagefile;
   std::string sSRS;
   std::string sLayer;
   bool bFill = false;
   bool bOverwrite = false;
   bool bVerbose = false;

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

   if (!vm.count("image") || !vm.count("srs") || !vm.count("layer"))
   {
      bError = true;
   }

   sImagefile = vm["image"].as<std::string>();
   sSRS = vm["srs"].as<std::string>();
   sLayer = vm["layer"].as<std::string>();

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

   DataSetInfo oInfo;

   if (!ProcessingUtils::init_gdal())
   {
      qLogger->Error("gdal-data directory not found!");
      return ERROR_GDAL;
   } 

   //---------------------------------------------------------------------------
   // Retrieve ImageLayerSettings:
   std::ostringstream oss;

   std::string sImageLayerDir = FilenameUtils::DelimitPath(qSettings->GetPath()) + sLayer;

   boost::shared_ptr<ImageLayerSettings> qImageLayerSettings = ImageLayerSettings::Load(sImageLayerDir);
   if (!qImageLayerSettings)
   {
      qLogger->Error("Failed retrieving image layer settings! Make sure to create it using 'createlayer'.");
      ProcessingUtils::exit_gdal();
      return ERROR_IMAGELAYERSETTINGS;
   }

   int lod = qImageLayerSettings->GetMaxLod();
   int64 tx0, ty0, tx1, ty1;
   qImageLayerSettings->GetTileExtent(tx0, ty0, tx1, ty1);

   oss << "Image Layer:\n";
   oss << "     name = " << qImageLayerSettings->GetLayerName() << "\n";
   oss << "   maxlod = " << lod << "\n";
   oss << "   extent = " << tx0 << ", " << ty0 << ", " << tx1 << ", " << ty1 << "\n";
 


   //---------------------------------------------------------------------------

   boost::shared_ptr<CoordinateTransformation> qCT;
   qCT = boost::shared_ptr<CoordinateTransformation>(new CoordinateTransformation(epsg, 3785));

   clock_t t0,t1;
   t0 = clock();

   ProcessingUtils::RetrieveDatasetInfo(sImagefile, qCT.get(), &oInfo, bVerbose);

   if (!oInfo.bGood)
   {
      qLogger->Error("Failed retrieving info!");
   }

   
   oss << "Loaded image info:\nImage Size: w= " << oInfo.nSizeX << ", h= " << oInfo.nSizeY << "\n";
   oss << "dest: " << oInfo.dest_lrx << ", " << oInfo.dest_lry << ", " << oInfo.dest_ulx << ", " << oInfo.dest_uly << "\n";
   qLogger->Info(oss.str());

   boost::shared_ptr<MercatorQuadtree> qQuadtree = boost::shared_ptr<MercatorQuadtree>(new MercatorQuadtree());
  
   int64 px0, py0, px1, py1;
   qQuadtree->MercatorToPixel(oInfo.dest_ulx, oInfo.dest_uly, lod, px0, py0);
   qQuadtree->MercatorToPixel(oInfo.dest_lrx, oInfo.dest_lry, lod, px1, py1);




   //---------------------------------------------------------------------------


   //---------------------------------------------------------------------------
   t1=clock();

   std::ostringstream out;
   out << "calculated in: " << double(t1-t0)/double(CLOCKS_PER_SEC) << " s \n";
   qLogger->Info(out.str());

   ProcessingUtils::exit_gdal();

   return 0;
}



//------------------------------------------------------------------------------

