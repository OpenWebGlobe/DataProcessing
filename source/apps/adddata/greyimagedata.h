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

#ifndef _GREYIMAGEDATA_H
#define _GREYIMAGEDATA_H

#include "math/mathutils.h"
#include "app/Logger.h"
#include "app/ProcessingSettings.h"
#include "ogprocess.h"
#include "errors.h"
#include <string>



namespace GreyImageData
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

      double valued;

      valued = (double(value00)*(1-uf)*(1-vf)+double(value10)*uf*(1-vf)+double(value01)*(1-uf)*vf+double(value11)*uf*vf)+0.5;
      
      *value = (float) valued;
   }

   //---------------------------------------------------------------------------

   int process( boost::shared_ptr<Logger> qLogger, boost::shared_ptr<ProcessingSettings> qSettings, std::string sLayer, bool bVerbose, bool bLock, int epsg, std::string sImagefile, bool bFill, int& out_lod, int64& out_x0, int64& out_y0, int64& out_x1, int64& out_y1 );



}


#endif