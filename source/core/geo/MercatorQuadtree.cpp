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

#include "geo/MercatorQuadtree.h"
#include "math/mathutils.h"
#include "geo/CoordinateTransformation.h"
#include <sstream>
#include <string/StringUtils.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

//-----------------------------------------------------------------------------

MercatorQuadtree::MercatorQuadtree(double ellipsoid_a, double ellipsoid_e, unsigned int nTextureSize)
{
   _EarthRadius = ellipsoid_a;
   //_MinLatitude = -85.084059050110696;
   //_MaxLatitude = 85.084059050110696;
   _MinLongitude = -180.0;
   _MaxLongitude = 180.0;
   _ellipsoid_e = ellipsoid_e;
   _nTextureSize = nTextureSize;

   double out_lng;
   double out_lat;
   Mercator::ReverseCustom(0, 1.0, out_lng, out_lat, _ellipsoid_e);

   _MaxLatitude = out_lat;
   _MinLatitude = -_MaxLatitude;

}

//-----------------------------------------------------------------------------

MercatorQuadtree::~MercatorQuadtree()
{

}

//-----------------------------------------------------------------------------

void MercatorQuadtree::Setup(double ellipsoid_a, double ellipsoid_e, unsigned int nTextureSize)
{
   _EarthRadius = ellipsoid_a;
   _MinLongitude = -180.0;
   _MaxLongitude = 180.0;
   _ellipsoid_e = ellipsoid_e;
   _nTextureSize = nTextureSize;

   double out_lng;
   double out_lat;
   Mercator::ReverseCustom(0, 1.0, out_lng, out_lat, _ellipsoid_e);

   _MaxLatitude = out_lat;
   _MinLatitude = -_MaxLatitude;

}

//-----------------------------------------------------------------------------

void MercatorQuadtree::SetupSphericWGS84(unsigned int nTextureSize)
{
   Setup(6378137.0, 0.0, nTextureSize);
}

//-----------------------------------------------------------------------------

void MercatorQuadtree::SetupEllipsoidWGS84(unsigned int nTextureSize)
{
   Setup(6378137.0,0.081819190842961775161887117288255, nTextureSize);
}                  

//-----------------------------------------------------------------------------

double MercatorQuadtree::GroundResolution(double latitude, int levelofdetail)
{
   latitude = math::Clip<double>(latitude, _MinLatitude, _MaxLatitude);
   return cos(latitude * M_PI / 180.0) * 2 * M_PI * _EarthRadius / MapSize(levelofdetail);
}

//-----------------------------------------------------------------------------

int64 MercatorQuadtree::MapSize(int levelofdetail)
{
   return (int64)_nTextureSize << (int64)levelofdetail;
}

//-----------------------------------------------------------------------------

void MercatorQuadtree::WGS84ToPixel(double lng, double lat, int levelofdetail, int64& outPixelX, int64& outPixelY)
{
   lng = math::Clip<double>(lng, _MinLongitude, _MaxLongitude);
   lat = math::Clip<double>(lat, _MinLatitude, _MaxLatitude);

   double out_x, out_y;
   Mercator::ForwardCustom(lng, lat, out_x, out_y, _ellipsoid_e);

   MercatorToPixel(out_x, out_y, levelofdetail, outPixelX, outPixelY);
}

//-----------------------------------------------------------------------------

void MercatorQuadtree::MercatorToWGS84(double& x, double& y)
{
   double outx, outy;
   Mercator::ReverseCustom(x,y, outx, outy, _ellipsoid_e);
   x = outx;
   y = outy;
}

//-----------------------------------------------------------------------------

void MercatorQuadtree::WGS84ToMercator(double& x, double& y)
{
   double outx, outy;
   Mercator::ForwardCustom(x,y, outx, outy, _ellipsoid_e);
   x = outx;
   y = outy;
}

//-----------------------------------------------------------------------------

void MercatorQuadtree::MercatorToPixel(double x, double y, int levelofdetail, int64& outPixelX, int64& outPixelY)
{
   int64 mapsize = MapSize(levelofdetail);

   double normX = (x + 1.0) / (2.0);
   double normY = (y + 1.0) / (2.0);

   outPixelX = (int64) math::Clip<int64>( int64(normX * (mapsize-1) + 0.5), 0, mapsize-1);
   outPixelY = (int64) math::Clip<int64>( int64(normY * (mapsize-1) + 0.5), 0, mapsize-1);

   // convert to pixel coord sys:
   outPixelY = mapsize-1-outPixelY;
}

//-----------------------------------------------------------------------------

void MercatorQuadtree::PixelToTileCoord(int64 PixelX, int64 PixelY , int64& TileX, int64& TileY)
{
   TileX = PixelX / _nTextureSize;
   TileY = PixelY / _nTextureSize;
}


//-----------------------------------------------------------------------------

std::string  MercatorQuadtree::TileCoordToQuadkey(int64 TileX, int64 TileY, int levelofdetail)
{
   std::string sQuadkey;
   std::ostringstream  oss;

   for (int i=levelofdetail; i>0;i--)
   {
      char digit = '0';
      int mask = 1 << (i-1);
      if ((TileX & mask) != 0)
      {
         digit++;
      }
      if ((TileY & mask) != 0)
      {
         digit++;
         digit++;
      }

      oss << digit;
   }

   sQuadkey = oss.str();
   return sQuadkey;
}

//-----------------------------------------------------------------------------

bool MercatorQuadtree::QuadKeyToTileCoord(const std::string& quadKey, int64& out_tileX, int64& out_tileY, int& out_levelOfDetail)
{
   out_tileX = out_tileY = 0;
   out_levelOfDetail = quadKey.length();
   for (int i = out_levelOfDetail; i > 0; i--)
   {
      int64 mask = 1 << (i - 1);
      switch (quadKey[out_levelOfDetail - i])
      {
      case '0':
         break;

      case '1':
         out_tileX |= mask;
         break;

      case '2':
         out_tileY |= mask;
         break;

      case '3':
         out_tileX |= mask;
         out_tileY |= mask;
         break;

      default:
         //wrong quadkey!
         return false;   
      }
   }

   return true;
}

//-----------------------------------------------------------------------------

static inline void _QuadCoordTranslate(vec3<double>& oVec, double fX, double fY)
{
   oVec.x += fX;
   oVec.y += fY;
}

//-----------------------------------------------------------------------------

// Convert Quadkey to mercator coordinate without floating point problems!
//
// It is very important to do this over quadkey and *not* over pixels, because
// of precision problems between int (or int64) and double as pixel coordinates
// are very big at lower resolutions.

bool MercatorQuadtree::QuadKeyToMercatorCoord(const std::string& quadKey, double& x0, double& y0, double &x1, double &y1)
{
   QuadKeyToNormalizedCoord(quadKey, x0, y0, x1, y1);

   // Create Mercator Coordinates [-1,-1]-[1,1]

   x0 = 2.0*x0 - 1.0;
   y0 = 2.0*y0 - 1.0;
   x1 = 2.0*x1 - 1.0;
   y1 = 2.0*y1 - 1.0;

   return true;
}

//-----------------------------------------------------------------------------

bool MercatorQuadtree::QuadKeyToNormalizedCoord(const std::string& quadKey, double& x0, double& y0, double &x1, double &y1)
{
   double scale = 1.0;
   vec3<double> vec(0,0,0);

   int nlevelOfDetail =  quadKey.length();

   for (int i = 0; i < nlevelOfDetail; i++)
   {
      scale /= 2.0;

      switch (quadKey[i])
      {
      case '0':
         _QuadCoordTranslate(vec, 0, scale);
         break;
      case '1':
         _QuadCoordTranslate(vec, scale, scale);
         break;
      case '2':
         break;
      case '3':
         _QuadCoordTranslate(vec, scale, 0);
         break;

      default:
         //wrong quadkey!
         return false;   
      }
   }


   // This is in the correct "cartesian" way, but now it must be converted to "pixel" based
   // representation (TL, BR) coordinates:
   // +-----------##
   // |            |
   // |            |
   // |            |   Bottom Left -->   Top Left
   // |            |   Top Right   -->   Bottom Right
   // |            |
   // |            |   in other words: swap(y0,y1)
   // ##-----------+

   x0 = vec.x;
   y0 = vec.y;
   x1 = x0+scale;
   y1 = y0+scale;

   std::swap(y0,y1);


   return true;
}




//-----------------------------------------------------------------------------

int  MercatorQuadtree::QuadtreePosition(const std::string& quadKey)
{
   if (quadKey.length()<1)
   {
      assert(false);
      return -1;
   }

   std::string s = StringUtils::Right(quadKey, 1);

   if (s.length()<1)
   {
      assert(false);
      return -1;
   }

   switch (s[0])
   {
   case '0':
      return 0;
      break;
   case '1':
      return 1;
      break;
   case '2':
      return 2;
      break;
   case '3':
      return 3;
      break;
   default:
      return -1;
   }

   return -1;
}

//-----------------------------------------------------------------------------

std::string  MercatorQuadtree::GetParent(const std::string& quadKey)
{
    if (quadKey.length()<2)
    {
       assert(false);
       return std::string();
    }

    return StringUtils::Left(quadKey, quadKey.length()-1);
}

//-----------------------------------------------------------------------------

int MercatorQuadtree::GetQuad(const std::string& quadKey, int pos)
{
   int l = (int)quadKey.length();
   if (pos>l-1 || pos<0)
   {
      assert(false);
      return -1;
   }

   switch (quadKey[pos])
   {
   case '0':
      return 0;
      break;
   case '1':
      return 1;
      break;
   case '2':
      return 2;
      break;
   case '3':
      return 3;
      break;
   default:
      return -1;
   }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

inline std::string SplitQuadKey(const std::string& quadKey, int a, int b)
{
   std::string sOut;
   int l = (int)quadKey.size();
   
   if (b>l)
      b = l;

   for (int i=a;i<b;i++)
   {
      sOut += quadKey[i];
   }

   return sOut;
}

//-----------------------------------------------------------------------------
