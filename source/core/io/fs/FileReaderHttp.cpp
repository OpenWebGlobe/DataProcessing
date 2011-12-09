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

#include "FileReaderHttp.h"
#include "http/Get.h"
#include <algorithm>

//------------------------------------------------------------------------------

FileReaderHttp::FileReaderHttp()
{
   _curpos = 0;
}

//------------------------------------------------------------------------------

FileReaderHttp::~FileReaderHttp()
{
   this->Close();
}

//------------------------------------------------------------------------------

bool FileReaderHttp::Open(const std::string& sUrl)
{
   _storage.clear();
   _curpos = 0;
  
   unsigned int ret;
   ret = HttpGet::Request(sUrl, _storage);

   if (ret == 200) 
   {
      return true;
   }

   return false;
}

//------------------------------------------------------------------------------

bool FileReaderHttp::Close()
{
   _storage.clear();
   return true; // always returns true...
}

//------------------------------------------------------------------------------

bool FileReaderHttp::ReadByte(unsigned char& byte)
{
   if (_curpos<_storage.size())
   {
      byte = _storage[_curpos];
      _curpos++;
      return true;
   }

   return false;
}

//------------------------------------------------------------------------------

bool FileReaderHttp::Read(std::vector<unsigned char>& data)
{
   if (_storage.size() == 0)
      return false;

   data = _storage;

   return true;
}

//------------------------------------------------------------------------------

