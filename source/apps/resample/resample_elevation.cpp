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

#include "resample_elevation.h"

#include "ogprocess.h"
#include "string/FilenameUtils.h"
#include "string/StringUtils.h"
#include "io/FileSystem.h"
#include "math/ElevationPoint.h"
#include "math/delaunay/DelaunayTriangulation.h"
#include "geo/ElevationReader.h"
#include "geo/ElevationTile.h"
#include <iostream>
#include <fstream>
#include <boost/shared_ptr.hpp>
#include <sstream>
#include <iostream>
#include <sstream>

void _resampleElevationFromParent(boost::shared_ptr<MercatorQuadtree> qQuadtree, int64 x, int64 y,int nLevelOfDetail, std::string sTileDir, std::string sTempTileDir, int nMaxpoints)
{
   // current tile:
   std::string qcCurrent = qQuadtree->TileCoordToQuadkey(x,y,nLevelOfDetail);

   // calculate parent quadkeys:
   std::string qc0,qc1,qc2,qc3;
   qc0 = qcCurrent + '0';
   qc1 = qcCurrent + '1';
   qc2 = qcCurrent + '2';
   qc3 = qcCurrent + '3';

   int64 _tx, _ty;
   int tmp_lod;

   qQuadtree->QuadKeyToTileCoord(qcCurrent, _tx, _ty, tmp_lod);
   std::string sCurrentTile_binary = ProcessingUtils::GetTilePath(sTempTileDir, ".tri" , tmp_lod, _tx, _ty);
   std::string sCurrentTile_json = ProcessingUtils::GetTilePath(sTileDir, ".json" , tmp_lod, _tx, _ty);

   qQuadtree->QuadKeyToTileCoord(qc0, _tx, _ty, tmp_lod);
   std::string sTilefile0_binary = ProcessingUtils::GetTilePath(sTempTileDir, ".tri" , tmp_lod, _tx, _ty);
   std::string sTilefile0_json = ProcessingUtils::GetTilePath(sTileDir, ".json" , tmp_lod, _tx, _ty);

   qQuadtree->QuadKeyToTileCoord(qc1, _tx, _ty, tmp_lod);
   std::string sTilefile1_binary = ProcessingUtils::GetTilePath(sTempTileDir, ".tri" , tmp_lod, _tx, _ty);
   std::string sTilefile1_json = ProcessingUtils::GetTilePath(sTileDir, ".json" , tmp_lod, _tx, _ty);

   qQuadtree->QuadKeyToTileCoord(qc2, _tx, _ty, tmp_lod);
   std::string sTilefile2_binary = ProcessingUtils::GetTilePath(sTempTileDir, ".tri" , tmp_lod, _tx, _ty);
   std::string sTilefile2_json = ProcessingUtils::GetTilePath(sTileDir, ".json" , tmp_lod, _tx, _ty);

   qQuadtree->QuadKeyToTileCoord(qc3, _tx, _ty, tmp_lod);
   std::string sTilefile3_binary = ProcessingUtils::GetTilePath(sTempTileDir, ".tri" , tmp_lod, _tx, _ty);
   std::string sTilefile3_json = ProcessingUtils::GetTilePath(sTileDir, ".json" , tmp_lod, _tx, _ty);

   // Create Empty Elevation Tiles:
   double x0,y0,x1,y1;
  
   qQuadtree->QuadKeyToMercatorCoord(qcCurrent, x0, y1, x1, y0);
   ElevationTile etCurrent(x0, y0, x1, y1);

   qQuadtree->QuadKeyToMercatorCoord(qc0, x0, y1, x1, y0);
   ElevationTile et0(x0, y0, x1, y1);

   qQuadtree->QuadKeyToMercatorCoord(qc1, x0, y1, x1, y0);
   ElevationTile et1(x0, y0, x1, y1);

   qQuadtree->QuadKeyToMercatorCoord(qc2, x0, y1, x1, y0);
   ElevationTile et2(x0, y0, x1, y1);

   qQuadtree->QuadKeyToMercatorCoord(qc3, x0, y1, x1, y0);
   ElevationTile et3(x0, y0, x1, y1);

   // read binary, failing is ok. In that case we just have an "empty tile".
   et0.ReadBinary(sTilefile0_binary);
   et1.ReadBinary(sTilefile1_binary);
   et2.ReadBinary(sTilefile2_binary);
   et3.ReadBinary(sTilefile3_binary);

   etCurrent.CreateFromParent(et0, et1, et2, et3);
   
   etCurrent.Reduce(nMaxpoints);
   
   etCurrent.WriteBinary(sCurrentTile_binary);
   
   std::string datastr = etCurrent.CreateJSON();

   std::ofstream fout(sCurrentTile_json.c_str());
   fout << datastr;
   fout.close();

}

