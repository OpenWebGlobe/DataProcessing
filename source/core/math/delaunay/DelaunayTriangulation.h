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

#ifndef DELAUNAYTRIANGULATION_H
#define DELAUNAYTRIANGULATION_H

#include "og.h"
#include "DelaunayTriangle.h"
#include "DelaunayLocationStructure.h"
#include "math/ElevationPoint.h"
#include <vector>
#include <boost/shared_ptr.hpp>
#include <limits.h>


namespace math
{  
   //--------------------------------------------------------------------------

   enum ElevationQuery
   {
      EQ_INTERIOR,
      EQ_EXTERIOR,
      EQ_UNDEFINED,
   };

   //--------------------------------------------------------------------------
   // TriangleVertex Structure:
   // 
   //     S0
   //     /\ T0     Triangle T0: index at S0, S1 -> idx0, (idx0+1)%3
   //S1  /__\ P     
   //    \  /
   //     \/ T1
   //     S2
   //---------------------------

   struct STriangleVertex
   {
      DelaunayTriangle* pTri;
      int idx0;
   };

   //--------------------------------------------------------------------------

   class OPENGLOBE_API DelaunayTriangulation
   {
   public:
      DelaunayTriangulation(double xmin, double ymin, double xmax, double ymax, EDelaunayLocationAlgorithms eAlgorithm = DELAUNAYLOCATION_LINEARLIST);
      virtual ~DelaunayTriangulation();

      void Clear();  // Clear Triangulation
      void InsertPoint(const ElevationPoint& pt);

      //! Retrieve vector Containing all Elevation Points
      void GetPointVec(std::vector<ElevationPoint>& lstElevationPoint);

      //! Retrieve Face Indices to previous GetPointList call.
      //! \warning This is only valid if "GetPointVec" was called before!
      void GetTriangleIndices(std::vector<int>& vIndices);

      //! Retrieves cut edges of triangulation (for holes).
      void GetCutEdges(std::vector<std::pair<int, int> >& vCutEdges);


      //! Delete a triangle from memory. The delaunay trianglulation still exists, but
      //! the triangle is removed from memory.
      void DeleteMemory(DelaunayTriangle* pTri);

      //! Retrieve list of all triangles. Please note that this list invalidates
      //! when new points are added to the triangulation or when the triangulation
      //! structure is invalidated.
      //! It is recommended to clear the vector when data is no longer neeed!
      std::vector<DelaunayTriangle*>& GetAllTriangles();

      //! Retrieve elevation at specified point (x,y). The elevation value is returned.
      //! Also a query result is returned providing more information about the query.
      //! (if point is outside a triangle or boundary).
      double GetElevationAt(double x, double y, ElevationQuery& query_result);

      //! Set Point Distance-Epsilon 
      void SetEpsilon(double epsilon);

      //! Calculate Vertex Errors
      void CalculateVertexErrors();

      //! Update Vertex Errors for specified Vertices
      void UpdateVertexErrors(std::vector<DelaunayVertex*>& vVertex);

      //! only valid after caling "CalculateVertexErrors"!!! 
      void RemoveLeastErrorVertex();


      //! Simplify dataset by removing unnecessary vertices. Returns
      //! number of simplification steps used.
      int Simplify(double epsilon = 0.0, int maxiterations = INT_MAX);


      //! Reduce a specified number of points from triangulation. The points with least error
      //! are used.
      void Reduce(int nPoints);


      // Remove the vertex clostest to point (x,y) (non supersimplex!)
      void RemoveVertex(double x, double y);

      // Get Triangles around a point in CCW Order
      void GetCCWTriangles(DelaunayTriangle* pTri, int vertex_index, std::vector<STriangleVertex>& outputVertices);

      // Get Vertices around a point in CCW Order
      void GetCCWVertices(DelaunayTriangle* pTri, int vertex_index, std::vector<DelaunayVertex*>& outputVertices);

      // create Wavefront OBJ 3D-Object of current triangulation
      std::string CreateOBJ(double xmin, double ymin, double xmax, double ymax);

      // Insert Line in Delaunay Triangulation.
      void InsertLine(double x0, double y0, double x1, double y1);

      // Virtually intersect Line Triangulation, returns:
      // returns false if operation is not possible
      // first: point at (x0,y0)
      // second: point at (x1, y1)
      // between: lists of points hitting the triangulation in between (unsorted, random order)
      void IntersectLine(double x0, double y0, double x1, double y1, ElevationPoint& first, ElevationPoint& second, std::vector<ElevationPoint>& between);

      // Intersect Rect. (Please note: x0<x1 and y0<y1)
      void IntersectRect(double x0, double y0, double x1, double y1, 
         ElevationPoint& NW,  // Point at (x0,y1)
         ElevationPoint& NE,  // Point at (x1,y1)
         ElevationPoint& SE,  // Point at (x1, y0) 
         ElevationPoint& SW,  // Point at (x0, y0)
         std::vector<ElevationPoint>& vNorth, // Points at north edge
         std::vector<ElevationPoint>& vEast,  // Points at east edge
         std::vector<ElevationPoint>& vSouth, // Points at south edge
         std::vector<ElevationPoint>& vWest,  // Points at west edge
         std::vector<ElevationPoint>& vMiddle); // points inside rectangle

      // All Vertices outside specified boundary will be marked as supersimplex.
      void InvalidateVertices(double x0, double y0, double x1, double y1);

   protected:
      void _GetElevation(double x, double y, ElevationPoint& out, double weight);
      void _RemoveVertex(DelaunayTriangle* pTri, int vtx);
      void _CreateSurroundingPolygon(DelaunayTriangle* pTri, int vertex_index, std::vector<ElevationPoint>& outputPolygon);
      void _CollectTriangle(DelaunayTriangle* pTri);
      void _ResetVertexErrors(DelaunayTriangle* pTri);
      void _CalcVertexErrors(DelaunayTriangle* pTri);
      void _UpdateVertexErrors(DelaunayTriangle* pTri);
      void _CalcVertexErrorsVtx(DelaunayTriangle* pTri, int vtx);
      void _CollectElevationPoints(DelaunayTriangle* pTri);
      void _CollectTriangulationStructure(DelaunayTriangle* pTri);
      void _ResetVertexId(DelaunayTriangle* pTri);
      void _Init();
      void _InsertPointSetId(const ElevationPoint& pt, int id);
      void _CutEdges(std::vector< std::pair<int,int> >& vCut);
      void _GetVertexAt(double x, double y, DelaunayTriangle*& pTri, int& idx);
      void _GetCCWVertices(DelaunayTriangle* pTri, int vertex_index, std::vector<DelaunayVertex*>& outputVertices);
      void _UpdateMinError();
      void _LineTraversal(DelaunayTriangle* pTri);
      void _SuperSimplexTraversal(DelaunayTriangle* pTri);
      void _MiddleTraversal(DelaunayTriangle* pTri);

      DelaunayTriangle*  _pStartTriangle;

      std::vector<DelaunayTriangle*> _vecTriangles;
      std::vector<ElevationPoint>   _vPts;
      std::vector<int>              _vIndex;
      std::vector<std::pair<int, int> > _vCutEdge;
      int _idcnt;
      double _xmin, _ymin, _xmax, _ymax;
      boost::shared_ptr<IDelaunayLocationStructure> _qLocationStructure;
      EDelaunayLocationAlgorithms                   _eLocationAlgorithm;
      
      bool     _bError; // true if errors are calculated and ok. false -> call CalculateVertexErrors() to have valid errors!
      double _minError;
      STriangleVertex  _oVertexMinError; // holds triangle / vertex with minimum error (only valid if _bError is true!!)
      
      ElevationPoint* _pt1;
      ElevationPoint* _pt2;
      std::vector<ElevationPoint> _vecEdgePoints;

      double _x0, _y0, _x1, _y1;

   private:
      DelaunayTriangulation(){}
   };
}

#endif