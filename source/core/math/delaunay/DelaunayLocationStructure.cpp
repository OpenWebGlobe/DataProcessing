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

#include "DelaunayLocationStructure.h"
#include <set>

//-----------------------------------------------------------------------------

#include "og.h"
#include <map>
#include "DelaunayLocationLinear.inl"
#include "DelaunayLocationQuadtree.inl"
#include "DelaunayLocationKdTree.inl"
#include <float.h>


namespace math
{
   IDelaunayLocationStructure::IDelaunayLocationStructure()
   { 
      _dEpsilon = DBL_EPSILON;
   }

   //--------------------------------------------------------------------------
   //--------------------------------------------------------------------------

   boost::shared_ptr<IDelaunayLocationStructure>   IDelaunayLocationStructure::CreateLocationStructure(double xmin, double ymin, double xmax, double ymax, EDelaunayLocationAlgorithms eAlgorithm)
   {
       return boost::shared_ptr<IDelaunayLocationStructure>(new DelaunayLocationLinear());


      if (eAlgorithm == DELAUNAYLOCATION_LINEARLIST)
      {
#ifdef _DEBUG
         std::cout << "Location Struct: Linear\n";
#endif
         return boost::shared_ptr<IDelaunayLocationStructure>(new DelaunayLocationLinear());
      }
      else if (eAlgorithm == DELAUNAYLOCATION_QUADTREE_HIERARCHY)
      {
#ifdef _DEBUG
         std::cout << "Location Struct: Quadtree Hierarchy\n";
#endif
         return boost::shared_ptr<IDelaunayLocationStructure>(new DelaunayLocationQuadtreeHierarchy(xmin, ymin, xmax, ymax));
      }
      else if (eAlgorithm == DELAUNAYLOCATION_KDTREE_HIERARCHY)
      {
         assert(false); // not yet implemented!!
         return boost::shared_ptr<IDelaunayLocationStructure>(new DelaunayLocationKdTreeHierarchy(xmin, ymin, xmax, ymax));
      }
      else
      {
         assert(false); // algorithm is not implemented!!
         return boost::shared_ptr<IDelaunayLocationStructure>();
      }
   }


   //--------------------------------------------------------------------------

   void IDelaunayLocationStructure::DeleteMemory(DelaunayTriangle* pTri)
   {
      DelaunayTriangle* n0 = pTri->GetTriangle(0);
      DelaunayTriangle* n1 = pTri->GetTriangle(1);
      DelaunayTriangle* n2 = pTri->GetTriangle(2);

      if (n0)
      {
         int nr0 = pTri->NeighbourReference(0);
         n0->SetTriangle(nr0, 0);
      }

      if (n1)
      {
         int nr1 = pTri->NeighbourReference(1);
         n1->SetTriangle(nr1, 0);
      }

      if (n2)
      {
         int nr2 = pTri->NeighbourReference(2);
         n2->SetTriangle(nr2, 0);
      }

      this->RemoveTriangle(pTri);
      DelaunayMemoryManager::Free(pTri);
   }

   //--------------------------------------------------------------------------

   DelaunayTriangle* IDelaunayLocationStructure::InsertVertex(DelaunayVertex* pVertex, DelaunayTriangle* pStartTriangle)
   {
      DelaunayTriangle* pResult = _InsertPointToTriangulation(pVertex);

      if (!pResult)
      {
#ifdef _DEBUG
            //std::cout << "Rejecting and deleting Vertex!\n";
#endif
            DelaunayMemoryManager::Free(pVertex);
            return pStartTriangle;
      }

      return pResult;
   }

   //--------------------------------------------------------------------------

   DelaunayTriangle* IDelaunayLocationStructure::_InsertPointToTriangulation(DelaunayVertex* pVertex)
   {
      ePointTriangleRelation eRelation;
      DelaunayTriangle* pTri = GetTriangleAt(pVertex->x(), pVertex->y(), eRelation);

      if (eRelation == PointTriangle_Inside)
      {
         assert(pTri);

         // test again with more precision!
         // this is only necessary if you go way below millimeter accuracy!
         //eRelation = GetPointTriangleRelationExact(pVertex, pTri);
         //if (eRelation == PointTriangle_Inside)
         {

            DelaunayTriangle* tri0 = DelaunayMemoryManager::AllocTriangle();
            DelaunayTriangle* tri1 = DelaunayMemoryManager::AllocTriangle();
            DelaunayTriangle* tri2 = DelaunayMemoryManager::AllocTriangle();

            tri0->SetVertex(0, pTri->GetVertex(0));
            tri0->SetVertex(1, pTri->GetVertex(1));
            tri0->SetVertex(2, pVertex);

            tri1->SetVertex(0, pTri->GetVertex(1));
            tri1->SetVertex(1, pTri->GetVertex(2));
            tri1->SetVertex(2, pVertex);

            tri2->SetVertex(0, pTri->GetVertex(2));
            tri2->SetVertex(1, pTri->GetVertex(0));
            tri2->SetVertex(2, pVertex);

            // Set new Triangle neighbours:
            tri0->SetTriangle(0, pTri->GetTriangle(0));
            tri0->SetTriangle(1, tri1);
            tri0->SetTriangle(2, tri2);

            tri1->SetTriangle(0, pTri->GetTriangle(1));
            tri1->SetTriangle(1, tri2);
            tri1->SetTriangle(2, tri0);

            tri2->SetTriangle(0, pTri->GetTriangle(2));
            tri2->SetTriangle(1, tri0);
            tri2->SetTriangle(2, tri1);

            this->AddTriangle(tri0);
            this->AddTriangle(tri1);
            this->AddTriangle(tri2);

            assert(tri0->IsCCW());
            assert(tri1->IsCCW());
            assert(tri2->IsCCW());

            // Update Neighbours
            int i,j,k;
            i = pTri->NeighbourReference(0);
            j = pTri->NeighbourReference(1);
            k = pTri->NeighbourReference(2);

            assert(pTri->IsCCW());

            DelaunayTriangle* pNeighbour0 = 0;
            DelaunayTriangle* pNeighbour1 = 0;
            DelaunayTriangle* pNeighbour2 = 0;

            if (i>=0)
            {
               pNeighbour0 = pTri->GetTriangle(0); 
            }

            if (j>=0)
            {
               pNeighbour1 = pTri->GetTriangle(1);
            }

            if (k>=0)
            {
               pNeighbour2 = pTri->GetTriangle(2);
            }

            this->DeleteMemory(pTri);

            if (pNeighbour0)
               pNeighbour0->SetTriangle(i, tri0);
            if (pNeighbour1)
               pNeighbour1->SetTriangle(j, tri1);
            if (pNeighbour2)
               pNeighbour2->SetTriangle(k, tri2);

            // LegalizeEdges:

            DelaunayTriangle::LegalizeEdges(tri0, 0);
            DelaunayTriangle::LegalizeEdges(tri1, 0);
            DelaunayTriangle::LegalizeEdges(tri2, 0);

            DelaunayTriangle::TestTriangle(tri0);
            DelaunayTriangle::TestTriangle(tri1);
            DelaunayTriangle::TestTriangle(tri2);

            assert(tri0->IsCCW());

            return tri0;
         }
         /*else
         {
         // other test result with higher precision
         std::cout << "Higher Precision: other result!!\n";
         }*/
      }

      if (eRelation == PointTriangle_Vertex0 ||
          eRelation == PointTriangle_Vertex1 ||
          eRelation == PointTriangle_Vertex2)
      {
         // If new point is on an existing vertex, it will be ignored (rejected)
         return 0; 
      }
      else if (eRelation == PointTriangle_Outside)
      {
         return 0;
      }
      else if (eRelation == PointTriangle_Invalid)
      {
         return 0;
      }
      else if (eRelation == PointTriangle_Edge0 ||
               eRelation == PointTriangle_Edge1 ||
               eRelation == PointTriangle_Edge2)
      {

         DelaunayVertex* K = pVertex;
         DelaunayVertex* A;
         DelaunayVertex* B;
         DelaunayVertex* C;
         DelaunayVertex* S = 0;
         DelaunayTriangle* pTriOpposite = 0;
         DelaunayTriangle* N0 = 0;
         DelaunayTriangle* N1 = 0;
         DelaunayTriangle* N2 = 0;
         DelaunayTriangle* N3 = 0;
         int N0Back = -1;
         int N1Back = -1;
         int N2Back = -1;
         int N3Back = -1;

         int k;

         if (eRelation == PointTriangle_Edge0)
         {
            k = 0;
         }
         else if (eRelation == PointTriangle_Edge1)
         {
            k = 1;
         }
         else if (eRelation == PointTriangle_Edge2)
         {
            k = 2;
         }
         else
         {
            assert(false);
         }

         A = pTri->GetVertex(k%3);
         B = pTri->GetVertex((k+1)%3);
         C = pTri->GetVertex((k+2)%3);

         S = pTri->GetOppositeVertex(k);
         pTriOpposite = pTri->GetTriangle(k);

         N0 = pTri->GetTriangle((k+2)%3);
         N3 = pTri->GetTriangle((k+1)%3);

         int t = pTri->NeighbourReference(k);

         N0Back = pTri->NeighbourReference((k+2)%3);
         N3Back = pTri->NeighbourReference((k+1)%3);

         if (pTriOpposite)
         {
            N1 = pTriOpposite->GetTriangle((t+1)%3);
            N2 = pTriOpposite->GetTriangle((t+2)%3);

            N1Back = pTriOpposite->NeighbourReference((t+1)%3);
            N2Back = pTriOpposite->NeighbourReference((t+2)%3);
         }


         DelaunayTriangle* T0 = 0;
         DelaunayTriangle* T1 = 0;
         DelaunayTriangle* T2 = 0;
         DelaunayTriangle* T3 = 0;

         if (pTri)
         {
            this->RemoveTriangle(pTri); // "recycle" triangle: pointer is not deleted!
         }
           
         T0 = pTri; // recycle triangle!
         T1 = DelaunayMemoryManager::AllocTriangle();

         if (pTriOpposite)
         {
            this->RemoveTriangle(pTriOpposite); // "recycle" triangle: pointer is not deleted!
            T2 = pTriOpposite; // recycle triangle!
            T3 = DelaunayMemoryManager::AllocTriangle();
         }

         T0->SetTriangle(0, N0);
         T0->SetTriangle(1, T2);
         T0->SetTriangle(2, T1);

         T0->SetVertex(0, C);
         T0->SetVertex(1, A);
         T0->SetVertex(2, K);

         T1->SetTriangle(0, N3);
         T1->SetTriangle(1, T0);
         T1->SetTriangle(2, T3);

         T1->SetVertex(0, B);
         T1->SetVertex(1, C);
         T1->SetVertex(2, K);

         if (T2 && T3)
         {
            T2->SetTriangle(0, N1);
            T2->SetTriangle(1, T3);
            T2->SetTriangle(2, T0);

            T2->SetVertex(0, A);
            T2->SetVertex(1, S);
            T2->SetVertex(2, K);

            T3->SetTriangle(0, N2);
            T3->SetTriangle(1, T1);
            T3->SetTriangle(2, T2);

            T3->SetVertex(0, S);
            T3->SetVertex(1, B);
            T3->SetVertex(2, K);
         }

         if (N0 && N0Back != -1)
         {
            N0->SetTriangle(N0Back, T0);
         }
         if (N1 && N1Back != -1)
         {
            N1->SetTriangle(N1Back, T2);
         }
         if (N2 && N2Back != -1)
         {
            N2->SetTriangle(N2Back, T3);
         }
         if (N3 && N3Back != -1)
         {
            N3->SetTriangle(N3Back, T1);
         }

         if (T0) this->AddTriangle(T0); // add "recycled" triangle ("old pointer")
         if (T2) this->AddTriangle(T2); // add "recycled" triangle ("old pointer")
         if (T1) this->AddTriangle(T1);
         if (T3) this->AddTriangle(T3);

#ifdef _DEBUG
         //std::cout << "EDGE!\n";
#endif

         if (T0) DelaunayTriangle::LegalizeEdges(T0, 0);
         if (T1) DelaunayTriangle::LegalizeEdges(T1, 0);
         if (T2) DelaunayTriangle::LegalizeEdges(T2, 0);
         if (T3) DelaunayTriangle::LegalizeEdges(T3, 0);

         if (T0) DelaunayTriangle::TestTriangle(T0);
         if (T1) DelaunayTriangle::TestTriangle(T1);
         if (T2) DelaunayTriangle::TestTriangle(T2);
         if (T3) DelaunayTriangle::TestTriangle(T3);

         return T0;
      }


      return 0;
   }


   //--------------------------------------------------------------------------
}