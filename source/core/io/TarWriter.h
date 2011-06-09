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

#ifndef _TARWRITER_H
#define _TARWRITER_H

#include "og.h"
#include <ostream>
#include <string>

class OPENGLOBE_API TarWriter
{
public:
   // ctor
   TarWriter(std::ostream& out);

   // dtor
   virtual ~TarWriter();

   // Finalize tar archive. Call after all files are added.
   void Finalize();

   // string to archive
   void AddString(const char* filename_archive, const std::string& s);

   // add data to archive
   void AddData(const char* filename_archive, const char* content, std::size_t len);

   // add existing file to archive
   void AddFile(const char* filename, const char* filename_archive);


protected:
   std::ostream& _out;

};

#endif
