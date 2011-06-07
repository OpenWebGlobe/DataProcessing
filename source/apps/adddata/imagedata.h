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

#ifndef _IMAGEDATA_H
#define _IMAGEDATA_H

#include "math/mathutils.h"
#include "app/Logger.h"
#include "app/ProcessingSettings.h"
#include "ogprocess.h"
#include "errors.h"
#include <string>



namespace ImageData
{
   //------------------------------------------------------------------------------
   // Image Operations

   inline void _ReadImageDataMem(unsigned char* buffer, int bufferwidth, int bufferheight, int x, int y, unsigned char* r, unsigned char* g, unsigned char* b, unsigned char* a)
   {
      if (x<0) x = 0;
      if (y<0) y = 0;
      if (x>bufferwidth-1) x = bufferwidth-1;
      if (y>bufferheight-1) y = bufferheight-1;

      *r = buffer[bufferwidth*3*y+3*x];
      *g = buffer[bufferwidth*3*y+3*x+1];
      *b = buffer[bufferwidth*3*y+3*x+2];
      *a = 255;
   }

   //---------------------------------------------------------------------------

   inline void _ReadImageValueBilinear(unsigned char* buffer, int bufferwidth, int bufferheight, double x, double y, unsigned char* r, unsigned char* g, unsigned char* b, unsigned char* a)
   {
      double uf = math::Fract<double>(x);
      double vf = math::Fract<double>(y);
      int nPixelX = int(x);
      int nPixelY = int(y);

      int u00,v00,u10,v10,u01,v01,u11,v11;
      u00 = nPixelX;
      v00 = nPixelY;
      u10 = nPixelX+1;
      v10 = nPixelY;
      u01 = nPixelX;
      v01 = nPixelY+1;
      u11 = nPixelX+1;
      v11 = nPixelY+1;

      unsigned char r00,g00,b00,a00;
      unsigned char r10,g10,b10,a10;
      unsigned char r01,g01,b01,a01;
      unsigned char r11,g11,b11,a11;

      _ReadImageDataMem(buffer, bufferwidth, bufferheight, u00,v00,&r00,&g00,&b00,&a00);
      _ReadImageDataMem(buffer, bufferwidth, bufferheight, u10,v10,&r10,&g10,&b10,&a10);
      _ReadImageDataMem(buffer, bufferwidth, bufferheight, u01,v01,&r01,&g01,&b01,&a01);
      _ReadImageDataMem(buffer, bufferwidth, bufferheight, u11,v11,&r11,&g11,&b11,&a11);

      double rd, gd, bd, ad;

      rd = (double(r00)*(1-uf)*(1-vf)+double(r10)*uf*(1-vf)+double(r01)*(1-uf)*vf+double(r11)*uf*vf)+0.5;
      gd = (double(g00)*(1-uf)*(1-vf)+double(g10)*uf*(1-vf)+double(g01)*(1-uf)*vf+double(g11)*uf*vf)+0.5;
      bd = (double(b00)*(1-uf)*(1-vf)+double(b10)*uf*(1-vf)+double(b01)*(1-uf)*vf+double(b11)*uf*vf)+0.5;
      ad = (double(a00)*(1-uf)*(1-vf)+double(a10)*uf*(1-vf)+double(a01)*(1-uf)*vf+double(a11)*uf*vf)+0.5;

      rd = math::Clamp<double>(rd, 0.0, 255.0);
      gd = math::Clamp<double>(gd, 0.0, 255.0);
      bd = math::Clamp<double>(bd, 0.0, 255.0);
      ad = math::Clamp<double>(ad, 0.0, 255.0);

      *r = (unsigned char) rd;
      *g = (unsigned char) gd;
      *b = (unsigned char) bd;
      *a = (unsigned char) ad;
   }

   //---------------------------------------------------------------------------

   int process( boost::shared_ptr<Logger> qLogger, boost::shared_ptr<ProcessingSettings> qSettings, std::string sLayer, bool bVerbose, int epsg, std::string sImagefile, bool bFill );



}


#endif