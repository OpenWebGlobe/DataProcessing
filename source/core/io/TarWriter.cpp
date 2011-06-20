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
// This code is based on                                                      //
// http://plindenbaum.blogspot.com/2010/08/creating-tar-file-in-c.html        //
//----------------------------------------------------------------------------//

#include "TarWriter.h"
#include <sstream>
#include <cassert>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <iostream>
#include <stdexcept>

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

//------------------------------------------------------------------------------
struct PosixTarHeader
{                       /* byte offset */
   char name[100];      /*   0 */
   char mode[8];        /* 100 */
   char uid[8];         /* 108 */
   char gid[8];         /* 116 */
   char size[12];       /* 124 */
   char mtime[12];      /* 136 */
   char chksum[8];      /* 148 */
   char typeflag;       /* 156 */
   char linkname[100];  /* 157 */
   char magic[6];       /* 257 */
   char version[2];     /* 263 */
   char uname[32];      /* 265 */
   char gname[32];      /* 297 */
   char devmajor[8];    /* 329 */
   char devminor[8];    /* 337 */
   char prefix[155];    /* 345 */
   char pad[12];        /* 500 */
};

void _init(PosixTarHeader* header);
void _checksum(PosixTarHeader* header);
void _size(PosixTarHeader* header,unsigned long fileSize);
void _filename(PosixTarHeader* header,const char* filename);
void _endRecord(std::size_t len, std::ostream& out);

//------------------------------------------------------------------------------

TarWriter::TarWriter(std::ostream& out)
   : _out(out)
{
   assert(sizeof(PosixTarHeader)==512);
}

//------------------------------------------------------------------------------

TarWriter::~TarWriter()
{
}

//------------------------------------------------------------------------------

void TarWriter::Finalize()
{
   PosixTarHeader header;
   std::memset((void*)&header,0,sizeof(PosixTarHeader));
   _out.write((const char*)&header,sizeof(PosixTarHeader));
   _out.write((const char*)&header,sizeof(PosixTarHeader));
   _out.flush();
}

//------------------------------------------------------------------------------

void TarWriter::AddString(const char* filename_archive,const std::string& s)
{
   AddData(filename_archive,s.c_str(),s.size());
}

//------------------------------------------------------------------------------

void TarWriter::AddData(const char* filename_archive,const char* content,std::size_t len)
{
   PosixTarHeader header;
   _init(&header);
   _filename(&header,filename_archive);
   header.typeflag=0;
   _size(&header,len);
   _checksum(&header);
   _out.write((const char*)&header,sizeof(PosixTarHeader));
   _out.write(content,len);
   _endRecord(len, _out);
}

//------------------------------------------------------------------------------

void TarWriter::AddFile(const char* filename,const char* filename_archive)
{
   char buff[BUFSIZ];
   std::FILE* in=std::fopen(filename,"rb");
   if(in==NULL)
   {
      std::ostringstream os;
      os << "Cannot open " << filename << " "<< std::strerror(errno);
      throw std::runtime_error(os.str());
   }
   std::fseek(in, 0L, SEEK_END);
   long int len= std::ftell(in);
   std::fseek(in,0L,SEEK_SET);

   PosixTarHeader header;
   _init(&header);
   _filename(&header,filename_archive);
   header.typeflag=0;
   _size(&header,len);
   _checksum(&header);
   _out.write((const char*)&header,sizeof(PosixTarHeader));


   std::size_t nRead=0;
   while((nRead=std::fread(buff,sizeof(char),BUFSIZ,in))>0)
   {
      _out.write(buff,nRead);
   }
   std::fclose(in);

   _endRecord(len, _out);
}

//------------------------------------------------------------------------------

void _init(PosixTarHeader* header)
{
   std::memset(header,0,sizeof(PosixTarHeader));
   std::sprintf(header->magic,"ustar  ");
   std::sprintf(header->mtime,"%011lo",time(NULL));
   std::sprintf(header->mode,"%07o",0644);
   //char * s = ::getlogin();
   //if(s!=NULL)  std::snprintf(header->uname,32,"%s",s);
   std::sprintf(header->uname,"%s","user");
   std::sprintf(header->gname,"%s","users");
}

//------------------------------------------------------------------------------

void _checksum(PosixTarHeader* header)
{
   unsigned int sum = 0;
   char *p = (char *) header;
   char *q = p + sizeof(PosixTarHeader);
   while (p < header->chksum) sum += *p++ & 0xff;
   for (int i = 0; i < 8; ++i)  
   {
      sum += ' ';
      ++p;
   }
   while (p < q) sum += *p++ & 0xff;

   std::sprintf(header->chksum,"%06o",sum);
}

//------------------------------------------------------------------------------

void _size(PosixTarHeader* header,unsigned long fileSize)
{
   std::sprintf(header->size,"%011llo",(long long unsigned int)fileSize);
}

//------------------------------------------------------------------------------

void _filename(PosixTarHeader* header,const char* filename)
{
   if(filename==NULL || filename[0]==0 || std::strlen(filename)>=100)
   {
      std::ostringstream os;
      os << "invalid archive name \"" << filename << "\"";
      throw std::runtime_error(os.str());
   }
   snprintf(header->name,100,"%s",filename);
}

//------------------------------------------------------------------------------

void _endRecord(std::size_t len, std::ostream& out)
{
   char c='\0';
   while((len%sizeof(PosixTarHeader))!=0)
   {
      out.write(&c,sizeof(char));
      ++len;
   }
}

