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
#                           robert.wueest@fhnw.ch                              #
********************************************************************************
*     Licensed under MIT License. Read the file LICENSE for more information   *
*******************************************************************************/

#include "rawimagedata.h"
#include "string/FilenameUtils.h"
#include "io/FileSystem.h"
#include "geo/ImageLayerSettings.h"
#include "geo/MercatorQuadtree.h"
#include "image/ImageLoader.h"
#include <sstream>
#include <ctime>


//------------------------------------------------------------------------------
namespace RawImageData
{
   //------------------------------------------------------------------------------
   const int tilesize = 256;
   const double dHanc = 1.0/(double(tilesize));
   const double dWanc = 1.0/(double(tilesize));
   //------------------------------------------------------------------------------

   int process( boost::shared_ptr<Logger> qLogger, boost::shared_ptr<ProcessingSettings> qSettings, std::string sLayer, bool bVerbose, bool bLock, int epsg, std::string sImagefile, bool bFill, int& out_lod, int64& out_x0, int64& out_y0, int64& out_x1, int64& out_y1/*int maxLod*/)
   {
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

      std::string sTileDir = FilenameUtils::DelimitPath(FilenameUtils::DelimitPath(sImageLayerDir) + "temp/tiles");

      boost::shared_ptr<ImageLayerSettings> qImageLayerSettings = ImageLayerSettings::Load(sImageLayerDir);
      if (!qImageLayerSettings)
      {
         qLogger->Error("Failed retrieving image layer settings! Make sure to create it using 'createlayer'.");
         ProcessingUtils::exit_gdal();
         return ERROR_IMAGELAYERSETTINGS;
      }

      int lod = qImageLayerSettings->GetMaxLod();
      out_lod = lod;
      int64 layerTileX0, layerTileY0, layerTileX1, layerTileY1;
      qImageLayerSettings->GetTileExtent(layerTileX0, layerTileY0, layerTileX1, layerTileY1);

      if (bVerbose)
      {
         oss << "\nImage Layer:\n";
         oss << "     name = " << qImageLayerSettings->GetLayerName() << "\n";
         oss << "   maxlod = " << lod << "\n";
         oss << "   extent = " << layerTileX0 << ", " << layerTileY0 << ", " << layerTileX1 << ", " << layerTileY1 << "\n";
      }


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

      if (bVerbose)
      {
         oss << "Loaded image info:\n   Image Size: w= " << oInfo.nSizeX << ", h= " << oInfo.nSizeY << "\n";
         oss << "   dest: " << oInfo.dest_lrx << ", " << oInfo.dest_lry << ", " << oInfo.dest_ulx << ", " << oInfo.dest_uly << "\n";
         qLogger->Info(oss.str());
         oss.str("");
      }
      
      boost::shared_ptr<MercatorQuadtree> qQuadtree = boost::shared_ptr<MercatorQuadtree>(new MercatorQuadtree());

      int64 px0, py0, px1, py1;
      qQuadtree->MercatorToPixel(oInfo.dest_ulx, oInfo.dest_uly, lod, px0, py0);
      qQuadtree->MercatorToPixel(oInfo.dest_lrx, oInfo.dest_lry, lod, px1, py1);

      int64 imageTileX0, imageTileY0, imageTileX1, imageTileY1;
      qQuadtree->PixelToTileCoord(px0, py0, imageTileX0, imageTileY0);
      qQuadtree->PixelToTileCoord(px1, py1, imageTileX1, imageTileY1);

      if (bVerbose)
      {
         oss << "\nTile Coords (image):";
         oss << "   (" << imageTileX0 << ", " << imageTileY0 << ")-(" << imageTileX1 << ", " << imageTileY1 << ")\n";
         qLogger->Info(oss.str());
         oss.str("");
      }

      // check if image is outside layer
      if (imageTileX0 > layerTileX1 || 
         imageTileY0 > layerTileY1 ||
         imageTileX1 < layerTileX0 ||
         imageTileY1 < layerTileY0)
      {
         qLogger->Info("The dataset is outside of the layer and not being added!");
         ProcessingUtils::exit_gdal();
         return 0;
      }

      // clip tiles to layer extent
      imageTileX0 = math::Max<int64>(imageTileX0, layerTileX0);
      imageTileY0 = math::Max<int64>(imageTileY0, layerTileY0);
      imageTileX1 = math::Min<int64>(imageTileX1, layerTileX1);
      imageTileY1 = math::Min<int64>(imageTileY1, layerTileY1);

      out_x0 = imageTileX0;
      out_y0 = imageTileY0;
      out_x1 = imageTileX1;
      out_y1 = imageTileY1;

      // Load image 
      boost::shared_array<float> vImage = ProcessingUtils::ImageToMemoryGreyScale(oInfo);
      float* pImage = vImage.get();

      if (!vImage)
      {
         qLogger->Error("Can't load image into memory!\n");
         ProcessingUtils::exit_gdal();
         return ERROR_NOMEMORY;
      }
      // iterate through all tiles and create them
#ifndef _DEBUG
#pragma omp parallel for
#endif
      for (int64 xx = imageTileX0; xx <= imageTileX1; ++xx)
      {
         for (int64 yy = imageTileY0; yy <= imageTileY1; ++yy)
         {
            boost::shared_array<float> vTile;

            std::string sQuadcode = qQuadtree->TileCoordToQuadkey(xx,yy,lod);
            std::string sTilefile = ProcessingUtils::GetTilePath(sTileDir, ".raw" , lod, xx, yy);

            if (bVerbose)
            {
               std::stringstream sst;
               sst << "processing " << sQuadcode << " (" << xx << ", " << yy << ")";
               qLogger->Info(sst.str());
            }

            //---------------------------------------------------------------------
            // LOCK this tile. If this tile is currently locked 
            //     -> wait until lock is removed.
            int lockhandle = bLock ? FileSystem::Lock(sTilefile) : -1;

            //---------------------------------------------------------------------
            // if mode is --fill: (bFill)
            //      * load possibly existing tile into vTile
            // ...  * if there is none, clear vTile (memset 0)
            // if mode is --overwrite (bOverwrite)
            //      * load possibly existing tile into vTile
            //      * if there is none, clear vTile (memset 0)
            //      * overwrite
            //_--------------------------------------------------------------------

            // load tile:

            // tile already exists ?
            bool bCreateNew = true;

            if (FileSystem::FileExists(sTilefile))
            {
               qLogger->Info(sTilefile + " already exists, updating");
               Raw32ImageObject outputimage;
               if (ImageLoader::LoadRaw32FromDisk(sTilefile, tilesize,tilesize, outputimage))
               {
                  if (outputimage.GetHeight() == tilesize && outputimage.GetWidth() == tilesize)
                  {
                     vTile = outputimage.GetRawData();
                     bCreateNew = false;
                  }
               }
            }

            if (bCreateNew)
            {
               // create new tile memory and clear to fully transparent
               vTile = boost::shared_array<float>(new float[tilesize*tilesize]);
               //vTile = boost::shared_array<unsigned char>(new unsigned char[tilesize*tilesize*4]);
               memset(vTile.get(),0,tilesize*tilesize*sizeof(float));
            }

            float* pTile = vTile.get();

            // Copy image to tile:
            double px0m, py0m, px1m, py1m;
            qQuadtree->QuadKeyToMercatorCoord(sQuadcode, px0m, py0m, px1m, py1m);

            double ulx = px0m;
            double uly = py1m;
            double lrx = px1m;
            double lry = py0m;

            double anchor_Ax = ulx; 
            double anchor_Ay = lry;
            double anchor_Bx = lrx; 
            double anchor_By = lry;
            double anchor_Cx = lrx; 
            double anchor_Cy = uly;
            double anchor_Dx = ulx; 
            double anchor_Dy = uly;

            // avoid calculating transformation per pixel using anchor point method
            qCT->TransformBackwards(&anchor_Ax, &anchor_Ay);
            qCT->TransformBackwards(&anchor_Bx, &anchor_By);
            qCT->TransformBackwards(&anchor_Cx, &anchor_Cy);
            qCT->TransformBackwards(&anchor_Dx, &anchor_Dy);

            // write current tile
            for (int ty=0;ty<tilesize;++ty)
            {
               for (int tx=0;tx<tilesize;++tx)
               {
                  double dx = (double)tx*dWanc;
                  double dy = (double)ty*dHanc;
                  double xd = (anchor_Ax*(1.0-dx)*(1.0-dy)+anchor_Bx*dx*(1.0-dy)+anchor_Dx*(1.0-dx)*dy+anchor_Cx*dx*dy);
                  double yd = (anchor_Ay*(1.0-dx)*(1.0-dy)+anchor_By*dx*(1.0-dy)+anchor_Dy*(1.0-dx)*dy+anchor_Cy*dx*dy);

                  // pixel coordinate in original image
                  double dPixelX = (oInfo.affineTransformation_inverse[0] + xd * oInfo.affineTransformation_inverse[1] + yd * oInfo.affineTransformation_inverse[2]);
                  double dPixelY = (oInfo.affineTransformation_inverse[3] + xd * oInfo.affineTransformation_inverse[4] + yd * oInfo.affineTransformation_inverse[5]);
                  float value;

                  

                  // out of image -> set transparent
                  if (dPixelX<0 || dPixelX>oInfo.nSizeX ||
                     dPixelY<0 || dPixelY>oInfo.nSizeY)
                  {
                     value = -9999.0f;
                  }
                  else
                  {
                     // read pixel in image pImage[dPixelX, dPixelY] (biliear, bicubic or nearest neighbour)
                     // and store as r,g,b
					   //_ReadImageDataMem(pImage, oInfo.nSizeX, oInfo.nSizeY, dPixelX, dPixelY, &value);
                     _ReadImageValueBilinear(pImage, oInfo.nSizeX, oInfo.nSizeY, dPixelX, dPixelY, &value);
                     // scale to 256 AND REMOVE VOID PIXELS (-9999 values) !!!! 
                     //if (value<-5000) value = -9999;
                     //value = math::Clamp<float>(unsigned char(value/1000.0f*256.0f), 0, 255);
                  }

                  size_t adr=1*ty*tilesize+1*tx;
                     if (bFill)
                     {
                        if (pTile[adr] < -9000.0f)
                        {
                           pTile[adr] = value;  
                        }
                        else if(bCreateNew)
                        {
                           pTile[adr] = value;  
                        }
                     }
                     else // if (bOverwrite)
                     {
                        pTile[adr] = value;  
                     }
               }
            }

            // save tile (pTile)
            if (bVerbose)
            {
               qLogger->Info("Storing tile: " + sTilefile);
            }

            ImageWriter::WriteRaw32(sTilefile, tilesize, tilesize, pTile);
            // --- DOWNSAMPLING   --------------------------------------------------------
            /*if(iMaxLod > lod)
            {
               int sX = xx;
               int sY = yy;
               processlod(sTileDir,pTile, lod, lod+1, iMaxLod,sX,sY);
            }*/
            //------------------------------------------------------------------------
            // TEMPORARY FILE OUT               
            //std::string fil = ProcessingUtils::GetTilePath(sTileDir, ".png" , lod, xx, yy);
            //_SaveFileAsPNG(pTile, tilesize, tilesize, fil);
            // --->
            // unlock file. Other computers/processes/threads can access it again.
            FileSystem::Unlock(sTilefile, lockhandle);
         }
      }

      //---------------------------------------------------------------------------


      //---------------------------------------------------------------------------
      t1=clock();

      std::ostringstream out;
      out << "calculated in: " << double(t1-t0)/double(CLOCKS_PER_SEC) << " s \n";
      qLogger->Info(out.str());

      ProcessingUtils::exit_gdal();

      return 0;
   }
   /*
   void processlod(std::string sTileDir, float* pTile, int nativeLod, int currentLod, int maxLod, int extentX, int extentY)
   {
      int sX = extentX;
      int sY = extentY;
      if(currentLod > nativeLod)
      {
         // update tile extents
         sX = sX*2;
         sY = sY*2;
      }
      std::stringstream lodDir; lodDir << sTileDir << currentLod;
      if(!FileSystem::DirExists(lodDir.str()))
               FileSystem::makedir(lodDir.str());
      for(size_t sxx = 0; sxx < 2; sxx++)
      {
         for(size_t syy = 0; syy < 2; syy++)
         {
            
            std::stringstream xDir; xDir << lodDir.str() << "/" << sX+sxx;
            if(!FileSystem::DirExists(xDir.str()))
               FileSystem::makedir(xDir.str());
            std::string sampleFile = ProcessingUtils::GetTilePath(sTileDir, ".raw" , currentLod, sX+sxx, sY+syy);
            boost::shared_array<float> sampleTile = boost::shared_array<float>(new float[tilesize*tilesize]);
            memset(sampleTile.get(),0,tilesize*tilesize*sizeof(float));
            
            for (int ty=0;ty<tilesize;++ty)
            {
               for (int tx=0;tx<tilesize;++tx)
               {
                  float value;
                  _ReadImageValueBilinear(pTile, tilesize, tilesize, double((tx/2.0)+(sxx*(tilesize/2.0))), double((ty/2.0)+(syy*(tilesize/2.0))), &value);
                  sampleTile[tx+tilesize*ty] = value;
               }
            }
            int lockh = FileSystem::Lock(sampleFile);
               ImageWriter::WriteRaw32(sampleFile, tilesize, tilesize, sampleTile.get());
            FileSystem::Unlock(sampleFile, lockh);
            // process downward
            if(currentLod < maxLod)
            {
               processlod(sTileDir, sampleTile.get(), nativeLod, currentLod+1, maxLod, sX+sxx, sY+syy);
            }
            // TEMPORARY FILE OUT               
            //std::string fil = ProcessingUtils::GetTilePath(sTileDir, ".png" , currentLod, sX+sxx, sY+syy);
            //_SaveFileAsPNG(sampleTile.get(), tilesize, tilesize, fil);
            // --->
         }
      }
   }*/

}