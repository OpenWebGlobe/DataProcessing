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
// Contains some code from GDAL library gdal_grid.cpp originally created by
// Andrey Kiselev <dron@ak4719.spb.edu>
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

/* Unoptimized formulas are :
    x = psData->z*((afWin[0] + afWin[3] + afWin[3] + afWin[6]) -
        (afWin[2] + afWin[5] + afWin[5] + afWin[8])) /
        (8.0 * psData->ewres * psData->scale);

    y = psData->z*((afWin[6] + afWin[7] + afWin[7] + afWin[8]) -
        (afWin[0] + afWin[1] + afWin[1] + afWin[2])) /
        (8.0 * psData->nsres * psData->scale);

    slope = M_PI / 2 - atan(sqrt(x*x + y*y));

    aspect = atan2(y,x);

    cang = sin(alt * degreesToRadians) * sin(slope) +
           cos(alt * degreesToRadians) * cos(slope) *
           cos(az * degreesToRadians - M_PI/2 - aspect);
*/

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

inline void process_hillshading(std::string filepath, HSProcessChunk pData, int x, int y, int zoom, int z_depth, int width = 256, int height = 256)
{
   int nXSize = pData.data.GetWidth();
   int nYSize = pData.data.GetHeight();
   int offsetX = nXSize - width;
   int offsetY = nYSize - height;

   boost::shared_array<unsigned char> vTile;
   // create new tile memory and clear to fully transparent
   vTile = boost::shared_array<unsigned char>(new unsigned char[width*height*4]);
   memset(vTile.get(),0,width*height*4);

   unsigned char* pTile = vTile.get();

   for(size_t x = 1; x < nXSize-1; x++)
   {
      for(size_t y = 1; y < nYSize-1;x++)
      {
         float afWin[9];
         //      0 1 2
         //      3 4 5
         //      6 7 8
         afWin[0] = pData.data.GetValue(x-1,y-1);
         afWin[1] = pData.data.GetValue(x-1,y-1);
         afWin[2] = pData.data.GetValue(x-1,y-1);
         afWin[3] = pData.data.GetValue(x-1,y-1);
         // Hotspot
         afWin[4] = pData.data.GetValue(x,y);
         // -->
         afWin[5] = pData.data.GetValue(x-1,y-1);
         afWin[6] = pData.data.GetValue(x-1,y-1);
         afWin[7] = pData.data.GetValue(x-1,y-1);
         afWin[8] = pData.data.GetValue(x-1,y-1);

         double  adfGeoTransform[6];
         adfGeoTransform[0] = 0;
         adfGeoTransform[1] = 1;
         adfGeoTransform[2] = 0;
         adfGeoTransform[3] = 0;
         adfGeoTransform[4] = 0;
         adfGeoTransform[5] = 1;

         GDALHillshadeAlgData* pCalcObj = (GDALHillshadeAlgData*)GDALCreateHillshadeData(adfGeoTransform, 5,0,0, 1);
         float value = GDALHillshadeAlg(afWin,-9999,pCalcObj);

         // Write PNG
         if(x >= offsetX && x < (nXSize - offsetX) && y >= offsetY && x < (nYSize - offsetY))
         {
            size_t adr=4*(y-offsetY)*width+4*(x-offsetX);
            unsigned char scaledValue = math::Floor(value*255.0);
            if (pTile[adr+3] == 0)
            {
               pTile[adr+0] = scaledValue;  
               pTile[adr+1] = scaledValue;  
               pTile[adr+2] = scaledValue; 
               pTile[adr+3] = 255;
            }
         }
      }
   }
   std::stringstring tilepath;
   tilepath << filepath << "/" << zoom << "/" << y << "/" << x;
   ImageWriter::WritePNG(tilepath.str(), pTile, width, height);

  /* char            **papszCreateOptions = NULL;
   std::stringstream ss;
   ss << "tile_" << y << ".tif";
   void  *abyRaster = CPLMalloc( nXSize * nYSize * GDALGetDataTypeSize(GDALDataType::GDT_Float64));

   const char *pszFormat = "GTiff";
   GDALDriver *poDriver;
   poDriver = GetGDALDriverManager()->GetDriverByName(pszFormat);
   GDALDataset *poDstDS;       
   poDstDS = poDriver->Create(ss.str().c_str(), nXSize, nYSize, 1, GDALDataType::GDT_Float64, papszCreateOptions );
   GDALRasterBand *poBand;
   

   OGRSpatialReference oSRS;
   oSRS.SetWellKnownGeogCS( "EPSG:3857" );
   char *pszSRS_WKT = NULL;
   oSRS.exportToWkt( &pszSRS_WKT );
   
   
   double  adfGeoTransform[6];
   adfGeoTransform[0] = 0;
   adfGeoTransform[1] = 1;
   adfGeoTransform[2] = 0;
   adfGeoTransform[3] = 0;
   adfGeoTransform[4] = 0;
   adfGeoTransform[5] = 1;

   poDstDS->SetGeoTransform( adfGeoTransform );
    
   oSRS.SetUTM( 11, TRUE );
   poDstDS->SetProjection( pszSRS_WKT );
   CPLFree( pszSRS_WKT );

   poBand = poDstDS->GetRasterBand(1);
     
   if (pData.adfX.size() == 0)
    {
        // FIXME: Shoulda' set to nodata value instead
      //poBand->Fill(0.0,0.0);
      CPLFree(abyRaster);
      CSLDestroy( papszCreateOptions );
      GDALClose(poDstDS);
      return;
    }
   // generate GDAL Dataset
   const double    dfDeltaX = (pData.dfXMax - pData.dfXMin ) / nXSize;
   const double    dfDeltaY = (pData.dfYMax - pData.dfYMin ) / nYSize;


   GDALGridAlgorithm algo = GGA_NearestNeighbor;
   GDALGridNearestNeighborOptions oOptions;
   
   oOptions.dfRadius1 = 0.0;
   oOptions.dfRadius2 = 0.0;
   oOptions.dfAngle = 0.0;

  
   GDALGridCreate( algo, (void*)&oOptions,
                            pData.adfX.size(), &(pData.adfX[0]), &(pData.adfY[0]), &(pData.adfElevation[0]),
                            pData.dfXMin,
                            pData.dfXMax,
                            pData.dfYMin,
                            pData.dfYMax,
                            nXSize, nYSize, GDALDataType::GDT_Float64, abyRaster,
                            GDALTermProgress, NULL);
   
   
    poBand->GetBlockSize(&nBlockXSize, &nBlockYSize);

    GUInt32 nBlock = 0;
    GUInt32 nBlockCount = ((nXSize + nBlockXSize - 1) / nBlockXSize)
        * ((nYSize + nBlockYSize - 1) / nBlockYSize);

    poBand->RasterIO( GF_Write, 0, 0, nXSize, nYSize, 
                     abyRaster, nXSize, nYSize, GDALDataType::GDT_Float64, 1, 0 ); 
    GUInt32 nXOffset, nYOffset;
    int     nBlockXSize, nBlockYSize;
    poBand->GetBlockSize(&nBlockXSize, &nBlockYSize);
    void    *ayRaster =
       CPLMalloc( nBlockXSize * nBlockYSize * GDALGetDataTypeSize(GDALDataType::GDT_Float64) );

    GUInt32 nBlock = 0;
    GUInt32 nBlockCount = ((nXSize + nBlockXSize - 1) / nBlockXSize)
        * ((nYSize + nBlockYSize - 1) / nBlockYSize);

    for ( nYOffset = 0; nYOffset < nYSize; nYOffset += nBlockYSize )
    {
        for ( nXOffset = 0; nXOffset < nXSize; nXOffset += nBlockXSize )
        {
            int nXRequest = nBlockXSize;
            if (nXOffset + nXRequest > nXSize)
                nXRequest = nXSize - nXOffset;

            int nYRequest = nBlockYSize;
            if (nYOffset + nYRequest > nYSize)
                nYRequest = nYSize - nYOffset;

            GDALGridCreate( algo, (void*)&oOptions,
                            pData.adfX.size(), &(pData.adfX[0]), &(pData.adfY[0]), &(pData.adfElevation[0]),
                            pData.dfXMin + dfDeltaX * nXOffset,
                            pData.dfXMin + dfDeltaX * (nXOffset + nXRequest),
                            pData.dfYMin + dfDeltaY * nYOffset,
                            pData.dfYMin + dfDeltaY * (nYOffset + nYRequest),
                            nXRequest, nYRequest, GDALDataType::GDT_Float64, ayRaster,
                            GDALTermProgress, NULL );

            GDALRasterIO( poBand, GF_Write, nXOffset, nYOffset,
                          nXRequest, nYRequest, ayRaster,
                          nXRequest, nYRequest, GDALDataType::GDT_Float64, 0, 0 );

        }
    }

   CPLFree( ayRaster );
   CPLFree(abyRaster);
  // CSLDestroy( papszCreateOptions );
   GDALClose( (GDALDatasetH) poDstDS );*/
}



#endif