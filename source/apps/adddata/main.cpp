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
#include "greyimagedata.h"
#include "pointdata.h"
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
#include <iostream>
#include <boost/program_options.hpp>
#include <sstream>
#include <omp.h>


enum ELayerType
{
   IMAGE_LAYER,
   ELEVATION_LAYER,
   GREY16IMAGE_LAYER,
   POINT_LAYER,
};


namespace po = boost::program_options;

int main(int argc, char *argv[])
{
   po::options_description desc("Program-Options");
   desc.add_options()
       ("image", po::value<std::string>(), "image file to add")
       ("elevation",  po::value<std::string>(), "elevation file to add")
       ("grey16image",  po::value<std::string>(), "grey16 image file to add")
	   ("point", po::value<std::string>(), "point file to add")
       ("srs", po::value<std::string>(), "spatial reference system for input file")
       ("layer", po::value<std::string>(), "name of layer to add the data")
       ("fill", "fill empty parts, don't overwrite already existing data")
       ("overwrite", "overwrite existing data")
       ("numthreads", po::value<int>(), "force number of threads")
       ("verbose", "verbose output")
       ("nolock", "disable file locking (also forcing 1 thread)")
       ("virtual", "enable temporary disk storage (instead of RAM) for large datasets")
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

   std::string sFile;
   std::string sSRS;
   std::string sLayer;
   bool bFill = false;
   bool bOverwrite = false;
   bool bVerbose = false;
   bool bLock = true;
   bool bVirtual = false;
   ELayerType eLayer = IMAGE_LAYER;
   bool bUseProcessStatus = true;

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

   if (!vm.count("image") && !vm.count("elevation") && !vm.count("grey16image") && !vm.count("point"))
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
   else if  (vm.count("grey16image"))
   {
      eLayer = GREY16IMAGE_LAYER;
      sFile = vm["grey16image"].as<std::string>();
   }
   else if  (vm.count("point"))
   {
      eLayer = POINT_LAYER;
      sFile = vm["point"].as<std::string>();
      bUseProcessStatus = false;
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

   if (vm.count("nolock"))
   {
      bLock = false;
      omp_set_num_threads(1);
   }

   if (vm.count("virtual"))
   {
      bVirtual = true;
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
   // CREATE / UPDATE PROCESS STATUS
   //---------------------------------------------------------------------------
   
   std::string sProcessStatusFile;
   boost::shared_ptr<ProcessStatus> qProcessStatus;
   int lockid;
   ProcessElement* pElement;

   if (bUseProcessStatus)
   {

      sProcessStatusFile = qSettings->GetPath() + "/" + sLayer + "/ProcessStatus.xml";
    
      // update process status (exclusive lock)
      lockid = bLock ? FileSystem::Lock(sProcessStatusFile) : -1;

      if (FileSystem::FileExists(sProcessStatusFile))
      {
         qProcessStatus = ProcessStatus::Load(sProcessStatusFile);
         // file already exists -> load it!
         if (!qProcessStatus)
         {
            qLogger->Error("Failed opening process status file\n");
            FileSystem::Unlock(sProcessStatusFile, lockid);
            return ERROR_FILE;
         }
      }
      else  
      {
         // create new Process Status file!
         qProcessStatus = boost::shared_ptr<ProcessStatus>(new ProcessStatus);
         qProcessStatus->SetLayerName(sLayer);
      }

      pElement = qProcessStatus->GetElement(sFile);

      if (pElement)
      {
         if (pElement->IsFinished())
         {
            // this file was already processed! Do not process again!
            qLogger->Warn("This file has already been added to the dataset. Ignoring it.\n");
            FileSystem::Unlock(sProcessStatusFile, lockid);
            return 0;
         }
         if (pElement->IsProcessing())
         {
            qLogger->Error("This file is currently being processed by another instance. Ignoring it.\n");
            FileSystem::Unlock(sProcessStatusFile, lockid);
            return 0;
         }

         // Element exists, but creation failed or didn't complete
         // Set Start Time again.
         pElement->SetStatusMessage("reprocessing");
         pElement->SetStartTime(); // update start time
      }
      else
      {
         ProcessElement newElement;
         newElement.SetFilename(sFile);
         newElement.SetStartTime();
         newElement.SetStatusMessage("processing");
         newElement.Processing();
         qProcessStatus->AddElement(newElement);
      }

      qProcessStatus->Save(sProcessStatusFile);

      FileSystem::Unlock(sProcessStatusFile, lockid);

   }


   //---------------------------------------------------------------------------
   int retval = 0;
   int lod = 0;
   int64 x0 = 0, y0 = 0, x1 = 0, y1 = 0;
   int64 z0 = 0, z1 = 0;

   if (eLayer == IMAGE_LAYER) 
   {
      retval = ImageData::process(qLogger, qSettings, sLayer, bVerbose, bLock, epsg, sFile, bFill, lod, x0, y0, x1, y1);
   }
   else if (eLayer == GREY16IMAGE_LAYER)
   {
      retval = GreyImageData::process(qLogger, qSettings, sLayer, bVerbose, bLock, epsg, sFile, bFill, lod, x0, y0, x1, y1);
   }
   else if (eLayer == ELEVATION_LAYER)
   {
      retval = ElevationData::process(qLogger, qSettings, sLayer, bVerbose, bLock, bVirtual, epsg, sFile, bFill, lod, x0, y0, x1, y1);
   }
   else if (eLayer == POINT_LAYER)
   {
      retval = PointData::process(qLogger, qSettings, sLayer, bVerbose, bLock, epsg, sFile, bFill, lod, x0, y0, z0, x1, y1, z1);
   }

   //---------------------------------------------------------------------------
   // UPDATE PROCESS STATUS
   //---------------------------------------------------------------------------

   if (bUseProcessStatus)
   {
      // Update Process XML (exclusive lock)
      lockid = bLock ? FileSystem::Lock(sProcessStatusFile) : -1;

      qProcessStatus = ProcessStatus::Load(sProcessStatusFile);
      if (!qProcessStatus)
      {
         qLogger->Error("Failed opening process status file (for updating).\n");
         FileSystem::Unlock(sProcessStatusFile, lockid);
         return ERROR_FILE;
      }

      pElement = qProcessStatus->GetElement(sFile);
      if (!pElement)
      {
         qLogger->Error("Can't find element in process status file.\n");
         FileSystem::Unlock(sProcessStatusFile, lockid);
         return ERROR_FILE;
      }

      pElement->SetFinishTime();

      if (retval == 0)
      {
         pElement->SetStatusMessage("success");
         pElement->SetLod(lod);
         if (eLayer == POINT_LAYER)
         {
            pElement->SetExtent(x0,y0,z0,x1,y1,z1);
         }
         else
         {
            pElement->SetExtent(x0,y0,x1,y1);
         }
         pElement->MarkFinished();
         pElement->FinishedProcessing();
      }
      else
      {
         pElement->SetStatusMessage("failed");
         pElement->MarkFailed();
      }

      qProcessStatus->Save(sProcessStatusFile);

      FileSystem::Unlock(sProcessStatusFile, lockid);
   }

   return retval;
}



//------------------------------------------------------------------------------

