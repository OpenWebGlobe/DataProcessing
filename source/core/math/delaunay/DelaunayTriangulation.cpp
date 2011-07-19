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

#include "DelaunayTriangulation.h"
#include "DelaunayMemoryManager.h"
#include "math/mathutils.h"
#include <float.h>
#include <cassert>
#include <set>
#include <list>
#include <iostream>
#include <boost/bind.hpp>
#include <sstream>

namespace math
{
   //--------------------------------------------------------------------------

   DelaunayTriangulation::DelaunayTriangulation(double xmin, double ymin, double xmax, double ymax, EDelaunayLocationAlgorithms eAlgorithm)
      : _xmin(xmin), _ymin(ymin), _xmax(xmax), _ymax(ymax), _pStartTriangle(0), _eLocationAlgorithm(eAlgorithm)
   {
      assert(_xmin<_xmax);
      assert(_ymin<_ymax);

      _pt1 = _pt2 = 0;

      _Init();
   }

   //--------------------------------------------------------------------------

   DelaunayTriangulation::~DelaunayTriangulation()
   {
      Clear();
      if (_pStartTriangle)
      {
         //std::cout << "<b>Removing Triangle</b>\n";
         _qLocationStructure->DeleteMemory(_pStartTriangle);

         // This assertion is only valid when one instance of DelaunayTriangulation exists!
         //assert(DelaunayMemoryManager::GetNumTriangles() == 0);
         //assert(DelaunayMemoryManager::GetNumVertices() == 0);

      }
   }

   //--------------------------------------------------------------------------

   void DelaunayTriangulation::_Init()
   {
      // calculate Triangle that surrounds area defined in _xmin, _ymin, _xmax, _ymax
      double Mx = _xmin + (_xmax-_xmin)/2.0;
      double My = _ymin + (_ymax-_ymin)/2.0;
      double r = sqrt((_xmax-Mx)*(_xmax-Mx)+(_ymax-My)*(_ymax-My));
      double Cy = _ymax + (_xmax-Mx)*(_xmax-Mx) / (_ymax-My);
      double Cx = Mx;
      double By = My -r;
      double Bx = Cx-(Cy-My+r)*(_xmax-Cx)/(_ymax-Cy); 
      double Ay = My - r;
      double Ax = Cx-(Cy-My+r)*(_xmin-Cx)/(_ymax-Cy);

      
      DelaunayVertex* A = DelaunayMemoryManager::AllocVertex(Ax,Ay,0,-1);
      DelaunayVertex* B = DelaunayMemoryManager::AllocVertex(Bx,By,0,-1);
      DelaunayVertex* C = DelaunayMemoryManager::AllocVertex(Cx,Cy,0,-1);

      _pStartTriangle = DelaunayMemoryManager::AllocTriangle();

      _pStartTriangle->SetVertex(0, A);
      _pStartTriangle->SetVertex(1, B);
      _pStartTriangle->SetVertex(2, C);

      double xmax, ymax, xmin, ymin;
      xmax = ymax = -1e20;
      xmin = ymin = 1e20;

      xmin = math::Min<double>(Ax, xmin);
      xmin = math::Min<double>(Bx, xmin);
      xmin = math::Min<double>(Cx, xmin);
      ymin = math::Min<double>(Ay, ymin);
      ymin = math::Min<double>(By, ymin);
      ymin = math::Min<double>(Cy, ymin);
      xmax = math::Max<double>(Ax, xmax);
      xmax = math::Max<double>(Bx, xmax);
      xmax = math::Max<double>(Cx, xmax);
      ymax = math::Max<double>(Ay, ymax);
      ymax = math::Max<double>(By, ymax);
      ymax = math::Max<double>(Cy, ymax);

      _qLocationStructure = IDelaunayLocationStructure::CreateLocationStructure(xmin, ymin, xmax, ymax, _eLocationAlgorithm);
      if (_qLocationStructure)
      {
         _qLocationStructure->AddTriangle(_pStartTriangle);
      }

      if (!_pStartTriangle->IsCCW())
      {
         //assert(false);
      }

      _bError = false;
   }

   //--------------------------------------------------------------------------

   void DelaunayTriangulation::_CollectTriangle(DelaunayTriangle* pTri)
   {
      _vecTriangles.push_back(pTri);
   }

   //--------------------------------------------------------------------------

   void DelaunayTriangulation::Clear()
   {
      if (_pStartTriangle)
      {
         _vecTriangles.clear();
         _qLocationStructure->Traverse(boost::bind(&DelaunayTriangulation::_CollectTriangle, this, _1));

         for (size_t i=0;i<_vecTriangles.size();i++)
         {
            _qLocationStructure->DeleteMemory(_vecTriangles[i]);
         }

         _pStartTriangle = 0;
         _Init();
      }
   }

   //--------------------------------------------------------------------------

 


   //--------------------------------------------------------------------------


   namespace internal
   {
      inline double _CalcTriArea(const ElevationPoint& A, const ElevationPoint& B, const ElevationPoint& C)
      {
         return math::ccw(A.x, A.y, B.x, B.y, C.x, C.y);
         //return 0.5*((B.x - A.x)*(C.y - A.y) - (B.y - A.y)*(C.x - A.x));
      }
   }


   double DelaunayTriangulation::GetElevationAt(double x, double y, ElevationQuery& query_result)
   {
      if (x<=_xmax && x>=_xmin &&
         y<=_ymax && y>=_ymin)
      {
         query_result = EQ_INTERIOR;
         ePointTriangleRelation relation;
         DelaunayTriangle* pTri = _qLocationStructure->GetTriangleAt(x,y,relation);

         if (pTri && !pTri->IsSuperSimplex())
         {
            ElevationPoint P;
            P.x = x; P.y = y; P.elevation = 0.0;

            ElevationPoint A = pTri->GetVertex(0)->GetElevationPointCopy();
            ElevationPoint B = pTri->GetVertex(1)->GetElevationPointCopy();
            ElevationPoint C = pTri->GetVertex(2)->GetElevationPointCopy();

            double elva = A.elevation;
            double elvb = B.elevation;
            double elvc = C.elevation;

            switch(relation)
            {
            case PointTriangle_Edge0:  // Point on Edge 0 (AB)
            case PointTriangle_Edge1:  // Point on Edge 1 (BC)
            case PointTriangle_Edge2:  // Point on Edge 2 (CA)
            case PointTriangle_Inside: // Point inside triangle
               {
                  A.elevation = 0.0;
                  B.elevation = 0.0;
                  C.elevation = 0.0;

                  double F_abc = internal::_CalcTriArea(A,B,C);
                  double F_pbc = internal::_CalcTriArea(P,B,C);
                  double F_apc = internal::_CalcTriArea(A,P,C);

                  double r = F_pbc/F_abc;
                  double s = F_apc/F_abc;
                  double t = 1.0-r-s;

                  A.elevation = elva;
                  B.elevation = elvb;
                  C.elevation = elvc;

                  double Pelv = r*A.elevation + s*B.elevation + t*C.elevation;
                  return Pelv;
               }
               break;
            case PointTriangle_Vertex0:  // Point lies on vertex 0
               return pTri->GetVertex(0)->elevation();
            case PointTriangle_Vertex1:  // Point lies on vertex 1
               return pTri->GetVertex(1)->elevation();
            case PointTriangle_Vertex2:  // Point lies on vertex 2
               return pTri->GetVertex(2)->elevation();
            default: // outside triangle / invalid triangle
               query_result = EQ_EXTERIOR;
               return 0.0;
            }
         }
         else
         {
            query_result = EQ_EXTERIOR;
            return 0.0;
         }
      }
      else
      {
         query_result = EQ_UNDEFINED; // outside valid triangulation area
         return 0.0;
      }

   }

   //--------------------------------------------------------------------------

   void DelaunayTriangulation::InsertPoint(const ElevationPoint& pt)
   {
      if (pt.x<=_xmax && pt.x>=_xmin &&
         pt.y<=_ymax && pt.y>=_ymin)
      {
         DelaunayVertex* pNewVertex = DelaunayMemoryManager::AllocVertex(pt);
         _pStartTriangle = _qLocationStructure->InsertVertex(pNewVertex, _pStartTriangle);
         _bError = false; // inserting a point invalidates errors!
      }
   }


   void DelaunayTriangulation::_InsertPointSetId(const ElevationPoint& pt, int id)
   {
      if (pt.x<=_xmax && pt.x>=_xmin &&
         pt.y<=_ymax && pt.y>=_ymin)
      {
         DelaunayVertex* pNewVertex = DelaunayMemoryManager::AllocVertex(pt);
         pNewVertex->SetId(id);
         _pStartTriangle = _qLocationStructure->InsertVertex(pNewVertex, _pStartTriangle);
         _bError = false; // inserting a point invalidates errors!
      }
   }

   //--------------------------------------------------------------------------

   void DelaunayTriangulation::_ResetVertexId(DelaunayTriangle* pTri)
   {
      assert(pTri);
      assert(pTri->GetVertex(0));
      assert(pTri->GetVertex(1));
      assert(pTri->GetVertex(2));

      pTri->GetVertex(0)->SetId(-1);
      pTri->GetVertex(1)->SetId(-1);
      pTri->GetVertex(2)->SetId(-1);
   }

   //--------------------------------------------------------------------------

   void DelaunayTriangulation::_CollectElevationPoints(DelaunayTriangle* pTri)
   {
      assert(pTri);
      assert(pTri->GetVertex(0));
      assert(pTri->GetVertex(1));
      assert(pTri->GetVertex(2));

      // ignore supersimplex triangles.
      if (!pTri->IsSuperSimplex())
      {
         for (int v=0;v<3;v++)
         {
            if (pTri->GetVertex(v)->GetId() == -1)
            { 
               pTri->GetVertex(v)->SetId(_idcnt);
               _vPts.push_back(pTri->GetVertex(v)->GetElevationPoint());
               _idcnt++;
            }

            _vIndex.push_back(pTri->GetVertex(v)->GetId());
         }
      }
   }


   //--------------------------------------------------------------------------

   // before calling this make sure to clear _vCutEdge, _vIndex and _vPts
   void DelaunayTriangulation::_CollectTriangulationStructure(DelaunayTriangle* pTri)
   {
      assert(pTri);
      assert(pTri->GetVertex(0));
      assert(pTri->GetVertex(1));
      assert(pTri->GetVertex(2));

      // ignore supersimplex triangles.
      if (!pTri->IsSuperSimplex())
      {
         for (int v=0;v<3;v++)
         {
            if (pTri->GetVertex(v)->GetId() == -1)
            { 
               pTri->GetVertex(v)->SetId(_idcnt);
               _vPts.push_back(pTri->GetVertex(v)->GetElevationPoint());
               _idcnt++;
            }

            _vIndex.push_back(pTri->GetVertex(v)->GetId());
         }

        
         if (pTri->GetTriangle(0) == 0)
         {
            _vCutEdge.push_back(std::pair<int, int>(pTri->GetVertex(0)->GetId(), pTri->GetVertex(1)->GetId()));
         }

         if (pTri->GetTriangle(1) == 0)
         {
            _vCutEdge.push_back(std::pair<int, int>(pTri->GetVertex(1)->GetId(), pTri->GetVertex(2)->GetId()));
         }

         if (pTri->GetTriangle(2) == 0)
         {
            _vCutEdge.push_back(std::pair<int, int>(pTri->GetVertex(2)->GetId(), pTri->GetVertex(0)->GetId()));
         }

      }

   }

   //--------------------------------------------------------------------------

   // Retrieve Triangulation as Point / Index List
   void DelaunayTriangulation::GetPointVec(std::vector<ElevationPoint>& vPoints)
   {
      

      // Reset All Vertices to 0
      _qLocationStructure->Traverse(boost::bind(&DelaunayTriangulation::_ResetVertexId, this, _1));

      _idcnt = 0; _vPts.clear(); _vIndex.clear(); _vCutEdge.clear();
      _qLocationStructure->Traverse(boost::bind(&DelaunayTriangulation::_CollectTriangulationStructure, this, _1));

      vPoints = _vPts;
      _vPts.clear();

   }

  
   //--------------------------------------------------------------------------

   void DelaunayTriangulation::GetTriangleIndices(std::vector<int>& vIndices)
   {
      vIndices = _vIndex;
   }

   //--------------------------------------------------------------------------

   void DelaunayTriangulation::GetCutEdges(std::vector<std::pair<int, int> >& vCutEdges)
   {
      vCutEdges = _vCutEdge;
   }
   //--------------------------------------------------------------------------

   std::vector<DelaunayTriangle*>& DelaunayTriangulation::GetAllTriangles()
   {
      _vecTriangles.clear();
      _qLocationStructure->Traverse(boost::bind(&DelaunayTriangulation::_CollectTriangle, this, _1));

      return _vecTriangles;
   }

   //--------------------------------------------------------------------------

   void DelaunayTriangulation::DeleteMemory(DelaunayTriangle* pTri)
   {
      _qLocationStructure->DeleteMemory(pTri);
   }

   //--------------------------------------------------------------------------

   void DelaunayTriangulation::_CutEdges(std::vector< std::pair<int,int> >& vCut)
   {
      // Traverse triangulation structure and delete corresponding triangles 

      std::vector<DelaunayTriangle*>& vTri = DelaunayTriangulation::GetAllTriangles();

      std::set<DelaunayTriangle*> setDelete;

      for (size_t i=0;i<vTri.size();i++)
      {
         DelaunayTriangle* pTri = vTri[i];
         DelaunayTriangle* pTri0 = pTri->GetTriangle(0);
         DelaunayTriangle* pTri1 = pTri->GetTriangle(1);
         DelaunayTriangle* pTri2 = pTri->GetTriangle(2);

         int A,B,C;
         A = pTri->GetVertex(0)->GetId();
         B = pTri->GetVertex(1)->GetId();
         C = pTri->GetVertex(2)->GetId();

         // does this triangle contain a cut edge ?

         for (size_t c=0;c<vCut.size();c++)
         {
            int start = vCut[c].first;
            int end = vCut[c].second;

            if (A == start && B == end)
            {
               setDelete.insert(pTri0);
            }
            else if (B == start && C == end)
            {
               setDelete.insert(pTri1);
            }
            else if (C == start && A == end)
            {
               setDelete.insert(pTri2);
            }
         }
      }

      std::set<DelaunayTriangle*>::iterator it = setDelete.begin();

      while (it != setDelete.end())
      {
         DeleteMemory(*it);
         it++;
      }

   }

   //--------------------------------------------------------------------------

   void DelaunayTriangulation::SetEpsilon(double epsilon)
   {
      assert(_qLocationStructure);

      if (_qLocationStructure)
      {
         _qLocationStructure->SetEpsilon(epsilon);
      }
   }

   //--------------------------------------------------------------------------

   void DelaunayTriangulation::_ResetVertexErrors(DelaunayTriangle* pTri)
   {
      pTri->GetVertex(0)->GetElevationPoint().error = -1.0;
      pTri->GetVertex(1)->GetElevationPoint().error = -1.0;
      pTri->GetVertex(2)->GetElevationPoint().error = -1.0;
   }

   //--------------------------------------------------------------------------

   void DelaunayTriangulation::_GetCCWVertices(DelaunayTriangle* pTri, int vertex_index, std::vector<DelaunayVertex*>& outputVertices)
   {
      outputVertices.clear();
      std::list<DelaunayVertex*> lst;

      const DelaunayTriangle* pStartTriangle = pTri;

      int vtx = vertex_index;
      int triangle_index = (vtx+2)%3;
      lst.push_front(pTri->GetVertex((vtx+2)%3));

      DelaunayTriangle* A = pTri;
      DelaunayTriangle* C = pTri->GetTriangle(triangle_index);


      while(C && C != pStartTriangle && !C->IsSuperSimplex() && !A->IsSuperSimplex())
      {
         if (A->GetVertex(vtx) == C->GetVertex(0))
         {
            vtx = 0;
         }
         else if (A->GetVertex(vtx) == C->GetVertex(1))
         {
            vtx = 1;
         }
         else if (A->GetVertex(vtx) == C->GetVertex(2))
         {
            vtx = 2;
         }
         else
         {
            assert(false);
            break;
         }

         lst.push_front(C->GetVertex((vtx+2)%3));

         triangle_index = (vtx+2)%3; 
         A = C;
         C = C->GetTriangle(triangle_index);
      }

      lst.push_back(pTri->GetVertex((vertex_index+1)%3));

      // ignore non ccw direction!
      // copy result to output
      std::list<DelaunayVertex*>::iterator it = lst.begin();
      while (it!=lst.end())
      {
         outputVertices.push_back(*it);
         it++;
      }

   }

   //--------------------------------------------------------------------------

   void DelaunayTriangulation::_CreateSurroundingPolygon(DelaunayTriangle* pTri, int vertex_index, std::vector<ElevationPoint>& outputPolygon)
   {
      outputPolygon.clear();
      std::list<ElevationPoint> lst;

      const DelaunayTriangle* pStartTriangle = pTri;

      int vtx = vertex_index;
      int triangle_index = (vtx+2)%3;
      lst.push_front(pTri->GetVertex((vtx+2)%3)->GetElevationPoint());

      DelaunayTriangle* A = pTri;
      DelaunayTriangle* B;
      DelaunayTriangle* C = pTri->GetTriangle(triangle_index);

      while(C && C != pStartTriangle && !C->IsSuperSimplex() && !A->IsSuperSimplex())
      {
         if (A->GetVertex(vtx) == C->GetVertex(0))
         {
            vtx = 0;
         }
         else if (A->GetVertex(vtx) == C->GetVertex(1))
         {
            vtx = 1;
         }
         else if (A->GetVertex(vtx) == C->GetVertex(2))
         {
            vtx = 2;
         }
         else
         {
            assert(false);
            break;
         }

         lst.push_front(C->GetVertex((vtx+2)%3)->GetElevationPoint());

         triangle_index = (vtx+2)%3; 
         A = C;
         C = C->GetTriangle(triangle_index);
      }

      // other direction and only if pTri->GetTriangle(vtx) != pStartTriangle
      lst.push_back(pTri->GetVertex((vertex_index+1)%3)->GetElevationPoint());

      if (C != pStartTriangle)
      {
         vtx = vertex_index;
         triangle_index = vtx;

         A = pTri;
         B = pTri->GetTriangle(triangle_index);

         while(B && B != pStartTriangle && !B->IsSuperSimplex() && !A->IsSuperSimplex())
         {
            if (A->GetVertex(vtx) == B->GetVertex(0))
            {
               vtx = 0;
            }
            else if (A->GetVertex(vtx) == B->GetVertex(1))
            {
               vtx = 1;
            }
            else if (A->GetVertex(vtx) == B->GetVertex(2))
            {
               vtx = 2;
            }


            lst.push_back(B->GetVertex((vtx+1)%3)->GetElevationPoint());
            triangle_index = vtx; 

            A = B;
            B = B->GetTriangle(triangle_index);
         }

      }

      // Convert list to vector (for returning values)
      if (lst.size()>2)
      {      
         std::list<ElevationPoint>::iterator it = lst.begin();
         while (it!=lst.end())
         {
            outputPolygon.push_back(*it);
            it++;
         }
      }
 
   }

   //--------------------------------------------------------------------------

   void DelaunayTriangulation::_CalcVertexErrorsVtx(DelaunayTriangle* pTri, int vtx)
   {
      assert(vtx>=0 && vtx<=3);
      assert(pTri);

      ElevationPoint pt = pTri->GetVertex(vtx)->GetElevationPoint();

      // corner points always have max error
      if (pt.weight < -2 
         || pTri->GetVertex((vtx+1)%3)->GetElevationPoint().weight < -2
         || pTri->GetVertex((vtx+2)%3)->GetElevationPoint().weight < -2)
      {
         pTri->GetVertex(vtx)->GetElevationPoint().error = DBL_MAX;
      }
      else
      {
         boost::shared_ptr<DelaunayTriangulation> qTriangulation;

         if (pTri->GetVertex(vtx)->GetElevationPoint().error <= -1.0)
         {
            std::vector<ElevationPoint> outputPolygon;
            _CreateSurroundingPolygon(pTri, vtx, outputPolygon);

            qTriangulation = boost::shared_ptr<DelaunayTriangulation>(new DelaunayTriangulation(_xmin, _ymin, _xmax, _ymax, _eLocationAlgorithm));
            qTriangulation->SetEpsilon(DBL_EPSILON); // can be removed later

            for (size_t i=0;i<outputPolygon.size();i++)
            {
               qTriangulation->InsertPoint(outputPolygon[i]);
            }

            math::ElevationQuery query;
            double elv = qTriangulation->GetElevationAt(pt.x, pt.y, query);

            if (query == EQ_INTERIOR)
            {
               double dError = fabs(elv - pt.elevation);
               pTri->GetVertex(vtx)->GetElevationPoint().error = dError;
               
               if (dError < _minError)
               {
                  _oVertexMinError.pTri = pTri;
                  _oVertexMinError.idx0 = vtx;
                  _minError = dError;
               }
               
            }
            else
            {
               pTri->GetVertex(vtx)->GetElevationPoint().error = DBL_MAX;
            }

         }

      }
   }

   //--------------------------------------------------------------------------

   void DelaunayTriangulation::_CalcVertexErrors(DelaunayTriangle* pTri)
   {
      assert(pTri);

      if (!pTri->IsSuperSimplex()) // ignore supersimplex triangles!
      {
         _CalcVertexErrorsVtx(pTri,0);
         _CalcVertexErrorsVtx(pTri,1);
         _CalcVertexErrorsVtx(pTri,2);
      }
   }

   //--------------------------------------------------------------------------

   void DelaunayTriangulation::CalculateVertexErrors()
   {
      _oVertexMinError.pTri = 0;
      _oVertexMinError.idx0 = -1;

      _minError = DBL_MAX;
      // Reset Errors:
      _qLocationStructure->Traverse(boost::bind(&DelaunayTriangulation::_ResetVertexErrors, this, _1));
      _qLocationStructure->Traverse(boost::bind(&DelaunayTriangulation::_CalcVertexErrors, this, _1));
   
      _bError = true;
      //std::cout << "MinError: " << _minError << "\n";
   }

   //--------------------------------------------------------------------------

   void DelaunayTriangulation::UpdateVertexErrors(std::vector<DelaunayVertex*>& vVertex)
   {

      ePointTriangleRelation e;
      DelaunayTriangle* pTri;
      int idx;
      
      for (size_t i=0;i<vVertex.size();i++)
      {
         idx = -1;
         pTri = _qLocationStructure->GetTriangleAt(vVertex[i]->x(),vVertex[i]->y(),e);
         if (pTri && !pTri->IsSuperSimplex())
         {
            if (pTri->GetVertex(0) == vVertex[i])
               idx = 0;
            else if (pTri->GetVertex(1) == vVertex[i])
               idx = 1;
            else if (pTri->GetVertex(2) == vVertex[i])
               idx = 2;

            if (idx != -1)
            {
               _CalcVertexErrorsVtx(pTri,idx);
            }
            else
            {
               assert(false); // vertex not found!! How can this be ??!??
            }
         }

      }
      
      _UpdateMinError();
   }

   //--------------------------------------------------------------------------

   void DelaunayTriangulation::_UpdateVertexErrors(DelaunayTriangle* pTri)
   {
      double dError;

      if (!pTri->IsSuperSimplex())
      {
         for (int i=0;i<3;i++)
         {
            dError = pTri->GetVertex(i)->GetElevationPoint().error;
            if (dError < _minError)
            {
               _oVertexMinError.pTri = pTri;
               _oVertexMinError.idx0 = i;
               _minError = dError;
            }
         }
      }
   }

   //--------------------------------------------------------------------------

   void DelaunayTriangulation::_UpdateMinError()
   {
      _minError = DBL_MAX;
      _qLocationStructure->Traverse(boost::bind(&DelaunayTriangulation::_UpdateVertexErrors, this, _1));    
   }

   //--------------------------------------------------------------------------

   void DelaunayTriangulation::RemoveLeastErrorVertex()
   {
      if (_minError == DBL_MAX)
         return;


      if (_oVertexMinError.pTri)
      {
         // GetNeighbourVertices and save in list
         if (_oVertexMinError.pTri->GetVertex(_oVertexMinError.idx0)->weight() == -3)
         {
            std::cout << "**** FATAL ERROR ****: Removing corner vertex!!\n";
         }

         _RemoveVertex(_oVertexMinError.pTri, _oVertexMinError.idx0);
         _oVertexMinError.pTri = 0;
         _oVertexMinError.idx0 = -1;
      }

   }


   //--------------------------------------------------------------------------

   int DelaunayTriangulation::Simplify(double epsilon, int maxiterations)
   {
      std::vector<DelaunayVertex*> vVertex;
      int rmvsteps = 0;
      if (!_bError)
         CalculateVertexErrors();

       while (_minError <= epsilon && _minError != DBL_MAX && rmvsteps < maxiterations)
       {
          GetCCWVertices(_oVertexMinError.pTri, _oVertexMinError.idx0, vVertex);
          RemoveLeastErrorVertex(); rmvsteps++;
          UpdateVertexErrors(vVertex);
       }

       /*if (rmvsteps >= maxiterations)
       {
          std::cout << "Simplify-Condition: Max Iterations reached!\n";
       }

       if (_minError > epsilon)
       {
          std::cout << "Simplify-Condition: Epsilon condition reached!\n";
       }*/

       return rmvsteps;

   }

   //--------------------------------------------------------------------------
   void DelaunayTriangulation::Reduce(int nPoints)
   {
      std::vector<DelaunayVertex*> vVertex;
      int rmvsteps = 0;
      if (!_bError)
         CalculateVertexErrors();

      while (_minError != DBL_MAX && rmvsteps < nPoints)
      {
         GetCCWVertices(_oVertexMinError.pTri, _oVertexMinError.idx0, vVertex);
         RemoveLeastErrorVertex(); rmvsteps++;
         UpdateVertexErrors(vVertex);
      }
   }

   //--------------------------------------------------------------------------

   void DelaunayTriangulation::_GetVertexAt(double x, double y, DelaunayTriangle*& pTri, int& idx)
   {
      ePointTriangleRelation e;
      pTri = _qLocationStructure->GetTriangleAt(x,y,e);
      if (pTri)
      {
         const ElevationPoint& P0 = pTri->GetVertex(0)->GetElevationPoint();
         const ElevationPoint& P1 = pTri->GetVertex(1)->GetElevationPoint();
         const ElevationPoint& P2 = pTri->GetVertex(2)->GetElevationPoint();

         double dist0, dist1, dist2;

         dist0 = sqrt((P0.x-x)*(P0.x-x)+(P0.y-y)*(P0.y-y));
         dist1 = sqrt((P1.x-x)*(P1.x-x)+(P1.y-y)*(P1.y-y));
         dist2 = sqrt((P2.x-x)*(P2.x-x)+(P2.y-y)*(P2.y-y));

         if (dist0 <= dist1 && dist0 <= dist2)
         {   
            idx = 0;
         }
         else if (dist1 <= dist0 && dist1 <= dist2)
         {
            idx = 1;
         }
         else //if (dist2 <= dist0 && dist2 <=dist1)
         {
            idx = 2;
         }
      }
      else
      {
         // no triangle found!!
         idx = -1;
      }
   }

   //--------------------------------------------------------------------------

   void DelaunayTriangulation::GetCCWTriangles(DelaunayTriangle* pTri, int vertex_index, std::vector<STriangleVertex>& outputVertices)
   {
      outputVertices.clear();
      std::list<STriangleVertex> lst;

      const DelaunayTriangle* pStartTriangle = pTri;

      STriangleVertex oElement;
      int vtx = vertex_index;
      int triangle_index = (vtx+2)%3;

      oElement.pTri = pTri;
      oElement.idx0 = (vertex_index+1)%3;
      lst.push_front(oElement);

      DelaunayTriangle* A = pTri;
      DelaunayTriangle* B;
      DelaunayTriangle* C = pTri->GetTriangle(triangle_index);


      while(C && C != pStartTriangle)
      {
         if (A->GetVertex(vtx) == C->GetVertex(0))
         {
            vtx = 0;
         }
         else if (A->GetVertex(vtx) == C->GetVertex(1))
         {
            vtx = 1;
         }
         else if (A->GetVertex(vtx) == C->GetVertex(2))
         {
            vtx = 2;
         }
         else
         {
            assert(false);
            break;
         }

         oElement.pTri = C;
         oElement.idx0 = (vtx+1)%3;
         lst.push_front(oElement);

         triangle_index = (vtx+2)%3; 
         A = C;
         C = C->GetTriangle(triangle_index);
      }

      // move around other side:

      if (C != pStartTriangle)
      {
         vtx = vertex_index;
         triangle_index = vtx;

         A = pTri;
         B = pTri->GetTriangle(triangle_index);

         while(B && B != pStartTriangle)
         {
            if (A->GetVertex(vtx) == B->GetVertex(0))
            {
               vtx = 0;
            }
            else if (A->GetVertex(vtx) == B->GetVertex(1))
            {
               vtx = 1;
            }
            else if (A->GetVertex(vtx) == B->GetVertex(2))
            {
               vtx = 2;
            }

            oElement.pTri = B;
            oElement.idx0 = (vtx+1)%3;
            lst.push_back(oElement);
            triangle_index = vtx; 

            A = B;
            B = B->GetTriangle(triangle_index);
         }

      }

      std::reverse(lst.begin(), lst.end());

      // copy result to output
      std::list<STriangleVertex>::iterator it = lst.begin();
      while (it!=lst.end())
      {
         outputVertices.push_back(*it);
         it++;
      }
   }

   //--------------------------------------------------------------------------

   void DelaunayTriangulation::GetCCWVertices(DelaunayTriangle* pTri, int vertex_index, std::vector<DelaunayVertex*>& outputVertices)
   {
      outputVertices.clear();
      std::list<DelaunayVertex*> lst;

      const DelaunayTriangle* pStartTriangle = pTri;

      int vtx = vertex_index;
      int triangle_index = (vtx+2)%3;

      lst.push_front(pTri->GetVertex((vertex_index+1)%3));

      DelaunayTriangle* A = pTri;
      DelaunayTriangle* B;
      DelaunayTriangle* C = pTri->GetTriangle(triangle_index);


      while(C && C != pStartTriangle)
      {
         if (A->GetVertex(vtx) == C->GetVertex(0))
         {
            vtx = 0;
         }
         else if (A->GetVertex(vtx) == C->GetVertex(1))
         {
            vtx = 1;
         }
         else if (A->GetVertex(vtx) == C->GetVertex(2))
         {
            vtx = 2;
         }
         else
         {
            assert(false);
            break;
         }

         lst.push_front(C->GetVertex((vtx+1)%3));

         triangle_index = (vtx+2)%3; 
         A = C;
         C = C->GetTriangle(triangle_index);
      }

      // move around other side:

      if (C != pStartTriangle)
      {
         vtx = vertex_index;
         triangle_index = vtx;

         A = pTri;
         B = pTri->GetTriangle(triangle_index);

         while(B && B != pStartTriangle)
         {
            if (A->GetVertex(vtx) == B->GetVertex(0))
            {
               vtx = 0;
            }
            else if (A->GetVertex(vtx) == B->GetVertex(1))
            {
               vtx = 1;
            }
            else if (A->GetVertex(vtx) == B->GetVertex(2))
            {
               vtx = 2;
            }

            lst.push_back(B->GetVertex((vtx+1)%3));
            triangle_index = vtx; 

            A = B;
            B = B->GetTriangle(triangle_index);
         }

      }

      std::reverse(lst.begin(), lst.end());

      // copy result to output
      std::list<DelaunayVertex*>::iterator it = lst.begin();
      while (it!=lst.end())
      {
         outputVertices.push_back(*it);
         it++;
      }
   }

   //--------------------------------------------------------------------------

   void DelaunayTriangulation::_RemoveVertex(DelaunayTriangle* pTri, int idx)
   {
      if (pTri && idx>=0 && idx<=3)
      {
         bool DebugOutput = false;
         DelaunayVertex* pVertex = pTri->GetVertex(idx); // this is the point P to be removed.

         std::vector<STriangleVertex> outputTriangles;
         GetCCWTriangles(pTri, idx, outputTriangles);

         if (outputTriangles.size() < 3 )
         {
            // this can't happen in a "normal" triangulation
            // unless you try to remove supersimplex corner.
            // however this can happen in a triangulation with holes. This
            // case is currently not supported!
            std::cout<< "*Error* Can't remove specified vertex! (Triangulation would be broken.)\n";
            return;
         }
         else
         {
            //std::cout << "-----------------------------\n";
            //std::cout << "Removing Point\n";
            do 
            {
               size_t num_changes = 0;

               //std::cout << "outputTriangles: " << outputTriangles.size() << "\n";

               if (outputTriangles.size() > 3)
               {


                  std::vector<STriangleVertex>::iterator it = outputTriangles.begin();

                  while (it != outputTriangles.end() && outputTriangles.size()>3)
                  {

                     DelaunayTriangle* pCurrentTriangle = (*it).pTri;
                     int idx0 = (*it).idx0;
                     int idx1 = (idx0+1)%3;


                     DelaunayVertex* s0 = pCurrentTriangle->GetVertex(idx0);
                     DelaunayVertex* s1 = pCurrentTriangle->GetVertex(idx1);
                     DelaunayVertex* s2 = pCurrentTriangle->GetOppositeVertex(idx1);

                     // for debugging reasons only:
                     ElevationPoint s0p = s0->GetElevationPoint(); 
                     ElevationPoint s1p = s1->GetElevationPoint(); 
                     ElevationPoint s2p = s2->GetElevationPoint(); 
                     ElevationPoint sP = pVertex->GetElevationPoint();

                     double ccwpredicate = math::ccw(s0,s1,s2);

                     if (DebugOutput)
                        std::cout << "ccwpredicate0 = " << ccwpredicate << "\n";

                     if (ccwpredicate<=0)
                     {
                        // this triangle-combination is ignored!
                     }
                     else
                     {
                        ccwpredicate = math::ccw(s0,s2,pVertex);
                        if (DebugOutput)
                           std::cout << "ccwpredicate1 = " << ccwpredicate << "\n";
                        if (ccwpredicate<0) // P encloses Triangle ?
                        {
                           
                           // this triangle-combination is ignored!
                        }
                        else
                        {
                           // Check if other points are inside cirumcircle!
                           bool bCircleTest = true;

                           std::vector<STriangleVertex>::iterator it2 = outputTriangles.begin();
                           while (it2 != outputTriangles.end())
                           {     
                              if ((*it2).pTri != (*it).pTri)
                              {
                                 int tidx0 = (*it2).idx0;
                                 int tidx1 = (tidx0+1)%3;

                                 DelaunayVertex* p0 = (*it2).pTri->GetVertex(tidx0);
                                 DelaunayVertex* p1 = (*it2).pTri->GetVertex(tidx1);

                                 assert(p0 != pVertex);
                                 assert(p1 != pVertex);
                                 
                                 if (p0 != s0 && p0 != s1 && p0 != s2 && 
                                     p1 != s0 && p1 != s1 && p1 != s2)
                                 {
                                    double circ = math::InCircleValue(s0,s1,s2,p0);
                                    if (DebugOutput)
                                       std::cout << "circ0 = " << circ << "\n";
                                    if (circ >= DBL_EPSILON)
                                    {
                                       bCircleTest = false;
                                    }
                                    circ = math::InCircleValue(s0,s1,s2,p1);
                                    if (DebugOutput)
                                       std::cout << "circ1 = " << circ << "\n";
                                    if (circ >= DBL_EPSILON)
                                    {
                                       bCircleTest = false;
                                    }
                                 }
                              }
                              it2++;
                           }

                           if (bCircleTest)
                           {
                              // this triangle and (next) are swapped and removed from list.
                              // new (swapped) triangle containing P will be added to list.

                              DelaunayTriangle* C;
                              DelaunayTriangle* D;

                              if (pCurrentTriangle->FlipEdge(idx1, &C, &D))
                              {
                                 it = outputTriangles.erase(it);
                                 if (it==outputTriangles.end())
                                 {
                                    it = outputTriangles.begin();
                                 }
                                 it = outputTriangles.erase(it);

                                 STriangleVertex newElement;

                                 if (C->GetVertex(0) == pVertex)
                                 {
                                    newElement.pTri = C; newElement.idx0 = 1;
                                 }
                                 else if (C->GetVertex(1) == pVertex)
                                 {
                                    newElement.pTri = C; newElement.idx0 = 2;
                                 }
                                 else if (C->GetVertex(2) == pVertex)
                                 {
                                    newElement.pTri = C; newElement.idx0 = 0;
                                 }
                                 else if (D->GetVertex(0) == pVertex)
                                 {
                                    newElement.pTri = D; newElement.idx0 = 1;
                                 }
                                 else if (D->GetVertex(1) == pVertex)
                                 {
                                    newElement.pTri = D; newElement.idx0 = 2;
                                 }
                                 else if (D->GetVertex(2) == pVertex)
                                 {
                                    newElement.pTri = D; newElement.idx0 = 0;
                                 }
                                 else
                                 {
                                    assert(false);
                                 }

                                 it = outputTriangles.insert(it, newElement);
                                 num_changes++;
                              }
                              else
                              {
                                 // can't flip
                              }
                           }

                        }
                     }


                     it++;
                  }  
               }

               if (outputTriangles.size()>3 && num_changes == 0)
               {
                  std::cout << "<b>*WARNING* Detected infinite loop!</b>\n";
                  pVertex->GetElevationPoint().error = -0.5;

                  if (DebugOutput)
                     break;
                  else
                     DebugOutput = true;
               }


               // 3 Remaining pairs: Remove 3 Triangles!!
               if (outputTriangles.size() == 3)
               {
                  STriangleVertex* st0 =  &outputTriangles[0];
                  STriangleVertex* st1 =  &outputTriangles[1];
                  STriangleVertex* st2 =  &outputTriangles[2];

                  // Create New Triangle
                  DelaunayTriangle* pNewTriangle = DelaunayMemoryManager::AllocTriangle();
                  pNewTriangle->SetVertex(0, st0->pTri->GetVertex(st0->idx0));
                  pNewTriangle->SetVertex(1, st1->pTri->GetVertex(st1->idx0));
                  pNewTriangle->SetVertex(2, st2->pTri->GetVertex(st2->idx0));


                  DelaunayTriangle* pNeighbour0 = st0->pTri->GetTriangle(st0->idx0);
                  int nr0 = st0->pTri->NeighbourReference(st0->idx0);

                  DelaunayTriangle* pNeighbour1 = st1->pTri->GetTriangle(st1->idx0);
                  int nr1 = st1->pTri->NeighbourReference(st1->idx0);

                  DelaunayTriangle* pNeighbour2 = st2->pTri->GetTriangle(st2->idx0);
                  int nr2 = st2->pTri->NeighbourReference(st2->idx0);

                  pNewTriangle->SetTriangle(0, pNeighbour0);
                  pNewTriangle->SetTriangle(1, pNeighbour1);
                  pNewTriangle->SetTriangle(2, pNeighbour2);

                  _qLocationStructure->DeleteMemory(st0->pTri);
                  _qLocationStructure->DeleteMemory(st1->pTri);
                  _qLocationStructure->DeleteMemory(st2->pTri);

                  if (pNeighbour0) 
                     pNeighbour0->SetTriangle(nr0, pNewTriangle);

                  if (pNeighbour1)
                     pNeighbour1->SetTriangle(nr1, pNewTriangle);

                  if (pNeighbour2)
                     pNeighbour2->SetTriangle(nr2, pNewTriangle);

                  _qLocationStructure->AddTriangle(pNewTriangle);

                  //assert(pNewTriangle->IsCCW()); // the mosted hated assertion
               }

            } 
            while (outputTriangles.size()>3);   
         }

      }
   }

   //--------------------------------------------------------------------------

   void DelaunayTriangulation::RemoveVertex(double x, double y)
   {
      int idx;
      DelaunayTriangle* pTri;
      _GetVertexAt(x,y,pTri,idx);
      _RemoveVertex(pTri,idx);
   }

   //--------------------------------------------------------------------------

   std::string DelaunayTriangulation::CreateOBJ(double xmin, double ymin, double xmax, double ymax)
   {
      const double twopi = (2.0*3.1415926535897932384626433832795028841971693993751);
      const double scale = (twopi*6378137.0) / 2.0;

      std::vector<ElevationPoint> vPoints;
      std::vector<int> vIndices;
      std::stringstream oss;
      oss.precision(DBL_DIG);

      oss << "# WaveFront *.obj file (generated by OpenWebGlobe Delaunay Triangulation)\n\n";

      GetPointVec(vPoints);
      GetTriangleIndices(vIndices);


      double xoffset = xmin + (xmax - xmin) * 0.5;
      double yoffset = ymin + (ymax - ymin) * 0.5;

      double width =  fabs(xmax - xmin) * scale;
      double height = fabs(ymax - ymin) * scale;

      for (size_t i=0;i<vPoints.size();i++)
      {
         ElevationPoint& A = vPoints[i];
         double ax, ay;

         ax = scale * (A.x - xoffset);  
         ay = scale * (yoffset - A.y);  // right handed: A.y - yoffset

         oss << "v " << scale * (A.x - xoffset) << " " << A.elevation << " " << scale * ( yoffset - A.y) << "\n";
      }

      std::vector<int>::const_iterator it = vIndices.begin();

      while (it!= vIndices.end())
      {
         int a = *it; ++it;
         int b = *it; ++it;
         int c = *it; ++it;

         oss << "f " << (a+1) << " " << (b+1) << " " << (c+1) << "\n";
      }


      return oss.str();
   }

   //---------------------------------------------------------------------------

   void DelaunayTriangulation::_LineTraversal(DelaunayTriangle* pTri)
   {
      double t;

      DelaunayVertex* pVertex0 = pTri->GetVertex(0);
      DelaunayVertex* pVertex1 = pTri->GetVertex(1);
      DelaunayVertex* pVertex2 = pTri->GetVertex(2);

      ElevationPoint pt;

      if (math::FindOrientedIntersection(pVertex0->x(), pVertex0->y(), pVertex1->x(), pVertex1->y(), _pt1->x, _pt1->y, _pt2->x, _pt2->y,t))
      {
         pt.x = pVertex0->x() + t * (pVertex1->x() - pVertex0->x());
         pt.y = pVertex0->y() + t * (pVertex1->y() - pVertex0->y());
         pt.elevation = pVertex0->elevation() + t * (pVertex1->elevation() - pVertex0->elevation());
         pt.weight = -2;
         _vecEdgePoints.push_back(pt);
      }

      if (math::FindOrientedIntersection(pVertex1->x(), pVertex1->y(), pVertex2->x(), pVertex2->y(), _pt1->x, _pt1->y, _pt2->x, _pt2->y,t))
      {
         pt.x = pVertex1->x() + t * (pVertex2->x() - pVertex1->x());
         pt.y = pVertex1->y() + t * (pVertex2->y() - pVertex1->y());
         pt.elevation = pVertex1->elevation() + t * (pVertex2->elevation() - pVertex1->elevation());
         pt.weight = -2;
         _vecEdgePoints.push_back(pt);
      }

      if (math::FindOrientedIntersection(pVertex2->x(), pVertex2->y(), pVertex0->x(), pVertex0->y(), _pt1->x, _pt1->y, _pt2->x, _pt2->y,t))
      {
         pt.x = pVertex2->x() + t * (pVertex0->x() - pVertex2->x());
         pt.y = pVertex2->y() + t * (pVertex0->y() - pVertex2->y());
         pt.elevation = pVertex2->elevation() + t * (pVertex0->elevation() - pVertex2->elevation());
         pt.weight = -2;
         _vecEdgePoints.push_back(pt);
      }
   }

   //--------------------------------------------------------------------------

   void DelaunayTriangulation::InsertLine(double x0, double y0, double x1, double y1)
   {
      ElevationPoint p1;
      ElevationPoint p2;
      p1.x = x0;
      p1.y = y0;
      p2.x = x1;
      p2.y = y1;

      ElevationQuery query_result;
      double elv = GetElevationAt(x0, y0, query_result);
      if (query_result == EQ_INTERIOR) { p1.elevation = elv;}
      else { p1.elevation = 0;}

      elv = GetElevationAt(x1, y1, query_result);
      if (query_result == EQ_INTERIOR) { p2.elevation = elv;}
      else { p2.elevation = 0;}

      p1.weight = -3; // corner point
      p2.weight = -3; // corner point

      InsertPoint(p1);
      InsertPoint(p2);

      _pt1 = &p1;
      _pt2 = &p2;

      _qLocationStructure->Traverse(boost::bind(&DelaunayTriangulation::_LineTraversal, this, _1));

      for (size_t i=0;i<_vecEdgePoints.size();i++)
      {
         InsertPoint(_vecEdgePoints[i]);
      }

      _vecEdgePoints.clear();

   }
   //---------------------------------------------------------------------------
   void DelaunayTriangulation::IntersectLine(double x0, double y0, double x1, double y1, ElevationPoint& first, ElevationPoint& second, std::vector<ElevationPoint>& between)
   {
      ElevationQuery query_result;
      double elv = GetElevationAt(x0, y0, query_result);
      first.x = x0;
      first.y = y0;
      if (query_result == EQ_INTERIOR) { first.elevation = elv;}
      else { first.elevation = 0;}

      elv = GetElevationAt(x1, y1, query_result);
      second.x = x1;
      second.y = y1;
      if (query_result == EQ_INTERIOR) { second.elevation = elv;}
      else { second.elevation = 0;}

      first.weight = -3; // mark as corner point
      first.weight = -3; // mark as corner point

      _pt1 = &first;
      _pt2 = &second;

      _qLocationStructure->Traverse(boost::bind(&DelaunayTriangulation::_LineTraversal, this, _1));

      between.clear();
      for (size_t i=0;i<_vecEdgePoints.size();i++)
      {
         between.push_back(_vecEdgePoints[i]);
      }

      _vecEdgePoints.clear();

   }

   //---------------------------------------------------------------------------

   void DelaunayTriangulation::_GetElevation(double x, double y, ElevationPoint& out, double weight)
   {
      ElevationQuery query_result;
      double elv = GetElevationAt(x, y, query_result);
      out.x = x;
      out.y = y;
      out.weight = weight;

      if (query_result == EQ_INTERIOR) 
      { 
         out.elevation = elv;
      }
      else
      { 
         out.elevation = 0;
      }
   }

   //---------------------------------------------------------------------------

   bool ep_xsort (const ElevationPoint& pt1, const ElevationPoint& pt2) { return (pt1.x<pt2.x); }
   bool ep_ysort (const ElevationPoint& pt1, const ElevationPoint& pt2) { return (pt1.y<pt2.y); }

   template <class T>
   void sortpoints(std::vector<T>& sites)
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

   inline void _xthin(std::vector<ElevationPoint>& pts, double maxdist)
   {
      if (pts.size()<1)
      {
         return;
      }
      std::vector<ElevationPoint> newvec;
      ElevationPoint cur = pts[0];
      newvec.push_back(cur);
      for (size_t i=1;i<pts.size();i++)
      {
         if (fabs(pts[i].x-cur.x)>maxdist)
         {
            newvec.push_back(pts[i]);
            cur = pts[i];
         }
      }

      pts.clear();
      for (size_t i=0;i<newvec.size();i++)
      {
         pts.push_back(newvec[i]);
      }
   }

   inline void _removeclosetocorner(std::vector<ElevationPoint>& pts, double x0, double y0, double x1, double y1, double mindist)
   {
      std::vector<ElevationPoint> newvec;

      for (size_t i=0;i<pts.size();i++)
      {
         if (  ((fabs(pts[i].x - x0)) < mindist) &&
               ((fabs(pts[i].y - y0)) < mindist))
         {
            // reject point
         }
         else if ( ((fabs(pts[i].x - x1)) < mindist) &&
                   ((fabs(pts[i].y - y1)) < mindist) )
         {
            // reject point
         }
         else if ( ((fabs(pts[i].x - x0)) < mindist) &&
                   ((fabs(pts[i].y - y1)) < mindist) )
         {
            // reject point
         }
         else if ( ((fabs(pts[i].x - x1)) < mindist) &&
                   ((fabs(pts[i].y - y0)) < mindist) )
         {
            // reject point
         }
         else
         {
            newvec.push_back(pts[i]);
         }
      }

      pts.clear();
      for (size_t i=0;i<newvec.size();i++)
      {
         pts.push_back(newvec[i]);
      }
   }

   //---------------------------------------------------------------------------

   inline void _ythin(std::vector<ElevationPoint>& pts, double maxdist)
   {
      if (pts.size()<1)
      {
         return;
      }
      std::vector<ElevationPoint> newvec;
      ElevationPoint cur = pts[0];
      newvec.push_back(cur);
      for (size_t i=1;i<pts.size();i++)
      {
         if (fabs(pts[i].y-cur.y)>maxdist)
         {
            newvec.push_back(pts[i]);
            cur = pts[i];
         }
      }

      pts.clear();
      for (size_t i=0;i<newvec.size();i++)
      {
         pts.push_back(newvec[i]);
      }
   }

   //---------------------------------------------------------------------------

   void DelaunayTriangulation::IntersectRect(double x0, double y0, double x1, double y1, 
      ElevationPoint& NW,  
      ElevationPoint& NE,  
      ElevationPoint& SE,  
      ElevationPoint& SW,  
      std::vector<ElevationPoint>& vNorth, 
      std::vector<ElevationPoint>& vEast,  
      std::vector<ElevationPoint>& vSouth, 
      std::vector<ElevationPoint>& vWest,  
      std::vector<ElevationPoint>& vMiddle) 
   {
      assert(x0<x1);
      assert(y0<y1);

      _GetElevation(x0,y1, NW, -3);
      _GetElevation(x1,y1, NE, -3);
      _GetElevation(x1,y0, SE, -3);
      _GetElevation(x0,y0, SW, -3);

      //-------------------------------------------------------------------------------------------
      // (1) North Points
      _pt1 = &NW;
      _pt2 = &NE;
      _qLocationStructure->Traverse(boost::bind(&DelaunayTriangulation::_LineTraversal, this, _1));
      vNorth.clear();
      for (size_t i=0;i<_vecEdgePoints.size();i++)
      {
         _vecEdgePoints[i].y = y1;
         vNorth.push_back(_vecEdgePoints[i]);
      }
      _vecEdgePoints.clear();
      //-------------------------------------------------------------------------------------------
      // (2) East Points
      _pt1 = &NE;
      _pt2 = &SE;
      _qLocationStructure->Traverse(boost::bind(&DelaunayTriangulation::_LineTraversal, this, _1));
      vEast.clear();
      for (size_t i=0;i<_vecEdgePoints.size();i++)
      {
         _vecEdgePoints[i].x = x1;
         vEast.push_back(_vecEdgePoints[i]);
      }
      _vecEdgePoints.clear();
      //-------------------------------------------------------------------------------------------
      // (3) South Points
      _pt1 = &SW;
      _pt2 = &SE;
      _qLocationStructure->Traverse(boost::bind(&DelaunayTriangulation::_LineTraversal, this, _1));
      vSouth.clear();
      for (size_t i=0;i<_vecEdgePoints.size();i++)
      {
         _vecEdgePoints[i].y = y0;
         vSouth.push_back(_vecEdgePoints[i]);
      }
      _vecEdgePoints.clear();
      //-------------------------------------------------------------------------------------------
      // (4) East Points
      _pt1 = &NW;
      _pt2 = &SW;
      _qLocationStructure->Traverse(boost::bind(&DelaunayTriangulation::_LineTraversal, this, _1));
      vWest.clear();
      for (size_t i=0;i<_vecEdgePoints.size();i++)
      {
         _vecEdgePoints[i].x = x0;
         vWest.push_back(_vecEdgePoints[i]);
      }
      _vecEdgePoints.clear();
      //-------------------------------------------------------------------------------------------

      _x0 = x0;
      _y0 = y0;
      _x1 = x1;
      _y1 = y1;

       // Reset All Vertices to 0
      _qLocationStructure->Traverse(boost::bind(&DelaunayTriangulation::_ResetVertexId, this, _1));

      _qLocationStructure->Traverse(boost::bind(&DelaunayTriangulation::_MiddleTraversal, this, _1));

      vMiddle.clear();
      for (size_t i=0;i<_vecEdgePoints.size();i++)
      {
         vMiddle.push_back(_vecEdgePoints[i]);
      }

      _vecEdgePoints.clear();

      // Recreate triangulation

      Clear();
      InsertPoint(NW);
      InsertPoint(NE);
      InsertPoint(SE);
      InsertPoint(SW);

      _removeclosetocorner(vNorth, x0, y0, x1, y1, fabs(NE.x-NW.x)/34.0);
      _removeclosetocorner(vSouth, x0, y0, x1, y1, fabs(NE.x-NW.x)/34.0);
      _removeclosetocorner(vEast, x0, y0, x1, y1, fabs(NE.x-NW.x)/34.0);
      _removeclosetocorner(vWest, x0, y0, x1, y1, fabs(NE.x-NW.x)/34.0);
      _removeclosetocorner(vMiddle, x0, y0, x1, y1, fabs(NE.x-NW.x)/34.0);

      std::sort(vNorth.begin(), vNorth.end(), ep_xsort);
      std::sort(vSouth.begin(), vSouth.end(), ep_xsort);
      std::sort(vEast.begin(), vEast.end(), ep_ysort);
      std::sort(vWest.begin(), vWest.end(), ep_ysort);

      _xthin(vNorth, fabs(NE.x-NW.x)/17.0);
      _xthin(vSouth, fabs(SE.x-SW.x)/17.0);
      _ythin(vEast, fabs(NE.y-SE.y)/17.0);
      _ythin(vWest, fabs(NW.y-SW.y)/17.0);

      if (vMiddle.size()>0)
      {
         sortpoints<ElevationPoint>(vMiddle);

         for (size_t i=0;i<vNorth.size();i++)
         {
            InsertPoint(vNorth[i]);
         }
         for (size_t i=0;i<vSouth.size();i++)
         {
            InsertPoint(vSouth[i]);
         }
         for (size_t i=0;i<vEast.size();i++)
         {
            InsertPoint(vEast[i]);
         }
         for (size_t i=0;i<vWest.size();i++)
         {
            InsertPoint(vWest[i]);
         }

         for (size_t i=0;i<vMiddle.size();i++)
         {
            InsertPoint(vMiddle[i]);
         }
      }
   }

   //---------------------------------------------------------------------------

   void DelaunayTriangulation::_MiddleTraversal(DelaunayTriangle* pTri)
   {
      DelaunayVertex* pVertex0 = pTri->GetVertex(0);
      DelaunayVertex* pVertex1 = pTri->GetVertex(1);
      DelaunayVertex* pVertex2 = pTri->GetVertex(2);

      if (pVertex0->GetId() == -1)
      {
         pVertex0->SetId(1);
         if (  pVertex0->x() > _x0 &&
            pVertex0->x() < _x1 &&
            pVertex0->y() > _y0 &&
            pVertex0->y() < _y1)
         {
            _vecEdgePoints.push_back(pVertex0->GetElevationPoint());
         }
      }

      if (pVertex1->GetId() == -1)
      {
         pVertex1->SetId(1);
         if (  pVertex1->x() > _x0 &&
            pVertex1->x() < _x1 &&
            pVertex1->y() > _y0 &&
            pVertex1->y() < _y1)
         {
            _vecEdgePoints.push_back(pVertex1->GetElevationPoint());
         }
      }

      if (pVertex2->GetId() == -1)
      {
         pVertex2->SetId(1);
         if (  pVertex2->x() > _x0 &&
            pVertex2->x() < _x1 &&
            pVertex2->y() > _y0 &&
            pVertex2->y() < _y1)
         {
            _vecEdgePoints.push_back(pVertex2->GetElevationPoint());
         }
      }
   }

   //---------------------------------------------------------------------------
   void DelaunayTriangulation::_SuperSimplexTraversal(DelaunayTriangle* pTri)
   {
      DelaunayVertex* pVertex0 = pTri->GetVertex(0);
      DelaunayVertex* pVertex1 = pTri->GetVertex(1);
      DelaunayVertex* pVertex2 = pTri->GetVertex(2);


      if (pVertex0->x() < _x0 ||
          pVertex0->x() > _x1 ||
          pVertex0->y() < _y0 ||
          pVertex0->y() > _y1)
      {
         pVertex0->GetElevationPoint().weight = -1;
      }

      if (pVertex1->x() < _x0 ||
          pVertex1->x() > _x1 ||
          pVertex1->y() < _y0 ||
          pVertex1->y() > _y1)
      {
         pVertex1->GetElevationPoint().weight = -1;
      }

      if (pVertex2->x() < _x0 ||
          pVertex2->x() > _x1 ||
          pVertex2->y() < _y0 ||
          pVertex2->y() > _y1)
      {
         pVertex2->GetElevationPoint().weight = -1;
      }
   }
   //---------------------------------------------------------------------------
   void DelaunayTriangulation::InvalidateVertices(double x0, double y0, double x1, double y1)
   {
      _x0 = x0-1e-12;
      _y0 = y0-1e-12;
      _x1 = x1+1e-12;
      _y1 = y1+1e-12;

      _qLocationStructure->Traverse(boost::bind(&DelaunayTriangulation::_SuperSimplexTraversal, this, _1));

   }

} // namespace

