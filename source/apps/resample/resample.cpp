
#include "resample.h"
#include <omp.h>

//------------------------------------------------------------------------------

TileBlock* _createTileBlockArray() 
{
   int maxthreads = omp_get_max_threads();
   TileBlock* pTileBlockArray = new TileBlock[maxthreads];
   return pTileBlockArray;
}

//------------------------------------------------------------------------------

void _destroyTileBlockArray(TileBlock* pTileBlockArray)
{
   if (pTileBlockArray) 
   {
      delete[] pTileBlockArray;
   }
}
//------------------------------------------------------------------------------
void _resampleFromParent( TileBlock* pTileBlockArray, boost::shared_ptr<MercatorQuadtree> qQuadtree, int64 x, int64 y,int nLevelOfDetail, std::string sTileDir, bool rawData) 
{
   int curthread = omp_get_thread_num();
   TileBlock& tile = pTileBlockArray[curthread];
   tile.Clear();

   std::string qcCurrent = qQuadtree->TileCoordToQuadkey(x,y,nLevelOfDetail);

   // calculate parent quadkeys:
   std::string qc0,qc1,qc2,qc3;
   qc0 = qcCurrent + '0';
   qc1 = qcCurrent + '1';
   qc2 = qcCurrent + '2';
   qc3 = qcCurrent + '3';

   int64 _tx, _ty;
   int tmp_lod;

   if(!rawData)
   {

      qQuadtree->QuadKeyToTileCoord(qcCurrent, _tx, _ty, tmp_lod);
      std::string sCurrentTile = ProcessingUtils::GetTilePath(sTileDir, ".png" , tmp_lod, _tx, _ty);

      qQuadtree->QuadKeyToTileCoord(qc0, _tx, _ty, tmp_lod);
      std::string sTilefile0 = ProcessingUtils::GetTilePath(sTileDir, ".png" , tmp_lod, _tx, _ty);

      qQuadtree->QuadKeyToTileCoord(qc1, _tx, _ty, tmp_lod);
      std::string sTilefile1 = ProcessingUtils::GetTilePath(sTileDir, ".png" , tmp_lod, _tx, _ty);

      qQuadtree->QuadKeyToTileCoord(qc2, _tx, _ty, tmp_lod);
      std::string sTilefile2 = ProcessingUtils::GetTilePath(sTileDir, ".png" , tmp_lod, _tx, _ty);

      qQuadtree->QuadKeyToTileCoord(qc3, _tx, _ty, tmp_lod);
      std::string sTilefile3 = ProcessingUtils::GetTilePath(sTileDir, ".png" , tmp_lod, _tx, _ty);

      ImageObject IH0, IH1, IH2, IH3;

      bool b0 = ImageLoader::LoadFromDisk(Img::Format_PNG, sTilefile0, Img::PixelFormat_RGBA, IH0);
      bool b1 = ImageLoader::LoadFromDisk(Img::Format_PNG, sTilefile1, Img::PixelFormat_RGBA, IH1);
      bool b2 = ImageLoader::LoadFromDisk(Img::Format_PNG, sTilefile2, Img::PixelFormat_RGBA, IH2);
      bool b3 = ImageLoader::LoadFromDisk(Img::Format_PNG, sTilefile3, Img::PixelFormat_RGBA, IH3);

      unsigned char* p0 = IH0.GetRawData().get();
      unsigned char* p1 = IH1.GetRawData().get();
      unsigned char* p2 = IH2.GetRawData().get();
      unsigned char* p3 = IH3.GetRawData().get();

      unsigned char cr;
      unsigned char cg;
      unsigned char cb;
      unsigned char ca;

      for (int y=0;y<tilesize;y++)
      {
         for (int x=0;x<tilesize;x++)
         {
            size_t adr = 4*y*tilesize+4*x;

            if (y<tilesize/2)
            {
               if (x<tilesize/2)
               {
                  // A
                  if (p0)
                  {
                     int x0 = 2*x;
                     int y0 = 2*y; 
                     int x1 = x0+1;
                     int y1 = y0+1;

                     size_t tileadr0 = 4*y0*tilesize+4*x0;
                     size_t tileadr1 = 4*y0*tilesize+4*x1;
                     size_t tileadr2 = 4*y1*tilesize+4*x0;
                     size_t tileadr3 = 4*y1*tilesize+4*x1;

                     _getInterpolatedColor(p0, tileadr0, tileadr1, tileadr2, tileadr3, &cr, &cg, &cb, &ca);
                  }
                  else
                  {
                     cr = cg = cb = ca = 0;
                  }
               }
               else
               {
                  // B 
                  if (p1)
                  {
                     int x0 = 2*(x-tilesize/2);
                     int y0 = 2*y; 
                     int x1 = x0+1;
                     int y1 = y0+1;

                     size_t tileadr0 = 4*y0*tilesize+4*x0;
                     size_t tileadr1 = 4*y0*tilesize+4*x1;
                     size_t tileadr2 = 4*y1*tilesize+4*x0;
                     size_t tileadr3 = 4*y1*tilesize+4*x1;

                     _getInterpolatedColor(p1, tileadr0, tileadr1, tileadr2, tileadr3, &cr, &cg, &cb, &ca);
                  }
                  else
                  {
                     cr = cg = cb = ca = 0;
                  }
               }
            }
            else
            {
               if (x<tilesize/2)
               {
                  // C
                  if (p2)
                  {
                     int x0 = 2*x;
                     int y0 = 2*(y-tilesize/2); 
                     int x1 = x0+1;
                     int y1 = y0+1;

                     size_t tileadr0 = 4*y0*tilesize+4*x0;
                     size_t tileadr1 = 4*y0*tilesize+4*x1;
                     size_t tileadr2 = 4*y1*tilesize+4*x0;
                     size_t tileadr3 = 4*y1*tilesize+4*x1;

                     _getInterpolatedColor(p2, tileadr0, tileadr1, tileadr2, tileadr3, &cr, &cg, &cb, &ca);
                  }
                  else
                  {
                     cr = cg = cb = ca = 0;
                  }
               }
               else
               {
                  // D 
                  if (p3)
                  {
                     int x0 = 2*(x-tilesize/2); 
                     int y0 = 2*(y-tilesize/2); 
                     int x1 = x0+1;
                     int y1 = y0+1;

                     size_t tileadr0 = 4*y0*tilesize+4*x0;
                     size_t tileadr1 = 4*y0*tilesize+4*x1;
                     size_t tileadr2 = 4*y1*tilesize+4*x0;
                     size_t tileadr3 = 4*y1*tilesize+4*x1;

                     _getInterpolatedColor(p3, tileadr0, tileadr1, tileadr2, tileadr3, &cr, &cg, &cb, &ca);
                  }
                  else
                  {
                     cr = cg = cb = ca = 0;
                  }
               }
            }

            tile.tile[adr+0] = cr;
            tile.tile[adr+1] = cg;
            tile.tile[adr+2] = cb;
            tile.tile[adr+3] = ca;
         }
      }

      ImageWriter::WritePNG(sCurrentTile, tile.tile, tilesize, tilesize);
      }
      else
      {
         qQuadtree->QuadKeyToTileCoord(qcCurrent, _tx, _ty, tmp_lod);
         std::string sCurrentTile = ProcessingUtils::GetTilePath(sTileDir, ".raw" , tmp_lod, _tx, _ty);

         qQuadtree->QuadKeyToTileCoord(qc0, _tx, _ty, tmp_lod);
         std::string sTilefile0 = ProcessingUtils::GetTilePath(sTileDir, ".raw" , tmp_lod, _tx, _ty);
         

         qQuadtree->QuadKeyToTileCoord(qc1, _tx, _ty, tmp_lod);
         std::string sTilefile1 = ProcessingUtils::GetTilePath(sTileDir, ".raw" , tmp_lod, _tx, _ty);

         qQuadtree->QuadKeyToTileCoord(qc2, _tx, _ty, tmp_lod);
         std::string sTilefile2 = ProcessingUtils::GetTilePath(sTileDir, ".raw" , tmp_lod, _tx, _ty);

         qQuadtree->QuadKeyToTileCoord(qc3, _tx, _ty, tmp_lod);
         std::string sTilefile3 = ProcessingUtils::GetTilePath(sTileDir, ".raw" , tmp_lod, _tx, _ty);

         Raw32ImageObject IH0, IH1, IH2, IH3;

         bool b0 = ImageLoader::LoadRaw32FromDisk(sTilefile0, tilesize,tilesize, IH0);
         bool b1 = ImageLoader::LoadRaw32FromDisk(sTilefile1, tilesize,tilesize, IH1);
         bool b2 = ImageLoader::LoadRaw32FromDisk(sTilefile2, tilesize,tilesize, IH2);
         bool b3 = ImageLoader::LoadRaw32FromDisk(sTilefile3, tilesize,tilesize, IH3);
         _resampleRawImages(&IH0,&IH1,&IH2,&IH3,sCurrentTile,tilesize,b0,b1,b2,b3);
      }
}

//------------------------------------------------------------------------------
   void _resampleRawImages(Raw32ImageObject* IH0, Raw32ImageObject* IH1,Raw32ImageObject* IH2,Raw32ImageObject* IH3, std::string sTargetFile, int tilesize,bool b0, bool b1, bool b2, bool b3) 
   {

      float* p0 = IH0->GetRawData().get();
      float* p1 = IH1->GetRawData().get();
      float* p2 = IH2->GetRawData().get();
      float* p3 = IH3->GetRawData().get();
      
      
     
      boost::shared_array<float> sampleTile = boost::shared_array<float>(new float[tilesize*tilesize]);
      memset(sampleTile.get(),0,tilesize*tilesize*sizeof(float));

       
      for (int y=0;y<tilesize;y++)
      {
         for (int x=0;x<tilesize;x++)
         {
            float cvalue = -9999.0f;
            size_t adr = y*tilesize+x;

            if (y<tilesize/2)
            {
               if (x<tilesize/2)
               {
                  // A
                  if (b0)
                  {
                     int x0 = 2*x;
                     int y0 = 2*y; 
                     int x1 = x0+1;
                     int y1 = y0+1;

                     size_t tileadr0 = y0*tilesize+x0;
                     size_t tileadr1 = y0*tilesize+x1;
                     size_t tileadr2 = y1*tilesize+x0;
                     size_t tileadr3 = y1*tilesize+x1;

                     _getInterpolatedRawColor(p0, tileadr0, tileadr1, tileadr2, tileadr3, &cvalue);
                  }
                  else
                  {
                     cvalue = -9999.0f;
                  }
               }
               else
               {
                  // B 
                  if (b1)
                  {
                     int x0 = 2*(x-tilesize/2);
                     int y0 = 2*y; 
                     int x1 = x0+1;
                     int y1 = y0+1;

                     size_t tileadr0 = y0*tilesize+x0;
                     size_t tileadr1 = y0*tilesize+x1;
                     size_t tileadr2 = y1*tilesize+x0;
                     size_t tileadr3 = y1*tilesize+x1;

                     _getInterpolatedRawColor(p1, tileadr0, tileadr1, tileadr2, tileadr3, &cvalue);
                  }
                  else
                  {
                     cvalue = -9999.0f;
                  }
               }
            }
            else
            {
               if (x<tilesize/2)
               {
                  // C
                  if (b2)
                  {
                     int x0 = 2*x;
                     int y0 = 2*(y-tilesize/2); 
                     int x1 = x0+1;
                     int y1 = y0+1;

                     size_t tileadr0 = y0*tilesize+x0;
                     size_t tileadr1 = y0*tilesize+x1;
                     size_t tileadr2 = y1*tilesize+x0;
                     size_t tileadr3 = y1*tilesize+x1;

                     _getInterpolatedRawColor(p2, tileadr0, tileadr1, tileadr2, tileadr3, &cvalue);
                  }
                  else
                  {
                     cvalue = -9999.0f;
                  }
               }
               else
               {
                  // D 
                  if (b3)
                  {
                     int x0 = 2*(x-tilesize/2); 
                     int y0 = 2*(y-tilesize/2); 
                     int x1 = x0+1;
                     int y1 = y0+1;

                     size_t tileadr0 = y0*tilesize+x0;
                     size_t tileadr1 = y0*tilesize+x1;
                     size_t tileadr2 = y1*tilesize+x0;
                     size_t tileadr3 = y1*tilesize+x1;

                     _getInterpolatedRawColor(p3, tileadr0, tileadr1, tileadr2, tileadr3, &cvalue);
                  }
                  else
                  {
                     cvalue = -9999.0f;
                  }
               }
            }
           sampleTile[adr] = cvalue;
         }
      }
      ImageWriter::WriteRaw32(sTargetFile,tilesize, tilesize, sampleTile.get());
   }