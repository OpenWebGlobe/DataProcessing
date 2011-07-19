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

#include "ImageLayerSettings.h"
#include "xml/xml.h"
#include "string/FilenameUtils.h"
#include <fstream>

//------------------------------------------------------------------------------
BeginPropertyMap(ImageLayerSettings);
  XMLProperty(ImageLayerSettings, "name" , _sLayername);
  XMLProperty(ImageLayerSettings, "type" , _sLayertype);
  XMLProperty(ImageLayerSettings, "srs", _srs);
  XMLProperty(ImageLayerSettings, "maxlod", _maxlod);
  XMLProperty(ImageLayerSettings, "extent", _tilecoord);
  XMLProperty(ImageLayerSettings, "format", _sFormat);
EndPropertyMap(ImageLayerSettings);
//------------------------------------------------------------------------------

std::string ImageLayerSettings::_xmlsettingsfile = "layersettings.xml";
std::string ImageLayerSettings::_jsonsettingsfile = "layersettings.json";


ImageLayerSettings::ImageLayerSettings()
{
   //_sLayername; // empty
   _sLayertype = "image";
   _sFormat = "png";
   _maxlod = 0;
   _srs = "EPSG:3857";
   _tilecoord.push_back(0);
   _tilecoord.push_back(0);
   _tilecoord.push_back(0);
   _tilecoord.push_back(0);
}


//------------------------------------------------------------------------------
// Load ImageLayerSettings from XML:
boost::shared_ptr<ImageLayerSettings> ImageLayerSettings::Load(const std::string& layerdir)
{
   boost::shared_ptr<ImageLayerSettings> ret;

   std::ifstream ifs;
   ifs.open((FilenameUtils::DelimitPath(layerdir) + _xmlsettingsfile).c_str());

   if (ifs.good())
   {
      ImageLayerSettings* pImageLayerSettings = (ImageLayerSettings*)Access::Class::FromXML(ifs, "ImageLayerSettings");

      if (pImageLayerSettings)
      {
         ret = boost::shared_ptr<ImageLayerSettings>(pImageLayerSettings);
      }
   }

   ifs.close();

   return ret;
}

//------------------------------------------------------------------------------
// Save current class instance to XML and json
bool ImageLayerSettings::Save(const std::string& layerdir)
{
   bool ret = true;
   std::ofstream out;
   out.open((FilenameUtils::DelimitPath(layerdir) + _xmlsettingsfile).c_str());
   if (out.good())
   {
      Access::Class::ToXML(out, "ImageLayerSettings", this);
   }
   else
   {
      ret = false;
   }

   out.close();

   // write json file (for WebGL version of the globe)
   std::ofstream jout;
   jout.open((FilenameUtils::DelimitPath(layerdir) + _jsonsettingsfile).c_str());

   if (jout.good())
   {

      jout << "{\n";
      jout << "   \"name\" : \"" << _sLayername << "\",\n";
      jout << "   \"type\" : \"" << _sLayertype << "\",\n";
      jout << "   \"format\" : \"" << _sFormat << "\",\n";
      jout << "   \"maxlod\" : " << _maxlod << ",\n";
      jout << "   \"extent\" : " << "[" << _tilecoord[0] << ", " << _tilecoord[1] << ", " << _tilecoord[2] << ", " << _tilecoord[3] << "]\n";
      jout << "}\n";
   }

   jout.close();


   return ret;
}
