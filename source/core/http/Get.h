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

#ifndef _HTTP_GET_
#define _HTTP_GET_

#include "og.h"
#include "Header.h"
#include <string>
#include <vector>

class OPENGLOBE_API HttpGet
{
public:
   HttpGet(){}
   virtual ~HttpGet(){}

   //! \description Request Data from URL. 
   //   Please note that the data is downloaded to memory. For downloading large files, use the downloader class.
   //! \param vData the retrieved data as std::vector
   //! \param pHeader optional header
   //! \return HTTP status code
   static unsigned int Request(const std::string& url, std::vector<unsigned char>& vData, Header* pHeader=0);


};


#endif