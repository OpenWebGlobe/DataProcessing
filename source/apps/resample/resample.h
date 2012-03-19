

#ifndef _RESAMPLE_IMAGE_H
#define _RESAMPLE_IMAGE_H

#include "ogprocess.h"
#include "app/ProcessingSettings.h"
#include "geo/MercatorQuadtree.h"
#include "string/FilenameUtils.h"
#include "string/StringUtils.h"
#include "io/FileSystem.h"
#include "geo/ImageLayerSettings.h"
#include "image/ImageLoader.h"
#include "image/ImageWriter.h"
#include <iostream>
#include <fstream>
#include <boost/shared_ptr.hpp>
#include <sstream>
#include <iostream>
#include <string>
#include <sstream>


//------------------------------------------------------------------------------
#define ERROR_CONFIG             3    // wrong configuration (setup.xml) (processing path or log-path is wrong)
#define ERROR_PARAMS             4    // wrong parameters
#define ERROR_IMAGELAYERSETTINGS 5
//------------------------------------------------------------------------------
const int tilesize = 256;
//------------------------------------------------------------------------------

// Holding/managing memory for an image tile
class TileBlock
{
public:
   TileBlock()
   {
      tile = new unsigned char[4*tilesize*tilesize];
   }

   virtual ~TileBlock()
   {
      if (tile) delete[] tile;
   }

   inline void Clear()
   {
      memset(tile, 0, tilesize*tilesize*4);
   }

   unsigned char* tile;
};
//------------------------------------------------------------------------------

   //------------------------------------------------------------------------------
   inline void _getInterpolatedRawColor(const float* rgbData, const size_t adr0, const size_t adr1, const size_t adr2, const size_t adr3, float* v)
   {
      float value = 0.0f;
      float v0, v1, v2, v3;
      
      v0 = rgbData[adr0];
      v1 = rgbData[adr1];
      v2 = rgbData[adr2];
      v3 = rgbData[adr3];
      value = v0 + v1 + v2 + v3;
      value/=4;
      *v = (float)value;
   }

//------------------------------------------------------------------------------

inline void _getInterpolatedColor(const unsigned char* rgbData, const size_t adr0, const size_t adr1, const size_t adr2, const size_t adr3, unsigned char* r, unsigned char* g, unsigned char* b, unsigned char* a)
{
   int red = 0;
   int green = 0;
   int blue = 0;
   int alpha = 0;

   int alpha0, alpha1, alpha2, alpha3;

   alpha0 = rgbData[adr0+3];
   alpha1 = rgbData[adr1+3];
   alpha2 = rgbData[adr2+3];
   alpha3 = rgbData[adr3+3];

   int nCount = 0;
   if (alpha0 > 0)
   {
      red   = red + rgbData[adr0];
      green = green + rgbData[adr0+1];
      blue  = blue + rgbData[adr0+2];
      alpha = alpha + alpha0;
      nCount++;      
   }

   if (alpha1 > 0)
   {
      red   = red + rgbData[adr1];
      green = green + rgbData[adr1+1];
      blue  = blue + rgbData[adr1+2];
      alpha = alpha + alpha1;
      nCount++;      
   }

   if (alpha2 > 0)
   {
      red   = red + rgbData[adr2];
      green = green + rgbData[adr2+1];
      blue  = blue + rgbData[adr2+2];
      alpha = alpha + alpha2;
      nCount++;      
   }

   if (alpha3 > 0)
   {
      red   = red + rgbData[adr3];
      green = green + rgbData[adr3+1];
      blue  = blue + rgbData[adr3+2];
      alpha = alpha + alpha3;
      nCount++;      
   }

   if (nCount>0)
   {
      red/=nCount;
      green/=nCount;
      blue/=nCount;
      alpha/=nCount;
   }

   if (red>255) red = 255;
   if (green>255) green=255;
   if (blue>255) blue=255;
   if (alpha>255) alpha=255;

   *r = (unsigned char)red;
   *g = (unsigned char)green;
   *b = (unsigned char)blue;
   *a = (unsigned char)alpha;
}

//------------------------------------------------------------------------------
TileBlock* _createTileBlockArray();
void _destroyTileBlockArray(TileBlock* pTileBlockArray);
void _resampleFromParent(TileBlock* pTileBlockArray, boost::shared_ptr<MercatorQuadtree> qQuadtree, int64 x, int64 y,int nLevelOfDetail, std::string sTileDir, bool rawData = false);
void _resampleRawImages(Raw32ImageObject* IH0, Raw32ImageObject* IH1,Raw32ImageObject* IH2,Raw32ImageObject* IH3, std::string sTargetFile, int tilesize, bool b0, bool b1, bool b2, bool b3);

//------------------------------------------------------------------------------


#endif
