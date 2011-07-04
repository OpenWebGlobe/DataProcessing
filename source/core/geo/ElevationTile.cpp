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

}

//------------------------------------------------------------------------------

ElevationTile::~ElevationTile()
{

}
/*
//------------------------------------------------------------------------------
#define MINELEVATION -1e20
//------------------------------------------------------------------------------

void ElevationTile::CategorizePoints(std::vector<ElevationPoint>& pts)
{
   //-----------------------------------
   // EPSILON SETTINGS:
   const double EDGE_EPSILON = 1e-15;
   //-----------------------------------
   
   _ptsCorner.clear(); _ptsNorth.clear(); _ptsEast.clear(); 
   _ptsSouth.clear(); _ptsWest.clear(); _ptsMiddle.clear();
   
   assert(_x1>_x0);
   assert(_y1>_y0);
   
   // Setup default corner points (with elevation = 0)
   ElevationPoint c0, c1, c2, c3;
   c0.x = x0;
   c0.y = y0;
   c0.elevation = MINELEVATION;
   c0.weight = -3;
   
   c1.x = x1;
   c1.y = y0;
   c1.elevation = MINELEVATION;
   c1.weight = -3;
   
   c2.x = x1;
   c2.y = y1;
   c2.elevation = MINELEVATION;
   c2.weight = -3;
   
   c3.x = x0;
   c3.y = y1;
   c3.elevation = MINELEVATION;
   c3.weight = -3;
   
   _ptsCorner.push_back(c0);
   _ptsCorner.push_back(c1);
   _ptsCorner.push_back(c2);
   _ptsCorner.push_back(c3);
   
   // precision voodoo
   double midx = x0 + 0.5*fabs(x1-x0);
   double midy = y0 + 0.5*fabs(y1-y0);
   const double boundaryEpsilon = 1e-13;
   
   for (size_t i=0;i<pts.size();i++)
   {   
      bool bReject = false;
   
      if (!bReject)
      {
         double w = pts[i].weight;
   
         bool bAdded = false;
         const double CORNER_EPSILON = DBL_EPSILON;
   
         for (size_t k=0;k<4;k++)
         {
            if (fabs(_ptsCorner[k].x - pts[i].x)<CORNER_EPSILON &&
               fabs(_ptsCorner[k].y - pts[i].y)<CORNER_EPSILON)
            {
               _ptsCorner[k].elevation = pts[i].elevation;
               bAdded = true;
            }
         }
   
   
         // Classify EDGE points (-2)
         if (!bAdded)
         {
   
            // it is very important to adjust axis of edge point to EXACT value!
            if (fabs(pts[i].x - x0) < EDGE_EPSILON) // west
            {
               ElevationPoint pt;
               pt = pts[i];
               pt.weight = -2;
               pt.x = x0;
               _ptsWest.push_back(pt);
               bAdded = true;
            }
            else if (fabs(pts[i].x - x1) < EDGE_EPSILON) // east
            {
               ElevationPoint pt;
               pt = pts[i];
               pt.weight = -2;
               pt.x = x1;
               _ptsEast.push_back(pt);
               bAdded = true;
            }
            else if ((fabs(pts[i].y - y0) < EDGE_EPSILON)) // south
            {
               ElevationPoint pt;
               pt = pts[i];
               pt.weight = -2;
               pt.y = y0;
               _ptsSouth.push_back(pt);
               bAdded = true;
            }
            else if ((fabs(pts[i].y - y1) < EDGE_EPSILON)) // north
            {
               ElevationPoint pt;
               pt = pts[i];
               pt.weight = -2;
               pt.y = y1;
               _ptsNorth.push_back(pt);
               bAdded = true;
            }
         }
   
         // Classify: Middle
         if (!bAdded)
         {
            _ptsMiddle.push_back(pts[i]);
         }
   
   
      }
   }
   
   sortpoints<ElevationPoint>(_ptsNorth);
   sortpoints<ElevationPoint>(_ptsEast);
   sortpoints<ElevationPoint>(_ptsSouth);
   sortpoints<ElevationPoint>(_ptsWest);
   sortpoints<ElevationPoint>(_ptsMiddle);
   
   EliminateCloseToCorner(_x0, DBL_EPSILON, _x1, DBL_EPSILON, _ptsNorth, fabs(_x1-_x0)/80.0);
   EliminateCloseToCorner(_x0, DBL_EPSILON, _x1, DBL_EPSILON, _ptsSouth, fabs(_x1-_x0)/80.0);
   EliminateCloseToCorner(DBL_EPSILON, _y0, DBL_EPSILON, _y1, _ptsEast, fabs(_y1-_y0)/80.0);
   EliminateCloseToCorner(DBL_EPSILON, _y0, DBL_EPSILON, _y1, _ptsWest, fabs(_y1-_y0)/80.0);
   EliminateCloseToCorner(_x0, _y0, _x1, _y1, _ptsMiddle, fabs(_y1-_y0)/80.0);
   
   EliminateDoubleEntriesX(_ptsNorth, fabs(_x1-_x0)/32.0);
   EliminateDoubleEntriesY(_ptsEast, fabs(_y1-_y0)/32.0);
   EliminateDoubleEntriesX(_ptsSouth, fabs(_x1-_x0)/32.0);
   EliminateDoubleEntriesY(_ptsWest, fabs(_y1-_y0)/32.0);
   
   // fix border tiles:
   
   //---------------------------------------------------------------------------
   // NEW 2010 - added by chm
   // if there are no middle tiles, don't connect edge tiles, just remove
   // edge tiles!!
   if (_ptsMiddle.size() == 0)
   {
      _ptsWest.clear();
      _ptsNorth.clear();
      _ptsSouth.clear();
      _ptsEast.clear();
   }
   //---------------------------------------------------------------------------
   
   for (int i=0;i<4;i++)
   {
      if (_ptsCorner[i].elevation == MINELEVATION)
      {
         _ptsCorner[i].elevation = 0.0;
      }
   }

}
*/
//------------------------------------------------------------------------------



