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


#include "FileReaderFactory.h"
#include "fs/FileReaderDisk.h"
#include "fs/FileReaderHttp.h"
#include <string>

boost::shared_ptr<IFileReader>  FileReaderFactory::Create(std::string& file)
{
   boost::shared_ptr<IFileReader> qFile;

   if (file.size() >= 7)
   {
      std::string substr = file.substr(0,7);

      if (substr == "http://")
      {
         FileReaderHttp* pHttpReader = new FileReaderHttp();
         qFile = boost::shared_ptr<IFileReader>(pHttpReader);

         if (!pHttpReader->Open(file))
         {
            boost::shared_ptr<IFileReader> qNull;
            return qNull;
         }

      }
      else if (substr == "file://")
      {
         std::string sFilename = file.substr(7,sFilename.length()-8);
         FileReaderDisk* pDiskReader = new FileReaderDisk();
         qFile = boost::shared_ptr<IFileReader>(pDiskReader);

         if (!pDiskReader->Open(sFilename))
         {
            boost::shared_ptr<IFileReader> qNull;
            return qNull;
         }
      }
   }
 
   return qFile;
}