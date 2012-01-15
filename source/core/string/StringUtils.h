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

#ifndef _STRINGUTILS_H_
#define _STRINGUTILS_H_

#include "og.h"
#include <cassert>
#include <string>
#include <vector>

//-----------------------------------------------------------------------------
//! \brief String Manipulation Utilities
//! \ingroup string
//! \author Martin Christen, martin.christen@fhnw.ch
class OPENGLOBE_API StringUtils
{
public:
   //! \brief Retrieve left part of a wide string.
   //! \param s The input string
   //! \param l Number of characters from the left.
   //! \return the string
   static std::wstring Left(const std::wstring& s, size_t l)
   {
      return s.substr(0,l);
   }

   //! \brief Retrieve left part of an ascii string.
   //! \param s The input string
   //! \param l Number of characters from the left.
   //! \return the string
   static std::string Left(const std::string& s, size_t l)
   {
      return s.substr(0,l);
   }

   //! \brief Retrieve right part of a wide string.
   //! \param s The input string
   //! \param l Number of characters from the right.
   //! \return the string
   static std::wstring Right(const std::wstring& s, size_t l)
   {
      return s.substr(s.size()-l, l);
   }

   //! \brief Retrieve right part of an ascii string.
   //! \param s The input string
   //! \param l Number of characters from the right.
   //! \return the string
   static std::string Right(const std::string& s, size_t l)
   {
      return s.substr(s.size()-l, l);
   }

   //! \brief Retrieve sub-string of a wide string.
   //! \param s The input string
   //! \param from Position inside the string
   //! \param len Number of characters.
   //! \return the string
   static std::wstring Mid(const std::wstring& s, size_t from, size_t len)
   {
      return s.substr(from, len);
   }

   //! \brief Retrieve sub-string of an ascii string.
   //! \param s The input string
   //! \param from Position inside the string
   //! \param len Number of characters.
   //! \return the string
   static std::string Mid(const std::string& s, size_t from, size_t len)
   {
      return s.substr(from, len);
   }

   //! \brief Convert UTF8 encoded string to a wstring
   //! \param s utf8string
   //! \return returns wide string
   static std::wstring Utf8_To_wstring(const std::string& utf8string);

   //! \brief Convert wide string to utf8
   //! \param widestring wide string (std::wstring)
   //! \return returns UTF8 encoded string
   static std::string wstring_To_Utf8(const std::wstring& widestring);

   //! \brief Convert an integer to a string using specified base 2 to 36
   static std::string IntegerToString(int value, int base);
   
   //! \brief Convert a string to an integer using specified base 2 to 36
   static unsigned int StringToInteger(const std::string& input, unsigned int base);
};






#endif