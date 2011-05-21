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

#ifndef _MERCATORQUADTREE_H
#define _MERCATORQUADTREE_H

#include "og.h"
#include <string>
#include <map>
#include <vector>
#include <list>
#include <boost/shared_ptr.hpp>
#include "math/vec3.h"

//#include "math/GeoCoord.h"

//-----------------------------------------------------------------------------

//! \class MercatorQuadtree
//! \author Martin Christen, martin.christen@fhnw.ch
class OPENGLOBE_API MercatorQuadtree
{
public:
   // For Earth:
   // +-------------------------------------------------------------+
   // |   WGS 84: ellipsoid_e = 0.081819190842961775161887117288255 |
   // |         ellipsoid_a = 6378137.0                             |
   // |                                                             |
   // |   Spherical: ellipsoid_e = 0                                |
   // |            ellipsoid_a = 6378137.0                          |
   // +-------------------------------------------------------------+

   MercatorQuadtree(double ellipsoid_a = 6378137.0, double ellipsoid_e=0.0, unsigned int nTextureSize=256);
   virtual ~MercatorQuadtree();

   //! \brief Change Ellipsoid parameters
   void Setup(double ellipsoid_a, double ellipsoid_e, unsigned int nTextureSize);
   
   //! \brief Change Ellipsoid parameters to Spherical Mercator Projection (WGS84)
   void SetupSphericWGS84(unsigned int nTextureSize = 256);

   //! \brief Change Ellipsoid parameters to Ellipsoid Mercator Projection (WGS84)
   void SetupEllipsoidWGS84(unsigned int nTextureSize = 256);

   //! \brief Calculate Ground Resolution at specified latitude / level of detail. [meters per pixel]
   //! \param latitude [DEG]
   //! \param level of detail [ranging from 1 to 31]
   //! \return ground resolutions in meters per pixel
   double GroundResolution(double latitude, int levelofdetail);


   //! \brief Returns Map Size at specified level of detail.
   //! \param level of detail [ranging from 1 to 31]
   //! \return width/height of map at specified level of detail
  int64 MapSize(int levelofdetail);

   //! \brief Converts WGS84 coordinate (lng, lat) to pixel coordinate.
   //! \param lng Longitude
   //! \param lat Latitude
   //! \param levelofdetail Level of Detail
   //! \param outPixelX Output Pixel coordinate X
   //! \param outPixelY Output Pixel coordinate Y
   void WGS84ToPixel(double lng, double lat, int levelofdetail, int64& outPixelX, int64& outPixelY);

   //! \brief Converts Mercator coordinate (x, y) to pixel coordinate.
   //! \param x x-value of Mercator projection (-1..1)
   //! \param y y-value of Mercator projection (-1..1)
   //! \param levelofdetail Level of Detail
   //! \param outPixelX Output Pixel coordinate X
   //! \param outPixelY Output Pixel coordinate Y
   void MercatorToPixel(double x, double y, int levelofdetail, int64& outPixelX, int64& outPixelY);

   //! \brief Convert Pixel coordinates to Tile Coordinates
   //! \param PixelX pixel coordinate x
   //! \param PixelY pixel coordinate y
   //! \param TileX tile coordinate x
   //! \param TileY tile coordinate y
   void PixelToTileCoord(int64 PixelX, int64 PixelY , int64& TileX, int64& TileY);


   //! \brief Convertes Mercator Value to WGS84 (x to longitude and y to latitude)
   void MercatorToWGS84(double& x, double& y);

   //! \brief Convertex WGS84 (lng = x, lat = y) to Mercator Projection.
   void WGS84ToMercator(double& x, double& y);

   //! \brief Convert TileXY Coordinate to Quadkey at specified level of detail
   //! \param TileX tile coordinate X
   //! \param TileY tile coordinate Y
   //! \param levelofdetail Level of detail
   //! \return std::string containing quad tree
   static std::string TileCoordToQuadkey(int64 TileX, int64 TileY, int levelofdetail);


   //! \brief Convert Quadkey to tile coordinates
   //! \param quadKey the quadkey
   //! \param out_tileX output tile coordinate X
   //! \param out_tileY output tile coordinate Y
   //! \param out_levelOfDetail output Level of detail
   //! \return true on success
   static bool QuadKeyToTileCoord(const std::string& quadKey, int64& out_tileX, int64& out_tileY, int& out_levelOfDetail);

   //! \brief Convert Quadkey to mercator coordinates
   static bool QuadKeyToMercatorCoord(const std::string& quadKey, double& x0, double& y0, double &x1, double &y1);


   //! \brief Convert Quadkey to normalized coordinates [0,0]-[1,1]
   static bool QuadKeyToNormalizedCoord(const std::string& quadKey, double& x0, double& y0, double &x1, double &y1);


   //! \brief Returns relation to parent element in quadtree (0,1,2,3). Returns -1 on invalid queries.
   //! \param quadKey the quad key ASCII string
   static int  QuadtreePosition(const std::string& quadKey);

   //! \brief Returns quadkey of parent quadtree element
   //! \param quadKey the quad key ASCII string
   static std::string  GetParent(const std::string& quadKey);

   //! \brief Returns quad value at specified position
   static int   GetQuad(const std::string& quadKey, int pos);

   //! \brief Retrieve Texture size (of one tile). This value was set in constructor.
   unsigned int GetTextureSize(){return _nTextureSize;}

private:
   double _EarthRadius;
   double _ellipsoid_e;
   double _MinLatitude;
   double _MaxLatitude;
   double _MinLongitude;
   double _MaxLongitude;
   unsigned int _nTextureSize;
};



#endif