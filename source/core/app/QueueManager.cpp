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

void QueueManager::CommitJobQueue(std::string filename, bool append)
{
   int lockhandle = FileSystem::Lock(filename);
   if(!FileSystem::FileExists(filename) || !append)
   {
      // create new
      std::fstream off(filename.c_str(), std::ios::out | std::ios::binary);
      if (off.good())
      {
         for(size_t i = 0; i < _vJobs.size(); i++)
         {
            int len = _vJobs[i].size;
            off.write((char*)_vJobs[i].data.get(), (std::streamsize)len);
         }
         off.close();
      }
      else 
      {
         std::cout << "###Queuemanager: Error Committing queue file!\n";
      }
   }
   else
   {
      // append
      std::fstream off(filename.c_str(),std::ios::out | std::ios::app | std::ios::binary);
      if (off.good())
      {
         for(size_t i = 0; i < _vJobs.size(); i++)
         {
            int len = _vJobs[i].size;
            off.write((char*)_vJobs[i].data.get(), (std::streamsize)len);
         }
         off.close();
      }
      else 
      {
         std::cout << "###Queuemanager: Error Committing queue file!\n";
      }
   }
   _vJobs.clear();
   _iCount = 0;
   FileSystem::Unlock(filename, lockhandle);
   return;
}

//------------------------------------------------------------------------------

void QueueManager::AddToJobQueue(std::string filename, QJob job, bool append, int autocommit)
{
   if(!append)
   {
      _vJobs.clear();
      _iCount = 0;
   }
   _vJobs.push_back(job);
   _iCount++;
   if(_iCount >= autocommit)
   {
      CommitJobQueue(filename, append);
   }
}

//------------------------------------------------------------------------------

std::vector<QJob> QueueManager::FetchJobList(std::string filename, int bytes_per_job, int amount, bool verbose)
{
   int lockhandle = FileSystem::Lock(filename);
   boost::filesystem3::path filepath(filename);
   std::vector<QJob> jobs;
   int64 currentSize = boost::filesystem3::file_size(filepath);

   // read seekpointer
   std::string sSeekPointerFile = filename + ".seek";
   int64 seekPointer = currentSize;
   if(FileSystem::FileExists(sSeekPointerFile))
   {
      if(verbose) std::cout << "Seekpointerfile: " << sSeekPointerFile << "\n" << std::flush;
      std::ifstream sfs;
      sfs.open(sSeekPointerFile.c_str(), std::ios::in | std::ios::binary);
      char tmp[sizeof(int64)];
      sfs.read(tmp, (std::streamsize)sizeof(int64));
      memcpy(&seekPointer,tmp,sizeof(int64));
      sfs.close();
      if(verbose) std::cout << "-->Seekpoint @ " << seekPointer << " bytes.\n" << std::flush;
      if(seekPointer <= 0)
      {
         FileSystem::Unlock(filename, lockhandle);
         return jobs;
      }
   }
   // --

   std::ifstream ifs;
   ifs.open(filename.c_str(), std::ios::in | std::ios::binary);
   int chunkSize = currentSize >= amount*bytes_per_job ? amount : ((int)currentSize/bytes_per_job);
   for(size_t i = 0; i < chunkSize; i++)
   {
      QJob newJob;
      newJob.data = boost::shared_array<char>(new char[bytes_per_job]);
      newJob.size = bytes_per_job;
      memset(newJob.data.get(),0,bytes_per_job*sizeof(char));
      char* data = newJob.data.get();
      ifs.seekg(seekPointer-bytes_per_job);
      ifs.read(data, bytes_per_job);
      jobs.push_back(newJob);
      seekPointer -= bytes_per_job;
   }
   ifs.close();
   if(verbose) std::cout << "-->Read " << chunkSize << " jobs ("<< (bytes_per_job*chunkSize) <<" bytes).\n" << std::flush;
   // update seekpointer
   std::fstream off(sSeekPointerFile.c_str(), std::ios::out | std::ios::binary);
   if (off.good())
   {
      off.write((char*)&seekPointer, (std::streamsize)sizeof(int64));
      off.close();
      if(verbose) std::cout << "-->Updating seek pointer to " << seekPointer << " bytes).\n" << std::flush;
   }
   FileSystem::Unlock(filename, lockhandle);
   return jobs;
}
/*
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
*/