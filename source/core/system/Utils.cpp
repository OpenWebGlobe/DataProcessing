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

#include "system/Utils.h"

#ifdef OS_WINDOWS
#include <windows.h>
#include "string/StringUtils.h"
#else
#include <unistd.h>
#endif

#include <string>
#include <iostream> 


#ifdef OS_WINDOWS

std::string SystemUtils::ComputerName()
{
   
   TCHAR cComputerName[MAX_COMPUTERNAME_LENGTH + 1]; 
   std::wstring result; 
   DWORD dwBufferSize = MAX_COMPUTERNAME_LENGTH + 1; 

   if(GetComputerName(cComputerName,&dwBufferSize)) 
   { 
      // if you get an error here: make sure to compile with unicode support!
      result = cComputerName; 
   } 

   // the string is converted to utf8!
   return StringUtils::wstring_To_Utf8(result);

}

#else

   std::string SystemUtils::ComputerName()
   {
      char cComputerName[255]; 
      gethostname(cComputerName, 255);
      result = cComputerName;
      return result;
   }

#endif
