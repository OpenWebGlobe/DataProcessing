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

#include "FileWriterFactory.h"
#include "fs/FileWriterDisk.h"
#include "fs/FileWriterHttp.h"
#include <string>

boost::shared_ptr<IFileWriter> FileWriterFactory::Create(std::string& file)
{
    boost::shared_ptr<IFileWriter> qFile;

   if (file.size() >= 7)
   {
      std::string substr = file.substr(0,7);

      if (substr == "http://")
      {
         FileWriterHttp* pHttpWriter = new FileWriterHttp();
         qFile = boost::shared_ptr<IFileWriter>(pHttpWriter);

         if (!pHttpWriter->Open(file))
         {
            boost::shared_ptr<FileWriterHttp> qNull;
            return qNull;
         }

      }
      else if (substr == "file://")
      {
         std::string sFilename = file.substr(7,sFilename.length()-8);
         FileWriterDisk* pDiskWriter = new FileWriterDisk();
         qFile = boost::shared_ptr<IFileWriter>(pDiskWriter);

         if (!pDiskWriter->Open(sFilename))
         {
            boost::shared_ptr<IFileWriter> qNull;
            return qNull;
         }
      }
   }
 
   return qFile;
}