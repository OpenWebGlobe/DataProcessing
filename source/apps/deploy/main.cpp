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
********************************************************************************
/*******************************************************************************/
// This is the deploy version without mpi (intended for regular workstations)
// OpenMP is required. All threads write archives.
//------------------------------------------------------------------------------

#include "og.h"
#include "io/TarWriter.h"
#include <system/Utils.h>
#include <sstream>
#include <iostream>
#include <omp.h>



//------------------------------------------------------------------------------

struct ThreadInfo
{
   std::string  sFileName;  // filename of archive
   TarWriter*   pTarWriter; // the writer
};

//------------------------------------------------------------------------------

int main(void)
{
   std::string sFilePath; // delimited file path where archives are written.

   int maxthreads = omp_get_max_threads();

   ThreadInfo* threadinfoarray = new ThreadInfo[maxthreads];
   if (!threadinfoarray)
   {
      return 1; // very unlikely out of memory.
   }

   // every thread writes to another tar archive this way we can write archives in parallel
   // store filename
   for (int i=0;i<maxthreads;i++)
   {
      std::ostringstream oss;
      oss << sFilePath << SystemUtils::ComputerName() << "_thread_" << i << ".tar";
      threadinfoarray[i].sFileName = oss.str();
      threadinfoarray[i].pTarWriter = 0;

      std::cout << "archive=" <<  threadinfoarray[i].sFileName << "\n";
   }

   // 



   // Clean up
   delete[] threadinfoarray;

   return 0;
}

//------------------------------------------------------------------------------
