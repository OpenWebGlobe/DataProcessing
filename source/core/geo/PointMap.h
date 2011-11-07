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

#ifndef _POINTMAP_H
#define _POINTMAP_H

#include "og.h"
#include "math/CloudPoint.h"
#include "io/FileSystem.h"

#include <map>
#include <list>
#include <set>
#include <string>
#include <sstream>
#include <fstream>



//------------------------------------------------------------------------
class PointMap_private;

class OPENGLOBE_API PointMap
{
public:
  
   // ctor
   PointMap(int levelofdetail);

   // dtor
   virtual ~PointMap();
   
   // clear all point data (free mem). Index is not cleared.
   void Clear();

   // add a point
   void AddPoint(int64 i, int64 j, int64 k, const CloudPoint& pt);
   
   // get number of points
   size_t GetNumPoints();

   // export data
   void ExportData(const std::string& path);

   // export index list to file
   void ExportIndex(const std::string& sFilename);
   

private:
   PointMap(){}
   PointMap_private* _pPriv;
   std::map<int64, std::list<CloudPoint> > _map;
   size_t _numpts;
   size_t _numkeys;
   int _lod;
   int64 _pow;
   int64 _dpow;
};


#endif