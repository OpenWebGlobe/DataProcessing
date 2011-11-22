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
#include <io/FileSystem.h>
#include <gdal.h>
#include <gdalgrid.h>
#include <gdal_priv.h>
#include <ogr_spatialref.h>
#include "cpl_string.h"

struct HSProcessChunk
{
   std::vector<double> adfX;
   std::vector<double> adfY;
   std::vector<double> adfElevation;
   double dfXMin, dfXMax, dfYMin, dfYMax;
   GDALGridAlgorithm eAlgorithm;
};

inline void process_hillshading(HSProcessChunk pData, int x, int y, int zoom, int z_depth, int width = 256, int height = 256, void *pOptions = NULL)
{
   int nXSize = width*3;
   int nYSize = height*3;
   const int dataSize = nXSize*nYSize;
   char            **papszCreateOptions = NULL;
   std::stringstream ss;
   ss << "tile_" << y;

   const char *pszFormat = "GTiff";
   GDALDriver *poDriver;
   poDriver = GetGDALDriverManager()->GetDriverByName(pszFormat);
   GDALDataset *poDstDS;       
   poDstDS = poDriver->Create("bla.tif", nXSize, nYSize, 1, GDALDataType::GDT_Float64, papszCreateOptions );
   GDALRasterBand *poBand;
   void  *abyRaster = CPLMalloc( nXSize * nYSize * GDALGetDataTypeSize(GDALDataType::GDT_Float64));

   OGRSpatialReference oSRS;
   oSRS.SetWellKnownGeogCS( "EPSG:3857" );
   char *pszSRS_WKT = NULL;
   oSRS.exportToWkt( &pszSRS_WKT );
   
   
   double  adfGeoTransform[6];
   adfGeoTransform[0] = pData.dfXMin;
   adfGeoTransform[1] = (pData.dfXMax - pData.dfXMin) / nXSize;
   adfGeoTransform[2] = 0.0;
   adfGeoTransform[3] = pData.dfYMin;
   adfGeoTransform[4] = 0.0;
   adfGeoTransform[5] = (pData.dfYMax -pData.dfYMin) / nYSize;

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
   int     nBlockXSize, nBlockYSize;

   GDALGridCreate( pData.eAlgorithm, pOptions,
                            pData.adfX.size(), &(pData.adfX[0]), &(pData.adfY[0]), &(pData.adfElevation[0]),
                            pData.dfXMin,
                            pData.dfXMax,
                            pData.dfYMin,
                            pData.dfYMax,
                            nXSize, nYSize, GDALDataType::GDT_Float64, abyRaster,
                            NULL, NULL);
   
   /*
    poBand->GetBlockSize(&nBlockXSize, &nBlockYSize);

    GUInt32 nBlock = 0;
    GUInt32 nBlockCount = ((nXSize + nBlockXSize - 1) / nBlockXSize)
        * ((nYSize + nBlockYSize - 1) / nBlockYSize);
    GUInt32 nXOffset, nYOffset;
    // Rasterize Points
    for ( nYOffset = 0; nYOffset < nYSize; nYOffset += nBlockYSize )
    {
        for ( nXOffset = 0; nXOffset < nXSize; nXOffset += nBlockXSize )
        {
            void *pScaledProgress;
            pScaledProgress =
                GDALCreateScaledProgress( 0.0,
                                          (double)++nBlock / nBlockCount,
                                          NULL, NULL );

            int nXRequest = nBlockXSize;
            if (nXOffset + nXRequest > nXSize)
                nXRequest = nXSize - nXOffset;

            int nYRequest = nBlockYSize;
            if (nYOffset + nYRequest > nYSize)
                nYRequest = nYSize - nYOffset;

            GDALGridCreate( pData.eAlgorithm, pOptions,
                            pData.adfX.size(), &(pData.adfX[0]), &(pData.adfY[0]), &(pData.adfElevation[0]),
                            pData.dfXMin + dfDeltaX * nXOffset,
                            pData.dfXMin + dfDeltaX * (nXOffset + nXRequest),
                            pData.dfYMin + dfDeltaY * nYOffset,
                            pData.dfYMin + dfDeltaY * (nYOffset + nYRequest),
                            nXRequest, nYRequest, GDALDataType::GDT_Float64, abyRaster,
                            GDALScaledProgress, pScaledProgress );

           poBand->RasterIO( GF_Write, nXOffset, nYOffset, nXRequest, nYRequest, 
                     abyRaster, nXRequest, nYRequest, GDALDataType::GDT_Float64, 0, 0 );  
            /*GDALRasterIO( hBand, GF_Write, nXOffset, nYOffset,
                          nXRequest, nYRequest, ppData,
                          nXRequest, nYRequest, GDALDataType::GDT_Float64, 0, 0 );
                          */
      /*      GDALDestroyScaledProgress( pScaledProgress );
        }
    }
    */
    // generate hillshading

   CPLFree(abyRaster);
   CSLDestroy( papszCreateOptions );
   GDALClose( (GDALDatasetH) poDstDS );
}

#endif