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

#ifndef _FILEREADERDISK_H
#define _FILEREADERDISK_H

#include "og.h"
#include "IFileReader.h"
#include <vector>
#include <fstream>

//------------------------------------------------------------------------------

class OPENGLOBE_API FileReaderDisk : public IFileReader
{
public:
   FileReaderDisk();
   virtual ~FileReaderDisk();

   bool Open(const std::string& sFilename);

   // Read one byte
   virtual bool ReadByte(unsigned char& byte);
 
   // Reads entire file and store in vector.
   virtual bool Read(std::vector<unsigned char>& data);

   // Close file
   virtual bool Close();

private:
   std::fstream _in;

};

//------------------------------------------------------------------------------

#endif


