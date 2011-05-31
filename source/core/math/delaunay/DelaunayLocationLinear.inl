
#include <boost/foreach.hpp>

namespace math
{
   class OPENGLOBE_API DelaunayLocationLinear : public IDelaunayLocationStructure
   {
   public:
      DelaunayLocationLinear() {}
      virtual ~DelaunayLocationLinear(){}

      //-----------------------------------------------------------------------
   protected:

      virtual void AddTriangle(DelaunayTriangle* pTriangle) 
      {
         _lstTriangles.insert(pTriangle);
      }

      //-----------------------------------------------------------------------

      virtual void RemoveTriangle(DelaunayTriangle* pTriangle)
      {
         std::set<DelaunayTriangle*>::iterator it = _lstTriangles.find(pTriangle);
         if (it != _lstTriangles.end())
         {
            _lstTriangles.erase(it);
         }
      }

      //-----------------------------------------------------------------------

      virtual DelaunayTriangle* GetTriangleAt(double x, double y, ePointTriangleRelation& eRelation)
      {
         eRelation = PointTriangle_Invalid;
         DelaunayVertex tmpVertex(x,y);

         BOOST_FOREACH( DelaunayTriangle* pTri, _lstTriangles )
         {
            eRelation = GetPointTriangleRelationRobust(&tmpVertex,pTri, _dEpsilon);
            if (eRelation != PointTriangle_Outside)
            {
               return pTri;
            }
         }

         return 0;


         /*
         std::set<DelaunayTriangle*>::iterator it = _lstTriangles.begin();

         DelaunayVertex tmpVertex(x,y);

         while (it != _lstTriangles.end())
         {
            eRelation = GetPointTriangleRelationRobust(&tmpVertex,(*it));

            if (eRelation != PointTriangle_Outside && eRelation != PointTriangle_Invalid)
            {
               return *it;
            }

            it++;
         }
         */

         return 0;
      }

      //-----------------------------------------------------------------------

      virtual void Traverse(boost::function<void(DelaunayTriangle*)> callback)
      {
         std::set<DelaunayTriangle*>::iterator it = _lstTriangles.begin();
         while (it != _lstTriangles.end())
         {
            callback(*it); 
            ++it;
         }
      }

      //-----------------------------------------------------------------------

      virtual void SpatialTraverse(double xmin, double ymin, double xmax, double ymax, boost::function<void(DelaunayTriangle*)> callback)
      {
         std::set<DelaunayTriangle*>::iterator it = _lstTriangles.begin();
         while (it != _lstTriangles.end())
         {
            DelaunayVertex* v0 = (*it)->GetVertex(0);
            DelaunayVertex* v1 = (*it)->GetVertex(1);
            DelaunayVertex* v2 = (*it)->GetVertex(2);

            if ((v0->GetElevationPoint().x >= xmin && v0->GetElevationPoint().x <= xmax && v0->GetElevationPoint().y >= ymin && v0->GetElevationPoint().y <= ymax) ||
               (v1->GetElevationPoint().x >= xmin && v1->GetElevationPoint().x <= xmax && v1->GetElevationPoint().y >= ymin && v1->GetElevationPoint().y <= ymax) ||
               (v2->GetElevationPoint().x >= xmin && v2->GetElevationPoint().x <= xmax && v2->GetElevationPoint().y >= ymin && v2->GetElevationPoint().y <= ymax))
            {
               callback(*it); 
            }
            ++it;
         }
      }

   protected:
      std::set<DelaunayTriangle*>  _lstTriangles;
   };
}

