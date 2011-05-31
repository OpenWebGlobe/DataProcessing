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

#ifndef DELAUNAY_TRIANGLE_H
#define DELAUNAY_TRIANGLE_H

#include "og.h"
#include "DelaunayVertex.h"
#include <memory/ReferenceCounter.h>
#include "Predicates.h"

namespace math
{
   /*inline double ccw(DelaunayVertex* P, DelaunayVertex* A, DelaunayVertex* B)
   {
      return (A->x() - P->x())*(B->y() - P->y()) - (A->y() - P->y())*(B->x() - P->x());
   }

   inline double TriArea(DelaunayVertex* a, DelaunayVertex* b, DelaunayVertex* c)
   {
      return (b->y()-a->y())*(c->x()-a->x())-(b->x()-a->x())*(c->y()-a->y());
   }

   inline bool InCircle(DelaunayVertex* a, DelaunayVertex* b, DelaunayVertex* c, DelaunayVertex* d)
   {
      double fRet = (a->x()*a->x() + a->y()*a->y()) * TriArea(b, c, d) -
         (b->x()*b->x() + b->y()*b->y()) * TriArea(a, c, d) +
         (c->x()*c->x() + c->y()*c->y()) * TriArea(a, b, d) -
         (d->x()*d->x() + d->y()*d->y()) * TriArea(a, b, c);

      return -fRet > DBL_EPSILON; 
      //return fRet < 0;
   }*/

   //--------------------------------------------------------------------------
   // Triangle <-> Point Relation
   enum ePointTriangleRelation
   {
      PointTriangle_Invalid,  // Triangle is invalid
      PointTriangle_Outside,  // Point is outside triangle
      PointTriangle_Inside,   // Point is inside triangle
      PointTriangle_Edge0,    // Point is on edge 0
      PointTriangle_Edge1,    // Point is on edge 1
      PointTriangle_Edge2,    // Point is on edge 2
      PointTriangle_Vertex0,  // Point lies on vertex 0
      PointTriangle_Vertex1,  // Point lies on vertex 1
      PointTriangle_Vertex2,  // Point lies on vertex 2
   };

   //--------------------------------------------------------------------------
   // Triangulation

   class OPENGLOBE_API DelaunayTriangle
   {
   public:
      DelaunayTriangle();
      virtual ~DelaunayTriangle();

       //! Set Vertex 0,1,2. (counterclockwise)
      void SetVertex(int v, DelaunayVertex* pVertex);

      //! Set Triangle 0,1,2, where 
      //!     0 is triangle at edge 0-1
      //!     1 is triangle at edge 1-2
      //!     2 is triangle at edge 2-0
      void SetTriangle(int t, DelaunayTriangle* pTriangle);

      //! Retrieve Vertex 0,1,2
      DelaunayVertex* GetVertex(int v);

      //! Retrieve Triangle 0,1,2, where 
      //!     0 is triangle at edge 0-1
      //!     1 is triangle at edge 1-2
      //!     2 is triangle at edge 2-0
      DelaunayTriangle* GetTriangle(int t);

      //! Returns the Edge (Triangle) number of the neighbour triangle t=[0,1,2]
      //! pointing to this triangle. Returns -1 if there is no neighbour at t.
      int NeighbourReference(int t);

      //! Flip Edge with triangle neighbour t=[0,1,2]
      //! returns true if edge is actually flipped
      //! C and D will contain the new triangles.
      bool FlipEdge(int t, DelaunayTriangle **Cret, DelaunayTriangle **Dret);

      //! Retrieve opposite ("non shared") vertex of neighbour triangle in direction t.
      DelaunayVertex* GetOppositeVertex(int t);

      //! Returns true if triangle is a supersimplex triangle
      bool IsSuperSimplex();

      //! Returns true if triangle is counterclockwise!
      //! This function is used for testing only because triangles must always be ccw!
      bool IsCCW();

      //! Legalize edges of a Triangle
      static void LegalizeEdges(DelaunayTriangle* pTri, int t);
      
      //! Test triangle integrity (ccw, neighbour relations, ...)
      static void TestTriangle(DelaunayTriangle *tri);

   private:
      static bool _IsConvex(DelaunayVertex* Pt[], int numPts);
      static bool _IsCollinear(DelaunayVertex* p[], int numPts);
      
   
      DelaunayVertex* _pVertex0; 
      DelaunayVertex* _pVertex1;
      DelaunayVertex* _pVertex2; 
      DelaunayTriangle* _pTriangle0;
      DelaunayTriangle* _pTriangle1;
      DelaunayTriangle* _pTriangle2;
   };

   //-------------------------------------
   // Get Point Triangle Relation using robust arithmetic (double precision)
   ePointTriangleRelation GetPointTriangleRelationRobust(DelaunayVertex* pVertex, DelaunayTriangle* pTriangle, double epsilon);

   // Get Point Triangle Relation using exact arithmetic (very slow)
   ePointTriangleRelation GetPointTriangleRelationExact(DelaunayVertex* pVertex, DelaunayTriangle* pTriangle);

}


#endif