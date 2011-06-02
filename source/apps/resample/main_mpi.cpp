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
/*                          workload based distribution                       */
/*                          execute 1x per compute node                       */
/*                                                                            */
/******************************************************************************/

#include "resample.h"
#include <mpi.h>
#include <omp.h>
#include <iostream>
#include <boost/program_options.hpp>
#include <ctime>
#include <cassert>
#include <stack>

//--------------------------
#define MAX_WORK_SIZE 1024
//--------------------------


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

struct SWork
{
   int64 sx, sy;
};

//------------------------------------------------------------------------------
// send work (must be called from rank 0!)
void SendWork(int count, SWork* workarray, int target_rank)
{
   // send data
   MPI_Send(workarray, count*sizeof(SWork), MPI_BYTE, target_rank, 1112, MPI_COMM_WORLD);
}

//------------------------------------------------------------------------------
// receive work:
void ReceiveWork(SWork* workarray, int& count)
{
   MPI_Status status;
   int msglen;
   MPI_Probe(0, 1112, MPI_COMM_WORLD, &status);
   MPI_Get_count(&status, MPI_BYTE, &msglen);
   count = msglen / sizeof(SWork);
   MPI_Recv(workarray, msglen, MPI_BYTE, MPI_ANY_SOURCE, 1112, MPI_COMM_WORLD, &status);
}

//------------------------------------------------------------------------------

void _worker( int workload, SWork* pWorkArray, TileBlock* pTileBlockArray, boost::shared_ptr<MercatorQuadtree> qQuadtree, int nLevelOfDetail, std::string sTileDir, bool bVerbose ) 
{
#     pragma omp parallel for
      for (int i=0;i<workload;i++)
      {
         _resampleFromParent(pTileBlockArray, qQuadtree, pWorkArray[i].sx, pWorkArray[i].sy, nLevelOfDetail, sTileDir);
      }
}

//------------------------------------------------------------------------------

int main(int argc, char *argv[])
{
   std::string sImageLayerDir;
   std::string sTileDir;
   int64 tx0,ty0,tx1,ty1;
   int maxlod;
   clock_t t0,t1;
   bool bVerbose = false;
   SWork* pWorkArray = new SWork[MAX_WORK_SIZE];
   std::stack<SWork> workstack;

   //---------------------------------------------------------------------------
   // MPI Init
   //---------------------------------------------------------------------------

   int rank, totalnodes;

   MPI_Init(&argc, &argv); 
   MPI_Comm_size(MPI_COMM_WORLD, &totalnodes);
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);


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
      sTileDir = FilenameUtils::DelimitPath(FilenameUtils::DelimitPath(sImageLayerDir) + "tiles");

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
   
   BroadcastString(sTileDir, 0);
   BroadcastString(sImageLayerDir, 0);
   BroadcastInt64(tx0, 0);
   BroadcastInt64(ty0, 0);
   BroadcastInt64(tx1, 0);
   BroadcastInt64(ty1, 0);
   BroadcastInt(maxlod, 0);
   BroadcastBool(bVerbose, 0);

   //std::cout << "rank = " << rank <<", TileDir = " << sTileDir << "\n";
   //std::cout << "rank = " << rank <<", maxlod = " << maxlod << "\n"; 
   //std::cout << "rank = " << rank <<", extent = " << tx0 << " " << ty0 << " " << tx1 << " " << ty1 << "\n";

   TileBlock* pTileBlockArray = _createTileBlockArray();

   boost::shared_ptr<MercatorQuadtree> qQuadtree = boost::shared_ptr<MercatorQuadtree>(new MercatorQuadtree());
   std::string qc0 = qQuadtree->TileCoordToQuadkey(tx0, ty0, maxlod);
   std::string qc1 = qQuadtree->TileCoordToQuadkey(tx1, ty1, maxlod);

   for (int nLevelOfDetail = maxlod - 1; nLevelOfDetail>0; nLevelOfDetail--)
   {
      if (bVerbose && rank == 0)
      {
         std::cout << "[LOD] starting processing lod " << nLevelOfDetail << "\n" << std::flush;
      }

      qc0 = StringUtils::Left(qc0, nLevelOfDetail);
      qc1 = StringUtils::Left(qc1, nLevelOfDetail);

      int tmp_lod;
      qQuadtree->QuadKeyToTileCoord(qc0, tx0, ty0, tmp_lod);
      qQuadtree->QuadKeyToTileCoord(qc1, tx1, ty1, tmp_lod);

      if (bVerbose && rank == 0)
      {
        std::cout << "[RANGE]: [" << tx0 << ", " << ty0 << "]-[" << tx1 << ", " << ty1 << "]\n" << std::flush;
      }

      if (rank == 0)
      {
         // create workstack: contains all work which will be distributed.
         SWork work;

         for (int64 y=ty0;y<=ty1;y++)
         {
            for (int64 x=tx0;x<=tx1;x++)
            {
               work.sx = x; work.sy = y;
               workstack.push(work);
            }
         }
      }

      // total number of tiles:
      int totaltiles = int((tx1-tx0+1)*(ty1-ty0+1));
      clock_t tprog0, tprog1;
      int count = 0;
      tprog0 = clock();

      int cnt;
      bool bFinished = false;
      bool* bFinishedArray = new bool[totalnodes];
      for (int i=0;i<totalnodes;i++) bFinishedArray[i] = false;

      do 
      {
         if (rank == 0)
         {
            for (int i=1;i<totalnodes;i++)
            {
               if (!bFinishedArray[i])
               {
                  int workcnt = 0;

                  for (int w=0;w<MAX_WORK_SIZE;w++)
                  {
                     if (workstack.size()>0)
                     {
                        pWorkArray[w] = workstack.top();
                        workstack.pop();
                        workcnt++;
                     }
                  }
                  //std::cout << "sending work [" << workcnt << "]" << " to " << i << "\n" << std::flush;
                  SendWork(workcnt, pWorkArray, i);

                  if (workcnt == 0)
                  {
                     bFinishedArray[i] = true;
                  }
               }
            
            }
         }
         if (rank != 0)
         {
            ReceiveWork(pWorkArray, cnt);
            if (cnt == 0)
            {
               bFinished = true;
            }
         }
         else
         {
            // generate some work for rank 0: we want to use all resources.
            // it is possible to reduce the work size for this node, but if you keep working size <= 1024 it is probably ok.
            cnt = 0;
            for (int w=0;w<MAX_WORK_SIZE;w++)
            {
               if (workstack.size()>0)
               {
                  pWorkArray[w] = workstack.top();
                  workstack.pop();
                  cnt++;
               }
            }

            if (workstack.size() == 0)
            {
               bool bt = true;
               for (int i=1;i<totalnodes;i++)
               {
                  bt = bt && bFinishedArray[i];
               }

               if (bt)
               {
                  bFinished = true;
               }
            }
         }

         //std::cout << "Compute Node " << rank << " received work [" << cnt << "]\n" << std::flush;

         if (cnt>0)
         {
            count = count + cnt;
            _worker(cnt, pWorkArray, pTileBlockArray, qQuadtree, nLevelOfDetail, sTileDir, bVerbose);
            
            if (bVerbose)
            {
               tprog1 = clock();
               double time_passed = double(tprog1-tprog0)/double(CLOCKS_PER_SEC);
               if (time_passed > 200) // print progress report after some time
               {
                  double progress = double(int(10000.0*double(count)/double(totaltiles))/100.0);
                  std::cout << "[PROGRESS] Compute Node " << rank << " processed " << count << "/" << totaltiles << " tiles (" << progress << "%)\n" << std::flush;
                  tprog0 = tprog1;
               }
            }
         
         
         }

      } while (!bFinished);

      delete[] bFinishedArray;
     
    
      if (bVerbose)
      {
         std::cout << "[FINISH] Compute Node " << rank << " finished lod " << nLevelOfDetail << "\n" << std::flush;        
      }

      MPI_Barrier(MPI_COMM_WORLD);
   }

   MPI_Finalize();

   // clean up
   _destroyTileBlockArray(pTileBlockArray);

   std::cout << std::flush;

   // output calculation time
   if (rank == 0)
   {
      t1=clock();
      std::cout << "calculated in: " << double(t1-t0)/double(CLOCKS_PER_SEC) << " s \n";
   }

   
   delete[] pWorkArray;
   return 0;
}

