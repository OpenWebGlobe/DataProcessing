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
#                           robert.wueest@fhnw.ch                              #
********************************************************************************
*     Licensed under MIT License. Read the file LICENSE for more information   *
*******************************************************************************/
// This is the resample version without mpi intended for regular 
// workstations. Multi cores are supported (OpenMP) and highly recommended.
//------------------------------------------------------------------------------

#ifndef _FUNCTIONS_H
#define _FUNCTIONS_H

#include <fstream>
#include <vector>
#include <string>
#include <string/StringUtils.h>
#include <boost/tokenizer.hpp>

struct Tile
{
   int x;
   int y;
   int zoom;
};
//------------------------------------------------------------------------------

inline std::vector<Tile> _readExpireList(std::string expire_list_file)
{
   std::ifstream file(expire_list_file.c_str(), std::ios::in);
   std::vector<Tile> expiredTiles;
   if (file.good())
   {
      char line[4096];
      std::string sLine;
      while (!file.eof())
      {
         file.getline(line, 4095);
         sLine = line;
         boost::char_separator<char> separator("/");
         boost::tokenizer<boost::char_separator<char> > tokens(sLine, separator);
         boost::tokenizer<boost::char_separator<char> >::iterator token_iter;
         int ct = 0;
         Tile t;
         for (token_iter = tokens.begin(); token_iter != tokens.end(); token_iter++)
         {
            if(ct == 0) {  t.zoom = StringUtils::StringToInteger(*token_iter,10); }
            if(ct == 1) {  t.x = StringUtils::StringToInteger(*token_iter,10); }
            if(ct == 2) {  t.y = StringUtils::StringToInteger(*token_iter,10); }
            ct++;
         }
         expiredTiles.push_back(t);
      }
   }
   return expiredTiles;
}

//------------------------------------------------------------------------------



#endif