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

#include "ElevationTile.h"
#include "math/GeoCoord.h"
#include "geo/CoordinateTransformation.h"
#include <sstream>
#include <iostream>

//------------------------------------------------------------------------------

// Eliminate Point that is close (epsilon) to rect (x0,y0,x1,y1)
inline void EliminateCloseToCorner(double x0, double y0, double x1, double y1, std::vector<ElevationPoint>& PointList, const double epsilon)
{
   std::vector<ElevationPoint>::iterator it = PointList.begin();

   bool advance = true;

   while (it!= PointList.end())
   {
      double dx0 = fabs(x0 - (*it).x);
      double dy0 = fabs(y0 - (*it).y);
      double dx1 = fabs(x1 - (*it).x);
      double dy1 = fabs(y1 - (*it).y);

      if (dx0 < epsilon || dy0 < epsilon || dx1 < epsilon || dy1 < epsilon)
      {

         it = PointList.erase(it);
      }
      else
      {
         it++;
      }
   }
}

//-----------------------------------------------------------------------------
// for performance reasons this only works on a sorted list!
inline void EliminateDoubleEntriesX(std::vector<ElevationPoint>& PointList, const double epsilon)
{
   std::vector<ElevationPoint>::iterator it = PointList.begin();

   bool advance = true;
   ElevationPoint current;

   while (it!= PointList.end())
   {
      if (advance)
      {
         current = *it;
         advance = false;
         it++;
      }

      if (it!= PointList.end())
      {
         double dx = fabs((*it).x - current.x);

         if (dx < epsilon)
         {
            it = PointList.erase(it);
         }
         else
         {
            advance = true;
         }
      }
   }
}


// for performance reasons this only works on a sorted list!
inline void EliminateDoubleEntriesY(std::vector<ElevationPoint>& PointList, const double epsilon)
{
   std::vector<ElevationPoint>::iterator it = PointList.begin();

   bool advance = true;
   ElevationPoint current;

   while (it!= PointList.end())
   {
      if (advance)
      {
         current = *it;
         advance = false;
         it++;
      }

      if (it!= PointList.end())
      {
         double dy = fabs((*it).y - current.y);

         if (dy < epsilon)
         {
            it = PointList.erase(it);
         }
         else
         {
            advance = true;
         }
      }
   }
}


//------------------------------------------------------------------------------

ElevationTile::ElevationTile(const std::string& sQuadcode, double x0, double y0, double x1, double y1)
{
   _bCategorized = false;
   _x0 = x0;
   _y0 = y0;
   _x1 = x1;
   _y1 = y1;
}

//------------------------------------------------------------------------------

ElevationTile::~ElevationTile()
{

}

//------------------------------------------------------------------------------

void ElevationTile::Setup(ElevationPoint& NW,  
         ElevationPoint& NE, 
         ElevationPoint& SE, 
         ElevationPoint& SW, 
         std::vector<ElevationPoint>& vNorth,
         std::vector<ElevationPoint>& vEast, 
         std::vector<ElevationPoint>& vSouth, 
         std::vector<ElevationPoint>& vWest,  
         std::vector<ElevationPoint>& vMiddle)
{
   _NW = NW;
   _NE = NE;
   _SE = SE;
   _SW = SW;
   _ptsNorth = vNorth;
   _ptsEast = vEast;
   _ptsSouth = vSouth;
   _ptsWest = vWest;
   _ptsMiddle = vMiddle;
   _bCategorized = true;

   _x0 = _SW.x;
   _y0 = _SW.y;
   _x1 = _NE.x;
   _y1 = _NE.y;
}

//------------------------------------------------------------------------------

int ElevationTile::GetNumPoints()
{
   size_t nPoints = 4 + _ptsNorth.size() + _ptsEast.size() + _ptsSouth.size() + _ptsWest.size() + _ptsMiddle.size();
   return (int)nPoints;
}

//------------------------------------------------------------------------------

void ElevationTile::Reduce(int numPoints)
{
   int n = GetNumPoints();
   if (n > numPoints)
   {
      std::vector<ElevationPoint> points;
      // basic reduce alrorithm:
      // 1) Create Triangulation
      // 2) loop: { Thin out Triangulation until we reach numPoints }
      // 3) Re-Categorize Points (also: remove edge points that are not on edge!)

      boost::shared_ptr<math::DelaunayTriangulation> qTriangulation;
      qTriangulation = this->CreateTriangulation();
      qTriangulation->GetPointVec(points);
      size_t n2 = points.size(); points.clear();

      if ((int)n2 > numPoints)
      {
         qTriangulation->Reduce(n2-numPoints);
         qTriangulation->GetPointVec(points);
         this->_Classify(points);
      }
      else
      {
         qTriangulation->GetPointVec(points);
         this->_Classify(points);
      }
   }
}

//------------------------------------------------------------------------------

std::string ElevationTile::CreateOBJ(double xmin, double ymin, double xmax, double ymax)
{
   std::string res;

   // 1) Create Triangulation (simple one)
   // 2) Create OBJ (simply call triangulation->CreateOBJ(...);

   boost::shared_ptr<math::DelaunayTriangulation> qTriangulation;
   qTriangulation = this->CreateTriangulation();

   if (qTriangulation)
   {
      res = qTriangulation->CreateOBJ(xmin, ymin, xmax, ymax);
   }

   return res;
}

//------------------------------------------------------------------------------

std::string ElevationTile::CreateJSON()
{
   // 1) Create Triangulation (with curtain)
   // 2) Export JSON Tile (according to OpenWebGlobe specification)

   _PrecomputeTriangulation(true); // this calculates: _idxcurtain; _lstElevationPointWGS84; _lstTexCoord; _lstIndices; _vOffset; _bbmin; _bbmax;

   std::ostringstream of;
   of.precision(FLT_DIG);  // floating point precision is required vertices


   of << "{\n";
   of << "   \"VertexSemantic\"  :  \"pt\",\n";
   of << "   \"Vertices\" : [ ";

   for (size_t i=0;i<_lstElevationPointWGS84.size();i++)
   {
      of << _lstElevationPointWGS84[i].x << ", ";
      of << _lstElevationPointWGS84[i].y << ", ";
      of << _lstElevationPointWGS84[i].z << ", ";
      of << _lstTexCoord[i].x << ", ";
      of << _lstTexCoord[i].y;
      if (i!= _lstElevationPointWGS84.size()-1)
      {
         of << ", ";
      }
   }
   of << " ],\n";
   of << "   \"IndexSemantic\"  :  \"TRIANGLES\",\n";
   of << "   \"Indices\"  : [ ";

   for (size_t i=0;i<_lstIndices.size();i++)
   {
      of << _lstIndices[i];
      if (i != _lstIndices.size()-1)
      {
         of << ", ";
      }
   }

   of << "],\n";

   // virtual camera offset and bounding box (must be stored in double precision!!)
   of.precision(DBL_DIG);
   of << "   \"Offset\"  :  [ " << _vOffset.x << ", " << _vOffset.y << ", " << _vOffset.z << "],\n";  
      
   of << "   \"BoundingBox\" : [[ " << _bbmin.x << ", " << _bbmin.y << ", " << _bbmin.z << " ],[ "
                                       << _bbmax.x << ", " << _bbmax.y << ", " << _bbmax.z << " ]],\n";
   of.precision(FLT_DIG);
   of << "   \"CurtainIndex\" : " << _idxcurtain << "\n";
   of << "}\n";   

   return of.str();
}

//------------------------------------------------------------------------------

boost::shared_ptr<math::DelaunayTriangulation> ElevationTile::CreateTriangulation()
{
   boost::shared_ptr<math::DelaunayTriangulation> qTriangulation;

   double eps = fabs(_x1 - _x0);
   assert(eps>0);

   qTriangulation = boost::shared_ptr<math::DelaunayTriangulation>(new math::DelaunayTriangulation(_x0-eps, _y0-eps, _x1+eps, _y1+eps));
   qTriangulation->SetEpsilon(DBL_EPSILON);

   // Sort Points (prevents bad triangles on very low level of detail)
   this->_Sort();

   // (1) Insert corner points
   qTriangulation->InsertPoint(_SW);
   qTriangulation->InsertPoint(_NW);
   qTriangulation->InsertPoint(_NE);
   qTriangulation->InsertPoint(_SE);

   // (2) Insert (sorted) Edge Points in this order: N,E,S,W
   for (size_t i=0;i<_ptsNorth.size();i++)
   {
      qTriangulation->InsertPoint(_ptsNorth[i]);
   }

   for (size_t i=0;i<_ptsEast.size();i++)
   {
      qTriangulation->InsertPoint(_ptsEast[i]);
   }

   for (size_t i=0;i<_ptsSouth.size();i++)
   {
      qTriangulation->InsertPoint(_ptsSouth[i]);
   }

   for (size_t i=0;i<_ptsWest.size();i++)
   {
      qTriangulation->InsertPoint(_ptsWest[i]);
   }

   for (size_t i=0;i<_ptsMiddle.size();i++)
   {
      qTriangulation->InsertPoint(_ptsMiddle[i]);
   }
   
   return qTriangulation;
}

//------------------------------------------------------------------------------

template <class T>
void sortpoints_et(std::vector<T>& sites)
{
   int nsites = (int)sites.size();

   for (int gap = nsites/2; gap > 0; gap /= 2)
   {
      for (int i = gap; i < nsites; i++)
      {
         for (int j = i-gap; 
            j >= 0 && (sites[j].x != sites[j+gap].x ? (sites[j].x > sites[j+gap].x) : (sites[j].y > sites[j+gap].y));j -= gap)

         {
            std::swap(sites[j], sites[j+gap]);
         }
      }
   }
}
//------------------------------------------------------------------------------

void ElevationTile::_Sort()
{
   sortpoints_et<ElevationPoint>(_ptsNorth);
   sortpoints_et<ElevationPoint>(_ptsEast);
   sortpoints_et<ElevationPoint>(_ptsSouth);
   sortpoints_et<ElevationPoint>(_ptsWest);
   sortpoints_et<ElevationPoint>(_ptsMiddle);
}

//------------------------------------------------------------------------------

namespace math
{
   template<typename T>
   inline bool flteq(T x, T y)
   {
      const double eps = 1e-12; 
      if (x == T(0)) return y == T(0);
      else return std::fabs(x-y) < eps*std::fabs(x);
   }
}

//------------------------------------------------------------------------------

void ElevationTile::_Classify(std::vector<ElevationPoint>& pts)
{
   const double eps = 1e-12;

   _bCategorized = true;

   _ptsNorth.clear();
   _ptsEast.clear();
   _ptsSouth.clear(); 
   _ptsWest.clear();
   _ptsMiddle.clear();

   for (size_t i=0;i<pts.size();i++)
   {
      // 1) is this a corner point?
      if (math::flteq<double>(_x0, pts[i].x) && math::flteq<double>(_y0, pts[i].y)) // SW-Corner
      {
         _SW.x = _x0; _SW.y = _y0;
         _SW.elevation = pts[i].elevation;
         _SW.weight = -3;
      }
      else if (math::flteq<double>(_x1, pts[i].x) && math::flteq<double>(_y0, pts[i].y)) // SE-Corner
      {
         _SE.x = _x1; _SE.y = _y0;
         _SE.elevation = pts[i].elevation;
         _SE.weight = -3;
      }
      else if (math::flteq<double>(_x1, pts[i].x) && math::flteq<double>(_y1, pts[i].y)) // NE-Corner
      {
         _NE.x = _x1; _NE.y = _y1;
         _NE.elevation = pts[i].elevation;
         _NE.weight = -3;
      }
      else if (math::flteq<double>(_x0, pts[i].x) && math::flteq<double>(_y1, pts[i].y)) // NW-Corner
      {
         _NW.x = _x0; _NW.y = _y1;
         _NW.elevation = pts[i].elevation;
         _NW.weight = -3;
      }
      else if (math::flteq<double>(_x0, pts[i].x)) // WEST BORDER
      {
         // west border!
         ElevationPoint pt;
         pt.weight = -2;
         pt.x = _x0;
         pt.y = pts[i].y;
         pt.elevation = pts[i].elevation;
         _ptsWest.push_back(pt);
      }
      else if (math::flteq<double>(_x1, pts[i].x)) // EAST BORDER
      {
         // west border!
         ElevationPoint pt;
         pt.weight = -2;
         pt.x = _x1;
         pt.y = pts[i].y;
         pt.elevation = pts[i].elevation;
         _ptsEast.push_back(pt);
      }
      else if (math::flteq<double>(_y0, pts[i].y)) // SOUTH BORDER
      {
         // west border!
         ElevationPoint pt;
         pt.weight = -2;
         pt.x = pts[i].x;
         pt.y = _y0;
         pt.elevation = pts[i].elevation;
         _ptsSouth.push_back(pt);
      }
      else if (math::flteq<double>(_y1, pts[i].y)) // NORTH BORDER
      {
         // west border!
         ElevationPoint pt;
         pt.weight = -2;
         pt.x = pts[i].x;
         pt.y = _y1;
         pt.elevation = pts[i].elevation;
         _ptsNorth.push_back(pt);
      }
      else
      {
         if (pts[i].x>_x0+eps && pts[i].y>_y0+eps && pts[i].x<_x1-eps && pts[i].y<_y1-eps)
         {
            _ptsMiddle.push_back(pts[i]);
         } 
      }
   }
}

//------------------------------------------------------------------------------

void ElevationTile::_PrecomputeTriangulation(bool bCurtain)
{
   // Precompute Triangulation
   GeoCoord oGeoCoord;
   GeoCoord oGeoCoordNorm;

   double lng, lat;
   double x,y,z;
   double minelv = 1e20;

   _bbmin.x = 1e20;
   _bbmin.y = 1e20;
   _bbmin.z = 1e20;
   _bbmax.x = -1e20;
   _bbmax.y = -1e20;
   _bbmax.z = -1e20;

   boost::shared_ptr<math::DelaunayTriangulation> qTriangulation;
   qTriangulation = CreateTriangulation();
   std::vector<ElevationPoint>   lstElevationPoint;

   _lstIndices.clear();
   _lstTexCoord.clear();
   _lstElevationPointWGS84.clear();

   qTriangulation->GetPointVec(lstElevationPoint);
   qTriangulation->GetTriangleIndices(_lstIndices);

   double TexCoordOffsetX = _NW.x;
   double TexCoordOffsetY = _NW.y;
   double TexCoordDX = fabs(_NE.x - _NW.x);
   double TexCoordDY = fabs(_SW.y - _NW.y);

   // Virtual Camera Offset (STORE)

   //**********************************************************
   // Calc virtual camera offset:
   //**********************************************************
   _vOffset.x = _NW.x /*+ TexCoordDX/2.0*/;
   _vOffset.y = _NW.y /*+ TexCoordDY/2.0*/;
   _vOffset.z = _NW.elevation;

   Mercator::ReverseCustom(_vOffset.x, _vOffset.y, lng, lat, 0.0);
   oGeoCoord.SetLatitude(lat);
   oGeoCoord.SetLongitude(lng);
   oGeoCoord.SetEllipsoidHeight(_vOffset.z);
   oGeoCoord.ToCartesian(&_vOffset.x,&_vOffset.y,&_vOffset.z);
   //**********************************************************

   std::vector<ElevationPoint>::iterator it  = lstElevationPoint.begin();
   while (it!=lstElevationPoint.end())
   {
      Mercator::ReverseCustom((*it).x, (*it).y, lng, lat, 0.0);
      oGeoCoord.SetLatitude(lat);
      oGeoCoord.SetLongitude(lng);
      oGeoCoord.SetEllipsoidHeight((*it).elevation);
      oGeoCoord.ToCartesian(&x,&y,&z);

      if (x<_bbmin.x) { _bbmin.x = x; }
      if (y<_bbmin.y) { _bbmin.y = y; }
      if (z<_bbmin.z) { _bbmin.z = z; }

      if (x>_bbmax.x) { _bbmax.x = x; }
      if (y>_bbmax.y) { _bbmax.y = y; }
      if (z>_bbmax.z) { _bbmax.z = z; }

      if ((*it).elevation < minelv)
      {
            minelv = (*it).elevation;
      }

      double u = ((*it).x - TexCoordOffsetX) / TexCoordDX;
      double v = ((*it).y - TexCoordOffsetY) / TexCoordDY;

      _lstTexCoord.push_back(vec2<float>((float)u,(float)v));
      _lstElevationPointWGS84.push_back(vec3<float>( 
                                                      (float) (x-_vOffset.x), 
                                                      (float) (y-_vOffset.y), 
                                                      (float) (z-_vOffset.z)));

      it++;
     
      //i++;
   }

   if (bCurtain)
   {
      // Create Curtain Geometry:
      // A)
      //  1) _NW
      //  2) _ptsNorth
      //  3) _NE
      // B)
      //  1) _NE
      //  2) _ptsEast
      //  3) _SE
      // C)
      //  1) _SE
      //  2) _ptsSouth
      //  3) _SW;
      // D)
      //  1) _SW;
      //  2) _ptsWest
      //  3) _NW;


      double curtainelv = minelv - 1000; // 1000 meters below lowest elevation point

      _idxcurtain = _lstElevationPointWGS84.size();

      int idxA = _lstElevationPointWGS84.size()+0;
      int idxB = _lstElevationPointWGS84.size()+1;
      int idxC = _lstElevationPointWGS84.size()+2;
      int idxD = _lstElevationPointWGS84.size()+3;
      _CreateCurtain(curtainelv, _SW, _SE, _ptsNorth, idxA, idxB, idxC, idxD);
      
      idxA = _lstElevationPointWGS84.size()+0;
      idxB = _lstElevationPointWGS84.size()+1;
      idxC = _lstElevationPointWGS84.size()+2;
      idxD = _lstElevationPointWGS84.size()+3;
      _CreateCurtain(curtainelv, _NE, _SE, _ptsEast, idxA, idxB, idxC, idxD);

      idxA = _lstElevationPointWGS84.size()+0;
      idxB = _lstElevationPointWGS84.size()+1;
      idxC = _lstElevationPointWGS84.size()+2;
      idxD = _lstElevationPointWGS84.size()+3;
      _CreateCurtain(curtainelv, _NW, _NE, _ptsSouth, idxA, idxB, idxC, idxD);

      idxA = _lstElevationPointWGS84.size()+0;
      idxB = _lstElevationPointWGS84.size()+1;
      idxC = _lstElevationPointWGS84.size()+2;
      idxD = _lstElevationPointWGS84.size()+3;
      _CreateCurtain(curtainelv, _NW, _SW, _ptsWest, idxA, idxB, idxC, idxD);
   

   }

}

//------------------------------------------------------------------------------

void ElevationTile::_CreateCurtain(double curtainelv, ElevationPoint& start, ElevationPoint& end, std::vector<ElevationPoint>& between,  int& idxA, int& idxB, int& idxC, int& idxD)
{
   double TexCoordOffsetX = _NW.x;
   double TexCoordOffsetY = _NW.y;
   double TexCoordDX = fabs(_NE.x - _NW.x);
   double TexCoordDY = fabs(_SW.y - _NW.y);


   std::vector< ElevationPoint > input;
   input.push_back(start);
   for (size_t i=0;i<between.size();i++)
   {
      input.push_back(between[i]);
   }
   input.push_back(end);


   //----------------
   //
   //  A *--* D
   //    | /|
   //    |/ |
   //  B *--* C
   //
   //----------------
   GeoCoord oGeoCoord;
   vec3<float> A,B,C,D;
   vec2<float> At, Bt, Ct, Dt;
   double lng, lat,x,y,z;

   // A:
   Mercator::ReverseCustom(input[0].x, input[0].y, lng, lat, 0.0);
   oGeoCoord.SetLatitude(lat);
   oGeoCoord.SetLongitude(lng);
   oGeoCoord.SetEllipsoidHeight(input[0].elevation);
   oGeoCoord.ToCartesian(&x,&y,&z);
   A.x = float(x-_vOffset.x); 
   A.y = float(y-_vOffset.y);
   A.z = float(z-_vOffset.z);
   At.x = float((input[0].x - TexCoordOffsetX) / TexCoordDX);
   At.y = float((input[0].y - TexCoordOffsetY) / TexCoordDY);


   //B:
   Mercator::ReverseCustom(input[0].x, input[0].y, lng, lat, 0.0);
   oGeoCoord.SetLatitude(lat);
   oGeoCoord.SetLongitude(lng);
   oGeoCoord.SetEllipsoidHeight(curtainelv);
   oGeoCoord.ToCartesian(&x,&y,&z);
   B.x = float(x-_vOffset.x); 
   B.y = float(y-_vOffset.y); 
   B.z = float(z-_vOffset.z);
   Bt.x = At.x;
   Bt.y = At.y;

   _lstElevationPointWGS84.push_back(A);
   _lstElevationPointWGS84.push_back(B);

   _lstTexCoord.push_back(At);
   _lstTexCoord.push_back(Bt);



   for (size_t i=1;i<input.size();i++)
   {
      Mercator::ReverseCustom(input[i].x, input[i].y, lng, lat, 0.0);
      oGeoCoord.SetLatitude(lat);
      oGeoCoord.SetLongitude(lng);
      oGeoCoord.SetEllipsoidHeight(input[i].elevation);
      oGeoCoord.ToCartesian(&x,&y,&z);
      D.x = float(x-_vOffset.x); 
      D.y = float(y-_vOffset.y); 
      D.z = float(z-_vOffset.z);
      Dt.x = float((input[i].x - TexCoordOffsetX) / TexCoordDX);
      Dt.y = float((input[i].y - TexCoordOffsetY) / TexCoordDY);

      Mercator::ReverseCustom(input[i].x, input[i].y, lng, lat, 0.0);
      oGeoCoord.SetLatitude(lat);
      oGeoCoord.SetLongitude(lng);
      oGeoCoord.SetEllipsoidHeight(curtainelv);
      oGeoCoord.ToCartesian(&x,&y,&z);
      C.x = float(x-_vOffset.x); 
      C.y = float(y-_vOffset.y); 
      C.z = float(z-_vOffset.z);
      Ct.x = Dt.x;
      Ct.y = Ct.y;

      _lstElevationPointWGS84.push_back(C);
      _lstElevationPointWGS84.push_back(D);
      _lstTexCoord.push_back(Ct);
      _lstTexCoord.push_back(Dt);

      // TRIANGLE ABD
      _lstIndices.push_back(idxA);
      _lstIndices.push_back(idxB);
      _lstIndices.push_back(idxD);

      // TRIANGLE BCD
      _lstIndices.push_back(idxB);
      _lstIndices.push_back(idxC);
      _lstIndices.push_back(idxD);

      //A = D;
      //B = C;
      idxA = idxD;
      idxB = idxC;
      idxC += 2;
      idxD += 2;
   }

}


//------------------------------------------------------------------------------
/*
   EliminateCloseToCorner(_x0, DBL_EPSILON, _x1, DBL_EPSILON, _ptsNorth, fabs(_x1-_x0)/80.0);
   EliminateCloseToCorner(_x0, DBL_EPSILON, _x1, DBL_EPSILON, _ptsSouth, fabs(_x1-_x0)/80.0);
   EliminateCloseToCorner(DBL_EPSILON, _y0, DBL_EPSILON, _y1, _ptsEast, fabs(_y1-_y0)/80.0);
   EliminateCloseToCorner(DBL_EPSILON, _y0, DBL_EPSILON, _y1, _ptsWest, fabs(_y1-_y0)/80.0);
   EliminateCloseToCorner(_x0, _y0, _x1, _y1, _ptsMiddle, fabs(_y1-_y0)/80.0);
   
   EliminateDoubleEntriesX(_ptsNorth, fabs(_x1-_x0)/32.0);
   EliminateDoubleEntriesY(_ptsEast, fabs(_y1-_y0)/32.0);
   EliminateDoubleEntriesX(_ptsSouth, fabs(_x1-_x0)/32.0);
   EliminateDoubleEntriesY(_ptsWest, fabs(_y1-_y0)/32.0);
   
   if (_ptsMiddle.size() == 0)
   {
      _ptsWest.clear();
      _ptsNorth.clear();
      _ptsSouth.clear();
      _ptsEast.clear();
   }
*/
//------------------------------------------------------------------------------



