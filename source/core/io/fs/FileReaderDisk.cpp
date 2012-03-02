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

#include "FileReaderDisk.h"

//------------------------------------------------------------------------------

FileReaderDisk::FileReaderDisk()
{
   
}

//------------------------------------------------------------------------------

FileReaderDisk::~FileReaderDisk()
{
   this->Close();
}

//------------------------------------------------------------------------------

bool FileReaderDisk::Open(const std::string& sFilename)
{
   _in.open(sFilename.c_str(), std::ios::binary | std::ios::in);
   return _in.good();
}

//------------------------------------------------------------------------------

bool FileReaderDisk::Close()
{
   try
   {
      if (_in.is_open())
      {
         _in.close();
      }
      return true;
   }
   catch (std::exception)
   {
      return false;
   }
}

//------------------------------------------------------------------------------

bool FileReaderDisk::ReadByte(unsigned char& byte)
{
   if (!_in.eof())
   {
      char c;
      _in.read(&c, 1);
      byte = (unsigned char)c;
      return true;
   }
   else
   {
      return false;
   }
}

//------------------------------------------------------------------------------

bool FileReaderDisk::Read(std::vector<unsigned char>& data)
{
   data.clear();
   std::ios::pos_type curpos = _in.tellg();
   _in.seekg(std::ios::beg);
   
   char c;
   while (!_in.eof())
   {
      _in.read(&c, 1);
      data.push_back((unsigned char)c);
   }

   _in.seekg(curpos);
   return false;
}

//------------------------------------------------------------------------------

