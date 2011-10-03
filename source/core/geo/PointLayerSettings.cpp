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

#include "PointLayerSettings.h"
#include "xml/xml.h"
#include "string/FilenameUtils.h"
#include <fstream>

//------------------------------------------------------------------------------
BeginPropertyMap(PointLayerSettings);
  XMLProperty(PointLayerSettings, "name" , _sLayername);
  XMLProperty(PointLayerSettings, "type" , _sLayertype);
  XMLProperty(PointLayerSettings, "srs", _srs);
  XMLProperty(PointLayerSettings, "maxlod", _maxlod);
  XMLProperty(PointLayerSettings, "extent", _boundary);
EndPropertyMap(PointLayerSettings);
//------------------------------------------------------------------------------

std::string PointLayerSettings::_xmlsettingsfile = "layersettings.xml";
std::string PointLayerSettings::_jsonsettingsfile = "layersettings.json";


PointLayerSettings::PointLayerSettings()
{
   //_sLayername; // empty
   _sLayertype = "point";
   _maxlod = 0;
   _srs = "EPSG:3857";
   _boundary.push_back(0.0);
   _boundary.push_back(0.0);
   _boundary.push_back(0.0);
   _boundary.push_back(0.0);
   _boundary.push_back(0.0);
   _boundary.push_back(0.0);
}


//------------------------------------------------------------------------------
// Load ImageLayerSettings from XML:
boost::shared_ptr<PointLayerSettings> PointLayerSettings::Load(const std::string& layerdir)
{
   boost::shared_ptr<PointLayerSettings> ret;

   std::ifstream ifs;
   ifs.open((FilenameUtils::DelimitPath(layerdir) + _xmlsettingsfile).c_str());

   if (ifs.good())
   {
      PointLayerSettings* pPointLayerSettings = (PointLayerSettings*)Access::Class::FromXML(ifs, "PointLayerSettings");

      if (pPointLayerSettings)
      {
         ret = boost::shared_ptr<PointLayerSettings>(pPointLayerSettings);
      }
   }

   ifs.close();

   return ret;
}

//------------------------------------------------------------------------------
// Save current class instance to XML and json
bool PointLayerSettings::Save(const std::string& layerdir)
{
   bool ret = true;
   std::ofstream out;
   out.open((FilenameUtils::DelimitPath(layerdir) + _xmlsettingsfile).c_str());
   if (out.good())
   {
      Access::Class::ToXML(out, "PointLayerSettings", this);
   }
   else
   {
      ret = false;
   }

   out.close();

   // write json file (for WebGL version of the globe)
   std::ofstream jout;
   jout.open((FilenameUtils::DelimitPath(layerdir) + _jsonsettingsfile).c_str());
   jout.precision(17);

   if (jout.good())
   {

      jout << "{\n";
      jout << "   \"name\" : \"" << _sLayername << "\",\n";
      jout << "   \"type\" : \"" << _sLayertype << "\",\n";
      jout << "   \"maxlod\" : " << _maxlod << ",\n";
      jout << "   \"extent\" : " << "[" << _boundary[0] << ", " << _boundary[1] << ", " << _boundary[2] << ", " << _boundary[3] << ", " << _boundary[4] << ", " << _boundary[5] << "]\n";
      jout << "}\n";
   }

   jout.close();

   return ret;
}
