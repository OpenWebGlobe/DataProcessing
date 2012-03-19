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
};

inline void process_hillshading(std::string filepath, HSProcessChunk pData, int x, int y, int zoom, double z_depth, double azimut, double altitude, double scale, double slopeScale = 1,bool generateSlope = false, bool generateNormalMap = false, int width = 256, int height = 256, bool overrideTile = true, bool lockEnabled = false, bool bNoData = false, bool colored = false, bool textured = false, boost::shared_array<ImageObject> textures = boost::shared_array<ImageObject>())
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
    ImageWriter::WritePNG(rawPath.str(), pTile1, nXSize, nYSize);
  */
   // create new tile memory and clear to fully transparent
   boost::shared_array<unsigned char> vTile;
   std::stringstream tilepath;
   tilepath << filepath << "/" << zoom;
   if(!FileSystem::DirExists(tilepath.str()))
      FileSystem::makedir(tilepath.str());
   tilepath << "/" << x;
   if(!FileSystem::DirExists(tilepath.str()))
      FileSystem::makedir(tilepath.str());
   tilepath << "/" << y << ".png";
   vTile = boost::shared_array<unsigned char>(new unsigned char[width*height*4]);
   memset(vTile.get(),0,width*height*4);

   if(!overrideTile && FileSystem::FileExists(tilepath.str()))
   {
      return;
   }
   else
   {
      for(size_t dx = offsetX; dx < (2*offsetX); dx++)
      {
         for(size_t dy = offsetY; dy < (2*offsetY);dy++)
         {
			 int ddx = dx;
			 int ddy = dy;
            float afWin[9];
            //      0 1 2
            //      3 4 5
            //      6 7 8 
            afWin[0] = vInputTile[(ddx-1)+(ddy-1)*nXSize]*SCALE;
            afWin[1] = vInputTile[(ddx)+(ddy-1)*nXSize]*SCALE;
            afWin[2] = vInputTile[(ddx+1)+(ddy-1)*nXSize]*SCALE;
            afWin[3] = vInputTile[(ddx-1)+(ddy)*nXSize]*SCALE;
            // Hotspot
            afWin[4] = vInputTile[(ddx)+(dy)*nXSize]*SCALE;
            // --
            afWin[5] = vInputTile[(ddx+1)+(ddy)*nXSize]*SCALE;
            afWin[6] = vInputTile[(ddx-1)+(ddy+1)*nXSize]*SCALE;
            afWin[7] = vInputTile[(ddx)+(ddy+1)*nXSize]*SCALE;
            afWin[8] = vInputTile[(ddx+1)+(ddy+1)*nXSize]*SCALE;
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
            unsigned char* pTile = vTile.get();
            if(generateNormalMap)
            {
               // Write PNG
               size_t adr=4*(dy-offsetY)*width+4*(dx-offsetX);
               vec3<float> value = SobleOperator(afWin, dem_z);
               if (pTile[adr+3] == 0)
               {
                  pTile[adr+0] = unsigned char((value.x + 1.0) * (255.0 / 2.0));

                  pTile[adr+1] = unsigned char((value.y + 1.0) * (255.0 / 2.0));  
                  pTile[adr+2] = unsigned char((value.z + 1.0) * (255.0 / 2.0)); 
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

               GDALHillshadeAlgData* pCalcObj = (GDALHillshadeAlgData*)GDALCreateHillshadeData(adfGeoTransform, dem_z,dem_scale,dem_altitude, dem_azimut, slopeScale,0,width);
               float value= 0;
               if(generateSlope)
               {
                  value = GDALSlopeHornAlg(afWin,0,pCalcObj);
                  float hValue = GDALHillshadeAlg(afWin,0,pCalcObj);
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
                  value = GDALHillshadeAlg(afWin,0,pCalcObj);
               }
               // Write PNG
               size_t adr=4*(dy-offsetY)*width+4*(dx-offsetX);
               unsigned char scaledValue = unsigned char(value); //(pData.data.GetValue(dx,dy)/500)*255; //math::Floor(value); 
			   if(colored)
			   {
				   double scaledHeight = afWin[4];
				   // COLORED
				   Color::hsv colHSV;
				   if(scaledHeight < 400)
				   {
					   colHSV.s = 0.45; colHSV.v = 0.7*double(scaledValue)/255.0;
					   colHSV.h = 65+(65*(scaledHeight/400));
				   }
				   else if(scaledHeight < 2000)
				   {
					   colHSV.s = 0.45; colHSV.v = 0.7*double(scaledValue)/255.0;
					   colHSV.h = 130-(100*((scaledHeight-400)/1600));
				   }
				   else
				   {
					   colHSV.s = 0.45+(0.55*(scaledHeight/9000)); colHSV.v = 0.7*double(scaledValue)/255.0;
					   colHSV.h = 30;
				   }
				   Color::rgb colRGB = Color::hsv2rgb(colHSV);
				   unsigned char sR = unsigned char((255.0)*colRGB.r);
				   unsigned char sG = unsigned char((255.0)*colRGB.g);
				   unsigned char sB = unsigned char((255.0)*colRGB.b);
				   if (pTile[adr+3] == 0)
				   {
					  pTile[adr+0] = sR; 
					  pTile[adr+1] = sG; 
					  pTile[adr+2] = sB;
					  pTile[adr+3] = foundNData ? 0 : 255;
				   }
			   }
			   else if (textured)
			   {
                ImageObject ground = textures[0];
                ImageObject grass = textures[1];
                ImageObject snow = textures[2];
                ImageObject rock = textures[3];
                ImageObject desert = textures[4];
                ImageObject water = textures[5];
				   // TEXTURED
				   double scaledHeight = afWin[4];
				   Color::rgb colRGB;
               if( afWin[4] == 0.0f)
               {
                  unsigned char r,g,b,a;
					   water.ReadPixel4((dx-offsetX),(dy-offsetY),r,g,b,a);
					   colRGB.r = double(r)/255.0;colRGB.g = double(g)/255.0;colRGB.b = double(b)/255.0;
               }
               else if(scaledHeight <= 300)
				   {
					   Color::rgb col1;
					   Color::rgb col2;
					   unsigned char r,g,b,a;
					   desert.ReadPixel4((dx-offsetX),(dy-offsetY),r,g,b,a);
					   col1.r = double(r)/255.0;col1.g = double(g)/255.0;col1.b = double(b)/255.0;
					   ground.ReadPixel4((dx-offsetX),(dy-offsetY),r,g,b,a);
					   col2.r = double(r)/255.0;col2.g = double(g)/255.0;col2.b = double(b)/255.0;
					   colRGB = Color::overblendrgb(col1,col2,(scaledHeight)/300);
				   }
				   else if(scaledHeight > 300 && scaledHeight <= 400)
				   {
					   Color::rgb col1;
					   Color::rgb col2;
					   unsigned char r,g,b,a;
					   ground.ReadPixel4((dx-offsetX),(dy-offsetY),r,g,b,a);
					   col1.r = double(r)/255.0;col1.g = double(g)/255.0;col1.b = double(b)/255.0;
					   grass.ReadPixel4((dx-offsetX),(dy-offsetY),r,g,b,a);
					   col2.r = double(r)/255.0;col2.g = double(g)/255.0;col2.b = double(b)/255.0;
					   colRGB = Color::overblendrgb(col1,col2,(scaledHeight-300)/100);
				   }
				   else if( scaledHeight <= 1400)
				   {
					   unsigned char r,g,b,a;
					   grass.ReadPixel4((dx-offsetX),(dy-offsetY),r,g,b,a);
					   colRGB.r = double(r)/255.0;colRGB.g = double(g)/255.0;colRGB.b = double(b)/255.0;
				   }
				  else if(scaledHeight > 1400 && scaledHeight <= 1600)
				   {
					   Color::rgb col1;
					   Color::rgb col2;
					   unsigned char r,g,b,a;
					   grass.ReadPixel4((dx-offsetX),(dy-offsetY),r,g,b,a);
					   col1.r = double(r)/255.0;col1.g = double(g)/255.0;col1.b = double(b)/255.0;
					   snow.ReadPixel4((dx-offsetX),(dy-offsetY),r,g,b,a);
					   col2.r = double(r)/255.0;col2.g = double(g)/255.0;col2.b = double(b)/255.0;
					   colRGB = Color::overblendrgb(col1,col2,((scaledHeight-1400)/200));
				   }
				   else //if(scaledHeight > 1600)
				   {
					   unsigned char r,g,b,a;
					   snow.ReadPixel4((dx-offsetX),(dy-offsetY),r,g,b,a);
					   colRGB.r = double(r)/255.0;colRGB.g = double(g)/255.0;colRGB.b = double(b)/255.0;
				   }
				   // Rock Slopes
				   if(slopeValue <= 1.0 && slopeValue > 0.1)
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
				   }
				   // -->
				   Color::rgb tRGB; tRGB.r = colRGB.r;tRGB.g = colRGB.g;tRGB.b = colRGB.b;
				   if(tRGB.r < 0.86  ||  tRGB.g < 0.86 || tRGB.b < 0.86)
				   {
					   Color::hsv tHSV = Color::rgb2hsv(tRGB);
					   tHSV.v = tHSV.v-0.8*(1-double(scaledValue)/255.0);
                  /* clamp */
                  if(tHSV.v > 1.0) { tHSV.v = 1.0; }
                  if(tHSV.v < 0.0) { tHSV.v = 0.0; }
					   colRGB = Color::hsv2rgb(tHSV);
				   }
				   else
				   {
					   colRGB.r = tRGB.r-0.8*(1-double(scaledValue)/255.0);
					   colRGB.g = tRGB.g-0.8*(1-double(scaledValue)/255.0);
					   colRGB.b = tRGB.b-0.8*(1-double(scaledValue)/255.0);
				   }
               // clamp
               colRGB.r = colRGB.r < 0.0f ? 0: colRGB.r > 0.99999f ? 1.0 : colRGB.r;
               colRGB.g = colRGB.g < 0.0f ? 0: colRGB.g > 0.99999f ? 1.0 : colRGB.g;
               colRGB.b = colRGB.b < 0.0f ? 0: colRGB.b > 0.99999f ? 1.0 : colRGB.b;

				   // -->
				   unsigned char sR = unsigned char((255.0)*colRGB.r);
				   unsigned char sG = unsigned char((255.0)*colRGB.g);
				   unsigned char sB = unsigned char((255.0)*colRGB.b);
				   if (pTile[adr+3] == 0)
				   {
					  pTile[adr+0] = sR; 
					  pTile[adr+1] = sG; 
					  pTile[adr+2] = sB;
					  pTile[adr+3] = foundNData ? 0 : 255;
				   }
			   }
			   else
			   {
				   if (pTile[adr+3] == 0)
				   {
					  pTile[adr+0] = scaledValue;  
					  pTile[adr+1] = scaledValue;  
					  pTile[adr+2] = scaledValue; 
					  pTile[adr+3] = foundNData ? 0 : 255;
				   }
			   }
               CPLFree(pCalcObj);
            }
         }
      }
      // OUTPUT
      if(lockEnabled)
      {
         int lockhandle = FileSystem::Lock(tilepath.str());
         ImageWriter::WritePNG(tilepath.str(), vTile.get(), width, height);
         FileSystem::Unlock(tilepath.str(), lockhandle);
      }
      else
      {
          ImageWriter::WritePNG(tilepath.str(), vTile.get(), width, height);
      }
   }
}



#endif