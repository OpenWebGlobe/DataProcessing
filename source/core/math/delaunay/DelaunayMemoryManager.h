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

#ifndef _DELAUNAY_MEMORY_MANAGER_H
#define _DELAUNAY_MEMORY_MANAGER_H


#include "og.h"
#include "math/ElevationPoint.h"
#include "DelaunayVertex.h"
#include "DelaunayTriangle.h"


// DMM_MEMORY_DEBUG prints memory information to std::cout
// be warned, this slows down everything!
//#define DMM_MEMORY_DEBUG

class OPENGLOBE_API DelaunayMemoryManager
{
public:
   static math::DelaunayVertex* AllocVertex(double x, double y, double elevation = 0.0, double weight = 0.0);
   static math::DelaunayVertex* AllocVertex(const ElevationPoint& pt);
   static math::DelaunayTriangle* AllocTriangle();

   static void Free(math::DelaunayVertex* v);
   static void Free(math::DelaunayTriangle* t);

   static int GetNumTriangles() {return _nTrianglesCount;}
   static int GetNumVertices(){return _nVerticesCount;}

   static double GetMemory(); // return occupied memory in MB

   static void DumpMemoryInfo();
   static void DumpMemoryInfoShort();

protected:
   static unsigned int _nTrianglesCount;
   static unsigned int _nVerticesCount;
};


#endif