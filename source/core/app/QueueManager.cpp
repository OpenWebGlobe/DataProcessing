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
#include "QueueManager.h"
#include <iostream>
#include <fstream>
#include <io/FileSystem.h>
#include <boost/filesystem.hpp>

//------------------------------------------------------------------------------

void QueueManager::AddToJobQueue(std::string filename, QJob job)
{
   int lockhandle = FileSystem::Lock(filename);

   if(!FileSystem::FileExists(filename))
   {
      // create new
      std::fstream off(filename.c_str(), std::ios::out | std::ios::binary);
      if (off.good())
      {
         int len = job.size;
         off.write((char*)job.data.get(), (std::streamsize)len);
         off.close();
      }
   }
   else
   {
      // append
      std::fstream off(filename.c_str(),std::ios::out | std::ios::app | std::ios::binary);
      if (off.good())
      {
         int len = job.size;
         off.write((char*)job.data.get(), (std::streamsize)len);
         off.close();
      }
   }
   FileSystem::Unlock(filename, lockhandle);
}

//------------------------------------------------------------------------------

std::vector<QJob> QueueManager::FetchJobList(std::string filename, int bytes_per_job, int amount)
{
   int lockhandle = FileSystem::Lock(filename);
   boost::filesystem3::path filepath(filename);
   std::vector<QJob> jobs;
   int currentSize = boost::filesystem3::file_size(filepath);
   std::ifstream ifs;
   ifs.open(filename.c_str(), std::ios::in | std::ios::binary);
   int chunkSize = currentSize >= amount*bytes_per_job ? amount : ((int)currentSize/bytes_per_job);
   int newSize = currentSize;
   std::cout << "Reading " << chunkSize << " jobs!\n";
   for(size_t i = 0; i < chunkSize; i++)
   {
      QJob newJob;
      newJob.data = boost::shared_array<char>(new char[bytes_per_job]);
      newJob.size = bytes_per_job;
      memset(newJob.data.get(),0,bytes_per_job*sizeof(char));
      char* data = newJob.data.get();
      ifs.seekg(newSize-bytes_per_job);
      ifs.read(data, bytes_per_job);
      jobs.push_back(newJob);
      newSize -= bytes_per_job;
   }
   // truncate file
   ifs.close();
   truncate(filename.c_str(), newSize);
   FileSystem::Unlock(filename, lockhandle);
   return jobs;
}

#ifdef OS_WINDOWS
#define WIN32_MEAN_AND_LEAN
#include <windows.h>

int truncate(const char *path, int64 size)
{
   LARGE_INTEGER li;
   HANDLE file;
   
   li.QuadPart = size;

   file = CreateFileA(path, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

   if (file == INVALID_HANDLE_VALUE)
   {
     printf("Error opening file: %d\n", GetLastError());
     return 1;
   }

   if (SetFilePointerEx(file, li, NULL, 0) == 0)
   {
     printf("Error seeking file: %d\n", GetLastError());
     return 1;  
   }

   if (SetEndOfFile(file) == 0 )
   {
     printf("Error setting end of file: %d\n", GetLastError());
     return 1;
   }
   CloseHandle(file);
       
   return 0;
}

#endif