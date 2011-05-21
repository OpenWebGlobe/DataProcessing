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

#include "ImageLayer.h"
#include "xml/xml.h"

//------------------------------------------------------------------------------
BeginPropertyMap(ImageLayer);
  XMLProperty(ImageLayer, "Layername" , _sLayername);
  XMLProperty(ImageLayer, "x0", _tilex0);
  XMLProperty(ImageLayer, "y0", _tiley0);
  XMLProperty(ImageLayer, "x1", _tilex1);
  XMLProperty(ImageLayer, "y1", _tiley1);
  XMLProperty(ImageLayer, "Path", _sPath);
EndPropertyMap(ImageLayer);
//------------------------------------------------------------------------------

ImageLayer::ImageLayer()
{
   //_sLayername;
   _maxlod = 0;
   _tilex0 =_tiley0 = _tilex1 = _tiley1 = 0;
   //_sPath;
}

