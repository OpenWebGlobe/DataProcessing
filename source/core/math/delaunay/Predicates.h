
#ifndef _PREDICATES_H
#define _PREDICATES_H

#include "og.h"
#include <math/delaunay/DelaunayVertex.h>
#include <cmath>


namespace math
{
   double OPENGLOBE_API ccw(double ax, double ay, double bx, double by, double cx, double cy);
   double OPENGLOBE_API ccw(DelaunayVertex* P, DelaunayVertex* A, DelaunayVertex* B);
   bool OPENGLOBE_API InCircle(DelaunayVertex* a, DelaunayVertex* b, DelaunayVertex* c, DelaunayVertex* d);
   double OPENGLOBE_API InCircleValue(DelaunayVertex* a, DelaunayVertex* b, DelaunayVertex* c, DelaunayVertex* d);

   inline double SignedTriArea(double ax, double ay, double bx, double by, double cx, double cy)
   {
      return (bx-ax)*(cy-ay) - (by-ay)*(cx-ax);
      //return math::ccw(ax, ay, bx, by, cx, cy);
   }

   // Find intersection between 2 lines
   inline bool FindIntersection(double ax, double ay, double bx, double by, double cx, double cy, double dx, double dy, double& t)
   {
      const double epsilon = DBL_EPSILON;

      double a1 = math::SignedTriArea(ax,ay,bx,by,dx,dy);
      double a2 = math::SignedTriArea(ax,ay,bx,by,cx,cy);

      if (fabs(a1)<epsilon)
         return false;

      if (fabs(a2)<epsilon)
         return false;

      if (a1*a2 < 0.0)
      {
         double a3 = math::SignedTriArea(cx,cy,dx,dy,ax,ay);

         if (fabs(a3)<epsilon)
            return false;

         double a4 = a3 + a2 - a1;

         if (a3*a4 < 0.0)
         {
            t = a3 / (a3 - a4);
            return true;
         }
      }

      return false;
   }

}

#endif

