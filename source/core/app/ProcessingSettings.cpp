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

#include "ProcessingSettings.h"
#include "xml/xml.h"
#include <fstream>

//------------------------------------------------------------------------------
BeginPropertyMap(ProcessingSettings);
  XMLProperty(ProcessingSettings, "processpath" , _sPath);
  XMLProperty(ProcessingSettings, "logpath" , _sLogPath);
EndPropertyMap(ProcessingSettings);
//------------------------------------------------------------------------------

ProcessingSettings::ProcessingSettings()
{
   
}
//------------------------------------------------------------------------------
ProcessingSettings::~ProcessingSettings()
{
}

//------------------------------------------------------------------------------
// Load ProcessingSettings from XML:
boost::shared_ptr<ProcessingSettings> ProcessingSettings::Load()
{
   boost::shared_ptr<ProcessingSettings> ret;

   std::ifstream ifs;
   ifs.open("setup.xml");

   if (ifs.good())
   {
      ProcessingSettings* pProcessingSettings = (ProcessingSettings*)access::Class::FromXML(ifs, "ProcessingSettings");

      if (pProcessingSettings)
      {
         ret = boost::shared_ptr<ProcessingSettings>(pProcessingSettings);
      }
   }

   ifs.close();

   return ret;
}

//------------------------------------------------------------------------------
// Save current class instance to XML
void ProcessingSettings::Save()
{
   std::ofstream out;
   out.open("setup.xml");
   if (out.good())
   {
      access::Class::ToXML(out, "ProcessingSettings", this);
   }

   out.close();
}

//------------------------------------------------------------------------------
