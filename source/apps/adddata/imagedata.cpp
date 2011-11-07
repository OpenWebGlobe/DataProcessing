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

#include "imagedata.h"
#include "string/FilenameUtils.h"
#include "io/FileSystem.h"
#include "geo/ImageLayerSettings.h"
#include "geo/MercatorQuadtree.h"
#include "image/ImageLoader.h"
#include "image/ImageWriter.h"
#include <sstream>
#include <ctime>

//------------------------------------------------------------------------------
namespace ImageData
{
   //------------------------------------------------------------------------------
   const int tilesize = 256;
   const double dHanc = 1.0/(double(tilesize)-1.0);
   const double dWanc = 1.0/(double(tilesize)-1.0);
   //------------------------------------------------------------------------------

   int process( boost::shared_ptr<Logger> qLogger, boost::shared_ptr<ProcessingSettings> qSettings, std::string sLayer, bool bVerbose, bool bLock, int epsg, std::string sImagefile, bool bFill, int& out_lod, int64& out_x0, int64& out_y0, int64& out_x1, int64& out_y1)
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
      std::string sTileDir = FilenameUtils::DelimitPath(FilenameUtils::DelimitPath(sImageLayerDir) + "tiles");

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
      boost::shared_array<unsigned char> vImage = ProcessingUtils::ImageToMemoryRGB(oInfo);
      unsigned char* pImage = vImage.get();

      if (!vImage)
      {
         qLogger->Error("Can't load image into memory!\n");
         return ERROR_NOMEMORY;
      }
      // iterate through all tiles and create them
   #pragma omp parallel for
      for (int64 xx = imageTileX0; xx <= imageTileX1; ++xx)
      {
         for (int64 yy = imageTileY0; yy <= imageTileY1; ++yy)
         {
            boost::shared_array<unsigned char> vTile;

            std::string sQuadcode = qQuadtree->TileCoordToQuadkey(xx,yy,lod);
            std::string sTilefile = ProcessingUtils::GetTilePath(sTileDir, ".png" , lod, xx, yy);

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
               ImageObject outputimage;
               if (ImageLoader::LoadFromDisk(Img::Format_PNG, sTilefile, Img::PixelFormat_RGBA, outputimage))
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
               vTile = boost::shared_array<unsigned char>(new unsigned char[tilesize*tilesize*4]);
               memset(vTile.get(),0,tilesize*tilesize*4);
            }

            unsigned char* pTile = vTile.get();

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

            /*if (bVerbose)
            {
            std::stringstream sst;
            sst << "Anchor points:\nA(" << anchor_Ax << ", " << anchor_Ay << ")" 
            << "\nB(" << anchor_Bx << ", " << anchor_By << ")" 
            << "\nC(" << anchor_Cx << ", " << anchor_Cy << ")"
            << "\nD(" << anchor_Dx << ", " << anchor_Dy << ")\n";
            qLogger->Info(sst.str());
            }*/

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
                  unsigned char r,g,b,a;

                  // out of image -> set transparent
                  if (dPixelX<0 || dPixelX>oInfo.nSizeX ||
                     dPixelY<0 || dPixelY>oInfo.nSizeY)
                  {
                     r = g = b = a = 0;
                  }
                  else
                  {
                     // read pixel in image pImage[dPixelX, dPixelY] (biliear, bicubic or nearest neighbour)
                     // and store as r,g,b
                     _ReadImageValueBilinear(pImage, oInfo.nSizeX, oInfo.nSizeY, dPixelX, dPixelY, &r, &g, &b, &a);
                  }

                  size_t adr=4*ty*tilesize+4*tx;

                  if (a>0)
                  {
                     if (bFill)
                     {
                        if (pTile[adr+3] == 0)
                        {
                           pTile[adr+0] = r;  
                           pTile[adr+1] = g;  
                           pTile[adr+2] = b; 
                           pTile[adr+3] = a;
                        }
                     }
                     else // if (bOverwrite)
                     {
                        // currently RGB for testing purposes!
                        pTile[adr+0] = r;  
                        pTile[adr+1] = g;  
                        pTile[adr+2] = b; 
                        pTile[adr+3] = a;
                     }
                  }
               }
            }

            // save tile (pTile)
            if (bVerbose)
            {
               qLogger->Info("Storing tile: " + sTilefile);
            }

            ImageWriter::WritePNG(sTilefile, pTile, tilesize, tilesize);

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

}