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

#define AGEPI       3.1415926535897932384626433832795028841971693993751
#define SCALE       1.1920930376163765926810017443897e-7;


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
} GDALHillshadeAlgData;

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
        cang = 1.0 + (254.0 * cang);
        
    return (float)cang;
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
                               int bZevenbergenThorne)
{
    GDALHillshadeAlgData* pData =
        (GDALHillshadeAlgData*)CPLMalloc(sizeof(GDALHillshadeAlgData));
        
    
    const double degreesToRadians = AGEPI / 180.0;
    pData->nsres = adfGeoTransform[5];
    pData->ewres = adfGeoTransform[1];
    pData->sin_altRadians = sin(alt * degreesToRadians);
    pData->azRadians = az * degreesToRadians;
    double z_scale_factor = z / (((bZevenbergenThorne) ? 2 : 8) * scale);
    pData->cos_altRadians_mul_z_scale_factor =
        cos(alt * degreesToRadians) * z_scale_factor;
    pData->square_z_scale_factor = z_scale_factor * z_scale_factor;
    return pData;
}


// ------------------------------ Hillshade generate

struct HSProcessChunk
{
   Raw32ImageObject data;
   double dfXMin, dfXMax, dfYMin, dfYMax;
};

inline void process_hillshading(std::string filepath, HSProcessChunk pData, int x, int y, int zoom, double z_depth, int width = 256, int height = 256)
{
   int nXSize = pData.data.GetWidth();
   int nYSize = pData.data.GetHeight();
   int offsetX = (nXSize - width)/2;
   int offsetY = (nYSize - height)/2;

   double dem_z  = z_depth;
   double dem_azimut = 315;
   double dem_altitude = 45;
   double dem_scale = 1;

   boost::shared_array<unsigned char> vTile;

  /* vTile = boost::shared_array<unsigned char>(new unsigned char[nXSize*nYSize*4]);
   memset(vTile.get(),0,nXSize*nYSize*4);
   unsigned char* pTile = vTile.get();

   for(size_t dx = 0; dx < nXSize; dx++)
   {
      for(size_t dy = 0; dy < nYSize;dy++)
      {
            // Write PNG
         size_t adr=4*(dy)*nXSize+4*(dx);
         unsigned char scaledValue = (pData.data.GetValue(dx,dy)/500)*255; //math::Floor(value*255.0); 
         if (pTile[adr+3] == 0)
         {
            pTile[adr+0] = scaledValue;  
            pTile[adr+1] = scaledValue;  
            pTile[adr+2] = scaledValue; 
            pTile[adr+3] = 255;
         }
      }
   }
   */
   // create new tile memory and clear to fully transparent
   
   vTile = boost::shared_array<unsigned char>(new unsigned char[width*height*4]);
   memset(vTile.get(),0,width*height*4);

   unsigned char* pTile = vTile.get();
   
   for(size_t dx = offsetX; dx < (2*offsetX); dx++)
   {
      for(size_t dy = offsetY; dy < (2*offsetY);dy++)
      {
         float afWin[9];
         //      0 1 2
         //      3 4 5
         //      6 7 8
         afWin[0] = pData.data.GetValue(dx-1,dy-1)*SCALE;
         afWin[1] = pData.data.GetValue(dx,dy-1)*SCALE;
         afWin[2] = pData.data.GetValue(dx+1,dy-1)*SCALE;
         afWin[3] = pData.data.GetValue(dx-1,dy)*SCALE;
         // Hotspot
         afWin[4] = pData.data.GetValue(dx,dy)*SCALE;
         // -->
         afWin[5] = pData.data.GetValue(dx+1,dy)*SCALE;
         afWin[6] = pData.data.GetValue(dx-1,dy+1)*SCALE;
         afWin[7] = pData.data.GetValue(dx,dy+1)*SCALE;
         afWin[8] = pData.data.GetValue(dx+1,dy+1)*SCALE;

         double  adfGeoTransform[6];
         adfGeoTransform[0] = pData.dfXMin;                                              // top left x 
         adfGeoTransform[1] = fabs(pData.dfXMax -pData.dfXMin) / pData.data.GetWidth();  //w-e pixel resolution 
         adfGeoTransform[2] = 0;                                                        // rotation, 0 if image is "north up" 
         adfGeoTransform[3] = pData.dfYMax;                                              // top left y 
         adfGeoTransform[4] = 0;                                                         // rotation, 0 if image is "north up" 
         adfGeoTransform[5] = -fabs(pData.dfYMax -pData.dfYMin) / pData.data.GetHeight();// n-s pixel resolution 

         GDALHillshadeAlgData* pCalcObj = (GDALHillshadeAlgData*)GDALCreateHillshadeData(adfGeoTransform, dem_z,dem_scale,dem_altitude, dem_azimut,0);
         float value = GDALHillshadeAlg(afWin,0,pCalcObj);

         // Write PNG
         size_t adr=4*(dy-offsetY)*width+4*(dx-offsetX);
         unsigned char scaledValue = unsigned char(value); //(pData.data.GetValue(dx,dy)/500)*255; //math::Floor(value); 
         if (pTile[adr+3] == 0)
         {
            pTile[adr+0] = scaledValue;  
            pTile[adr+1] = scaledValue;  
            pTile[adr+2] = scaledValue; 
            pTile[adr+3] = 255;
         }
      }
   }
   std::stringstream tilepath;
   tilepath << filepath << "/" << zoom << "/" << x << "/" << y << ".png";
   ImageWriter::WritePNG(tilepath.str(), pTile, width, height);

}



#endif