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

#include "StringUtils.h"
#include "ConvertUTF.h"

//------------------------------------------------------------------------------

std::wstring StringUtils::Utf8_To_wstring(const std::string& utf8string)
{
   if (utf8string.length()==0)
   {
      return std::wstring();
   }
   size_t widesize = utf8string.length();
   if (sizeof(wchar_t) == 2)
   {
      std::wstring resultstring;
      resultstring.resize(widesize, L'\0');
      const UTF8* sourcestart = reinterpret_cast<const UTF8*>(utf8string.c_str());
      const UTF8* sourceend = sourcestart + widesize;
      UTF16* targetstart = reinterpret_cast<UTF16*>(&resultstring[0]);
      UTF16* targetend = targetstart + widesize;
      ConversionResult res = ConvertUTF8toUTF16(&sourcestart, sourceend, &targetstart, targetend, strictConversion);
      if (res != conversionOK)
      {
         return std::wstring(utf8string.begin(), utf8string.end());
      }
      *targetstart = 0;
      return std::wstring(resultstring.c_str());
   }
   else if (sizeof(wchar_t) == 4)
   {
      std::wstring resultstring;
      resultstring.resize(widesize, L'\0');
      const UTF8* sourcestart = reinterpret_cast<const UTF8*>(utf8string.c_str());
      const UTF8* sourceend = sourcestart + widesize;
      UTF32* targetstart = reinterpret_cast<UTF32*>(&resultstring[0]);
      UTF32* targetend = targetstart + widesize;
      ConversionResult res = ConvertUTF8toUTF32(&sourcestart, sourceend, &targetstart, targetend, strictConversion);
      if (res != conversionOK)
      {
         return std::wstring(utf8string.begin(), utf8string.end());
      }
      *targetstart = 0;
      return std::wstring(resultstring.c_str());
   }
   else
   {
      assert(false);
   }
   return L"";
}

//------------------------------------------------------------------------------

std::string StringUtils::wstring_To_Utf8(const std::wstring& widestring)
{
   size_t widesize = widestring.length();

   if (sizeof(wchar_t) == 2)
   {
      size_t utf8size = 3 * widesize + 1;
      std::string resultstring;
      resultstring.resize(utf8size, '\0');
      const UTF16* sourcestart = reinterpret_cast<const UTF16*>(widestring.c_str());
      const UTF16* sourceend = sourcestart + widesize;
      UTF8* targetstart = reinterpret_cast<UTF8*>(&resultstring[0]);
      UTF8* targetend = targetstart + utf8size;
      ConversionResult res = ConvertUTF16toUTF8(&sourcestart, sourceend, &targetstart, targetend, strictConversion);
      if (res != conversionOK)
      {
         return std::string(widestring.begin(), widestring.end());
      }
      *targetstart = 0;
      return std::string(resultstring.c_str());
   }
   else if (sizeof(wchar_t) == 4)
   {
      size_t utf8size = 4 * widesize + 1;
      std::string resultstring;
      resultstring.resize(utf8size, '\0');
      const UTF32* sourcestart = reinterpret_cast<const UTF32*>(widestring.c_str());
      const UTF32* sourceend = sourcestart + widesize;
      UTF8* targetstart = reinterpret_cast<UTF8*>(&resultstring[0]);
      UTF8* targetend = targetstart + utf8size;
      ConversionResult res = ConvertUTF32toUTF8(&sourcestart, sourceend, &targetstart, targetend, strictConversion);
      if (res != conversionOK)
      {
         return std::string(widestring.begin(), widestring.end());
      }
      *targetstart = 0;
      return std::string(resultstring.c_str());
   }
   else
   {
      assert(false);
   }
   return "";
}

//------------------------------------------------------------------------------

std::string StringUtils::IntegerToString(int value, int base)
{
   if(value == 0) return "0";
   static char NUMS[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
   std::string result = "";
   do
   {
      result.push_back(NUMS[value%base]);
      value /= base;
   }
   while(value != 0); 

   return std::string(result.rbegin(), result.rend());
}

//------------------------------------------------------------------------------

inline unsigned int _number( char ch )
{
   if ( ch >= '0' && ch <= '9' ) return ch-'0';
   ch = toupper(ch);
   if ( ch >= 'A' && ch <= 'Z' ) return ch-'A';

   // every other char is treated as 0
   return 0;
}

//------------------------------------------------------------------------------

unsigned int StringUtils::StringToInteger(const std::string& input, unsigned int base)
{
   unsigned int total = 0;
   for (size_t i = 0; i < input.size(); ++i )
   {
      total = total*base + _number(input[i]);
   }
   return total;
}

//------------------------------------------------------------------------------

