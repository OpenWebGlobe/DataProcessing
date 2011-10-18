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

#include "google_projection.h"
#include <math/mathutils.h>

#define AGEPI       3.1415926535897932384626433832795028841971693993751
#define AGEHALFPI   1.5707963267948966192313216916395
#define RAD_2_DEG 180/AGEPI
#define DEG_2_RAD  AGEPI/180.0


#define WGS84_a            6378137.0          
#define WGS84_b            6356752.314245     
#define WGS84_F_INV        298.257223563  
#define WGS84_E_SQUARED    0.006694379990197 
#define WGS84_E            0.081819190842961775161887117288255
#define WGS84_E_SQUARED2   0.006739496742
#define WGS84_RN_POLE      6.399593625758673e+006

GoogleProjection::GoogleProjection(int levels)
{
    int c = 256;
    for (int d = 0; d <= levels; d++)
	{
		double e = c/2;
		_Bc.push_back(c/360.0);
        _Cc.push_back(c/(2 * AGEPI));
        _zc.push_back(ituple(e,e));
        _Ac.push_back(c);
        c *= 2;
	}
}

GoogleProjection::~GoogleProjection()
{
}

dtuple GoogleProjection::pixel2GeoCoord(ituple p, int zoom)
{
	ituple e = _zc[zoom];
	double f = (p.a - e.a)/_Bc[zoom];
	double g = (p.b - e.b)/-_Cc[zoom];
	
	double expg = exp(g);
	double atang = atan(expg);

	double h = RAD_2_DEG*( 2 * atan(exp(g)) - 0.5 * AGEPI);
	return dtuple(f,h);
}

ituple GoogleProjection::geoCoord2Pixel(dtuple c, int zoom)
{
	ituple d = _zc[zoom];
	double e = d.a + c.a * _Bc[zoom];
	math::Round<double>(e,0);
	double f = math::Clip(sin(DEG_2_RAD*(c.b)),-0.9999,0.9999);

	double g = d.b + 0.5*log((1+f)/(1-f))*-_Cc[zoom];
	math::Round(g, 0);
	return ituple(int(e),int(g));
}