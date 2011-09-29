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
// createlayer must be called before adding data to a layer, it does:
//   1) create info files for tile layout (JSON and XML)
//   2) create all directories for tile layout. 
//
// Note: The tile layout is compatible to the OpenStreetMap tile layout.
// [This application is designed to run on one compute node only.]
//------------------------------------------------------------------------------

#include "ogprocess.h"
#include "app/ProcessingSettings.h"
#include "geo/MercatorQuadtree.h"
#include "geo/CoordinateTransformation.h"
#include "string/FilenameUtils.h"
#include "string/StringUtils.h"
#include "geo/ImageLayerSettings.h"
#include "geo/ElevationLayerSettings.h"
#include "geo/PointLayerSettings.h"
#include "io/FileSystem.h"
#include "app/Logger.h"
#include <iostream>
#include <boost/program_options.hpp>
#include <sstream>
#include <omp.h>

//-----------------------------------------------------------------------------

int _start(int argc, char *argv[], boost::shared_ptr<Logger> qLogger, const std::string& processpath);
int _createimagelayer(const std::string& sLayerName,  const std::string& sLayerPath, int nLod, const std::vector<int64>& vecExtent, boost::shared_ptr<Logger> qLogger);
int _createelevationlayer(const std::string& sLayerName,  const std::string& sLayerPath, int nLod, const std::vector<int64>& vecExtent, boost::shared_ptr<Logger> qLogger);
int _createpointlayer(const std::string& sLayerName,  const std::string& sLayerPath, int nLod, const std::vector<int64>& vecExtent, boost::shared_ptr<Logger> qLogger);
int _createDirectoriesXY( const std::string& sLayerPath, boost::shared_ptr<Logger> qLogger, const std::vector<int64>& vecExtent, int nLod, bool bTemp); 
int _createDirectoriesXYZ( const std::string& sLayerPath, boost::shared_ptr<Logger> qLogger, const std::vector<int64>& vecExtent, int nLod, bool bTemp); 


//-----------------------------------------------------------------------------
// ERROR CODES:

// App Specific:
#define ERROR_CONFIG             3     // wrong configuration (setup.xml) (processing path or log-path is wrong)
#define ERROR_PARAMS             4     // wrong parameters
#define ERROR_LAYEREXISTS        10    // layer already exists (use --force to delete old one)
#define ERROR_LAYERDIR           15    // can't create directory for data processing
#define ERROR_WRITE_PERMISSION   16    // can't write into layer directory
#define ERROR_DELETE_PERMISSION  17    // can't delete file/directory
#define ERROR_UNSUPPORTED        80    // unsupported feature

// General Errors:
#define ERROR_OUTOFMEMORY        101   // not enough memory

//------------------------------------------------------------------------------


int main(int argc, char *argv[])
{
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


   return _start(argc, argv, qLogger, qSettings->GetPath());
   return 0;
}

//------------------------------------------------------------------------------

enum ELayerType
{
   IMAGE_LAYER,
   ELEVATION_LAYER,
   POI_LAYER,
   POINT_LAYER,
   GEOMETRY_LAYER,
};
//------------------------------------------------------------------------------

namespace po = boost::program_options;

int _start(int argc, char *argv[], boost::shared_ptr<Logger> qLogger, const std::string& processpath)
{
   bool bError = false;

   po::options_description desc("Program-Options");
   desc.add_options()
       ("name", po::value<std::string>(), "layer name (string)")
       ("lod", po::value<int>(), "desired level of detail (integer)")
       ("extent", po::value< std::vector<int64> >()->multitoken(), "desired level of detail (tx0 ty0 tx1 ty1)")
       ("force", "[optional] force creation. (Warning: if this layer already exists it will be deleted)")
       ("numthreads", po::value<int>(), "[optional] force number of threads")
       ("type",  po::value<std::string>(), "[optional] layer type. This can be image, elevation, poi, pointcloud, geometry. image is default value.")
       ;

   po::variables_map vm;

   try
   {
      po::store(po::parse_command_line(argc, argv, desc), vm);
      po::notify(vm);
   }
   catch (std::exception&)
   {
      bError = true;
   }

   std::string sLayerName;
   int nLod = 0;
   std::vector<int64> vecExtent;
   bool bForce = false;
   ELayerType eLayer = IMAGE_LAYER;

   
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

   if (vm.count("type"))
   {
      std::string sLayerType = vm["type"].as< std::string >();
      if (sLayerType == "image")
      {
         eLayer = IMAGE_LAYER;
      }
      else if (sLayerType == "elevation")
      {
         eLayer = ELEVATION_LAYER;
      }
      else if (sLayerType == "poi")
      {
         eLayer = POI_LAYER;
      }
      else if (sLayerType == "point")
      {
         eLayer = POINT_LAYER;
      }
      else if (sLayerType == "geometry")
      {
         eLayer = GEOMETRY_LAYER;
      }
      else
      {
         bError = true;
      }
   }
   else
   {
       qLogger->Warn("It is highly recommended to use --type! Using default --type image");
   }

   if (eLayer == POINT_LAYER)
   {
      if (vecExtent.size() != 6 )
      {
         qLogger->Error("extent must have 6 values: x0,y0,z0,x1,y1,z1");
         bError = true;
      }
   }
   else
   {
      if (vecExtent.size() != 4 )
      {
         qLogger->Error("extent must have 4 values: x0,y0,x1,y1");
         bError = true;
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

   std::string sLayerPath = FilenameUtils::DelimitPath(processpath) + sLayerName;
   qLogger->Info("Target directory: " + sLayerPath);
   
   if (FileSystem::DirExists(sLayerPath))
   {
      if (!bForce)
      {
         qLogger->Error("Layer already exists!!");
         qLogger->Error("the directory " + sLayerPath + " already exists. Please delete manually or choose another layer name or use the --force option");
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

   if (eLayer == IMAGE_LAYER)
   {
      return _createimagelayer(sLayerName, sLayerPath, nLod, vecExtent, qLogger);
   }
   else if (eLayer == ELEVATION_LAYER)
   {
      return _createelevationlayer(sLayerName, sLayerPath, nLod, vecExtent, qLogger);
   }
   else if (eLayer == POINT_LAYER)
   {
      return _createpointlayer(sLayerName, sLayerPath, nLod, vecExtent, qLogger);
   }
   else
   {
      return ERROR_UNSUPPORTED;
   }
   
}

//------------------------------------------------------------------------------

int _createimagelayer(const std::string& sLayerName, const std::string& sLayerPath, int nLod, const std::vector<int64>& vecExtent, boost::shared_ptr<Logger> qLogger)
{
   if (!FileSystem::makedir(sLayerPath))
   {
      qLogger->Error("Can't create directory " + sLayerPath);
      return ERROR_LAYERDIR;
   }

   boost::shared_ptr<ImageLayerSettings> qImageLayerSettings = boost::shared_ptr<ImageLayerSettings>(new ImageLayerSettings());
   if (!qImageLayerSettings) {return ERROR_OUTOFMEMORY;}

   qImageLayerSettings->SetLayerName(sLayerName);
   qImageLayerSettings->SetMaxLod(nLod);
   qImageLayerSettings->SetTileExtent(vecExtent[0], vecExtent[1], vecExtent[2], vecExtent[3]);

   if (!qImageLayerSettings->Save(sLayerPath))
   {
      qLogger->Error("Can't write into layer path: " + sLayerPath);
      return ERROR_WRITE_PERMISSION;
   }

   return _createDirectoriesXY(sLayerPath, qLogger, vecExtent, nLod, false);
}

//------------------------------------------------------------------------------

int _createelevationlayer(const std::string& sLayerName, const std::string& sLayerPath, int nLod, const std::vector<int64>& vecExtent, boost::shared_ptr<Logger> qLogger)
{
   if (!FileSystem::makedir(sLayerPath))
   {
      qLogger->Error("Can't create directory " + sLayerPath);
      return ERROR_LAYERDIR;
   }

   boost::shared_ptr<ElevationLayerSettings> qElevationLayerSettings = boost::shared_ptr<ElevationLayerSettings>(new ElevationLayerSettings());
   if (!qElevationLayerSettings) {return ERROR_OUTOFMEMORY;}

   qElevationLayerSettings->SetLayerName(sLayerName);
   qElevationLayerSettings->SetMaxLod(nLod);
   qElevationLayerSettings->SetTileExtent(vecExtent[0], vecExtent[1], vecExtent[2], vecExtent[3]);

   if (!qElevationLayerSettings->Save(sLayerPath))
   {
      qLogger->Error("Can't write into layer path: " + sLayerPath);
      return ERROR_WRITE_PERMISSION;
   }

   return _createDirectoriesXY(sLayerPath, qLogger, vecExtent, nLod, true);

}

//------------------------------------------------------------------------------

int _createpointlayer(const std::string& sLayerName,  const std::string& sLayerPath, int nLod, const std::vector<int64>& vecExtent, boost::shared_ptr<Logger> qLogger)
{
    if (!FileSystem::makedir(sLayerPath))
   {
      qLogger->Error("Can't create directory " + sLayerPath);
      return ERROR_LAYERDIR;
   }

   boost::shared_ptr<PointLayerSettings> qPointLayerSettings = boost::shared_ptr<PointLayerSettings>(new PointLayerSettings());
   if (!qPointLayerSettings) {return ERROR_OUTOFMEMORY;}

   qPointLayerSettings->SetLayerName(sLayerName);
   qPointLayerSettings->SetMaxLod(nLod);
   qPointLayerSettings->SetTileExtent(vecExtent[0], vecExtent[1], vecExtent[2], vecExtent[3], vecExtent[4], vecExtent[5]);

   if (!qPointLayerSettings->Save(sLayerPath))
   {
      qLogger->Error("Can't write into layer path: " + sLayerPath);
      return ERROR_WRITE_PERMISSION;
   }

   return _createDirectoriesXYZ(sLayerPath, qLogger, vecExtent, nLod, true);
}

//------------------------------------------------------------------------------

int _createDirectoriesXY( const std::string& sLayerPath, boost::shared_ptr<Logger> qLogger, const std::vector<int64>& vecExtent, int nLod, bool bTemp) 
{
   // Create Quadtree (default constructor represents WebMercator: EPSG 3857)
   boost::shared_ptr<MercatorQuadtree> qQuadtree = boost::shared_ptr<MercatorQuadtree>(new MercatorQuadtree());
   if (!qQuadtree) {return ERROR_OUTOFMEMORY;}

   // now iterate through all tiles and create required subdirectories
   std::string targetdir = FilenameUtils::DelimitPath(sLayerPath);
   std::string tiledir = FilenameUtils::DelimitPath(targetdir + "tiles");
   std::string temptiledir;
  
   if (bTemp)
   {
      temptiledir = FilenameUtils::DelimitPath(targetdir + "temp");
      FileSystem::makedir(temptiledir);
      temptiledir = FilenameUtils::DelimitPath(temptiledir + "tiles");
      FileSystem::makedir(temptiledir);
   }

   FileSystem::makedir(tiledir);
   std::string quadcode;
   std::string newfile;
   std::string newdir;

   qLogger->Info("Creating all required subdirectories...");

   //
   std::string qc0 = qQuadtree->TileCoordToQuadkey(vecExtent[0], vecExtent[1], nLod);
   std::string qc1 = qQuadtree->TileCoordToQuadkey(vecExtent[2], vecExtent[3], nLod);

   std::cout << qc0 << "\n";
   std::cout << qc1 << "\n";

   clock_t t0,t1;
   t0 = clock();

   for (int nLevelOfDetail = 1; nLevelOfDetail<=nLod; nLevelOfDetail+=1)
   {
      std::string qq0 = StringUtils::Left(qc0, nLevelOfDetail);
      std::string qq1 = StringUtils::Left(qc1, nLevelOfDetail);

      int64 tx0,ty0,tx1,ty1;
      int tmp_lod;
      qQuadtree->QuadKeyToTileCoord(qq0, tx0, ty0, tmp_lod);
      qQuadtree->QuadKeyToTileCoord(qq1, tx1, ty1, tmp_lod);

      std::ostringstream oss1;
      oss1 << tiledir << nLevelOfDetail;
      qLogger->Info("creating LOD directory: " + oss1.str());

      FileSystem::makedir(oss1.str());

      if (bTemp)
      { 
         std::ostringstream oss1_tmp;
         oss1_tmp << temptiledir << nLevelOfDetail;
         FileSystem::makedir(oss1_tmp.str());
      }

      // Creating directories in parallel speeds up the whole thing!

#     pragma omp parallel for
      for (int64 x=tx0;x<=tx1;x+=1)
      {
         std::ostringstream oss2;
         oss2 << tiledir << nLevelOfDetail << "/" << x;
         FileSystem::makedir(oss2.str());

         if (bTemp)
         {
            std::ostringstream oss2_tmp;
            oss2_tmp << temptiledir << nLevelOfDetail << "/" << x;
            FileSystem::makedir(oss2_tmp.str());
         }
      }
   }

   t1=clock();
   std::ostringstream out;
   out << "calculated in: " << double(t1-t0)/double(CLOCKS_PER_SEC) << " s \n";
   qLogger->Info(out.str());

   qLogger->Info("All required subdirectories created...");

   return 0;
}

//------------------------------------------------------------------------------

int _createDirectoriesXYZ( const std::string& sLayerPath, boost::shared_ptr<Logger> qLogger, const std::vector<int64>& vecExtent, int nLod, bool bTemp)
{
   // #todo
   return 0;
}

//------------------------------------------------------------------------------


