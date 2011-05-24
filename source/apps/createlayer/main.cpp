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
//------------------------------------------------------------------------------

#include "og.h"
#include "gdal.h"
#include "ogr_api.h"
#include "cpl_conv.h"
#include "app/ProcessingSettings.h"
#include "geo/MercatorQuadtree.h"
#include "geo/CoordinateTransformation.h"
#include "string/FilenameUtils.h"
#include "geo/ImageLayerSettings.h"
#include "io/FileSystem.h"
#include "app/Logger.h"
#include <iostream>
#include <boost/program_options.hpp>

//-----------------------------------------------------------------------------

bool init_gdal();
void exit_gdal();
int _start(int argc, char *argv[], boost::shared_ptr<Logger> qLogger, const std::string& processpath);
int _createlayer(const std::string& sLayerName,  const std::string& sLayerPath, int nLod, const std::vector<int64>& vecExtent, boost::shared_ptr<Logger> qLogger);

//-----------------------------------------------------------------------------
// ERROR CODES:

// App Specific:
#define ERROR_CONFIG             3;    // wrong configuration (setup.xml) (processing path or log-path is wrong)
#define ERROR_PARAMS             4;    // wrong parameters
#define ERROR_LAYEREXISTS        10;   // layer already exists (use --force to delete old one)
#define ERROR_LAYERDIR           15;   // can't create directory for data processing
#define ERROR_WRITE_PERMISSION   16;   // can't write into layer directory
#define ERROR_DELETE_PERMISSION  17;   // can't delete file/directory

// General Errors:
#define ERROR_OUTOFMEMORY        101;  // not enough memory

//------------------------------------------------------------------------------


int main(int argc, char *argv[])
{
   boost::shared_ptr<Logger> qLogger;
   
   // <ogwLoadSettings>
   boost::shared_ptr<ProcessingSettings> qSettings;
   qSettings = ProcessingSettings::Load();
   std::string sPath;
   std::string sLogPath;

   if (qSettings)
   {
      sPath = qSettings->GetPath();
      sLogPath = qSettings->GetLogPath();

      if (!FileSystem::DirExists(sLogPath))
      {
         std::cerr << "ERROR: logging path doesn't exist. Please edit setup.xml.\n"; 
         return ERROR_CONFIG;
      }

      qLogger = boost::shared_ptr<Logger>(new Logger(sLogPath, "createlayer"));
      qLogger->Info("Logging started");

      if (!FileSystem::DirExists(sPath))
      {
         qLogger->Error("Processing path doesn't exist. Please edit setup.xml");
         std::cerr << "ERROR: Processing path doesn't exist. Please edit setup.xml.\n"; 
         return ERROR_CONFIG;
      }
   }
   else
   {
      std::cerr << "ERROR: Can't find setup.xml file!\n";
      return 1;
   }

   // </ogwLoadSettings>

   return _start(argc, argv, qLogger, sPath);
   return 0;
}

//------------------------------------------------------------------------------

namespace po = boost::program_options;

int _start(int argc, char *argv[], boost::shared_ptr<Logger> qLogger, const std::string& processpath)
{

   po::options_description desc("Program-Options");
   desc.add_options()
       ("name", po::value<std::string>(), "layer name (string)")
       ("lod", po::value<int>(), "desired level of detail (integer)")
       ("extent", po::value< std::vector<int64> >()->multitoken(), "desired level of detail (tx0 ty0 tx1 ty1)")
       ("force", "force creation. (Warning: if this layer already exists it will be deleted)")
       ;

   po::variables_map vm;
   po::store(po::parse_command_line(argc, argv, desc), vm);
   po::notify(vm);

   std::string sLayerName;
   int nLod = 0;
   std::vector<int64> vecExtent;
   bool bForce = false;

   bool bError = false;
   if (!vm.count("name"))
   {
      qLogger->Error("layer name is not specified!");
      bError = true;
   }
   else
   {
      sLayerName = vm["name"].as<std::string>();
      if (sLayerName.length() == 0)
      {
         qLogger->Error("layer name is empty!");
         bError = true;
      }
   }

   if (!vm.count("lod"))
   {
      qLogger->Error("lod not specified!");
      bError = true;
   }
   else
   {
      nLod = vm["lod"].as<int>();
   }

   if (vm.count("force"))
   {
      bForce = true;
   }

   if (!vm.count("extent"))
   {
      qLogger->Error("extent is not specified!");
      bError = true;
   }
   else
   {
      vecExtent = vm["extent"].as< std::vector<int64> >();
      if (vecExtent.size() != 4)
      {
         qLogger->Error("extent must have 4 values!");
         bError = true;
      }
   }

   if (bError)
   {
      qLogger->out << desc << "\n";
      return ERROR_PARAMS;
   }

   qLogger->Info("Command arguments:");
   qLogger->out << "Layername = " << sLayerName << "\n";
   qLogger->out << "lod = " << nLod << "\n";
   qLogger->out << "extent = (" << vecExtent[0] << ", " << vecExtent[1] << ", " << vecExtent[2] << ", " << vecExtent[3] << ")" << "\n";

   std::string sLayerPath = FilenameUtils::DelimitPath(processpath) + sLayerName;
   qLogger->Info("Target directory: " + sLayerPath);
   
   if (FileSystem::DirExists(sLayerPath))
   {
      if (!bForce)
      {
         qLogger->Error("Layer already exists!!");
         qLogger->out << "the directory " << sLayerPath << " already exists. Please delete manually or choose another layer name or use the --force option\n";
         return ERROR_LAYEREXISTS;
      }
      else
      {
         qLogger->Info("Force option detected. Deleting already existing layer... this may take a while");
         if (!FileSystem::rm_all(sLayerPath))
         {
            qLogger->Error("Can't delete old layer (file permission).");
            return ERROR_DELETE_PERMISSION;
         }
         else
         {
            qLogger->Info("ok.. layer deleted.");
         }
      }
   }

   return _createlayer(sLayerName, sLayerPath, nLod, vecExtent, qLogger);
}

//------------------------------------------------------------------------------
bool init_gdal()
{
/*
   // find gdal-data directory
   bool has_data_dir = FileSystem::DirExists("gdal-data");

   GDALAllRegister();
   //CPLSetConfigOption("GDAL_CACHEMAX", "0");
   if (has_data_dir) 
   {
      CPLSetConfigOption("GDAL_DATA", "gdal-data");
   }
   OGRRegisterAll();

   return has_data_dir;
*/
   return true;
}
//------------------------------------------------------------------------------

void exit_gdal()
{
  /*
   GDALDestroyDriverManager();
   OGRCleanupAll();
  */
}
//------------------------------------------------------------------------------

int _createlayer(const std::string& sLayerName, const std::string& sLayerPath, int nLod, const std::vector<int64>& vecExtent, boost::shared_ptr<Logger> qLogger)
{
   int retcode = 0;
   if (!init_gdal())
   {
      qLogger->Warn("'gdal-data'-directory not found! This may result in wrong processing!");
   }
   
   if (!FileSystem::makedir(sLayerPath))
   {
      qLogger->Error("Can't create directory " + sLayerPath);
      exit_gdal();
      return ERROR_LAYERDIR;
   }

   boost::shared_ptr<ImageLayerSettings> qImageLayerSettings = boost::shared_ptr<ImageLayerSettings>(new ImageLayerSettings());
   if (!qImageLayerSettings) {exit_gdal(); return ERROR_OUTOFMEMORY;}

   qImageLayerSettings->SetLayerName(sLayerName);
   qImageLayerSettings->SetMaxLod(nLod);
   qImageLayerSettings->SetTileExtent(vecExtent[0], vecExtent[1], vecExtent[2], vecExtent[3]);

   if (!qImageLayerSettings->Save(sLayerPath))
   {
      qLogger->Error("Can't write into layer path: " + sLayerPath);
      exit_gdal();
      return ERROR_WRITE_PERMISSION;
   }

   // Create Quadtree (default constructor represents WebMercator: EPSG 3857)
   boost::shared_ptr<MercatorQuadtree> qQuadtree = boost::shared_ptr<MercatorQuadtree>(new MercatorQuadtree());
   if (!qQuadtree) {exit_gdal(); return ERROR_OUTOFMEMORY;}

   // now iterate through all tiles and create required subdirectories

   std::string targetdir = FilenameUtils::DelimitPath(sLayerPath);
   std::string tiledir = FilenameUtils::DelimitPath(targetdir + "tiles");
   std::string quadcode;
   std::string newfile;
   std::string newdir;

   qLogger->Info("Creating all required subdirectories...");

   for (int64 y=vecExtent[1];y<=vecExtent[3];y++)
   {
      for (int64 x=vecExtent[0];x<=vecExtent[2];x++)
      {
         quadcode = qQuadtree->TileCoordToQuadkey(x,y,nLod);
         newfile =  FilenameUtils::MakeHierarchicalFileName(tiledir + quadcode + ".tmp", 2);
         newdir = FilenameUtils::GetFileRoot(newfile);
         FileSystem::makeallsubdirs(newdir);
      }
   }

   qLogger->Info("All required subdirectories created...");

   
   exit_gdal();
   return retcode;
}

//------------------------------------------------------------------------------

