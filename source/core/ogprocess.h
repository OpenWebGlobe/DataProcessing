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

#ifndef _OG_PROCESS_H
#define _OG_PROCESS_H

#include "og.h"
#include "app/Logger.h"
#include "app/ProcessingSettings.h"
#include "geo/CoordinateTransformation.h"
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <limits>
#include <string>
#include <cmath>
#include <sstream>


struct DataSetInfo
{
   double      dest_ulx;
   double      dest_lry;
   double      dest_lrx;
   double      dest_uly;
   double      affineTransformation[6];
   double      affineTransformation_inverse[6];
   double      pixelsize;
   int         nBands;
   int         nSizeX;
   int         nSizeY;
   bool        bGood;     // true if this data is valid, otherwise couldn't load or failed some way
   std::string sFilename;
};

namespace ProcessingUtils
{
   inline std::string GetTilePath(const std::string& sBaseTilePath, const std::string& sExtension, int lod, int64 tx, int64 ty)
   {
      std::ostringstream oss;
      oss << sBaseTilePath << lod << "/" << tx << "/" << ty << sExtension;
      return oss.str();
   }
   //---------------------------------------------------------------------------
   inline bool InvertGeoMatrix(double* mGeoMatrix, double* mInvGeoMatrix)
   {
      double det, inv_det;
      det = mGeoMatrix[1] * mGeoMatrix[5] - mGeoMatrix[2] * mGeoMatrix[4];

      if( fabs(det) < std::numeric_limits<double>::epsilon() )
         return false;

      inv_det = 1.0 / det;

      mInvGeoMatrix[1] =  mGeoMatrix[5] * inv_det;
      mInvGeoMatrix[4] = -mGeoMatrix[4] * inv_det;
      mInvGeoMatrix[2] = -mGeoMatrix[2] * inv_det;
      mInvGeoMatrix[5] =  mGeoMatrix[1] * inv_det;
      mInvGeoMatrix[0] = ( mGeoMatrix[2] * mGeoMatrix[3] - mGeoMatrix[0] * mGeoMatrix[5]) * inv_det;
      mInvGeoMatrix[3] = (-mGeoMatrix[1] * mGeoMatrix[3] + mGeoMatrix[0] * mGeoMatrix[4]) * inv_det;

      return true;
   }
   //---------------------------------------------------------------------------
   // initialize gdal
   OPENGLOBE_API bool init_gdal();

   //---------------------------------------------------------------------------
   // clean up gdal
   OPENGLOBE_API void exit_gdal();
   //---------------------------------------------------------------------------

   OPENGLOBE_API void RetrieveDatasetInfo(const std::string& filename, CoordinateTransformation* pCT, DataSetInfo* pDataset, bool bVerbose=false);
   //---------------------------------------------------------------------------

   OPENGLOBE_API boost::shared_ptr<ProcessingSettings> LoadAppSettings();

   //---------------------------------------------------------------------------

   OPENGLOBE_API boost::shared_ptr<Logger> CreateLogger(const std::string& appname, boost::shared_ptr<ProcessingSettings> qSettings);

   //---------------------------------------------------------------------------
   // Load image with 3 channels to RGB.
   OPENGLOBE_API boost::shared_array<unsigned char> ImageToMemoryRGB(const DataSetInfo& oDataset);


   //---------------------------------------------------------------------------


   


}


#endif
