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

#ifndef _POINTLAYERSETTINGS_H
#define _POINTLAYERSETTINGS_H

#include "og.h"
#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>

class OPENGLOBE_API PointLayerSettings
{
public:
   PointLayerSettings();
   virtual ~PointLayerSettings(){}

   // Setters/Getters:
   void SetLayerName(const std::string& sLayername) {_sLayername = sLayername;} 
   void SetMaxLod(int maxlod) {_maxlod = maxlod;}
   void SetTileExtent(int64 x0, int64 y0, int64 z0, int64 x1, int64 y1, int64 z1) { _tilecoord[0] = x0; _tilecoord[1] = y0; _tilecoord[2] = z0; _tilecoord[3] = x1; _tilecoord[4] = y1; _tilecoord[5] = z1;}

   std::string GetLayerName(){return _sLayername;}
   int GetMaxLod(){return _maxlod;}
   void GetTileExtent(int64& x0, int64& y0, int64& z0, int64& x1, int64& y1, int64& z1){x0 = _tilecoord[0]; y0 = _tilecoord[1]; z0 = _tilecoord[2]; x1 = _tilecoord[3]; y1 = _tilecoord[4]; z1 = _tilecoord[5];}

   // Load from XML
   static boost::shared_ptr<PointLayerSettings> Load(const std::string& layerdir);

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
