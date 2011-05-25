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

#ifndef _IMAGELAYER_H
#define _IMAGELAYER_H

#include "og.h"
#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>

class OPENGLOBE_API ImageLayerSettings
{
public:
   ImageLayerSettings();
   virtual ~ImageLayerSettings(){}

   // Setters/Getters:
   void SetLayerName(const std::string& sLayername) {_sLayername = sLayername;} 
   void SetMaxLod(int maxlod) {_maxlod = maxlod;}
   void SetTileExtent(int64 x0, int64 y0, int64 x1, int64 y1) { _tilecoord[0] = x0; _tilecoord[1] = y0; _tilecoord[2] = x1; _tilecoord[3] = y1;}

   std::string GetLayerName(){return _sLayername;}
   int GetMaxLod(){return _maxlod;}
   void GetTileExtent(int64& x0, int64& y0, int64& x1, int64& y1){x0 = _tilecoord[0]; y0 = _tilecoord[1]; x1 = _tilecoord[2]; y1 = _tilecoord[3];}

   // Load from XML
   static boost::shared_ptr<ImageLayerSettings> Load(const std::string& layerdir);

   // Save to XML
   bool Save(const std::string& layerdir);

protected:
   std::string _sLayername;
   std::string _sLayertype;
   int         _maxlod;
   std::string _srs;
   std::vector<int64> _tilecoord;
   

private:
   static std::string _xmlsettingsfile;
   static std::string _jsonsettingsfile;
};


#endif
