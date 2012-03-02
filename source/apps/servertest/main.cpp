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

#include <iostream>

#include "http/Get.h"
#include "http/Post.h"
#include "io/FileReaderFactory.h"
#include "io/FileWriterFactory.h"

int main(void)
{
   /*std::string url("http://www.openwebglobe.org/downloads/text.txt");
   std::vector<unsigned char> vData;
   Header header;

   unsigned int ret = HttpGet::Request(url, vData, &header);
   
   // print header:
   for (size_t i=0;i<header.GetHeader().size();i++)
   {
      std::cout << header.GetHeader()[i].first.c_str() << ": " << header.GetHeader()[i].second.c_str() << "\n";
   }
   std::cout << "**************************\n";

   // print contents:
   for (size_t i=0;i<vData.size();i++)
   {
      std::cout << vData[i];
   }*/


   // Example for POST:
   /*std::string url("http://localhost/post.php");
   std::string form_name("pic");
   std::string form_filename("myfile.bla");

   unsigned char* data = (unsigned char*)malloc(1024);
   memset(data,66,1024);

   HttpPost::SendBinary(url, form_name, form_filename, data, 1024);*/


   //boost::shared_ptr<IFileReader> qFileReader = FileReaderFactory::Create(std::string("file://c:/data/test.txt"));
   /*boost::shared_ptr<IFileReader> qFileReader = FileReaderFactory::Create(std::string("http://www.openwebglobe.org/downloads/text.txt"));
   if (qFileReader)
   {
      unsigned char c;
      while(qFileReader->ReadByte(c))
      {
         std::cout << c;
      }
   }*/


   //boost::shared_ptr<IFileWriter> qFileWriter = FileWriterFactory::Create(std::string("file://c:/data/myfile.txt"));
   boost::shared_ptr<IFileWriter> qFileWriter = FileWriterFactory::Create(std::string("http://localhost/post.php"));

   std::string s("Hello World,\n this is a text. Sent to WebServer!!\n");

   for (size_t i=0;i<s.size();i++)
   {
      qFileWriter->WriteByte(s[i]);
   }

   qFileWriter->Close();

   return 0;
}