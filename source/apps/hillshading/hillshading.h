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
// This is the triangulate version without mpi intended for regular 
// workstations. Multi cores are supported (OpenMP) and highly recommended.
// -----------------------------------------------------------------------------
// Contains some code from GDAL library file GDALDEM.CPP originally created by
// Matthew Perry, perrygeo at gmail.com
// Even Rouault, even dot rouault at mines dash paris dot org
// Howard Butler, hobu.inc at gmail.com
// Chris Yesson, chris dot yesson at ioz dot ac dot uk
//------------------------------------------------------------------------------
#ifndef _HILLSHADING_H
#define _HILLSHADING_H
#include <vector>
#include <string>
#include <string/FilenameUtils.h>
#include <string/StringUtils.h>
#include <image/ImageWriter.h>
#include "geo/MercatorQuadtree.h"
#include <io/FileSystem.h>
#include <gdal.h>
#include <gdalgrid.h>
#include <gdal_priv.h>
#include <ogr_spatialref.h>
#include "cpl_string.h"
#include "colorconversion.h"

#define AGEPI       3.1415926535897932384626433832795028841971693993751
#define SCALE       1 //1.1920930376163765926810017443897e-7
#define WGS84       6378137.0
#define MERC        20037508.34278924307658

// Move a 3x3 pafWindow over each cell 
// (where the cell in question is #4)
// 
//      0 1 2
//      3 4 5
//      6 7 8

/************************************************************************/
/*                         GDALHillshade()                              */
/************************************************************************/

typedef struct
{
    double nsres;
    double ewres;
    double sin_altRadians;
    double cos_altRadians_mul_z_scale_factor;
    double azRadians;
    double square_z_scale_factor;
    double slopeScale;
    int tileSize;
} GDALHillshadeAlgData;


inline float Normalize(vec3<float> &vec)
{
   float fLength = vec.Length();

   if (fLength > 1e-06f)
   {
      float fInvLength = ((float)1.0)/fLength;
      vec.Set(vec.x * fInvLength, vec.y * fInvLength, vec.z * fInvLength);
   }
   else
   {
      fLength = 0.0;
      vec.Set(0.0,0.0,0.0);
   }
   return fLength;
}


inline float GDALHillshadeAlg (float* afWin, float fDstNoDataValue, void* pData)
{
    GDALHillshadeAlgData* psData = (GDALHillshadeAlgData*)pData;
    double x, y, aspect, xx_plus_yy, cang;
    
    // First Slope ...
    x = ((afWin[0] + afWin[3] + afWin[3] + afWin[6]) -
        (afWin[2] + afWin[5] + afWin[5] + afWin[8])) / psData->ewres;

    y = ((afWin[6] + afWin[7] + afWin[7] + afWin[8]) -
        (afWin[0] + afWin[1] + afWin[1] + afWin[2])) / psData->nsres;

    xx_plus_yy = x * x + y * y;

    // ... then aspect...
    aspect = atan2(y,x);

    // ... then the shade value
    cang = (psData->sin_altRadians -
           psData->cos_altRadians_mul_z_scale_factor * sqrt(xx_plus_yy) *
           sin(aspect - psData->azRadians)) /
           sqrt(1 + psData->square_z_scale_factor * xx_plus_yy);

    if (cang <= 0.0) 
        cang = 1.0;
    else
        cang = 1.0 + (255.0 * cang);
        
    return (float)cang;
}

inline float GDALSlopeHornAlg (float* afWin, float fDstNoDataValue, void* pData, bool degreeMode = false)
{
    const double radiansToDegrees = 180.0 / AGEPI;
    GDALHillshadeAlgData* psData = (GDALHillshadeAlgData*)pData;
    double dx, dy, key;
    
    dx = ((afWin[0] + afWin[3] + afWin[3] + afWin[6]) - 
          (afWin[2] + afWin[5] + afWin[5] + afWin[8]))/psData->ewres;

    dy = ((afWin[6] + afWin[7] + afWin[7] + afWin[8]) - 
          (afWin[0] + afWin[1] + afWin[1] + afWin[2]))/psData->nsres;

    key = (dx * dx + dy * dy);

    if (degreeMode) 
        return (float) (atan(sqrt(key) / (8*psData->slopeScale)) * radiansToDegrees);
    else
        return (float) (100*(sqrt(key) / (8*psData->slopeScale)));
}

inline vec3<float> SobleOperator (float* afWin, double z_factor = 12000)
{

      // their intensities
      const double tl = afWin[0];
      const double t =  afWin[1];
      const double tr = afWin[2];
      const double r =  afWin[5];
      const double br = afWin[8];
      const double b =  afWin[7];
      const double bl = afWin[6];
      const double l =  afWin[3];

      // sobel filter
      const double dX = (tr + 2.0 * r + br) - (tl + 2.0 * l + bl);
      const double dY = (bl + 2.0 * b + br) - (tl + 2.0 * t + tr);
      const double dZ = 1.0 / z_factor;
      vec3<float> newVec(dX, dY, dZ);
      Normalize(newVec);
      return newVec;
}

inline float GDALHillshadeZevenbergenThorneAlg (float* afWin, float fDstNoDataValue, void* pData)
{
    GDALHillshadeAlgData* psData = (GDALHillshadeAlgData*)pData;
    double x, y, aspect, xx_plus_yy, cang;
    
    // First Slope ...
    x = (afWin[3] - afWin[5]) / psData->ewres;

    y = (afWin[7] - afWin[1]) / psData->nsres;

    xx_plus_yy = x * x + y * y;

    // ... then aspect...
    aspect = atan2(y,x);

    // ... then the shade value
    cang = (psData->sin_altRadians -
           psData->cos_altRadians_mul_z_scale_factor * sqrt(xx_plus_yy) *
           sin(aspect - psData->azRadians)) /
           sqrt(1 + psData->square_z_scale_factor * xx_plus_yy);

    if (cang <= 0.0) 
        cang = 1.0;
    else
        cang = 1.0 + (254.0 * cang);
        
    return (float) cang;
}

inline void*  GDALCreateHillshadeData(double* adfGeoTransform,
                               double z,
                               double scale,
                               double alt,
                               double az,
                               double slopeScale,
                               int bZevenbergenThorne, int tileSize)
{
    GDALHillshadeAlgData* pData =
        (GDALHillshadeAlgData*)CPLMalloc(sizeof(GDALHillshadeAlgData));
        
    
    const double degreesToRadians = AGEPI / 180.0;
    pData->nsres = adfGeoTransform[5];
    pData->ewres = adfGeoTransform[1];
    pData->tileSize = tileSize;
    pData->sin_altRadians = sin(alt * degreesToRadians);
    pData->azRadians = az * degreesToRadians;
    double z_scale_factor = z / (((bZevenbergenThorne) ? 2 : 8) * scale);
    pData->cos_altRadians_mul_z_scale_factor =
        cos(alt * degreesToRadians) * z_scale_factor;
    pData->square_z_scale_factor = z_scale_factor * z_scale_factor;
    pData->slopeScale = slopeScale;
    return pData;
}


// ------------------------------ Hillshade generate

struct HSProcessChunk
{
   Raw32ImageObject data;
   double dfXMin, dfXMax, dfYMin, dfYMax;
   int layerLod;
};
//---------------------------------------------------------------------------
inline void _ReadRawImageDataMem(float* buffer, int bufferwidth, int bufferheight, int x, int y, float* value)
{
   if (x<0) x = 0;
   if (y<0) y = 0;
   if (x>bufferwidth-1) x = bufferwidth-1;
   if (y>bufferheight-1) y = bufferheight-1;

   *value = buffer[bufferwidth*1*y+1*x];
}
//---------------------------------------------------------------------------
inline void _ReadRawImageValueBilinear(float* buffer, int bufferwidth, int bufferheight, double x, double y, float* value)
{
   double uf = math::Fract<double>(x);
   double vf = math::Fract<double>(y);
   int nPixelX = int(x);
   int nPixelY = int(y);

   int u00,v00,u10,v10,u01,v01,u11,v11;
   u00 = nPixelX;
   v00 = nPixelY;
   u10 = nPixelX+1;
   v10 = nPixelY;
   u01 = nPixelX;
   v01 = nPixelY+1;
   u11 = nPixelX+1;
   v11 = nPixelY+1;

   float value00;
   float value10;
   float value01;
   float value11;

   _ReadRawImageDataMem(buffer, bufferwidth, bufferheight, u00,v00,&value00);
   _ReadRawImageDataMem(buffer, bufferwidth, bufferheight, u10,v10,&value10);
   _ReadRawImageDataMem(buffer, bufferwidth, bufferheight, u01,v01,&value01);
   _ReadRawImageDataMem(buffer, bufferwidth, bufferheight, u11,v11,&value11);


   if (value00<-1000.0f || value10<-1000.0f || value01<-1000.0f || value11<-1000.0f)
   {
      *value = -9999.0f;
      return;
   }
   /*if(value00 == 0.0f)
   {
      *value = 0.0f;
      return;
   }*/

   double valued;

   valued = (double(value00)*(1-uf)*(1-vf)+double(value10)*uf*(1-vf)+double(value01)*(1-uf)*vf+double(value11)*uf*vf)+0.5;
      
   *value = (float) valued;
}

//---------------------------------------------------------------------------

inline void _ReadImageDataMem(unsigned char* buffer, int bufferwidth, int bufferheight, int x, int y, unsigned char* r, unsigned char* g, unsigned char* b, unsigned char* a)
{
   if (x<0) x = 0;
   if (y<0) y = 0;
   if (x>bufferwidth-1) x = bufferwidth-1;
   if (y>bufferheight-1) y = bufferheight-1;

   *r = buffer[bufferwidth*4*y+4*x];
   *g = buffer[bufferwidth*4*y+4*x+1];
   *b = buffer[bufferwidth*4*y+4*x+2];
   *a = buffer[bufferwidth*4*y+4*x+3];
}

//---------------------------------------------------------------------------

inline void _ReadImageValueBilinear(unsigned char* buffer, int bufferwidth, int bufferheight, double x, double y, unsigned char* r, unsigned char* g, unsigned char* b, unsigned char* a)
{
   double uf = math::Fract<double>(x);
   double vf = math::Fract<double>(y);
   int nPixelX = int(x);
   int nPixelY = int(y);

   int u00,v00,u10,v10,u01,v01,u11,v11;
   u00 = nPixelX;
   v00 = nPixelY;
   u10 = nPixelX+1;
   v10 = nPixelY;
   u01 = nPixelX;
   v01 = nPixelY+1;
   u11 = nPixelX+1;
   v11 = nPixelY+1;

   unsigned char r00,g00,b00,a00;
   unsigned char r10,g10,b10,a10;
   unsigned char r01,g01,b01,a01;
   unsigned char r11,g11,b11,a11;

   _ReadImageDataMem(buffer, bufferwidth, bufferheight, u00,v00,&r00,&g00,&b00,&a00);
   _ReadImageDataMem(buffer, bufferwidth, bufferheight, u10,v10,&r10,&g10,&b10,&a10);
   _ReadImageDataMem(buffer, bufferwidth, bufferheight, u01,v01,&r01,&g01,&b01,&a01);
   _ReadImageDataMem(buffer, bufferwidth, bufferheight, u11,v11,&r11,&g11,&b11,&a11);

   double rd, gd, bd, ad;

   rd = (double(r00)*(1-uf)*(1-vf)+double(r10)*uf*(1-vf)+double(r01)*(1-uf)*vf+double(r11)*uf*vf)+0.5;
   gd = (double(g00)*(1-uf)*(1-vf)+double(g10)*uf*(1-vf)+double(g01)*(1-uf)*vf+double(g11)*uf*vf)+0.5;
   bd = (double(b00)*(1-uf)*(1-vf)+double(b10)*uf*(1-vf)+double(b01)*(1-uf)*vf+double(b11)*uf*vf)+0.5;
   ad = (double(a00)*(1-uf)*(1-vf)+double(a10)*uf*(1-vf)+double(a01)*(1-uf)*vf+double(a11)*uf*vf)+0.5;

   rd = math::Clamp<double>(rd, 0.0, 255.0);
   gd = math::Clamp<double>(gd, 0.0, 255.0);
   bd = math::Clamp<double>(bd, 0.0, 255.0);
   ad = math::Clamp<double>(ad, 0.0, 255.0);

   *r = (unsigned char) rd;
   *g = (unsigned char) gd;
   *b = (unsigned char) bd;
   *a = (unsigned char) ad;
}


inline void process_hillshading(std::string filepath, HSProcessChunk pData, boost::shared_ptr<MercatorQuadtree> qQuadtree, int x, int y, int zoom, double z_depth, double azimut, double altitude, double scale, double slopeScale = 1,bool generateSlope = false, bool generateNormalMap = false, int width = 256, int height = 256, bool overrideTile = true, bool lockEnabled = false, bool bNoData = false, bool bJPEG = false, bool colored = false, bool textured = false, boost::shared_array<ImageObject> textures = boost::shared_array<ImageObject>())
{
   int nXSize = pData.data.GetWidth();
   int nYSize = pData.data.GetHeight();
   int offsetX = (nXSize - width)/2;
   int offsetY = (nYSize - height)/2;

   double dem_z  = z_depth;
   double dem_azimut = azimut; //315;
   double dem_altitude = altitude; //45;
   double dem_scale = scale;//1;

   
   boost::shared_array<float> vInputTile;
   vInputTile = boost::shared_array<float>(new float[nXSize*nYSize]);
   // ---- Gauss filtering
   float filter[5][5] =
   {   {0.0037,    0.0147,    0.0256,    0.0147,    0.0037},
       {0.0147,    0.0586,    0.0952,    0.0586,    0.0147},
       {0.0256,    0.0952,    0.1502,    0.0952,    0.0256},
       {0.0147,    0.0586,    0.0952,    0.0586,    0.0147},
       {0.0037,    0.0147,    0.0256,    0.0147,    0.0037} };
   for(size_t gx = 2; gx < nXSize-2; gx++)
   {
      for(size_t gy = 2; gy < nYSize-2; gy++)
      {
         float val = 0;
         for(size_t fx = 0; fx < 5; fx++)
         {
            for(size_t fy = 0; fy < 5; fy++)
            {
               val += filter[fx][fy]*pData.data.GetValue(gx-2+fx,gy-2+fy);
            }
         }
         vInputTile[gx+gy*nXSize] = val;
      }
   }

   // ---- HEADER OUT
   /*boost::shared_array<unsigned char> vTile1;
   vTile1 = boost::shared_array<unsigned char>(new unsigned char[nXSize*nYSize*4]);
   memset(vTile1.get(),0,nXSize*nYSize*4);
   unsigned char* pTile1 = vTile1.get();

   for(size_t dx = 0; dx < nXSize; dx++)
   {
      for(size_t dy = 0; dy < nYSize;dy++)
      {
            // Write PNG
         size_t adr3=4*(dy)*nXSize+4*(dx);
         unsigned char scaledValue = (vInputTile[dx+dy*nXSize]/1500)*255;//(pData.data.GetValue(dx,dy)/1500)*255; //math::Floor(value*255.0); 
         if (pTile1[adr3+3] == 0)
         {
            pTile1[adr3+0] = scaledValue;  
            pTile1[adr3+1] = scaledValue;  
            pTile1[adr3+2] = scaledValue; 
            pTile1[adr3+3] = 255;
         }
      }
   }
   std::stringstream rawPath;
    rawPath << filepath << "/" << zoom << "/" << x << "/" << y << "_raw.png";
    ImageWriter::WritePNG(rawPath.str(), pTile1, nXSize, nYSize);*/
  
   // create new tile memory and clear to fully transparent
   boost::shared_array<unsigned char> vTile;
   boost::shared_array<unsigned char> vPatternTile;
   std::stringstream tilepath;
   tilepath << filepath << zoom;
   if(!FileSystem::DirExists(tilepath.str()))
      FileSystem::makedir(tilepath.str());
   tilepath << "/" << x;
   if(!FileSystem::DirExists(tilepath.str()))
      FileSystem::makedir(tilepath.str());
   tilepath << "/" << y << (bJPEG ? ".jpg" : ".png");
   std::string bla = tilepath.str();
   vTile = boost::shared_array<unsigned char>(new unsigned char[width*height*4]);
   memset(vTile.get(),0,width*height*4);
   vPatternTile = boost::shared_array<unsigned char>(new unsigned char[width*height*4]);
   memset(vPatternTile.get(),0,width*height*4);
   unsigned char* pTile = vTile.get();
   unsigned char* pPatternTile = vPatternTile.get();

   if(!overrideTile && FileSystem::FileExists(tilepath.str()))
   {
      return;
   }
   else
   {
      // interpolation parameters
      double mx0,my0,mx1,my1;
      int64 x0,y0,x1,y1;
      std::string sCurrentQuadcode = qQuadtree->TileCoordToQuadkey(x,y,zoom);
      std::string sParentQuadcode = StringUtils::Left(sCurrentQuadcode,pData.layerLod);
      int64 parentX, parentY;
      int parentLod;
      MercatorQuadtree::QuadKeyToTileCoord(sParentQuadcode,parentX,parentY,parentLod);
      MercatorQuadtree::QuadKeyToMercatorCoord(sCurrentQuadcode,mx0,my0,mx1,my1);
      qQuadtree->MercatorToPixel(mx0,my0,pData.layerLod,x0,y0);
      qQuadtree->MercatorToPixel(mx1,my1,pData.layerLod,x1,y1);
      x0+=-parentX*256+offsetX+1;y0+=-parentY*256+offsetY;x1+=-parentX*256+offsetX+1;y1+=-parentY*256+offsetY;
      double deltaX = (x1-x0)/256.0;
      double deltaY = (y1-y0)/256.0;
      // --->
      for(size_t dx = offsetX; dx < (2*offsetX); dx++)
      {
         for(size_t dy = offsetY; dy < (2*offsetY);dy++)
         {
			   int ddx = dx;
			   int ddy = dy;
            float afWin[9];
            float ifWin[9];
            //      0 1 2
            //      3 4 5
            //      6 7 8 
            
            afWin[0] = vInputTile[(ddx-1)+(ddy-1)*nXSize]*SCALE;  ifWin[0] = afWin[0];
            afWin[1] = vInputTile[(ddx)+(ddy-1)*nXSize]*SCALE;    ifWin[1] = afWin[1];
            afWin[2] = vInputTile[(ddx+1)+(ddy-1)*nXSize]*SCALE;  ifWin[2] = afWin[2];
            afWin[3] = vInputTile[(ddx-1)+(ddy)*nXSize]*SCALE;    ifWin[3] = afWin[3];
            // Hotspot
            afWin[4] = vInputTile[(ddx)+(ddy)*nXSize]*SCALE;      ifWin[4] = afWin[4];
            // -
            afWin[5] = vInputTile[(ddx+1)+(ddy)*nXSize]*SCALE;    ifWin[5] = afWin[5];
            afWin[6] = vInputTile[(ddx-1)+(ddy+1)*nXSize]*SCALE;  ifWin[6] = afWin[6];
            afWin[7] = vInputTile[(ddx)+(ddy+1)*nXSize]*SCALE;    ifWin[7] = afWin[7];
            afWin[8] = vInputTile[(ddx+1)+(ddy+1)*nXSize]*SCALE;  ifWin[8] = afWin[8];
            if(zoom  > pData.layerLod)
            {
               float v0,v1,v2,v3,v4,v5,v6,v7,v8;
               double posX =  x0+(ddx-offsetX)*deltaX;
               double posY =  y0+(ddy-offsetY)*deltaY;
               _ReadRawImageValueBilinear(vInputTile.get(),nXSize,nYSize,posX-deltaX,posY-deltaY,&v0);
               _ReadRawImageValueBilinear(vInputTile.get(),nXSize,nYSize,posX,posY-deltaY,&v1);
               _ReadRawImageValueBilinear(vInputTile.get(),nXSize,nYSize,posX+deltaX,posY-deltaY,&v2);
               _ReadRawImageValueBilinear(vInputTile.get(),nXSize,nYSize,posX-deltaX,posY,&v3);

               _ReadRawImageValueBilinear(vInputTile.get(),nXSize,nYSize,posX,posY,&v4);

               _ReadRawImageValueBilinear(vInputTile.get(),nXSize,nYSize,posX+deltaX,posY,&v5);
               _ReadRawImageValueBilinear(vInputTile.get(),nXSize,nYSize,posX-deltaX,posY+deltaY,&v6);
               _ReadRawImageValueBilinear(vInputTile.get(),nXSize,nYSize,posX,posY+deltaY,&v7);
               _ReadRawImageValueBilinear(vInputTile.get(),nXSize,nYSize,posX+deltaX,posY+deltaY,&v8);
               ifWin[0] = v0*SCALE;ifWin[1] = v1*SCALE;ifWin[2] = v2*SCALE;
               ifWin[3] = v3*SCALE;ifWin[4] = v4*SCALE;ifWin[5] = v5*SCALE;
               ifWin[6] = v6*SCALE;ifWin[7] = v7*SCALE;ifWin[8] = v8*SCALE;
            }
            bool foundNData = false;
            // found no data value
            /*if( afWin[4] < -9000.0 && !bNoData)
            {
               foundNData = true;
            }*/
            for(size_t i = 0; i < 9; i++)
            {
                  if(afWin[i] < -1000.0f && !bNoData)
                  {
                     foundNData = true;
                     break;
                  }
            }
            
            if(generateNormalMap)
            {
               // Write FILE
               size_t adr=4*(dy-offsetY)*width+4*(dx-offsetX);
               vec3<float> value = SobleOperator(afWin, dem_z);
               if (pTile[adr+3] == 0)
               {
                  pTile[adr+0] = (unsigned char)((value.x + 1.0) * (255.0 / 2.0));
                  pTile[adr+1] = (unsigned char)((value.y + 1.0) * (255.0 / 2.0));  
                  pTile[adr+2] = (unsigned char)((value.z + 1.0) * (255.0 / 2.0)); 
                  pTile[adr+3] = foundNData ? 0.0 : 255;
               } 
            }
            else
            {
               double  adfGeoTransform[6];
               adfGeoTransform[0] = pData.dfXMin;                                             // top left x 
               adfGeoTransform[1] = fabs((pData.dfXMax*MERC) -(pData.dfXMin*MERC)) / pData.data.GetWidth();  //w-e pixel resolution 
               adfGeoTransform[2] = 0;                                                        // rotation, 0 if image is "north up" 
               adfGeoTransform[3] = pData.dfYMax;                                              // top left y 
               adfGeoTransform[4] = 0;                                                         // rotation, 0 if image is "north up" 
               adfGeoTransform[5] = -fabs((pData.dfYMax*MERC) -(pData.dfYMin*MERC)) / pData.data.GetHeight();// n-s pixel resolution 
			      double slopeValue = 0.0;

               GDALHillshadeAlgData* pCalcObj = (GDALHillshadeAlgData*)GDALCreateHillshadeData(adfGeoTransform, dem_z,dem_scale,dem_altitude, dem_azimut, slopeScale,1,width);
               float value= 0;
               if(generateSlope)
               {
                  value = GDALSlopeHornAlg(afWin,0,pCalcObj);
                  float hValue = GDALHillshadeZevenbergenThorneAlg(afWin,-9999.0,pCalcObj);
				      slopeValue=value/255;
                  value = (255-value)*0.8;
                  if(hValue < 180)
                  {
                     value -= (180-hValue)*0.5;
                     if(value < 0) 
                     {
                        value = 0.0;
                     }
                  }
                  else
                  {
                     value += (hValue-180);
                     if(value > 255) 
                     {
                        value = 255.0;
                     }
                  }
               }
               else
               {
                  value = GDALHillshadeZevenbergenThorneAlg(afWin,-9999.0,pCalcObj);// GDALHillshadeAlg(afWin,0,pCalcObj);
               }
               // Write PNG
               size_t adr=4*(dy-offsetY)*width+4*(dx-offsetX);
               unsigned char scaledValue = (unsigned char)value; //(pData.data.GetValue(dx,dy)/500)*255; //math::Floor(value); 
			      if(colored)
			      {
				      double scaledHeight = ifWin[4];
				      // COLORED
				      Color::hsv colHSV;
				      if(scaledHeight < 400)
				      {
					      colHSV.s = 0.45; colHSV.v = 0.5;//0.7*double(scaledValue)/255.0;
					      colHSV.h = 65+(65*(scaledHeight/400));
				      }
				      else if(scaledHeight < 2000)
				      {
					      colHSV.s = 0.45; colHSV.v = 0.5;// 0.7*double(scaledValue)/255.0;
					      colHSV.h = 130-(100*((scaledHeight-400)/1600));
				      }
				      else
				      {
					      colHSV.s = 0.45+(0.55*(scaledHeight/9000)); colHSV.v =0.5;// 0.7*double(scaledValue)/255.0;
					      colHSV.h = 30;
				      }
				      Color::rgb colRGB = Color::hsv2rgb(colHSV);
				      unsigned char sR = (unsigned char)((255.0)*colRGB.r);
				      unsigned char sG = (unsigned char)((255.0)*colRGB.g);
				      unsigned char sB = (unsigned char)((255.0)*colRGB.b);
				      if (pPatternTile[adr+3] == 0)
				      {
					     pPatternTile[adr+0] = foundNData ? 0 : sR; 
					     pPatternTile[adr+1] = foundNData ? 0 : sG; 
					     pPatternTile[adr+2] = foundNData ? 0 : sB;
					     pPatternTile[adr+3] = foundNData ? 0 : 255;
				      }
			      }
			      else if (textured)
			      {
                   ImageObject ground = textures[0];
                   ImageObject grass = textures[1];
				   /* Temp SNOW is 2 */
                   ImageObject snow = textures[3];
                   ImageObject rock = textures[3];
                   ImageObject desert = textures[4];
                   ImageObject water = textures[5];
				      // TEXTURED
				      double scaledHeight = ifWin[4];
				      Color::rgb colRGB;
				  int step1 = 200;
				  int step2 = 600;
				  int step3 = 1300;
				  int step4 = 2200;
                  /*if( ifWin[4] == 0.0f)
                  {
                     unsigned char r,g,b,a;
					      water.ReadPixel4((dx-offsetX),(dy-offsetY),r,g,b,a);
					      colRGB.r = double(r)/255.0;colRGB.g = double(g)/255.0;colRGB.b = double(b)/255.0;
                  }
                  else*/ if(scaledHeight <= step1)
				      {
					      Color::rgb col1;
					      Color::rgb col2;
					      unsigned char r,g,b,a;
					      desert.ReadPixel4((dx-offsetX),(dy-offsetY),r,g,b,a);
					      col1.r = double(r)/255.0;col1.g = double(g)/255.0;col1.b = double(b)/255.0;
					      ground.ReadPixel4((dx-offsetX),(dy-offsetY),r,g,b,a);
					      col2.r = double(r)/255.0;col2.g = double(g)/255.0;col2.b = double(b)/255.0;
					      colRGB = Color::overblendrgb(col1,col2,(scaledHeight)/step1);
				      }
				      else if(scaledHeight > step1 && scaledHeight <= step2)
				      {
					      Color::rgb col1;
					      Color::rgb col2;
					      unsigned char r,g,b,a;
					      ground.ReadPixel4((dx-offsetX),(dy-offsetY),r,g,b,a);
					      col1.r = double(r)/255.0;col1.g = double(g)/255.0;col1.b = double(b)/255.0;
					      grass.ReadPixel4((dx-offsetX),(dy-offsetY),r,g,b,a);
					      col2.r = double(r)/255.0;col2.g = double(g)/255.0;col2.b = double(b)/255.0;
					      colRGB = Color::overblendrgb(col1,col2,(scaledHeight-step1)/(step2-step1));
				      }
				      else if( scaledHeight <= step3)
				      {
					      unsigned char r,g,b,a;
					      grass.ReadPixel4((dx-offsetX),(dy-offsetY),r,g,b,a);
					      colRGB.r = double(r)/255.0;colRGB.g = double(g)/255.0;colRGB.b = double(b)/255.0;
				      }
				     else if(scaledHeight > step3 && scaledHeight <= step4)
				      {
					      Color::rgb col1;
					      Color::rgb col2;
					      unsigned char r,g,b,a;
					      grass.ReadPixel4((dx-offsetX),(dy-offsetY),r,g,b,a);
					      col1.r = double(r)/255.0;col1.g = double(g)/255.0;col1.b = double(b)/255.0;
					      snow.ReadPixel4((dx-offsetX),(dy-offsetY),r,g,b,a);
					      col2.r = double(r)/255.0;col2.g = double(g)/255.0;col2.b = double(b)/255.0;
					      colRGB = Color::overblendrgb(col1,col2,((scaledHeight-step3)/(step4-step3)));
				      }
				      else //if(scaledHeight > 1600)
				      {
					      unsigned char r,g,b,a;
					      snow.ReadPixel4((dx-offsetX),(dy-offsetY),r,g,b,a);
					      colRGB.r = double(r)/255.0;colRGB.g = double(g)/255.0;colRGB.b = double(b)/255.0;
				      }
				      // Rock Slopes
				      /*if(slopeValue <= 1.0 && slopeValue > 0.1)
				      {
					      Color::rgb slopeCol;
					      unsigned char r,g,b,a;
					      rock.ReadPixel4((dx-offsetX),(dy-offsetY),r,g,b,a);
					      slopeCol.r = double(r)/255.0;slopeCol.g = double(g)/255.0;slopeCol.b = double(b)/255.0;
                     double alpha = 0.0;
					      if(slopeValue < 0.25)
					      {
							   alpha = (slopeValue-0.1)/0.15;
					      } else
                     {
                        alpha = 1.0;
                     }
					      colRGB = Color::overblendrgb(colRGB,slopeCol,alpha);
				      }*/
				      // -->
				      /*Color::rgb tRGB; tRGB.r = colRGB.r;tRGB.g = colRGB.g;tRGB.b = colRGB.b;
				      if(tRGB.r < 0.86  ||  tRGB.g < 0.86 || tRGB.b < 0.86)
				      {
					      Color::hsv tHSV = Color::rgb2hsv(tRGB);
					      tHSV.v = tHSV.v-0.8*(1-double(scaledValue)/255.0);
                     // clamp
                     if(tHSV.v > 1.0) { tHSV.v = 1.0; }
                     if(tHSV.v < 0.0) { tHSV.v = 0.0; }
					      colRGB = Color::hsv2rgb(tHSV);
				      }
				      else
				      {
					      colRGB.r = tRGB.r-0.8*(1-double(scaledValue)/255.0);
					      colRGB.g = tRGB.g-0.8*(1-double(scaledValue)/255.0);
					      colRGB.b = tRGB.b-0.8*(1-double(scaledValue)/255.0);
				      }*/
                  // clamp
                  colRGB.r = colRGB.r < 0.0f ? 0: colRGB.r > 0.99999f ? 1.0 : colRGB.r;
                  colRGB.g = colRGB.g < 0.0f ? 0: colRGB.g > 0.99999f ? 1.0 : colRGB.g;
                  colRGB.b = colRGB.b < 0.0f ? 0: colRGB.b > 0.99999f ? 1.0 : colRGB.b;

				      // -->
				      unsigned char sR = (unsigned char)((255.0)*colRGB.r);
				      unsigned char sG = (unsigned char)((255.0)*colRGB.g);
				      unsigned char sB = (unsigned char)((255.0)*colRGB.b);
				      if (pPatternTile[adr+3] == 0)
				      {
					     pPatternTile[adr+0] = foundNData ? 0 : sR; 
					     pPatternTile[adr+1] = foundNData ? 0 : sG; 
					     pPatternTile[adr+2] = foundNData ? 0 : sB;
					     pPatternTile[adr+3] = foundNData ? 0 : 255;
				      }
			      }
				   if (pTile[adr+3] == 0)
				   {
					   pTile[adr+0] = foundNData ? 0 : scaledValue;  
					   pTile[adr+1] = foundNData ? 0 : scaledValue;  
					   pTile[adr+2] = foundNData ? 0 : scaledValue; 
					   pTile[adr+3] = foundNData ? 0 : 255;
				   }
               CPLFree(pCalcObj);
            }
         }
      }
      // scale up if necessary
      unsigned char * pTempTile = vTile.get();
      boost::shared_array<unsigned char> vInterpolatedTile;
      if(zoom  > pData.layerLod)
      {
         vInterpolatedTile = boost::shared_array<unsigned char>(new unsigned char[width*height*4]);
         for(size_t dx = 0; dx < width; dx++)
         {
            for(size_t dy = 0; dy < height; dy++)
            {
               unsigned char r;
               unsigned char g;
               unsigned char b;
               unsigned char a;
               double posX =  x0+(dx*deltaX)-offsetX;
               double posY =  y0+(dy*deltaY)-offsetY;
               _ReadImageValueBilinear(pTile,width,height,posX,posY, &r, &g, &b, &a);
               size_t adr = 4*dx+dy*width*4;
               vInterpolatedTile[adr+0] = r;
               vInterpolatedTile[adr+1] = g;
               vInterpolatedTile[adr+2] = b;
               vInterpolatedTile[adr+3] = a;
            }
         }
         pTempTile = vInterpolatedTile.get();
      }
      // Finalize Image
      if(textured || colored)
      {
         for(size_t dx = 0; dx < width; dx++)
         {
            for(size_t dy = 0; dy < height; dy++)
            {
               size_t adr = 4*dx+dy*width*4;
               pTempTile[adr+0] = math::Max<int>(0,pPatternTile[adr+0]-0.7*(255-double(pTempTile[adr+0])));  
				   pTempTile[adr+1] = math::Max<int>(0,pPatternTile[adr+1]-0.7*(255-double(pTempTile[adr+1])));  
				   pTempTile[adr+2] = math::Max<int>(0,pPatternTile[adr+2]-0.7*(255-double(pTempTile[adr+2])));  
               pTempTile[adr+3] = pPatternTile[adr+3];
            }
         }
      }

      // OUTPUT
      if(lockEnabled)
      {
         int lockhandle = FileSystem::Lock(tilepath.str());
         ImageWriter::WritePNG(tilepath.str(), pTempTile, width, height);
         if(bJPEG)
         {
            ImageObject img;
            img.AllocateImage(width,height, Img::PixelFormat_RGB);
            img.FillFromRGBA(pTempTile);
            bool success = ImageWriter::WriteJPG(tilepath.str() , img, 78);
            if(!success)
            {
               std::cout << tilepath.str() << " couldn't be written!\n";
            }
         }
         else
         {
            ImageWriter::WritePNG(tilepath.str(), pTempTile, width, height);
         }
         FileSystem::Unlock(tilepath.str(), lockhandle);
      }
      else
      {
          if(bJPEG)
         {
            ImageObject img;
            img.AllocateImage(width,height, Img::PixelFormat_RGB);
            img.FillFromRGBA(pTempTile);
            bool success = ImageWriter::WriteJPG(tilepath.str() , img, 78);
            if(!success)
            {
               std::cout << tilepath.str() << " couldn't be written!\n";
            }
         }
         else
         {
            ImageWriter::WritePNG(tilepath.str(), pTempTile, width, height);
         }
      }
   }
}



#endif