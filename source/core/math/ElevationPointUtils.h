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

#ifndef _ELEVATIONPOINT_UTILS_H
#define _ELEVATIONPOINT_UTILS_H 

#include "math/ElevationPoint.h"
#include <float.h>
#include <vector>

namespace math
{ 
   //--------------------------------------------------------------------------
   inline bool PointInPolygon(const ElevationPoint& point, const std::vector<ElevationPoint>& polygon)
   {
      size_t i, j;
      bool c = false;
      for (i = 0, j = polygon.size()-1; i < polygon.size(); j = i++) 
      {
         if ( ((polygon[i].y>=point.y) != (polygon[j].y>=point.y)) &&
            (point.x <= (polygon[j].x-polygon[i].x) * (point.y-polygon[i].y) / (polygon[j].y-polygon[i].y) + polygon[i].x) )
            c = !c;
      }
      return c;
   }

   //--------------------------------------------------------------------------
}

#endif
