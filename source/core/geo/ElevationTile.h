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

#ifndef _ELEVATIONTILE_H
#define _ELEVATIONTILE_H

#include "og.h"
#include "math/ElevationPoint.h"
#include "math/delaunay/DelaunayTriangulation.h"
#include <string>
#include <boost/shared_ptr.hpp>

class OPENGLOBE_API ElevationTile
{
public:
   ElevationTile(const std::string& sQuadcode, double x0, double y0, double x1, double y1);
   virtual ~ElevationTile();

   /*bool LoadElevationTile(const std::string& sQuadcode, const std::string& sRepository, const std::string& sLayer, bool bLayer = true);
   bool SaveElevationTile(const std::string& sQuadcode, const std::string& sRepository, const std::string& sLayer);
   bool SaveElevationTileJSON(const std::string& sQuadcode, const std::string& sRepository, const std::string& sLayer);
   bool SaveElevationTileOBJ(const std::string& sQuadcode, const std::string& sRepository, const std::string& sLayer);*/

   //void AddPoints(std::vector<ElevationPoints>& pts);
   //void AddCorners(const ElevationPoint& ptNW, const ElevationPoint& ptNE, const ElevationPoint& ptSE, const ElevationPoint& ptSW);
   //void AddNorthEdges(std::vector<ElevationPoints>& pts);
   //void AddEastEdges(std::vector<ElevationPoints>& pts);
   //void AddSouthEdges(std::vector<ElevationPoints>& pts);
   //void AddWestEdges(std::vector<ElevationPoints>& pts);
   
   //void CategorizePoints(double x0, double y0, double x1, double y1, std::vector<ElevationPoint>& pts);

   //boost::shared_ptr<math::DelaunayTriangulation> CreateTriangulation(bool bBorderOnly =false);

protected:
   std::vector<ElevationPoint> _ptsCorner; // 0: NW, 1: NE, 2: SE, 3:SW
   std::vector<ElevationPoint> _ptsNorth;
   std::vector<ElevationPoint> _ptsEast;
   std::vector<ElevationPoint> _ptsSouth; 
   std::vector<ElevationPoint> _ptsWest;
   std::vector<ElevationPoint> _ptsMiddle;
   std::string _sQuadcode;
   double _x0, _y0, _x1, _y1;

private:

};


#endif

