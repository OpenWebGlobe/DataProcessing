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

#ifndef _ELEVATIONTILE_H
#define _ELEVATIONTILE_H

#include "og.h"
#include "math/ElevationPoint.h"
#include "math/delaunay/DelaunayTriangulation.h"
#include "math/vec2.h"
#include "math/vec3.h"

#include <string>

#include <boost/shared_ptr.hpp>

class OPENGLOBE_API ElevationTile
{
public:
   ElevationTile(const std::string& sQuadcode, double x0, double y0, double x1, double y1);
   virtual ~ElevationTile();

   // Setup from catagorized data
   void Setup(ElevationPoint& NW,               // Point at (x0,y1)
         ElevationPoint& NE,                    // Point at (x1,y1)
         ElevationPoint& SE,                    // Point at (x1, y0) 
         ElevationPoint& SW,                    // Point at (x0, y0)
         std::vector<ElevationPoint>& vNorth,   // Points at north edge
         std::vector<ElevationPoint>& vEast,    // Points at east edge
         std::vector<ElevationPoint>& vSouth,   // Points at south edge
         std::vector<ElevationPoint>& vWest,    // Points at west edge
         std::vector<ElevationPoint>& vMiddle); // points inside rectangle

   // retrieve number of points of this tile (including corners and edges!)
   int GetNumPoints();

   // thin out points so that we have numPoints or less.
   void Reduce(int numPoints);

   // create OBJ tile:
   std::string CreateOBJ(double xmin, double ymin, double xmax, double ymax);

   // create JSON tile:
   std::string CreateJSON();

   // write tile binary
   void WriteBinary(const std::string& sTempfilename);

   // read tile binary
   void ReadBinary(const std::string& sTimefilename);

   boost::shared_ptr<math::DelaunayTriangulation> CreateTriangulation();

protected:
   void _Sort();
   void _Classify(std::vector<ElevationPoint>& pts);
   
   void _PrecomputeTriangulation(bool bCurtain);
   void _CreateCurtain(double curtainelv, ElevationPoint& start, ElevationPoint& end, std::vector<ElevationPoint>& between,  int& idxA, int& idxB, int& idxC, int& idxD);

   ElevationPoint                _NW, _NE, _SE, _SW;
   std::vector<ElevationPoint>   _ptsNorth;
   std::vector<ElevationPoint>   _ptsEast;
   std::vector<ElevationPoint>   _ptsSouth; 
   std::vector<ElevationPoint>   _ptsWest;
   std::vector<ElevationPoint>   _ptsMiddle;
   std::string                   _sQuadcode;
   double                        _x0, _y0, _x1, _y1;
   bool                          _bCategorized;

   // for export:
   int _idxcurtain;
   std::vector< vec3<float> >    _lstElevationPointWGS84;  // Elevation Points (Cartesian WGS84 minus offset)
   std::vector< vec2<float> >    _lstTexCoord;             //
   std::vector<int>              _lstIndices;              // Indices
   vec3<double>                  _vOffset;                 // Proposed offset for virtual camera
   vec3<double>                  _bbmin;            // min coord for bounding box
   vec3<double>                  _bbmax;            // max coord for bounding box

private:

};


#endif

