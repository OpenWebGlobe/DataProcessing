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
//------------------------------------------------------------------------------
// Code adapted from: generate_tiles.py
// Found at: http://trac.openstreetmap.org/browser/applications/rendering/mapnik
//------------------------------------------------------------------------------
#ifndef _GOOGLE_PROJECTION_H
#define _GOOGLE_PROJECTION_H

#include <vector>

struct dtuple
{
   dtuple(double first, double second) { a = first; b= second; }
   double a;
   double b;
};

struct ituple
{
   ituple(int first, int second) { a = first; b= second; }
   int a;
   int b;
};

class GoogleProjection
{
public:
   GoogleProjection();
   GoogleProjection(int levels);
   virtual ~GoogleProjection();
   ituple geoCoord2Pixel(dtuple c, int zoom);
   dtuple pixel2GeoCoord(ituple p, int zoom);
private:
   std::vector<double> _Bc;
   std::vector<double> _Cc;
   std::vector<ituple> _zc;
   std::vector<double> _Ac;
};

#endif