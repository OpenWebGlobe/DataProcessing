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

#include "FileWriterHttp.h"
#include "http/Post.h"
#include "string/FilenameUtils.h"

//------------------------------------------------------------------------------

FileWriterHttp::FileWriterHttp()
{

}

//------------------------------------------------------------------------------

FileWriterHttp::~FileWriterHttp()
{
   Close();
}

//------------------------------------------------------------------------------

bool FileWriterHttp::Open(const std::string& sFilename)
{
   _sFilename = sFilename;
   _out.clear();
   return true;
}

//------------------------------------------------------------------------------

bool FileWriterHttp::WriteByte(unsigned char byte)
{
   _out.push_back(byte);
   return true;
}

//------------------------------------------------------------------------------

bool FileWriterHttp::Write(unsigned char* data, size_t len)
{
   _out.clear();

   for (size_t i=0;i<len;i++)
   {
      _out.push_back(data[i]);
   }

   return true;
}

//------------------------------------------------------------------------------

bool FileWriterHttp::Close()
{
   if (_out.size()>0)
   {
      std::string form_name("pic");
      std::string form_filename = FilenameUtils::ExtractFileName("filename"); // todo: "meta-data"

      unsigned int result = HttpPost::SendBinary(_sFilename, form_name, form_filename, &_out[0], _out.size());
      _out.clear();
      return true;
   }
   else
   {
      return false;
   }
}

//------------------------------------------------------------------------------
