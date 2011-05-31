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

namespace math
{
   namespace DelaunayAcceleration
   {

      //-----------------------------------------------------------------------

      inline bool PointInRect(double px, double py, double xmin, double ymin, double xmax, double ymax)
      {
         bool bResult = true;
         bResult = bResult && (px >= xmin && px<= xmax); 
         bResult = bResult && (py >= ymin && py<= ymax); 
         return bResult;
      }

      //-----------------------------------------------------------------------

      inline bool RectInRect(double xmin_A, double ymin_A, double xmax_A, double ymax_A,  
         double xmin_B, double ymin_B, double xmax_B, double ymax_B)
      {
         if (xmax_A <= xmin_B)
            return false;

         if (ymax_A <= ymin_B)
            return false;

         if (xmax_B <= xmin_A)
            return false;

         if (ymax_B <= ymin_A)
            return false;

         return true;
      };

      //-----------------------------------------------------------------------

      inline bool TriangleInRect(DelaunayTriangle* pTri, double xmin, double ymin, double xmax, double ymax)
      {
         assert(pTri);

         const ElevationPoint& PA = pTri->GetVertex(0)->GetElevationPoint();
         bool bA = PointInRect(PA.x, PA.y, xmin, ymin, xmax, ymax);
         const ElevationPoint& PB = pTri->GetVertex(1)->GetElevationPoint();
         bool bB = PointInRect(PB.x, PB.y, xmin, ymin, xmax, ymax);
         const ElevationPoint& PC = pTri->GetVertex(2)->GetElevationPoint();
         bool bC = PointInRect(PC.x, PC.y, xmin, ymin, xmax, ymax);

         return (bA && bB && bC);
      }

      //-----------------------------------------------------------------------

      class OPENGLOBE_API QuadtreeNode
      {
      public:

         //-----------------------------------------------------------------------

         QuadtreeNode(int nDepth)
            : _nDepth(nDepth)
         {
            _child[0] =  _child[1] = _child[2] = _child[3] = 0;
            _xmin = _ymin = _xmax = _ymax = 0.0;
         }

         //-----------------------------------------------------------------------

         QuadtreeNode(int nDepth, double xmin, double ymin, double xmax, double ymax)
            : _nDepth(nDepth)
         {
            _child[0] =  _child[1] = _child[2] = _child[3] = 0;
            _xmin = xmin;
            _ymin = ymin;
            _xmax = xmax;
            _ymax = ymax;
         }

         //--------------------------------------------------------------------
         
         virtual ~QuadtreeNode()
         {
            for (int i=0;i<4;i++)
            {
               if (_child[i])
               {
                  delete _child[i];
                  _child[i] = 0;
               }
            }
         }

         //--------------------------------------------------------------------
         // Child Numbering:
         //
         // +---+---+
         // | 0 | 1 |
         // +---+---+
         // | 2 | 3 |
         // +---+---+
         //
         void GetChildRectangle(int nChild, double& xmin, double& ymin, double& xmax, double& ymax)
         {
            double l = _xmax-_xmin;

            switch (nChild)
            {
            case 0:
               xmin = _xmin;       
               ymin = _ymin+l/2.0;
               xmax = _xmin+l/2.0;
               ymax = _ymin+l;
               break;
            case 1:
               xmin = _xmin+l/2.0;
               ymin = _ymin+l/2.0;
               xmax = _xmax;
               ymax = _ymax;
               break;
            case 2:
               xmin = _xmin;
               ymin = _ymin;       
               xmax = _xmin+l/2.0;
               ymax = _ymin+l/2.0;
               break;
            case 3:
               xmin = _xmin+l/2.0;
               ymin = _ymin;
               xmax = _xmax;
               ymax = _ymin+l/2.0;
               break;
            default:
               assert(false); // child must be 0,1,2 or 3. Other values are not allowed!!
            }
           
         }

         //--------------------------------------------------------------------

         bool TriangleFitsInside(DelaunayTriangle* pTri, int nChild)
         {
            assert(pTri);
            assert(nChild<=3 && nChild>=0);

            double xmin, ymin, xmax, ymax;
            GetChildRectangle(nChild, xmin, ymin, xmax, ymax);

            return TriangleInRect(pTri, xmin, ymin, xmax, ymax); 
         }

         //--------------------------------------------------------------------
         
         bool PointFitsInside(double x, double y, int nChild)
         {
            assert(nChild<=3 && nChild>=0);

            if (_child[nChild])
            {
               return PointInRect(x, y, _child[nChild]->_xmin, _child[nChild]->_ymin, _child[nChild]->_xmax, _child[nChild]->_ymax);
            }
            else
               return false;
         }

         //--------------------------------------------------------------------
         
         void CreateChildIfNecessary(int nChild)
         {
            assert(nChild<=3 && nChild>=0);

            if (_child[nChild] == 0)
            {
               double xmin, ymin, xmax, ymax;
               GetChildRectangle(nChild, xmin, ymin, xmax, ymax);
               _child[nChild] = new QuadtreeNode(_nDepth+1, xmin, ymin, xmax, ymax); 
            }
         }

         //--------------------------------------------------------------------

         void InsertTriangle(DelaunayTriangle* pTri, const int nMaxDepth, std::map<DelaunayTriangle*, QuadtreeNode*>* pMap)
         {
            if (TriangleFitsInside(pTri, 0))
            {
               if (_nDepth < nMaxDepth)
               {
                  CreateChildIfNecessary(0);
                  _child[0]->InsertTriangle(pTri, nMaxDepth, pMap);
               }
               else
               {
                  _lstTriangles.insert(pTri);
                  pMap->insert(std::pair<DelaunayTriangle*, QuadtreeNode*>(pTri, this));
               }  
            }
            else if (TriangleFitsInside(pTri, 1))
            {
               if (_nDepth < nMaxDepth)
               {
                  CreateChildIfNecessary(1);
                  _child[1]->InsertTriangle(pTri, nMaxDepth, pMap);
               }
               else
               {
                  _lstTriangles.insert(pTri);
                  pMap->insert(std::pair<DelaunayTriangle*, QuadtreeNode*>(pTri, this));
               }
            }
            else if (TriangleFitsInside(pTri, 2))
            {
               if (_nDepth < nMaxDepth)
               {
                  CreateChildIfNecessary(2);
                  _child[2]->InsertTriangle(pTri, nMaxDepth, pMap);
               }
               else
               {
                  _lstTriangles.insert(pTri);
                  pMap->insert(std::pair<DelaunayTriangle*, QuadtreeNode*>(pTri, this));
               }
            }
            else if (TriangleFitsInside(pTri, 3))
            {
               if (_nDepth < nMaxDepth)
               {
                  CreateChildIfNecessary(3);
                  _child[3]->InsertTriangle(pTri, nMaxDepth, pMap);
               }
               else
               {
                  _lstTriangles.insert(pTri);
                  pMap->insert(std::pair<DelaunayTriangle*, QuadtreeNode*>(pTri, this));
               }
            }
            else
            {
               // doesn't fit in a child cell, put it in current cell!
               _lstTriangles.insert(pTri);
               pMap->insert(std::pair<DelaunayTriangle*, QuadtreeNode*>(pTri, this));
            }
         }
         //--------------------------------------------------------------------

         void GetTriangleAt(double x, double y, DelaunayTriangle*& out_pTriangle, ePointTriangleRelation& out_eRelation, const double epsilon)
         {
            if (out_pTriangle != 0) // found triangle ?
               return;

            // Test if Triangle is in current list...
            { // local scope intentional
               DelaunayVertex tmpVertex(x,y);

               BOOST_FOREACH( DelaunayTriangle* pTri, _lstTriangles )
               {
                  out_eRelation = GetPointTriangleRelationRobust(&tmpVertex,pTri, epsilon);

                  if (out_eRelation != PointTriangle_Outside)
                  {
                     out_pTriangle = pTri;
                     return;
                  }
               }
            }

            if (PointFitsInside(x, y, 0))
            {
               _child[0]->GetTriangleAt(x,y,out_pTriangle, out_eRelation, epsilon);
            }
            
            if (PointFitsInside(x, y, 1))
            {
               _child[1]->GetTriangleAt(x,y,out_pTriangle, out_eRelation, epsilon);
            }
            
            if (PointFitsInside(x, y, 2))
            { 
               _child[2]->GetTriangleAt(x,y,out_pTriangle, out_eRelation, epsilon);
            }
            
            if (PointFitsInside(x, y, 3))
            {
               _child[3]->GetTriangleAt(x,y,out_pTriangle, out_eRelation, epsilon);
            }
         }

         //--------------------------------------------------------------------

         void Traverse(boost::function<void(DelaunayTriangle*)> callback)
         {
            // Callback for each triangle in current list...
            {
               BOOST_FOREACH( DelaunayTriangle* pTri, _lstTriangles )
               {
                   callback(pTri);
               }
            }
         
            if (_child[0])
            {
               _child[0]->Traverse(callback);
            }

            if (_child[1])
            {
               _child[1]->Traverse(callback);
            }

            if (_child[2])
            {
               _child[2]->Traverse(callback);
            }

            if (_child[3])
            {
               _child[3]->Traverse(callback);
            }
         
         }

         //--------------------------------------------------------------------

         void SpatialTraverse(double xmin, double ymin, double xmax, double ymax, boost::function<void(DelaunayTriangle*)> callback)
         {
            if (RectInRect(xmin, ymin, xmax, ymax, _xmin, _ymin, _xmax, _ymax))
            {
               BOOST_FOREACH( DelaunayTriangle* pTri, _lstTriangles )
               {
                  if (TriangleInRect(pTri, xmin, ymin, xmax, ymax))
                  {
                     callback(pTri);
                  }
               }

               if (_child[0])
               {
                  _child[0]->SpatialTraverse(xmin, ymin, xmax, ymax, callback);
               }

               if (_child[1])
               {
                  _child[1]->SpatialTraverse(xmin, ymin, xmax, ymax, callback);
               }

               if (_child[2])
               {
                  _child[2]->SpatialTraverse(xmin, ymin, xmax, ymax, callback);
               }

               if (_child[3])
               {
                  _child[3]->SpatialTraverse(xmin, ymin, xmax, ymax, callback);
               }
            }
         }

         //--------------------------------------------------------------------

         bool RemoveTriangle(DelaunayTriangle* pTriangle)
         {
            //#todo: if last triangle was removed and there are no children
            //       then delete this quadtree node!
            std::set<DelaunayTriangle*>::iterator it = _lstTriangles.find(pTriangle);
            if (it != _lstTriangles.end())
            {
               _lstTriangles.erase(it);
               pTriangle = 0;
               return true;
            } 

            return false;
         }


  
         //--------------------------------------------------------------------


      protected:
         double _xmin, _ymin, _xmax, _ymax;
         QuadtreeNode* _child[4];
         int _nDepth;
         std::set<DelaunayTriangle*> _lstTriangles;
      private:
         QuadtreeNode(){}
      };

   }

   //--------------------------------------------------------------------------

   class OPENGLOBE_API DelaunayLocationQuadtreeHierarchy : public IDelaunayLocationStructure
   {
   public:

      //-----------------------------------------------------------------------

      DelaunayLocationQuadtreeHierarchy(double xmin, double ymin, double xmax, double ymax) 
         : _xmin(xmin), _ymin(ymin), _xmax(xmax), _ymax(ymax)
      {
         _nMaxDepth = 20; // Maximum Quadtree Depth

         _pQuadtree = new DelaunayAcceleration::QuadtreeNode(0, _xmin, _ymin, _xmax, _ymax);
        
      }

      //-----------------------------------------------------------------------
      
      virtual ~DelaunayLocationQuadtreeHierarchy()
      {
         if (_pQuadtree)
         {
            delete _pQuadtree;
            _pQuadtree = 0;
         }
      }

      //-----------------------------------------------------------------------

      virtual void AddTriangle(DelaunayTriangle* pTriangle) 
      {
         // Triangle must have position info!!
         assert(pTriangle->GetVertex(0));
         assert(pTriangle->GetVertex(1));
         assert(pTriangle->GetVertex(2));

         _pQuadtree->InsertTriangle(pTriangle, _nMaxDepth, &_TriQuadMap);
      }

      //-----------------------------------------------------------------------

      virtual void RemoveTriangle(DelaunayTriangle* pTriangle)
      {

         std::map<DelaunayTriangle*, DelaunayAcceleration::QuadtreeNode*>::iterator it;
         it = _TriQuadMap.find(pTriangle);

         if (it != _TriQuadMap.end())
         {
            DelaunayAcceleration::QuadtreeNode* pNode = it->second;
            _TriQuadMap.erase(it);

            if (pNode)
            {
               if (pNode->RemoveTriangle(pTriangle))
                  return;
            }
         }
    
         
         //std::cout << "<b>**FATAL ERROR**: FAILED REMOVING TRIANGLE!!</b>\n";
      
      }

      //-----------------------------------------------------------------------

      virtual DelaunayTriangle* GetTriangleAt(double x, double y, ePointTriangleRelation& eRelation)
      {
         DelaunayTriangle* pTriangle = 0;
         _pQuadtree->GetTriangleAt(x, y, pTriangle, eRelation, _dEpsilon);
         
         if (pTriangle == 0) // no triangle found at specified position!
         {
            eRelation = PointTriangle_Invalid;
         }
         
         return pTriangle;
      }

      //-----------------------------------------------------------------------

      virtual void Traverse(boost::function<void(DelaunayTriangle*)> callback)
      {
         // Traverse all triangles and call specified function
         _pQuadtree->Traverse(callback);
      }

      //-----------------------------------------------------------------------

      virtual void SpatialTraverse(double xmin, double ymin, double xmax, double ymax, boost::function<void(DelaunayTriangle*)> callback)
      {
         // Traverse all triangles that fit in specified region!
         _pQuadtree->SpatialTraverse(xmin, ymin, xmax, ymax, callback);
      }

      //-----------------------------------------------------------------------

   protected:
      DelaunayAcceleration::QuadtreeNode*  _pQuadtree;
      double _xmin, _ymin, _xmax, _ymax;
      int _nMaxDepth;

      std::map<DelaunayTriangle*, DelaunayAcceleration::QuadtreeNode*> _TriQuadMap;
   };

}
