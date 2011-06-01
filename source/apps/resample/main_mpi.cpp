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
/*                         execute 1x per compute node                        */
/*                                                                            */
/******************************************************************************/

#include "resample.h"
#include <mpi.h>
#include <omp.h>
#include <iostream>
#include <boost/program_options.hpp>
#include <ctime>
#include <cassert>

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

int main(int argc, char *argv[])
{
   std::string sImageLayerDir;
   std::string sTileDir;
   int64 tx0,ty0,tx1,ty1;
   int maxlod;
   clock_t t0,t1;
   

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
      bool bVerbose = false;


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

   }
   
   BroadcastString(sTileDir, 0);
   BroadcastString(sImageLayerDir, 0);
   BroadcastInt64(tx0, 0);
   BroadcastInt64(ty0, 0);
   BroadcastInt64(tx1, 0);
   BroadcastInt64(ty1, 0);
   BroadcastInt(maxlod, 0);

   //std::cout << "rank = " << rank <<", TileDir = " << sTileDir << "\n";
   //std::cout << "rank = " << rank <<", maxlod = " << maxlod << "\n"; 
   //std::cout << "rank = " << rank <<", extent = " << tx0 << " " << ty0 << " " << tx1 << " " << ty1 << "\n";

   TileBlock* pTileBlockArray = _createTileBlockArray();

   boost::shared_ptr<MercatorQuadtree> qQuadtree = boost::shared_ptr<MercatorQuadtree>(new MercatorQuadtree());
   std::string qc0 = qQuadtree->TileCoordToQuadkey(tx0, ty0, maxlod);
   std::string qc1 = qQuadtree->TileCoordToQuadkey(tx1, ty1, maxlod);

   for (int nLevelOfDetail = maxlod - 1; nLevelOfDetail>0; nLevelOfDetail--)
   {
      std::cout << "compute node" << rank << ": processing lod " << nLevelOfDetail << "\n" << std::flush;

      qc0 = StringUtils::Left(qc0, nLevelOfDetail);
      qc1 = StringUtils::Left(qc1, nLevelOfDetail);

      int tmp_lod;
      qQuadtree->QuadKeyToTileCoord(qc0, tx0, ty0, tmp_lod);
      qQuadtree->QuadKeyToTileCoord(qc1, tx1, ty1, tmp_lod);

      //------------------------------------------------------------------------
      // PARTITION TILE LAYOUT:
      double w = double(tx1-tx0+1);
      double h = double(ty1-ty0+1);

      int64 startx = tx0;
      int64 px0 = -1;
      int64 py0 = -1;
      int64 px1 = -1;
      int64 py1 = -1;

      for (int i=0;i<totalnodes;i++)
      {
         int n = int(ceil((w-1-double(i))/double(totalnodes)));
         if (i == rank)
         {
            if (n>0)
            {
               px0 = startx; py0 = ty0;
               px1 = startx + n; py1 = ty1;
            }
         }
         startx+=n;
      }

      //------------------------------------------------------------------------
      if (px0 > 0)
      {
#        pragma omp parallel for
         for (int64 y=py0;y<=py1;y++)
         {
            for (int64 x=px0;x<=px1;x++)
            {
               _resampleFromParent(pTileBlockArray, qQuadtree, x, y, nLevelOfDetail, sTileDir);
            }
         }
      }

      MPI_Barrier(MPI_COMM_WORLD);
   }


   //std::cout << "Hello after barrier from process " << rank << " of " << totalnodes << std::endl;

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

   

   return 0;
}

