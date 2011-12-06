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
#                           robert.wueest@fhnw.ch                              #
********************************************************************************
*     Licensed under MIT License. Read the file LICENSE for more information   *
*******************************************************************************/
// Parallel processing utility
#include "og.h"
#include <vector>
#include <string>

#include <boost/shared_array.hpp>

#ifndef _QUEUEMANAGER_H
#define _QUEUEMANAGER_H

#ifdef OS_WINDOWS
   extern int truncate(const char *path, int64 length);
#else
#  include <unistd.h>
#endif

class OPENGLOBE_API QJob
{
public:
   QJob(){}
   virtual ~QJob() {}
   boost::shared_array<char> data;
   int size;
};

class OPENGLOBE_API QueueManager
{
public:
   QueueManager(){ _iCount = 0; }
   virtual ~QueueManager(){}
   void AddToJobQueue(std::string filename, QJob job, bool append = true, int autocommit = 1000);
   void CommitJobQueue(std::string filename, bool append = true);
   std::vector<QJob> FetchJobList(std::string filename, int bytes_per_job, int amount, bool verbose = false);
private:
   std::vector<QJob> _vJobs;
   int _iCount;
};


#endif

