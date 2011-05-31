

namespace math
{

   class DelaunayLocationKdTreeHierarchy : public IDelaunayLocationStructure
   {
   public:
      DelaunayLocationKdTreeHierarchy(double xmin, double ymin, double xmax, double ymax) 
      { 
         _xmin = xmin, _ymin = ymin, _xmax = xmax, _ymax = ymax;
      }

      //-----------------------------------------------------------------------

      virtual ~DelaunayLocationKdTreeHierarchy(){}

      //-----------------------------------------------------------------------

      virtual void AddTriangle(DelaunayTriangle* pTriangle) 
      {
      }

      //-----------------------------------------------------------------------

      virtual void RemoveTriangle(DelaunayTriangle* pTriangle)
      {

      }

      //-----------------------------------------------------------------------

      virtual DelaunayTriangle* GetTriangleAt(double x, double y, ePointTriangleRelation& eRelation)
      {
         return 0;
      }

      //-----------------------------------------------------------------------

      virtual void Traverse(boost::function<void(DelaunayTriangle*)> callback)
      {

      }

      //-----------------------------------------------------------------------

      virtual void SpatialTraverse(double xmin, double ymin, double xmax, double ymax, boost::function<void(DelaunayTriangle*)> callback)
      {

      }

      //-----------------------------------------------------------------------

   protected:
      double _xmin, _ymin, _xmax, _ymax;
   };

}
