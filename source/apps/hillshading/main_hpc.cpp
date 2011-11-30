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
********************************************************************************
*     Licensed under MIT License. Read the file LICENSE for more information   *
*******************************************************************************/

#include "ogprocess.h"
#include "errors.h"
#include "app/ProcessingSettings.h"
#include "geo/MercatorQuadtree.h"
#include "geo/CoordinateTransformation.h"
#include "geo/ElevationLayerSettings.h"
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
#include <app/QueueManager.h>
#include  "hillshading.h"

namespace po = boost::program_options;

//-------------------------------------------------------------------
// Job-Struct
struct SJob
{
   int xx;
   int yy;
   int lod;
};

bool bError = false;
std::string sLayerPath;
int iLayerMaxZoom;
std::string sAlgorithm;
int iNumThreads = 8;
bool bVerbose = false;
int queueSize = 4096;
int inputX = 768;
int inputY = 768;
int outputX = 256;
int outputY = 256;
int iX = 0;
int iY = 0;
std::string sTempTileDir;
std::string sTileDir;
boost::shared_ptr<MercatorQuadtree> qQuadtree;
int64 layerTileX0, layerTileY0, layerTileX1, layerTileY1;

//------------------------------------------------------------------------------
// MPI Job callback function (called every thread/compute node)
void jobCallback(const SJob& job, int rank)
{
   std::string sCurrentQuadcode = qQuadtree->TileCoordToQuadkey(job.xx,job.yy,job.lod);

   //std::cout << sCurrentQuadcode << "\n";
   HSProcessChunk pData;
   pData.dfXMax = -1e20;
   pData.dfYMax = -1e20;
   pData.dfXMin = 1e20;
   pData.dfYMin = 1e20;
   pData.data.AllocateImage(inputX, inputY);
   for (int ty=-1;ty<=1;ty++)
   {
      for (int tx=-1;tx<=1;tx++)
      {
         std::string sQuadcode = qQuadtree->TileCoordToQuadkey(job.xx+tx,job.yy+ty,job.lod);
         std::string sTilefile = ProcessingUtils::GetTilePath(sTempTileDir, ".raw" , job.lod, job.xx+tx, job.yy+ty);
                  
         double sx0, sy1, sx1, sy0;
         qQuadtree->QuadKeyToMercatorCoord(sQuadcode, sx0, sy1, sx1, sy0);
               
         pData.dfXMax = math::Max<double>(pData.dfXMax, sx1);
         pData.dfYMax = math::Max<double>(pData.dfXMax, sy1);
         pData.dfXMin = math::Min<double>(pData.dfXMin, sx0);
         pData.dfYMin = math::Min<double>(pData.dfXMin, sy0);

         assert(sx0 < sx1);
         assert(sy0 < sy1);

         //std::cout << "   " << sTilefile << "\n";

         std::ifstream fin;
         fin.open(sTilefile.c_str(), std::ios::binary);
         int posX = (tx+1)*(inputX/3);
         int posY = (ty+1)*(inputY/3);
         int offX = 0;
         int offY = 0;
         if (fin.good())
         {
            while (!fin.eof())
            {
                     
               float value;
               fin.read((char*)&(value), sizeof(float));
               if (!fin.eof())
               {
                  pData.data.SetValue(posX+offX, posY+offY, value);
               }
               offX++;
               if(offX % 256 == 0)
               {
                  offX = 0;
                  offY++;
               }

            }
         }
         fin.close();
      }
   }
   // Generate tile
   process_hillshading(sTileDir, pData, job.xx, job.yy, job.lod, 2, outputX, outputY);
}
//---

//------------------------------------------------------------------------------------
void GenerateRenderJobs(int count, int lod, std::vector<SJob> &vJobs)
{
   int idx = 0;
   if(bVerbose)
     std::cout << " Generating " << count << " jobs starting from (z, x, y) " << "(" << lod << ", " << iX << ", " << iY << ")\n";
   for (int64 xx = iX; xx < layerTileX1; ++xx)
   {
      for (int64 yy = iY; yy < layerTileY1; ++yy)
      {
         SJob work;
         work.xx = xx; 
         work.yy = yy;
         work.lod = lod;
         vJobs.push_back(work);
         iY = (yy < layerTileY1-1)? yy+1 : -1;
         iX = (yy < layerTileY1-1) ? xx: xx+1;
         if(idx >= (count-1))
         {
            return;
         }
         idx++;
      }
      iY = layerTileY0+1;
   }
}

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

int main(int argc, char *argv[])
{
   for(int i = 0; i < 100; i++)
   {
      QJob job;
      SJob sJob;
      sJob.lod = 0;
      sJob.xx = 0;
      sJob.yy = i;
      job.data = boost::shared_array<char>(new char[sizeof(SJob)]);
      memcpy(job.data.get(), &sJob, sizeof(SJob));
      job.size = sizeof(SJob);
      QueueManager::AddToJobQueue("myjobs.hbx", job);
   }
   int amount = 15;
   std::vector<SJob> vecConverted;
   std::vector<QJob> jobs;
   do
   {
      
      jobs = QueueManager::FetchJobList("myjobs.hbx", sizeof(SJob), amount);
      ConvertJobs(jobs, vecConverted);
   }while(jobs.size() >= amount);

      po::options_description desc("Program-Options");
      desc.add_options()
         ("layer_path", po::value<std::string>(), "path of layer to generate data")
         ("layer_zoom", po::value<int>(), "maximum zoom which has to be generated previously using ogAddData")
         ("numthreads", po::value<int>(), "[optional] force number of threads")
         ("mpi_queue_size", po::value<int>(), "[optional] mpi queue size (Default: 10000)")
         ("verbose", "[optional] verbose output")
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

      if(vm.count("layer_path"))
      {
        sLayerPath = vm["layer_path"].as<std::string>();
        if(!(sLayerPath.at(sLayerPath.length()-1) == '\\' || sLayerPath.at(sLayerPath.length()-1) == '/'))
           sLayerPath = sLayerPath + "/";
      }
      else
         bError = true;
      if(vm.count("layer_zoom"))
         iLayerMaxZoom = vm["layer_zoom"].as<int>();
      else
         bError = true;
      if(vm.count("num_threads"))
         iNumThreads = vm["num_threads"].as<int>();
      if(vm.count("mpi_queue_size"))
         queueSize = vm["mpi_queue_size"].as<int>();
      if(vm.count("verbose"))
         bVerbose = true;

      //---------------------------------------------------------------------------
      // init options:

      boost::shared_ptr<ProcessingSettings> qSettings =  ProcessingUtils::LoadAppSettings();

      if (!qSettings)
      {
         std::cout << "Error in configuration! Check setup.xml\n";
         return ERROR_CONFIG;
      }
      if(bError)
      {
         std::cout << "Wrong parameters!\n";
         std::cout << desc << "\n";
      }

   //---------------------------------------------------------------------------
   // -- Beginn process
   sTempTileDir = sLayerPath + "temp/tiles/";
   sTileDir = sLayerPath + "tiles/";

   boost::shared_ptr<ImageLayerSettings> qImageLayerSettings = ImageLayerSettings::Load(sLayerPath);
   if (!qImageLayerSettings)
   {
      std::cout << "Failed retrieving image layer settings! Make sure to create it using 'createlayer'.\n";
      return ERROR_IMAGELAYERSETTINGS;
   }
   int lod = qImageLayerSettings->GetMaxLod();
   
   qImageLayerSettings->GetTileExtent(layerTileX0, layerTileY0, layerTileX1, layerTileY1);
   if (bVerbose)
   {
      std::cout << "\nRaw Image Layer:\n";
      std::cout << "   name = " << qImageLayerSettings->GetLayerName() << "\n";
      std::cout << "   maxlod = " << lod << "\n";
      std::cout << "   extent = " << layerTileX0 << ", " << layerTileY0 << ", " << layerTileX1 << ", " << layerTileY1 << "\n";
   }
   int64 width = layerTileX1-layerTileX0+1;
   int64 height = layerTileY1-layerTileY0+1;
   if (width<3 || height<3)
   {
      std::cout << "Extent is too small for hillshading processing\n";
      return ERROR_ELVLAYERSETTINGS;
   }

   // Retrieve dataset extent in mercator coord:
   qQuadtree = boost::shared_ptr<MercatorQuadtree>(new MercatorQuadtree());
   double xmin, ymin, xmax, ymax;
   std::string qc0 = qQuadtree->TileCoordToQuadkey(layerTileX0,layerTileY0,lod);
   std::string qc1 = qQuadtree->TileCoordToQuadkey(layerTileX1,layerTileY1,lod);

   double x00, y00, x10, y10;
   double x01, y01, x11, y11;
   qQuadtree->QuadKeyToMercatorCoord(qc0, x00,y00,x10,y10);
   qQuadtree->QuadKeyToMercatorCoord(qc1, x01,y01,x11,y11);

   xmin = x00;
   ymin = y11;
   xmax = x11;
   ymax = y00;
   if (bVerbose)
   {
      std::cout << "\nExtent mercator:";
      std::cout << "   extent = " << xmin << ", " << ymin << ", " << xmax << ", " << ymax << "\n";
   }

   if (!ProcessingUtils::init_gdal())
   {
      std::cout << "Warning: gdal-data directory not found. Ouput may be wrong!\n";
      return 1;
   }   

   


   bool bDone = false;
   //---------------------------------------------------------------------------
   // -- performance measurement
   int tileCount = 0;
   int currentJobQueueSize = 0;
   clock_t t_0, t_1;
   t_0 = clock();
   /*while (!bDone)
   {
      if (rank == 0)
      {
         std::vector<SJob> vJobs;
         if(iX <= 0) { iX = layerTileX0+1; }
         if(iY <= 0) { iY = layerTileY0+1; } 
         GenerateRenderJobs(totalnodes*queueSize, lod, vJobs);
         currentJobQueueSize = vJobs.size();
         tileCount += currentJobQueueSize;
         if (vJobs.size() == 0) // no more jobs
         {
            bDone = true;
            t_1 = clock();
            double time=(double(t_1-t_0)/double(CLOCKS_PER_SEC));
            double tps = tileCount/time;
            std::cout << ">>> Finished processing " << tileCount << " tiles at " << tps << " tiles per second! TOTAL TIME: " << time << "<<<\n" << std::flush;
         }
         else
         {
            jobmgr.AddJobs(vJobs);
         }
      }
 
      if (!bDone)
      {  
         clock_t t0,t1;
         t0 = clock();
         jobmgr.Process(jobCallback, bVerbose);
         t1 = clock();
         double tilesPerSecond = currentJobQueueSize/(double(t1-t0)/double(CLOCKS_PER_SEC));
      }
      else
      {
         MPI_Finalize();
      } 
     
   }*/
   return 0;
}

