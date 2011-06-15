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

#include "deploy.h"
#include "ogprocess.h"
#include "geo/ImageLayerSettings.h"
#include "geo/ElevationLayerSettings.h"
#include "io/FileSystem.h"
#include "string/FilenameUtils.h"
#include "string/StringUtils.h"
#include "image/ImageLoader.h"
#include "image/JPEGHandler.h"
#include "image/ImageWriter.h"
#include "geo/MercatorQuadtree.h"
#include <sstream>
#include <ctime>
#include <fstream>
#include <omp.h>

namespace Deploy
{
   //---------------------------------------------------------------------------
   struct ThreadInfo
   {
      std::string    sFileName;  // filename of archive
      std::ofstream* pFileout;
      TarWriter*     pTarWriter; // the writer
   };
   //---------------------------------------------------------------------------


   ThreadInfo* GenerateThreadInfo()
   {
      std::string sFilePath; // delimited file path where archives are written.

      int maxthreads = omp_get_max_threads();

      ThreadInfo* threadinfoarray = new ThreadInfo[maxthreads];
      if (!threadinfoarray)
      {
         return 0; // very unlikely out of memory.
      }

      // every thread writes to another tar archive this way we can write archives in parallel
      // store filename
      for (int i=0;i<maxthreads;i++)
      {
         std::ostringstream oss;
         oss << sFilePath << SystemUtils::ComputerName() << "_thread_" << i << ".tar";
         threadinfoarray[i].sFileName = oss.str();
         threadinfoarray[i].pTarWriter = 0;

      }

      return threadinfoarray;
   }

   //---------------------------------------------------------------------------

   void DestroyThreadInfo(ThreadInfo* pthreadinfo)
   {
      if (pthreadinfo)
      {
         delete[] pthreadinfo;
      }
   }

   //---------------------------------------------------------------------------

   void DeployImageLayer(boost::shared_ptr<Logger> qLogger, boost::shared_ptr<ProcessingSettings> qSettings, const std::string& sLayer, const std::string& sPath, bool bArchive, EOuputImageFormat imageformat, int quality)
   {
      std::ostringstream oss;

      std::string sImageLayerDir = FilenameUtils::DelimitPath(qSettings->GetPath()) + sLayer;
      std::string sTileDir = FilenameUtils::DelimitPath(FilenameUtils::DelimitPath(sImageLayerDir) + "tiles");

      boost::shared_ptr<ImageLayerSettings> qImageLayerSettings = ImageLayerSettings::Load(sImageLayerDir);
      if (!qImageLayerSettings)
      {
         qLogger->Error("Failed retrieving image layer settings!");
         return;
      }

      int64 tx0,ty0,tx1,ty1;
      qImageLayerSettings->GetTileExtent(tx0,ty0,tx1,ty1);
      int maxlod = qImageLayerSettings->GetMaxLod();

      oss << "tile extent: " << tx0 << ", " << ty0 << ", " << tx1  << ", " << ty1 << "\n";
      qLogger->Info(oss.str());
      oss.str("");

      ThreadInfo* pThreadInfo = GenerateThreadInfo(); 

      clock_t t0,t1;
      t0 = clock();

      boost::shared_ptr<MercatorQuadtree> qQuadtree = boost::shared_ptr<MercatorQuadtree>(new MercatorQuadtree());
      std::string qc0 = qQuadtree->TileCoordToQuadkey(tx0, ty0, maxlod);
      std::string qc1 = qQuadtree->TileCoordToQuadkey(tx1, ty1, maxlod);

      for (int nLevelOfDetail = maxlod; nLevelOfDetail>0; nLevelOfDetail--)
      {
         std::ostringstream oss;
         oss << "Deploying Level of Detail " << nLevelOfDetail;
         qLogger->Info(oss.str());

         qc0 = StringUtils::Left(qc0, nLevelOfDetail);
         qc1 = StringUtils::Left(qc1, nLevelOfDetail);

         int tmp_lod;
         qQuadtree->QuadKeyToTileCoord(qc0, tx0, ty0, tmp_lod);
         qQuadtree->QuadKeyToTileCoord(qc1, tx1, ty1, tmp_lod);

#     pragma omp parallel for
         for (int64 y=ty0;y<=ty1;y++)
         {
            for (int64 x=tx0;x<=tx1;x++)
            {
               if (bArchive)
               {
                  int i = omp_get_thread_num();
                  if (pThreadInfo[i].pTarWriter == 0)
                  {
                     std::string sFilename = FilenameUtils::DelimitPath(sPath) + pThreadInfo[i].sFileName;
                     pThreadInfo[i].pFileout = new std::ofstream();
                     pThreadInfo[i].pFileout->open(sFilename.c_str(), std::ios::binary);
                     pThreadInfo[i].pTarWriter = new TarWriter(*pThreadInfo[i].pFileout);
                  }

                  if (imageformat == OUTFORMAT_PNG)
                  {
                     std::string sOrigTile = ProcessingUtils::GetTilePath(sTileDir, ".png" , nLevelOfDetail, x, y);
                     std::string sArchiveTile = ProcessingUtils::GetTilePath("tiles/", ".png" , nLevelOfDetail, x, y);

                     if (FileSystem::FileExists(sOrigTile))
                     {
                        pThreadInfo[i].pTarWriter->AddFile(sOrigTile.c_str(), sArchiveTile.c_str());
                     }
                  }
                  else if (imageformat == OUTFORMAT_JPG)
                  {
                     std::string sOrigTile = ProcessingUtils::GetTilePath(sTileDir, ".png" , nLevelOfDetail, x, y);
                     std::string sArchiveTile = ProcessingUtils::GetTilePath("tiles/", ".jpg" , nLevelOfDetail, x, y);

                     if (FileSystem::FileExists(sOrigTile))
                     {
                        ImageObject img;
                        // load as RGB as jpeg doesn't support alpha.
                        if (ImageLoader::LoadFromDisk(Img::Format_PNG, sOrigTile, Img::PixelFormat_RGB, img))
                        {
                           boost::shared_array<unsigned char> outjpg;
                           int len;
                           JPEGHandler::RGBToJpeg(img.GetRawData().get(), img.GetWidth(), img.GetHeight(), quality, outjpg, len);
                           pThreadInfo[i].pTarWriter->AddData(sArchiveTile.c_str(), (char*)outjpg.get(), len);
                        }
                     }
                  }
               }
               else
               {
                  //
               }
            }
         }
      }

      // todo: put info file into first archive (pThreadInfo[0])
      

      // close all streams:

      for (int i=0;i<omp_get_max_threads();i++)
      {
         if (pThreadInfo[i].pTarWriter)
         {
            pThreadInfo[i].pTarWriter->Finalize();
            pThreadInfo[i].pFileout->close();
            delete pThreadInfo[i].pFileout;
         }
      }

      DestroyThreadInfo(pThreadInfo);

      // output time to calculate resampling:
      t1=clock();
      oss << "calculated in: " << double(t1-t0)/double(CLOCKS_PER_SEC) << " s \n";
      qLogger->Info(oss.str());
      oss.str("");
   }

   //--------------------------------------------------------------------------

   void DeployElevationLayer(boost::shared_ptr<Logger> qLogger, boost::shared_ptr<ProcessingSettings> qSettings, const std::string& sLayer, const std::string& sPath, bool bArchive, EOutputElevationFormat elevationformat)
   {

   }


}


