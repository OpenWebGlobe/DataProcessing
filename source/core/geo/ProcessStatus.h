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

#ifndef _PROCESS_STATUS_H
#define _PROCESS_STATUS_H

#include "og.h"
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>


class OPENGLOBE_API ProcessElement
{
public:
   ProcessElement();
   virtual ~ProcessElement();

   bool IsProcessing(){return _bProcessing;}
   void Processing() { _bProcessing = true;}
   void FinishedProcessing(){_bProcessing = false;}

   bool IsFinished(){return _bFinished;}
   void MarkFinished(){_bFinished = true;}
   void MarkFailed(){_bFinished = false;}

   void SetFilename(const std::string sFilename) { _sFilename = sFilename;} 
   std::string GetFilename() {return _sFilename;}

   void SetLod(int lod){_lod = lod;}
   int GetLod(){return _lod;}

   void SetExtent(int64 x0, int64 y0, int64 x1, int64 y1);
   void GetExtent(int64& x0, int64& y0, int64& x1, int64& y1);

   void SetStatusMessage(const std::string sMsg) { _sStatusMessage = sMsg;} 
   std::string GetStatusMessage() {return _sStatusMessage;}

   void SetStartTime(); // set current time as "start time"
   void SetFinishTime();   // set current time as "finish time"

protected:
   std::string _sFilename;       // filename (= unique ID)
   std::string _sStatusMessage;  // Status
   bool _bFinished;              // finished processing (true/false)
   bool _bProcessing;
   std::string _sStartTime;      // time when processing started
   std::string _sFinishTime;     // time when processing ended
   int _lod;                     // level of detail
   std::vector<int64> _vExtent;  // extent

};

//------------------------------------------------------------------------------

class OPENGLOBE_API ProcessStatus
{
public:
   ProcessStatus();
   virtual ~ProcessStatus();

   // Retrieve Element, returns 0 if not found.
   ProcessElement* GetElement(const std::string& sFilename);

   // AddElement: if element doesn't exist yet, it will be added. Returns true if it was added.
   bool AddElement(ProcessElement& element);

   // Save to XML
   bool Save(const std::string& sFilename);

   // Load from XML
   static boost::shared_ptr<ProcessStatus> Load(const std::string& sFilename);


   void SetLayerName(const std::string& sLayername) { _sLayername = sLayername;}

protected:
   std::string _sLayername;
   std::vector<ProcessElement> _vElements;  // contains all Elements to be processed and their status

};



#endif