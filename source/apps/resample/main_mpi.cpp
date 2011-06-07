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
/*                                                                            */
/*                  MPI Version of resample (for cluster/cloud)               */
/*                      asynchronous workload distribution                    */
/*          this is the recommended version for all clusters / clouds         */
/*                for multicore single computers use resample.cpp             */
/*                                                                            */
/******************************************************************************/

#include "resample.h"
#include "mpi/Utils.h"

#include <mpi.h>
#include <omp.h>
#include <iostream>
#include <boost/program_options.hpp>
#include <ctime>
#include <cassert>
#include <stack>
#include <iostream>


//-------------------------------------------------------------------
// Job-Struct
struct Job
{
   int64 sx, sy;
};

//------------------------------------------------------------------------------
// globals:
int g_Lod = 0;
boost::shared_ptr<MercatorQuadtree> q_qQuadtree;
TileBlock* g_pTileBlockArray = 0;
std::string g_sTileDir;

//------------------------------------------------------------------------------
// MPI Job callback function (called every thread/compute node)
void jobCallback(const Job& job, int rank)
{
   _resampleFromParent(g_pTileBlockArray, q_qQuadtree, job.sx, job.sy, g_Lod, g_sTileDir);
}

//------------------------------------------------------------------------------

namespace po = boost::program_options;

//------------------------------------------------------------------------------
void BroadcastString(std::string& sStr, int sender)
{
   unsigned int len = sStr.length();
   MPI_Bcast(&len, 1, MPI_UNSIGNED, sender, MPI_COMM_WORLD);
   if (sStr.length() < len ) 
   {
      sStr.insert(sStr.end(),(size_t)(len-sStr.length()), ' ');
   }
   else if (sStr.length() > len ) 
   {
      sStr.erase( len,sStr.length()-len);
   }

   MPI_Bcast(const_cast<char *>(sStr.data()), len, MPI_CHAR, sender, MPI_COMM_WORLD);
}
//------------------------------------------------------------------------------
void BroadcastInt(int& val, int sender)
{
   MPI_Bcast(&val, 1, MPI_INT, sender, MPI_COMM_WORLD);
}
//------------------------------------------------------------------------------
void BroadcastInt64(int64& val, int sender)
{
   MPI_Bcast(&val, 1, MPI_LONG_LONG, sender, MPI_COMM_WORLD);
}
//------------------------------------------------------------------------------
void BroadcastBool(bool& val, int sender)
{
   MPI_Datatype bool_type;
   if (sizeof(bool) == 1) bool_type= MPI_BYTE;
   else if (sizeof(bool) == 2) bool_type = MPI_SHORT;
   else if (sizeof(bool) == 4) bool_type = MPI_INT;
   else { assert(false); return;}
   MPI_Bcast(&val, 1, bool_type, sender, MPI_COMM_WORLD);
}

//------------------------------------------------------------------------------

int main(int argc, char *argv[])
{
   std::string sImageLayerDir;
   int64 tx0,ty0,tx1,ty1;
   int maxlod;
   clock_t t0,t1;
   bool bVerbose = false;

   //---------------------------------------------------------------------------
   // MPI Init
   //---------------------------------------------------------------------------

   int rank, totalnodes;

   MPI_Init(&argc, &argv); 
   MPI_Comm_size(MPI_COMM_WORLD, &totalnodes);
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);

   MPIJobManager<Job> jobmgr(4096);

   // arguments are read in rank 0. The arguments 
   // are parsed and interpreted and the 
   // result (tiledir etc.) is broadcasted.
   if (rank == 0)
   {
      t0 = clock();
      po::options_description desc("Program-Options");
      desc.add_options()
         ("layer", po::value<std::string>(), "image layer to resample")
         ("numthreads", po::value<int>(), "force number of threads (for each compute node)")
         ("verbose", "optional info")
         ;

      po::variables_map vm;

      //---------------------------------------------------------------------------
      // init options:

      boost::shared_ptr<ProcessingSettings> qSettings =  ProcessingUtils::LoadAppSettings();

      if (!qSettings)
      {
         std::cout << "Error in configuration! Check setup.xml\n";
         return MPI_Abort(MPI_COMM_WORLD, ERROR_PARAMS);
      }

      // --------------------------------------------------------------------------
      std::string sLayer;
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

      if (!vm.count("layer"))
      {
         bError = true;
      }
      else
      {
         sLayer = vm["layer"].as<std::string>();
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
            std::cout << "Forcing number of threads to " << n << " per node\n";
            omp_set_num_threads(n);
         }
      }

      //---------------------------------------------------------------------------
      if (bError)
      {
         std::cout << desc;
         return MPI_Abort(MPI_COMM_WORLD, ERROR_PARAMS);
      }

      //---------------------------------------------------------------------------
      sImageLayerDir = FilenameUtils::DelimitPath(qSettings->GetPath()) + sLayer;
      g_sTileDir = FilenameUtils::DelimitPath(FilenameUtils::DelimitPath(sImageLayerDir) + "tiles");

      boost::shared_ptr<ImageLayerSettings> qImageLayerSettings = ImageLayerSettings::Load(sImageLayerDir);
      if (!qImageLayerSettings)
      {
         std::cout << "**ERROR: Failed retrieving image layer settings!\n";
         return MPI_Abort(MPI_COMM_WORLD, ERROR_IMAGELAYERSETTINGS);
      }

      qImageLayerSettings->GetTileExtent(tx0,ty0,tx1,ty1);
      maxlod = qImageLayerSettings->GetMaxLod();


      if (bVerbose)
      {
         std::cout << "\nResample Setup:\n";
         std::cout << "         name = " << qImageLayerSettings->GetLayerName() << "\n";
         std::cout << "       maxlod = " << maxlod << "\n";
         std::cout << "       extent = " << tx0 << ", " << ty0 << ", " << tx1 << ", " << ty1 << "\n";
         std::cout << "compute nodes = " << totalnodes << "\n\n" << std::flush;
      }

   }
   
   BroadcastString(g_sTileDir, 0);
   BroadcastString(sImageLayerDir, 0);
   BroadcastInt64(tx0, 0);
   BroadcastInt64(ty0, 0);
   BroadcastInt64(tx1, 0);
   BroadcastInt64(ty1, 0);
   BroadcastInt(maxlod, 0);
   BroadcastBool(bVerbose, 0);

   g_pTileBlockArray = _createTileBlockArray();

   q_qQuadtree= boost::shared_ptr<MercatorQuadtree>(new MercatorQuadtree());
   std::string qc0 = q_qQuadtree->TileCoordToQuadkey(tx0, ty0, maxlod);
   std::string qc1 = q_qQuadtree->TileCoordToQuadkey(tx1, ty1, maxlod);

   for (int nLevelOfDetail = maxlod - 1; nLevelOfDetail>0; nLevelOfDetail--)
   {
      g_Lod = nLevelOfDetail;

      if (bVerbose && rank == 0)
      {
         std::cout << "[LOD] starting processing lod " << nLevelOfDetail << "\n" << std::flush;
      }

      qc0 = StringUtils::Left(qc0, nLevelOfDetail);
      qc1 = StringUtils::Left(qc1, nLevelOfDetail);

      int tmp_lod;
      q_qQuadtree->QuadKeyToTileCoord(qc0, tx0, ty0, tmp_lod);
      q_qQuadtree->QuadKeyToTileCoord(qc1, tx1, ty1, tmp_lod);

      if (bVerbose && rank == 0)
      {
        std::cout << "[RANGE]: [" << tx0 << ", " << ty0 << "]-[" << tx1 << ", " << ty1 << "]\n" << std::flush;
      }

      // Create Jobs
      if (rank == 0)
      {
         // create workstack: contains all work which will be distributed.
         Job work;
         
         for (int64 y=ty0;y<=ty1;y++)
         {
            for (int64 x=tx0;x<=tx1;x++)
            {
               work.sx = x; work.sy = y;
               jobmgr.AddJob(work);
            }
         }
      }

      jobmgr.Process(jobCallback, bVerbose);
   }

   MPI_Finalize();

   // clean up
   _destroyTileBlockArray(g_pTileBlockArray);

   std::cout << std::flush;

   // output calculation time
   if (rank == 0)
   {
      t1=clock();
      std::cout << "calculated in: " << double(t1-t0)/double(CLOCKS_PER_SEC) << " s \n";
   }

   return 0;
}


