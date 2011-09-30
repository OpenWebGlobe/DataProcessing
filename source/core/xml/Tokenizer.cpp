/*******************************************************************************
Project       : Extrusive class instantiation and serialization/deserization
Version       : 1.1.0
Purpose       : Create a class instance using class name (string).
Last Modified : 4.4.2010
Author        : Martin Christen
License       : MIT License
Copyright     : (c) 2007-2010 by Martin Christen. All Rights Reserved

Permission is hereby granted, free of charge, to any person obtaining a copy of 
this software and associated documentation files (the "Software"), to deal in 
the Software without restriction, including without limitation the rights to 
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of 
the Software, and to permit persons to whom the Software is furnished to do so, 
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all 
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS 
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR 
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER 
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN 
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*******************************************************************************/

#include "xml.h" 

#ifdef ENABLE_UTF8 
#include <string/StringUtils.h>
#endif

#include <cstdlib>

//---------------------------------------------------------------------------

void Tokenize(std::string& sInput, char cSep, std::vector<std::string>& sOut)
{
   sOut.clear();
   std::string sCurrent;
   for (size_t i=0;i<sInput.length();i++)
   {
      char cCmp = sInput.c_str()[i];
      if (cCmp == cSep)
      {
         // found separator
         sOut.push_back(sCurrent);
         sCurrent.clear();
      }
      else
      {
         sCurrent += cCmp;
      }
   }

   // add last value too
   sOut.push_back(sCurrent);
}


//---------------------------------------------------------------------------

#ifdef ENABLE_UTF8

void Tokenize(std::string& sInputUtf8, wchar_t cSep, std::vector<std::wstring>& sOut)
{
   std::wstring sInput = StringUtils::Utf8_To_wstring(sInputUtf8);

   sOut.clear();
   std::wstring sCurrent;
   for (size_t i=0;i<sInput.length();i++)
   {
      wchar_t cCmp = sInput.c_str()[i];
      if (cCmp == cSep)
      {
         // found separator
         sOut.push_back(sCurrent);
         sCurrent.clear();
      }
      else
      {
         sCurrent += cCmp;
      }
   }

   // add last value too
   sOut.push_back(sCurrent);
}

#endif

//---------------------------------------------------------------------------

void Tokenize(std::string& sInput, char cSep, std::vector<int>& sOut)
{
   sOut.clear();
   std::string sCurrent;
   for (size_t i=0;i<sInput.length();i++)
   {
      char cCmp = sInput.c_str()[i];
      if (cCmp == cSep)
      {
         // found separator
         sOut.push_back(atoi(sCurrent.c_str()));
         sCurrent.clear();
      }
      else
      {
         sCurrent += cCmp;
      }
   }

   // add last value too
   sOut.push_back(atoi(sCurrent.c_str()));
}

//---------------------------------------------------------------------------

void Tokenize(std::string& sInput, char cSep, std::vector<unsigned int>& sOut)
{
   sOut.clear();
   std::string sCurrent;
   for (size_t i=0;i<sInput.length();i++)
   {
      char cCmp = sInput.c_str()[i];
      if (cCmp == cSep)
      {
         // found separator
         sOut.push_back(atoi(sCurrent.c_str()));
         sCurrent.clear();
      }
      else
      {
         sCurrent += cCmp;
      }
   }

   // add last value too
   sOut.push_back(atoi(sCurrent.c_str()));
}

//---------------------------------------------------------------------------
#ifdef ENABLE_64BIT

namespace internal
{
   int64 atoi64(const char *nptr);
   uint64 atoui64(const char *nptr);
}

void Tokenize(std::string& sInput, char cSep, std::vector<int64>& sOut)
{
   sOut.clear();
   std::string sCurrent;
   for (size_t i=0;i<sInput.length();i++)
   {
      char cCmp = sInput.c_str()[i];
      if (cCmp == cSep)
      {
         // found separator
         sOut.push_back(internal::atoi64(sCurrent.c_str()));
         sCurrent.clear();
      }
      else
      {
         sCurrent += cCmp;
      }
   }

   // add last value too
   sOut.push_back(internal::atoi64(sCurrent.c_str()));
}
#endif 
//---------------------------------------------------------------------------

void Tokenize(std::string& sInput, char cSep, std::vector<long>& sOut)
{
   sOut.clear();
   std::string sCurrent;
   for (size_t i=0;i<sInput.length();i++)
   {
      char cCmp = sInput.c_str()[i];
      if (cCmp == cSep)
      {
         // found separator
         sOut.push_back(atol(sCurrent.c_str()));
         sCurrent.clear();
      }
      else
      {
         sCurrent += cCmp;
      }
   }

   // add last value too
   sOut.push_back(atol(sCurrent.c_str()));
}

//---------------------------------------------------------------------------

void Tokenize(std::string& sInput, char cSep, std::vector<unsigned long>& sOut)
{
   sOut.clear();
   std::string sCurrent;
   for (size_t i=0;i<sInput.length();i++)
   {
      char cCmp = sInput.c_str()[i];
      if (cCmp == cSep)
      {
         // found separator
         sOut.push_back(atol(sCurrent.c_str()));
         sCurrent.clear();
      }
      else
      {
         sCurrent += cCmp;
      }
   }

   // add last value too
   sOut.push_back(atol(sCurrent.c_str()));
}

//---------------------------------------------------------------------------

void Tokenize(std::string& sInput, char cSep, std::vector<float>& sOut)
{
   sOut.clear();
   std::string sCurrent;
   for (size_t i=0;i<sInput.length();i++)
   {
      char cCmp = sInput.c_str()[i];
      if (cCmp == cSep)
      {
         // found separator
         sOut.push_back((float)atof(sCurrent.c_str()));
         sCurrent.clear();
      }
      else
      {
         sCurrent += cCmp;
      }
   }

   // add last value too
   sOut.push_back((float)atof(sCurrent.c_str()));
}

//---------------------------------------------------------------------------
void Tokenize(std::string& sInput, std::string& cSeps, std::vector<double>& sOut)
{
   sOut.clear();
   std::string sCurrent;
   for (size_t i=0;i<sInput.length();i++)
   {
      bool bfound = false;
      char cCmp = sInput.c_str()[i];
      for (size_t j=0;j<cSeps.length();j++)
      {
         if (cCmp == cSeps[j])
         {
            // found separator
            sOut.push_back(atof(sCurrent.c_str()));
            sCurrent.clear();
            bfound = true;
         }
      }

      if (!bfound)
      {
         sCurrent += cCmp;
      }
   }

   // add last value too (if there is a value)
   if (sCurrent.length()>0)
      sOut.push_back(atof(sCurrent.c_str()));
}

void Tokenize(std::string& sInput, char cSep, std::vector<double>& sOut)
{
   sOut.clear();
   std::string sCurrent;
   for (size_t i=0;i<sInput.length();i++)
   {
      char cCmp = sInput.c_str()[i];
      if (cCmp == cSep)
      {
         // found separator
         sOut.push_back(atof(sCurrent.c_str()));
         sCurrent.clear();
      }
      else
      {
         sCurrent += cCmp;
      }
   }

   // add last value too
   sOut.push_back(atof(sCurrent.c_str()));
}
