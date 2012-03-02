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

#ifndef _FILEWRITERDISK_H
#define _FILEWRITERDISK_H

#include "og.h"
#include <vector>
#include <fstream>
#include "IFileWriter.h"

//------------------------------------------------------------------------------

class OPENGLOBE_API FileWriterDisk : public IFileWriter
{
public:
   FileWriterDisk();
   virtual ~FileWriterDisk();

   bool Open(const std::string& sFilename);

   // Write single byte. This may be slow.
   virtual bool WriteByte(unsigned char byte);

   // Write entire file
   virtual bool Write(unsigned char* data, size_t len);

   virtual bool Close();

protected:
   std::fstream _out;
};

//------------------------------------------------------------------------------

#endif


