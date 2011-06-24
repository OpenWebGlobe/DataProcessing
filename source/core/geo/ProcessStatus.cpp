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

#include "ProcessStatus.h"
#include "xml/xml.h"
#include <fstream>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace boost::posix_time;
using namespace boost::gregorian;

//------------------------------------------------------------------------------

BeginPropertyMap(ProcessElement);
   XMLProperty(ProcessElement, "File", _sFilename);
   XMLProperty(ProcessElement, "Status", _sStatusMessage);
   XMLProperty(ProcessElement, "Finished", _bFinished);
   XMLProperty(ProcessElement, "Processing", _bProcessing);
   XMLProperty(ProcessElement, "StartTime", _sStartTime);
   XMLProperty(ProcessElement, "FinishTime", _sFinishTime);
   //XMLProperty(ProcessElement, "lod", _lod);
   //XMLProperty(ProcessElement, "extent", _vExtent);
EndPropertyMap(ProcessElement);

// ProcessElement can be seerialized as std::vector<ProcessElement>
SerializeVector(ProcessElement);

//------------------------------------------------------------------------------

BeginPropertyMap(ProcessStatus);
   XMLProperty(ImageLayerSettings, "layer" , _sLayername);
   XMLProperty(ImageLayerSettings, "Elements", _vElements);
EndPropertyMap(ProcessStatus);

//------------------------------------------------------------------------------

ProcessElement::ProcessElement()
{
   _bFinished = false;
   _bProcessing = false;
   _sStatusMessage = "unknown";
   /*_vExtent.push_back(0);
   _vExtent.push_back(0);
   _vExtent.push_back(0);
   _vExtent.push_back(0);*/

}


//------------------------------------------------------------------------------

ProcessElement::~ProcessElement()
{

}

//------------------------------------------------------------------------------

/*void ProcessElement::SetExtent(int64 x0, int64 y0, int64 x1, int64 y1)
{
   _vExtent[0] = x0;
   _vExtent[1] = y0;
   _vExtent[2] = x1;
   _vExtent[3] = y1;
}

//------------------------------------------------------------------------------

void ProcessElement::GetExtent(int64& x0, int64& y0, int64& x1, int64& y1)
{
   x0 = _vExtent[0];
   y0 = _vExtent[1];
   x1 = _vExtent[2];
   y1 = _vExtent[3];
}
*/
//------------------------------------------------------------------------------

void ProcessElement::SetStartTime()
{
   ptime now = microsec_clock::local_time();
   _sStartTime = to_iso_string(now);
}

//------------------------------------------------------------------------------

void ProcessElement::SetFinishTime()
{
   ptime now = microsec_clock::local_time();
   _sFinishTime = to_iso_string(now);
}

//------------------------------------------------------------------------------
/******************************************************************************/
//------------------------------------------------------------------------------

ProcessStatus::ProcessStatus()
{

}

//------------------------------------------------------------------------------

ProcessStatus::~ProcessStatus()
{

}

//------------------------------------------------------------------------------

ProcessElement* ProcessStatus::GetElement(const std::string& sFilename)
{
   for (size_t i=0;i<_vElements.size();i++)
   {
      if (_vElements[i].GetFilename() == sFilename)
      {
         return &_vElements[i];
      }
   }

   return 0;
}

//------------------------------------------------------------------------------

bool ProcessStatus::AddElement(ProcessElement& element)
{
   bool bNew = true;
   for (size_t i=0;i<_vElements.size();i++)
   {
      if (_vElements[i].GetFilename() == element.GetFilename())
      {
         return false; // already exists
      }
   }

   // new element: add it..
   _vElements.push_back(element);
   return true;

}

//------------------------------------------------------------------------------

boost::shared_ptr<ProcessStatus> ProcessStatus::Load(const std::string& sFilename)
{
   boost::shared_ptr<ProcessStatus> ret;

   std::ifstream ifs;
   ifs.open(sFilename.c_str());

   if (ifs.good())
   {
      ProcessStatus* pProcessStatus = (ProcessStatus*)Access::Class::FromXML(ifs, "ProcessStatus");

      if (pProcessStatus)
      {
         ret = boost::shared_ptr<ProcessStatus>(pProcessStatus);
      }
   }

   ifs.close();

   return ret;
}

//------------------------------------------------------------------------------

bool ProcessStatus::Save(const std::string& sFilename)
{
   bool ret = true;

   std::ofstream out;
   out.open(sFilename.c_str());
   if (out.good())
   {
      Access::Class::ToXML(out, "ProcessStatus", this);
   }
   else
   {
      ret = false;
   }

   out.close();

   return ret;
}

//------------------------------------------------------------------------------