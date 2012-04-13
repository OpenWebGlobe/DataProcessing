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
#                           robert.wueest@fhnw.ch                              #
********************************************************************************
*     Licensed under MIT License. Read the file LICENSE for more information   *
*******************************************************************************/

#ifndef _RAWIMAGEDATA_H
#define _RAWIMAGEDATA_H

#include "math/mathutils.h"
#include "app/Logger.h"
#include "app/ProcessingSettings.h"
#include "ogprocess.h"
#include "errors.h"
#include "image/ImageWriter.h"
#include <string>


namespace RawImageData
{
   //------------------------------------------------------------------------------
   // Image Operations

   inline void _ReadImageDataMem(float* buffer, int bufferwidth, int bufferheight, int x, int y, float* value)
   {
      if (x<0) x = 0;
      if (y<0) y = 0;
      if (x>bufferwidth-1) x = bufferwidth-1;
      if (y>bufferheight-1) y = bufferheight-1;

      *value = buffer[bufferwidth*1*y+1*x];
   }

   //---------------------------------------------------------------------------

   inline void _ReadImageValueBilinear(float* buffer, int bufferwidth, int bufferheight, double x, double y, float* value)
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

      float value00;
      float value10;
      float value01;
      float value11;

      _ReadImageDataMem(buffer, bufferwidth, bufferheight, u00,v00,&value00);
      _ReadImageDataMem(buffer, bufferwidth, bufferheight, u10,v10,&value10);
      _ReadImageDataMem(buffer, bufferwidth, bufferheight, u01,v01,&value01);
      _ReadImageDataMem(buffer, bufferwidth, bufferheight, u11,v11,&value11);


      if (value00<-5000.0f || value10<-5000.0f || value01<-5000.0f || value11<-5000.0f)
      {
         *value = -9999.0f;
         return;
      }
      if(value00 == 0.0f)
      {
         *value = 0.0f;
         return;
      }

      double valued;

      valued = (double(value00)*(1-uf)*(1-vf)+double(value10)*uf*(1-vf)+double(value01)*(1-uf)*vf+double(value11)*uf*vf)+0.5;
      
      *value = (float) valued;
   }

   //---------------------------------------------------------------------------
   inline float cubicInterpolate (float p[4], float x) {
	   return p[1] + 0.5 * x*(p[2] - p[0] + x*(2.0*p[0] - 5.0*p[1] + 4.0*p[2] - p[3] + x*(3.0*(p[1] - p[2]) + p[3] - p[0])));
	}

	inline float bicubicInterpolate (float p[4][4], float x, float y) {
	   float arr[4];
	   arr[0] = cubicInterpolate(p[0], y);
	   arr[1] = cubicInterpolate(p[1], y);
	   arr[2] = cubicInterpolate(p[2], y);
	   arr[3] = cubicInterpolate(p[3], y);
	   return cubicInterpolate(arr, x);
	}

   inline void _ReadImageValueBicubic(float* buffer, int bufferwidth, int bufferheight, double x, double y, float* value)
   {
      double uf = math::Fract<double>(x);
      double vf = math::Fract<double>(y);

      int nPixelX = int(x)-1;
      int nPixelY = int(y)-1;

      int u00,v00,u10,v10,u20,v20,u30,v30,
          u01,v01,u11,v11,u21,v21,u31,v31,
          u02,v02,u12,v12,u22,v22,u32,v32,
          u03,v03,u13,v13,u23,v23,u33,v33;

      u00 = nPixelX;
         v00 = nPixelY;
      u10 = nPixelX+1;
         v10 = nPixelY;
      u20 = nPixelX+2;
         v20 = nPixelY;
      u30 = nPixelX+3;
         v30 = nPixelY;
      u01 = nPixelX;
         v01 = nPixelY+1;
      u11 = nPixelX+1;
         v11 = nPixelY+1;
      u21 = nPixelX+2;
         v21 = nPixelY+1;
      u31 = nPixelX+3;
         v31 = nPixelY+1;
      u02 = nPixelX;
         v02 = nPixelY+2;
      u12 = nPixelX+1;
         v12 = nPixelY+2;
      u22 = nPixelX+2;
         v22 = nPixelY+2;
      u32 = nPixelX+3;
         v32 = nPixelY+2;
      u03 = nPixelX;
         v03 = nPixelY+3;
      u13 = nPixelX+1;
         v13 = nPixelY+3;
      u23 = nPixelX+2;
         v23 = nPixelY+3;
      u33 = nPixelX+3;
         v33 = nPixelY+3;

      float p[4][4];

      _ReadImageDataMem(buffer, bufferwidth, bufferheight, u00,v00,&p[0][0]);
      _ReadImageDataMem(buffer, bufferwidth, bufferheight, u01,v01,&p[0][1]);
      _ReadImageDataMem(buffer, bufferwidth, bufferheight, u02,v02,&p[0][2]);
      _ReadImageDataMem(buffer, bufferwidth, bufferheight, u03,v03,&p[0][3]);
      _ReadImageDataMem(buffer, bufferwidth, bufferheight, u10,v10,&p[1][0]);
      _ReadImageDataMem(buffer, bufferwidth, bufferheight, u11,v11,&p[1][1]);
      _ReadImageDataMem(buffer, bufferwidth, bufferheight, u12,v12,&p[1][2]);
      _ReadImageDataMem(buffer, bufferwidth, bufferheight, u13,v13,&p[1][3]);
      _ReadImageDataMem(buffer, bufferwidth, bufferheight, u20,v20,&p[2][0]);
      _ReadImageDataMem(buffer, bufferwidth, bufferheight, u21,v21,&p[2][1]);
      _ReadImageDataMem(buffer, bufferwidth, bufferheight, u22,v22,&p[2][2]);
      _ReadImageDataMem(buffer, bufferwidth, bufferheight, u23,v23,&p[2][3]);
      _ReadImageDataMem(buffer, bufferwidth, bufferheight, u30,v30,&p[3][0]);
      _ReadImageDataMem(buffer, bufferwidth, bufferheight, u31,v31,&p[3][1]);
      _ReadImageDataMem(buffer, bufferwidth, bufferheight, u32,v32,&p[3][2]);
      _ReadImageDataMem(buffer, bufferwidth, bufferheight, u33,v33,&p[3][3]);

      *value = bicubicInterpolate(p,uf, vf);
   }

              
   //------------------------------------------------------------------------
   inline void _SaveFileAsPNG(float* buffer, int bufferwidth, int bufferheight, std::string file)
   {
      // TEMP PNG OUT -----
      boost::shared_array<char> pngTile = boost::shared_array<char>(new char[bufferwidth*bufferheight*4]);
      memset(pngTile.get(),0,bufferwidth*bufferheight*4*sizeof(char));
      for(size_t xi = 0; xi < bufferwidth*bufferheight; xi++)
      {
         size_t adr=4*xi;
         unsigned char scaledValue = (unsigned char)((buffer[xi]/1000.0f)*255.0f); //(pData.data.GetValue(dx,dy)/500)*255; //math::Floor(value);
         pngTile[adr+0] = scaledValue;  
         pngTile[adr+1] = scaledValue;  
         pngTile[adr+2] = scaledValue;
         if (buffer[xi] < -5000.0f)
            pngTile[adr+3] = 0;
         else
            pngTile[adr+3] = 255;
      }
      
      ImageWriter::WritePNG(file,(unsigned char*)pngTile.get(), 256,256);
      // ---------------->
   }
   //---------------------------------------------------------------------------

   int process( boost::shared_ptr<Logger> qLogger, boost::shared_ptr<ProcessingSettings> qSettings, std::string sLayer, bool bVerbose, bool bLock, int epsg, std::string sImagefile, bool bFill, int& out_lod, int64& out_x0, int64& out_y0, int64& out_x1, int64& out_y1 /*int maxLod = 0*/);
   //void processlod(std::string sTileDir, float* pTile, int nativeLod, int currentLod, int maxLod, int extentX, int extentY);


}


#endif
