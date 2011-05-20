/*******************************************************************************
Project       : Extrusive class instantiation and serialization/deserization
Version       : 1.1.0
Purpose       : Create a class instance using class name (string).
Last Modified : 4.4.2010
Author        : Martin Christen
License       : MIT License
Copyright     : (c) 2007-2010 by Martin Christen. All Rights Reserved

This is UNPUBLISHED PROPRIETARY SOURCE CODE of Martin Christen.
the contents of this file may not be disclosed to third parties,
copied or duplicated in any form without the prior written
permission of Martin Christen, martin.christen@gmail.com
*******************************************************************************/

#include "xml.h" 

#ifdef ENABLE_UTF8 
#include <string/StringUtils.h>
#endif

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
         sOut.push_back(_atoi64(sCurrent.c_str()));
         sCurrent.clear();
      }
      else
      {
         sCurrent += cCmp;
      }
   }

   // add last value too
   sOut.push_back(_atoi64(sCurrent.c_str()));
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
