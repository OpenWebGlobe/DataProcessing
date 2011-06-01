
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
void _resampleFromParent( TileBlock* pTileBlockArray, boost::shared_ptr<MercatorQuadtree> qQuadtree, int64 x, int64 y,int nLevelOfDetail, std::string sTileDir ) 
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

   qQuadtree->QuadKeyToTileCoord(qcCurrent, _tx, _ty, tmp_lod);
   std::string sCurrentTile = GetTilePath(sTileDir, ".png" , tmp_lod, _tx, _ty);

   qQuadtree->QuadKeyToTileCoord(qc0, _tx, _ty, tmp_lod);
   std::string sTilefile0 = GetTilePath(sTileDir, ".png" , tmp_lod, _tx, _ty);

   qQuadtree->QuadKeyToTileCoord(qc1, _tx, _ty, tmp_lod);
   std::string sTilefile1 = GetTilePath(sTileDir, ".png" , tmp_lod, _tx, _ty);

   qQuadtree->QuadKeyToTileCoord(qc2, _tx, _ty, tmp_lod);
   std::string sTilefile2 = GetTilePath(sTileDir, ".png" , tmp_lod, _tx, _ty);

   qQuadtree->QuadKeyToTileCoord(qc3, _tx, _ty, tmp_lod);
   std::string sTilefile3 = GetTilePath(sTileDir, ".png" , tmp_lod, _tx, _ty);

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

//------------------------------------------------------------------------------
