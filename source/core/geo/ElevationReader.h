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
/*
   Utility class to read various Elevation Data Formats and transform them
   into another projection (for example WebMercator).
*/


#ifndef _ELEVATION_READER_H
#define _ELEVATION_READER_H

#include "og.h"
#include <string>
#include <vector>
#include "math/ElevationPoint.h"

//! \class ElevationReader
class OPENGLOBE_API ElevationReader
{
public:
   // ctor
   ElevationReader();
   
   // dtor
   virtual ~ElevationReader();

   // Open dataset. DestESPG default is WebMercator.
   bool Open(std::string& sFilename, int nSourceEPSG=0, int nDestEPSG=3785);

   // Close File   
   void Close();

   // Transform Elevation points and write points to binary stream. Returns points written and bounding box
   bool Import(std::vector<ElevationPoint>& result, double& inout_xmin, double& inout_ymin, double& inout_xmax, double& inout_ymax);

 
protected:
   // importer for xyz, xyzw (xyz+weight)
   bool _ImportXYZ(std::vector<ElevationPoint>& result, double& inout_xmin, double& inout_ymin, double& inout_xmax, double& inout_ymax);
   bool _ImportRaster(std::vector<ElevationPoint>& result, double& inout_xmin, double& inout_ymin, double& inout_xmax, double& inout_ymax);

   void _Free();
   void _ReadBlock(int bx, int by, int& valid_width, int& valid_height);

   inline void GetSourcePixel(double x_src, double y_src, double* x, double* y)
   {
      *x = (_affineTransformation_inverse[0] + x_src * _affineTransformation_inverse[1] + y_src * _affineTransformation_inverse[2]);
      *y = (_affineTransformation_inverse[3] + x_src * _affineTransformation_inverse[4] + y_src * _affineTransformation_inverse[5]);
   }

   inline void GetSourceCoord(double x, double y, double* xs, double* ys)
   {
      *ys = _affineTransformation[3] + x*_affineTransformation[4] + y*_affineTransformation[5];
      *xs = _affineTransformation[0] + x*_affineTransformation[1] + y*_affineTransformation[2];
   }

private:
   int            _nSourceEPSG;
   int            _nDestEPSG;
   int            _nBlockWidth, _nBlockHeight;
   int            _nXBlocks, _nYBlocks;
   unsigned int   _nRasterSizeX;
   unsigned int   _nRasterSizeY;

   void*          _pDataset;
   void*          _pElv;
   void*          _pBlockElv;
   double         _dNoDataValue;
   int            _datatype;
   int            _datatype_bytes;

   double         _affineTransformation[6];
   double         _affineTransformation_inverse[6];
   double         _ulx, _uly, _lrx, _lry;

   bool           _bImportXYZ;
   std::string    _sFilename;
};

#endif

