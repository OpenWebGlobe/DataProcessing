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

#ifndef _POINTCLOUD_READER_H
#define _POINTCLOUD_READER_H

#include "og.h"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include "math/CloudPoint.h"

enum PointCloudType
{
   PCT_INVALID,   // invalid data!
   PCT_XYZ,       // xyz only
   PCT_XYZI,      // xyz with intensity
   PCT_XYZRGB,    // xyz, rgb
   PCT_XYZIRGB,   // xyz, intensity, rgb
};

//-----------------------------------------------------------------------------

//! \class PointCloudReader
//! \author Martin Christen, martin.christen@fhnw.ch
class OPENGLOBE_API PointCloudReader
{
public:
   PointCloudReader();
   virtual ~PointCloudReader();

   //! Open (new) File. You can use multiple files and the results adds up to the extent/center/...
   bool Open(const std::string& sFilename, int nSourceEPSG=0);
   
   //! Close File   
   void Close();

   // Reads next point (normalized mercator coordinates + orthometric elevation). Returns false when all points are read.
   bool ReadPoint(CloudPoint& point);
 
   //! Get Point Cloud type (valid after reading first point)
   PointCloudType GetPointCloudtype(){return _pct;}


private:
   std::ifstream  _ifstream;
   int            _nSourceEPSG;
   std::string    _sFilenameA;
   PointCloudType _pct;
   std::string    _line;
   std::string    _separators;
   size_t         _ptsread;

};

#endif