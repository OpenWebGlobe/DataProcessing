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

#include "DelaunayTriangle.h"
#include "math/vec2.h"
#include <limits>
#include <math/mathutils.h>
#include <ctime>
#include "DelaunayMemoryManager.h"

namespace math
{
   //--------------------------------------------------------------------------
   DelaunayTriangle::DelaunayTriangle()
   {
      _pVertex0 = _pVertex1 = _pVertex2 = 0;
      _pTriangle0 = _pTriangle1 = _pTriangle2 = 0;
   }
   //--------------------------------------------------------------------------
   DelaunayTriangle::~DelaunayTriangle()
   {
      if (_pVertex0)
      {
         _pVertex0->DecRef();
         _pVertex0 = 0;
      }

      if (_pVertex1)
      {
         _pVertex1->DecRef();
         _pVertex1 = 0;
      }

      if (_pVertex2)
      {
         _pVertex2->DecRef();
         _pVertex2 = 0;
      }
   }
   //--------------------------------------------------------------------------
   void DelaunayTriangle::SetVertex(int v, DelaunayVertex* pVertex)
   {
      if (v==0)
      {
         if (_pVertex0)
            _pVertex0->DecRef();
         
         _pVertex0 = pVertex;
         
         if (_pVertex0)
            _pVertex0->IncRef();
      }
      else if (v==1)
      {
         if (_pVertex1)
            _pVertex1->DecRef();
         
         _pVertex1 = pVertex;
         
         if (_pVertex1)
            _pVertex1->IncRef();
      }
      else if (v==2)
      {
         if (_pVertex2)
            _pVertex2->DecRef();
         
         _pVertex2 = pVertex;
         
         if (_pVertex2)
            _pVertex2->IncRef();
      }
      else
      {
         assert(false);
      }
   }
   //--------------------------------------------------------------------------
   void DelaunayTriangle::SetTriangle(int t, DelaunayTriangle* pTriangle)
   {
      if (t==0)
      {
         _pTriangle0 = pTriangle;
      }
      else if (t==1)
      {
         _pTriangle1 = pTriangle;
      }
      else if (t==2)
      {
         _pTriangle2 = pTriangle;
      }
      else
      {
         assert(false);
      }
   }
   //--------------------------------------------------------------------------
   bool DelaunayTriangle::IsSuperSimplex()
   {
      if (GetVertex(0)->weight() == -1) return true;
      if (GetVertex(1)->weight() == -1) return true;
      if (GetVertex(2)->weight() == -1) return true;

      return false;

      //return (_pTriangle0 == 0 || _pTriangle1 == 0 || _pTriangle2 == 0);
   }
   //--------------------------------------------------------------------------
   DelaunayVertex* DelaunayTriangle::GetVertex(int v)
   {
      if (v==0)
      {
         return _pVertex0;
      }
      else if (v==1)
      {
         return _pVertex1;
      }
      else if (v==2)
      {
         return _pVertex2;
      }
      else
      {
         assert(false);
      }
      return 0;
   }
   //--------------------------------------------------------------------------
   DelaunayTriangle* DelaunayTriangle::GetTriangle(int t)
   {
      if (t==0)
      {
         return _pTriangle0;
      }
      else if (t==1)
      {
         return _pTriangle1;
      }
      else if (t==2)
      {
         return _pTriangle2;
      }
      else
      {
         assert(false);
      }
      return 0;
   }

   //--------------------------------------------------------------------------

   template <typename T> inline T dot(const vec2<T>& a, const vec2<T>& b)
   {
      return a.x*b.x + a.y*b.y;
   }

   //--------------------------------------------------------------------------

   bool DelaunayTriangle::IsCCW()
   {
      double dCCW = math::ccw(_pVertex0, _pVertex1, _pVertex2);

      /*if (dCCW <= 0)
      {
         std::cout << "*WARNING* ccw= " << dCCW << "\n";
      }*/

      return  dCCW >= 0;
   }

   //--------------------------------------------------------------------------

   // This function updates point P to closest point on segment AB
   // if P is updated to A or B (start/end point of segment) then
   // false is returned and point is not updated.
   inline bool UpdateToClosestPoint(DelaunayVertex* A, DelaunayVertex* B, DelaunayVertex* P, double epsilon)
   {
      double APx = P->x() - A->x();
      double APy = P->y() - A->y();
      double ABx = B->x() - A->x();
      double ABy = B->y() - A->y();

      double ab2 = ABx*ABx + ABy*ABy;
      double ap_ab = APx*ABx + APy*ABy;
      double t = ap_ab / ab2;
     
      if (t <= epsilon || t>=1.0-epsilon) 
      {
         return false;
      }
      
      P->GetElevationPoint().x = A->x() + ABx * t;
      P->GetElevationPoint().y = A->y() + ABy * t;

      return true;
   }




   // This Epsilon is for sub millimeter accuracy when mapping the earth between
   // [-1,+1]. (For 1 meter resolution an epsilon of 1e-9 is good enough.)

//#define DELAUNAY_EDGE_EPSILON 1.0e-12
//#define DELAUNAY_POINT_EPSILON 1.0e-12

   ePointTriangleRelation GetPointTriangleRelationRobust(DelaunayVertex* pVertex, DelaunayTriangle* pTriangle, const double epsilon)
   {
      DelaunayVertex* P = pVertex;
      DelaunayVertex* A = pTriangle->GetVertex(0);
      DelaunayVertex* B = pTriangle->GetVertex(1);
      DelaunayVertex* C = pTriangle->GetVertex(2);

      const double edgeepsilon = DBL_EPSILON;
      const double ptepsilon = 1e-12;

      double p0 = math::ccw(P, A, B);
      double p1 = math::ccw(P, B, C);
      double p2 = math::ccw(P, C, A);

      if (fabs(A->GetElevationPoint().x - P->GetElevationPoint().x)<ptepsilon &&
          fabs(A->GetElevationPoint().y - P->GetElevationPoint().y)<ptepsilon)
      {
         return PointTriangle_Vertex0;
      }

      if (fabs(B->GetElevationPoint().x - P->GetElevationPoint().x)<ptepsilon &&
         fabs(B->GetElevationPoint().y - P->GetElevationPoint().y)<ptepsilon)
      {
         return PointTriangle_Vertex1;
      }

      if (fabs(C->GetElevationPoint().x - P->GetElevationPoint().x)<ptepsilon &&
         fabs(C->GetElevationPoint().y - P->GetElevationPoint().y)<ptepsilon)
      {
         return PointTriangle_Vertex2;
      }

      if (p0>=0 && p1>=0 && p2>=0)
      {
         if (p0 <= edgeepsilon && p1 <= edgeepsilon && p2 <= edgeepsilon)
         {
            //std::cout << "Invalid point detected!\n";
            return PointTriangle_Invalid;
         }

         /*if (p2 <= edgeepsilon && p0 <= edgeepsilon)
         {
            return PointTriangle_Vertex0;
         }
         else if (p0 <= edgeepsilon && p1 <= edgeepsilon)
         {
            return PointTriangle_Vertex1;
         }
         else if (p1 <= edgeepsilon && p2 <= edgeepsilon)
         {
            return PointTriangle_Vertex2;
         }
         */

         if (p0<=edgeepsilon)
         {
            if (UpdateToClosestPoint(A, B, P, DBL_EPSILON))
            {
               if (math::ccw(P, A, B)<=DBL_EPSILON)
               {
                  return PointTriangle_Edge0;
               }

            }

            //std::cout << "Failed updating edge0\n";
            return PointTriangle_Invalid;

         }
         else if (p1<=edgeepsilon)
         {
            if (UpdateToClosestPoint(B, C, P, DBL_EPSILON))
            {
               if (math::ccw(P, B, C)<=DBL_EPSILON)
               {
                  return PointTriangle_Edge1;
               }
            }

            return PointTriangle_Invalid;

            
         }
         else if (p2<=edgeepsilon)
         {
            if (UpdateToClosestPoint(C, A, P, DBL_EPSILON))
            {
               if (math::ccw(P, C, A)<=DBL_EPSILON)
               {
                  return PointTriangle_Edge2;
               }
            }
            
            //std::cout << "Failed updating edge2\n";
            return PointTriangle_Invalid;
         }

         if (p0>=edgeepsilon && p1>=edgeepsilon && p2>=edgeepsilon)
            return PointTriangle_Inside;
         else
            return PointTriangle_Outside;
      }
      else
      {
         return PointTriangle_Outside;
      }
   }

   //--------------------------------------------------------------------------

   int DelaunayTriangle::NeighbourReference(int t)
   {
      DelaunayTriangle* pNeighbourTriangle = GetTriangle(t);
      if (pNeighbourTriangle == 0)
      {
         return -1;
      }

      if (pNeighbourTriangle->GetTriangle(0) == this)
      {
         return 0;
      }
      else if (pNeighbourTriangle->GetTriangle(1) == this)
      {
         return 1;
      }
      else if (pNeighbourTriangle->GetTriangle(2) == this)
      {
         return 2;
      }
      else
      {
         assert(false);
         return -1;
      }
   }

   //--------------------------------------------------------------------------

   bool DelaunayTriangle::FlipEdge(int t, DelaunayTriangle **Cret, DelaunayTriangle **Dret)
   {
      DelaunayTriangle* A = this;
      DelaunayTriangle* B = this->GetTriangle(t);
      *Cret = 0;
      *Dret = 0;

      // neighbour MUST exist to flip!!
      if (!B)
      {
         return false;
      }

      int NRefB = A->NeighbourReference(t);
      int i,j,k,l;

      if (NRefB == 0)
      { k = 1; l = 2; }
      else if (NRefB == 1)
      { k = 2; l = 0; }
      else if (NRefB == 2)
      { k = 0; l = 1; }

      if (t == 0)
      { j = 2; i = 1; }
      else if (t == 1)
      { j = 0; i = 2;}
      else if (t == 2)
      { j = 1; i = 0;}

      DelaunayVertex* P[4];

      P[0] = A->GetVertex(t);
      P[1] = A->GetOppositeVertex(t); //equal to: B->GetVertex(l);
      P[2] = A->GetVertex((t+1)%3);
      P[3] = A->GetVertex((t+2)%3);
    
      DelaunayTriangle* C = A;
      DelaunayTriangle* D = B;

      DelaunayTriangle* C20 = A->GetTriangle(j);
      DelaunayTriangle* C01 = B->GetTriangle(k);
      DelaunayTriangle* C12 = D;
      DelaunayTriangle* D20 = B->GetTriangle(l);
      DelaunayTriangle* D01 = A->GetTriangle(i);
      DelaunayTriangle* D12 = C;

      int q; 
    
      q = A->NeighbourReference(i);
      if (q>=0)
      {
         A->GetTriangle(i)->SetTriangle(q, D);
      }
      q = A->NeighbourReference(j);
      if (q>=0)
      {    
         A->GetTriangle(j)->SetTriangle(q, C);
      }
      q = B->NeighbourReference(k);
      if (q>=0)
      {
         B->GetTriangle(k)->SetTriangle(q, C);
      }
      q = B->NeighbourReference(l);
      if (q>=0)
      {
         B->GetTriangle(l)->SetTriangle(q, D);
      }

      C->SetTriangle(0, C01);
      C->SetTriangle(1, C12);
      C->SetTriangle(2, C20);
      D->SetTriangle(0, D01);
      D->SetTriangle(1, D12);
      D->SetTriangle(2, D20);

      C->SetVertex(0, P[0]);
      C->SetVertex(1, P[1]);
      C->SetVertex(2, P[3]);
      D->SetVertex(0, P[2]);
      D->SetVertex(1, P[3]);
      D->SetVertex(2, P[1]);

      // martin.christen@fhnw.ch 2009-04-17
      // WARNING: MOVED ASSERTION
      // Degenerate cases are allowed for point removal...
      // This assertion is not done here anymore, instead the test was
      // moved to "LegalizeEdges" function.
      //assert(C->IsCCW());
      //assert(D->IsCCW());

      *Cret = C;
      *Dret = D;

      return true;
   }

   //--------------------------------------------------------------------------

   DelaunayVertex* DelaunayTriangle::GetOppositeVertex(int t)
   {
      DelaunayTriangle* B = this->GetTriangle(t);
      if (B == 0)
      {
         return 0;
      }

      int NRefB = NeighbourReference(t);
      return B->GetVertex((NRefB+2)%3);
   }

   //--------------------------------------------------------------------------

   void DelaunayTriangle::LegalizeEdges(DelaunayTriangle* pTri, int t)
   {
      if (!pTri || t<0)
         return;

      DelaunayVertex* P[4];

      P[0] = pTri->GetVertex(t);
      P[1] = pTri->GetOppositeVertex(t); //equal to: B->GetVertex(l);
      P[2] = pTri->GetVertex((t+1)%3);
      P[3] = pTri->GetVertex((t+2)%3);

      if (P[1] == 0)
      {
         return; // there is no opposite vertex...
      }

      // quadliteral must be convex!
      if (!_IsConvex(P, 4))
      {
        return;
      }

      if (P[1]->weight() == -1)
      {
         return;
      }

      if (math::InCircle(P[0], P[2], P[3], P[1]))
      {
         if (_IsCollinear(P,4))
         {
            return;
         }
         else
         {
            DelaunayTriangle *C, *D;
            if (pTri->FlipEdge(t, &C, &D))
            {
               assert(C->IsCCW());
               assert(D->IsCCW());
               DelaunayTriangle::LegalizeEdges(C, 0);
               DelaunayTriangle::LegalizeEdges(D, 2);
            }
         }
      }

   }

   //--------------------------------------------------------------------------
   bool DelaunayTriangle::_IsCollinear(DelaunayVertex* p[], int n)
   {
      if (n < 3)
      {
         return false; // not a polygon!
      }

      for (int i=0;i<n;i++)
      {
         double v = math::ccw(p[i%n], p[(i+1)%n], p[(i+2)%n]);
         if (v<=DBL_EPSILON) 
            return true;
      }

      return false;
   }

   //------------------------------------------------------------------------

   bool DelaunayTriangle::_IsConvex(DelaunayVertex* p[], int n)
   {
      int i,j,k;
      int flag = 0;
      double z;

      if (n < 3)
      {
         return false; // not a polygon!
      }

      for (i=0;i<n;i++) 
      {
         j = (i + 1) % n;
         k = (i + 2) % n;
         z  = (p[j]->x() - p[i]->x()) * (p[k]->y() - p[j]->y());
         z -= (p[j]->y() - p[i]->y()) * (p[k]->x() - p[j]->x());
         if (z < 0)
            flag |= 1;
         else if (z > 0)
            flag |= 2;
         if (flag == 3)
            return false; // concave!
      }

      if (flag != 0)
      {
         return true;
      }
      else
      {
         return false; // collinear (this should never happen!)
      }
   }

   //--------------------------------------------------------------------------
   // Test if triangle is correct. (topologic)
   // This is for debug reasons and to ensure stability!
   void DelaunayTriangle::TestTriangle(DelaunayTriangle *tri)
   {
      if (!tri)
      {
         return;
      }

      //assert(tri->IsCCW());

      if (!tri->IsCCW())
      {
         //std::cout << "CCW CONDITION FAILURE!\n";
      }
   
      DelaunayTriangle* pNeighbour0 = tri->GetTriangle(0);
      DelaunayTriangle* pNeighbour1 = tri->GetTriangle(1);
      DelaunayTriangle* pNeighbour2 = tri->GetTriangle(2);


      int NR0 = tri->NeighbourReference(0);
      int NR1 = tri->NeighbourReference(1);
      int NR2 = tri->NeighbourReference(2);

      if (pNeighbour0)
      {
         assert(pNeighbour0->GetTriangle(NR0) == tri);
         assert(pNeighbour0->GetVertex((NR0+0)%3) == tri->GetVertex(1));
         assert(pNeighbour0->GetVertex((NR0+1)%3) == tri->GetVertex(0));
      }
      if (pNeighbour1)
      {
         assert(pNeighbour1->GetTriangle(NR1) == tri);
         assert(pNeighbour1->GetVertex((NR1+0)%3) == tri->GetVertex(2));
         assert(pNeighbour1->GetVertex((NR1+1)%3) == tri->GetVertex(1));
      }
      if (pNeighbour2)
      {
         assert(pNeighbour2->GetTriangle(NR2) == tri);
         assert(pNeighbour2->GetVertex((NR2+0)%3) == tri->GetVertex(0));
         assert(pNeighbour2->GetVertex((NR2+1)%3) == tri->GetVertex(2));
      }
   }

   //--------------------------------------------------------------------------

} // namespace math







