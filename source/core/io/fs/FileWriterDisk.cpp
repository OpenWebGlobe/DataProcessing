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

#include "FileWriterDisk.h"

//------------------------------------------------------------------------------

FileWriterDisk::FileWriterDisk()
{
}

//------------------------------------------------------------------------------

FileWriterDisk::~FileWriterDisk()
{
   Close();
}

//------------------------------------------------------------------------------

bool FileWriterDisk::Open(const std::string& sFilename)
{
   _out.open(sFilename.c_str(), std::ios::binary | std::ios::out);
   return _out.good();
}

//------------------------------------------------------------------------------

bool FileWriterDisk::WriteByte(unsigned char byte)
{
   char c = (char)byte;
   _out.write(&c,1);
   return true;
}

//------------------------------------------------------------------------------

bool FileWriterDisk::Write(unsigned char* data, size_t len)
{
   _out.write((char*)data, len);
   return true;
}

//------------------------------------------------------------------------------

bool FileWriterDisk::Close()
{
   if (_out.is_open())
   {
      _out.close();
   }
   return true;
}

//------------------------------------------------------------------------------
