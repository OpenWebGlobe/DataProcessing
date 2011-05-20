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
#include "string/StringUtils.h"
#endif

#include <list>
#include <vector>
#include <map>
#include <ctype.h>

#ifdef ENABLE_64BIT
namespace internal
{
   int64 atoi64(const char *nptr)
   {
     char *s = (char *)nptr;
     int64 acc = 0;
     int neg = 0;

     if (nptr == NULL)
     {
       return 0;
     }

     while(isspace((int)*s))
     {
        s++;
     }

     if (*s == '-')
     {
         neg = 1;
         s++;
     }
     else if (*s == '+')
     {
       s++;
     }

     while (isdigit((int)*s))
     {
         acc = 10 * acc + ((int)*s - '0');
         s++;
     }

     if (neg)
     {
       acc *= -1;
     }

     return acc;
   }
   //
   uint64 atoui64(const char *nptr)
   {
      char *s = (char *)nptr;
      uint64 acc = 0;

      if (nptr == NULL)
      {
         return 0;
      }

      while(isspace((int)*s))
      {
         s++;
      }

      if (*s == '-')
      {
         assert(false); // unsigned can't be negative
         s++;
      }
      else if (*s == '+')
      {
         s++;
      }

      while (isdigit((int)*s))
      {
         acc = 10 * acc + ((int)*s - '0');
         s++;
      }

      return acc;
   }  
}
#endif 


//=============================================================================
// PART 1: CONVERT CUSTOM TYPE TO STRING
//=============================================================================

std::string BoolToString(void* pAddress)
{
   std::ostringstream os;
   bool* data_int = (bool*)(pAddress);
   if (*data_int)
      os << "true";
   else
      os << "false";

   return os.str();
}
TYPE_TO_STRING_CONVERSION(bool, BoolToString)

//-----------------------------------------------------------------------------

std::string CharToString(void* pAddress)
{
   std::ostringstream os;
   char* data_int = (char*)(pAddress);
   os << *data_int;
   return XMLUtils::Encode(os.str());
}
TYPE_TO_STRING_CONVERSION(char, CharToString)

//-----------------------------------------------------------------------------

std::string UnsignedCharToString(void* pAddress)
{
   std::ostringstream os;
   unsigned char* data_int = (unsigned char*)(pAddress);
   os << *data_int;
   return XMLUtils::Encode(os.str());
}
TYPE_TO_STRING_CONVERSION(unsigned char, UnsignedCharToString)

//-----------------------------------------------------------------------------

std::string IntToString(void* pAddress)
{
   std::ostringstream os;
   int* data_int = (int*)(pAddress);
   os << *data_int;
   return os.str();
}
TYPE_TO_STRING_CONVERSION(int, IntToString)

//-----------------------------------------------------------------------------

std::string UnsignedIntToString(void* pAddress)
{
   std::ostringstream os;
   unsigned int* data_int = (unsigned int*)(pAddress);
   os << *data_int;
   return os.str();
}
TYPE_TO_STRING_CONVERSION(unsigned int, UnsignedIntToString)

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

std::string LongToString(void* pAddress)
{
   std::ostringstream os;
   long* data_int = (long*)(pAddress);
   os << *data_int;
   return os.str();
}
TYPE_TO_STRING_CONVERSION(long, LongToString)

//-----------------------------------------------------------------------------

std::string UnsignedLongToString(void* pAddress)
{
   std::ostringstream os;
   unsigned long* data_int = (unsigned long*)(pAddress);
   os << *data_int;
   return os.str();
}
TYPE_TO_STRING_CONVERSION(unsigned long, UnsignedLongToString)

//-----------------------------------------------------------------------------
#ifdef ENABLE_64BIT
std::string Int64ToString(void* pAddress)
{
   std::ostringstream os;
   int64* data_int = (int64*)(pAddress);
   os << *data_int;
   return os.str();
}
TYPE_TO_STRING_CONVERSION(int64, Int64ToString)
#endif
//-----------------------------------------------------------------------------
#ifdef ENABLE_64BIT
std::string UnsignedInt64ToString(void* pAddress)
{
   std::ostringstream os;
   uint64* data_int = (uint64*)(pAddress);
   os << *data_int;
   return os.str();
}
TYPE_TO_STRING_CONVERSION(uint64, UnsignedInt64ToString)
#endif
//-----------------------------------------------------------------------------

std::string FloatToString(void* pAddress)
{
   std::ostringstream os;
   float* data_int = (float*)(pAddress);
   os.precision(12);
   os << *data_int;
   return os.str();
}
TYPE_TO_STRING_CONVERSION(float, FloatToString)

//-----------------------------------------------------------------------------

std::string DoubleToString(void* pAddress)
{
   std::ostringstream os;
   double* data_int = (double*)(pAddress);
   os.precision(17);
   os << *data_int;
   return os.str();
}
TYPE_TO_STRING_CONVERSION(double, DoubleToString)

//-----------------------------------------------------------------------------

std::string StringTypeToString(void* pAddress)
{
   std::ostringstream os;
   std::string* data_int = (std::string*)(pAddress);
   os << *data_int;
   return XMLUtils::Encode(os.str());
}
TYPE_TO_STRING_CONVERSION(std::string, StringTypeToString)

//-----------------------------------------------------------------------------
#ifdef ENABLE_UTF8
std::string WStringTypeToString(void* pAddress)
{
   std::ostringstream os;

   std::wstring* data_str = (std::wstring*)(pAddress);
   std::string s = StringUtils::wstring_To_Utf8(*data_str);

   return XMLUtils::Encode(std::string(s.c_str()));
}
TYPE_TO_STRING_CONVERSION(std::wstring, WStringTypeToString)
#endif
//-----------------------------------------------------------------------------

//=============================================================================
// PART 2: CONVERT CUSTOM TYPE TO XML
//=============================================================================

std::string IntVecToXML(void* pAddress, std::string sTag)
{
   std::ostringstream os;
   std::vector<int>* pData = (std::vector<int>*)(pAddress);

   if (pData->size()>0) // only write this tag if there is actually data!
   {
      os << "<" << sTag << ">";

      for (size_t i=0;i<pData->size();i++)
      {
         os << (*pData)[i];
         if (i!=pData->size()-1) os << " ";
      }
      os << "</" << sTag << ">";
   }
   return os.str();
}
TYPE_TO_XML_CONVERSION(std::vector<int>, IntVecToXML)

//-----------------------------------------------------------------------------

std::string UnsignedIntVecToXML(void* pAddress, std::string sTag)
{
   std::ostringstream os;
   std::vector<unsigned int>* pData = (std::vector<unsigned int>*)(pAddress);

   if (pData->size()>0)
   {
      os << "<" << sTag << ">";

      for (size_t i=0;i<pData->size();i++)
      {
         os << (*pData)[i];
         if (i!=pData->size()-1) os << " ";
      }

      os << "</" << sTag << ">";
   }

   return os.str();
}
TYPE_TO_XML_CONVERSION(std::vector<unsigned int>, UnsignedIntVecToXML)

//-----------------------------------------------------------------------------

//=============================================================================
// PART 3: CONVERT STRING TO CUSTOM TYPE
//=============================================================================

void StringToBool(void* pAddress, std::string sValue)
{
   // Set a char using first character of string...
   bool* data_bool = (bool*)(pAddress);
   
   if (sValue == std::string("true"))
   {
      *data_bool = true;
   }
   else
   {
      *data_bool = false;
   }
}
STRING_TO_TYPE_CONVERSION(bool, StringToBool)

//-----------------------------------------------------------------------------

void StringToChar(void* pAddress, std::string sValue)
{
   // Set a char using first character of string...
   char* data_int = (char*)(pAddress);
   sValue = XMLUtils::Decode(sValue);
   if (sValue.length()>=1)
      *data_int = sValue[0];
}
STRING_TO_TYPE_CONVERSION(char, StringToChar)

//-----------------------------------------------------------------------------

void StringToUnsignedChar(void* pAddress, std::string sValue)
{
   // Set a char using first character of string...
   unsigned char* data_int = (unsigned char*)(pAddress);
   sValue = XMLUtils::Decode(sValue);
   if (sValue.length()>=1)
      *data_int = sValue[0];
}
STRING_TO_TYPE_CONVERSION(unsigned char, StringToUnsignedChar)

//-----------------------------------------------------------------------------

void StringToInt(void* pAddress, std::string sValue)
{
   int* data_int = (int*)(pAddress);
   *data_int = atoi(sValue.c_str());
}
STRING_TO_TYPE_CONVERSION(int, StringToInt)

//-----------------------------------------------------------------------------

void StringToUnsignedInt(void* pAddress, std::string sValue)
{
   unsigned int* data_int = (unsigned int*)(pAddress);
   *data_int = atoi(sValue.c_str());
}
STRING_TO_TYPE_CONVERSION(unsigned int, StringToUnsignedInt)

//-----------------------------------------------------------------------------

void StringToLong(void* pAddress, std::string sValue)
{
   long* data_int = (long*)(pAddress);
   *data_int = atoi(sValue.c_str());
}
STRING_TO_TYPE_CONVERSION(long, StringToLong)

//-----------------------------------------------------------------------------

void StringToUnsignedLong(void* pAddress, std::string sValue)
{
   unsigned long* data_int = (unsigned long*)(pAddress);
   *data_int = atoi(sValue.c_str());
}
STRING_TO_TYPE_CONVERSION(unsigned long, StringToUnsignedLong)

//-----------------------------------------------------------------------------
#ifdef ENABLE_64BIT
void StringToInt64(void* pAddress, std::string sValue)
{
   int64* data_int = (int64*)(pAddress);
#ifdef _MSC_VER
   *data_int = _atoi64(sValue.c_str());
#else
   *data_int = internal::atoi64(sValue.c_str());
#endif
}
STRING_TO_TYPE_CONVERSION(int64, StringToInt64)
#endif
//-----------------------------------------------------------------------------
#ifdef ENABLE_64BIT
void StringToUnsignedInt64(void* pAddress, std::string sValue)
{
   uint64* data_int = (uint64*)(pAddress);
#ifdef _MSC_VER
   *data_int = _atoi64(sValue.c_str());
#else
   *data_int = internal::atoui64(sValue.c_str());
#endif
}
STRING_TO_TYPE_CONVERSION(uint64, StringToUnsignedInt64)
#endif
//-----------------------------------------------------------------------------

void StringToFloat(void* pAddress, std::string sValue)
{
   float* data_dbl = (float*)(pAddress);
   *data_dbl = (float)atof(sValue.c_str());
}
STRING_TO_TYPE_CONVERSION(float, StringToFloat)

//-----------------------------------------------------------------------------

void StringToDouble(void* pAddress, std::string sValue)
{
   double* data_dbl = (double*)(pAddress);
   *data_dbl = (double)atof(sValue.c_str());
}
STRING_TO_TYPE_CONVERSION(double, StringToDouble)

//-----------------------------------------------------------------------------

void StringToStringType(void* pAddress, std::string sValue)
{
   std::string* data_str = (std::string*)(pAddress);
   sValue = XMLUtils::Decode(sValue);
   *data_str = sValue;
}
STRING_TO_TYPE_CONVERSION(std::string, StringToStringType)

//-----------------------------------------------------------------------------
#ifdef ENABLE_UTF8
void StringToWStringType(void* pAddress, std::string sValue)
{
   sValue = XMLUtils::Decode(sValue);
   std::wstring strW = StringUtils::Utf8_To_wstring(sValue);

   std::wstring* data_str = (std::wstring*)(pAddress);
   *data_str = strW;
}
STRING_TO_TYPE_CONVERSION(std::wstring, StringToWStringType)
#endif
//-----------------------------------------------------------------------------

//=============================================================================
// PART 4: CONVERT XML TO CUSTOM TYPE
//=============================================================================

void XMLToIntVec(void* pAddress, std::string sXML)
{
   std::stringstream os;
   os << sXML;

   std::vector<int>* pData = (std::vector<int>*)(pAddress);
   pData->clear();

   bool bValueTag;
   std::string sTag = ObjectFactory::GetTag(os, bValueTag);
   std::string sValue = ObjectFactory::GetValue(os);

   Tokenize(sValue, ' ', *pData);
}
XML_TO_TYPE_CONVERSION(std::vector<int>, XMLToIntVec)

//-----------------------------------------------------------------------------

void XMLToUnsignedIntVec(void* pAddress, std::string sXML)
{
   std::stringstream os;
   os << sXML;

   std::vector<unsigned int>* pData = (std::vector<unsigned int>*)(pAddress);
   pData->clear();

   bool bValueTag;
   std::string sTag = ObjectFactory::GetTag(os, bValueTag);
   std::string sValue = ObjectFactory::GetValue(os);

   Tokenize(sValue, ' ', *pData);
}
XML_TO_TYPE_CONVERSION(std::vector<unsigned int>, XMLToUnsignedIntVec)

//-----------------------------------------------------------------------------

std::string StringVectorToXML(void* pAddress, std::string sTag)
{
   std::ostringstream os;

   os << "<" << sTag << ">";

   std::vector<std::string>* pData = (std::vector<std::string>*)(pAddress);
   std::vector<std::string>::iterator it = pData->begin();

   int cnt = 0;
   while (it != pData->end())
   {
      std::string s = *it;
      os << s.c_str();
      if (cnt!=pData->size()-1) os << ";\n";
      it++;
      cnt++;
   }

   os << "</" << sTag << ">";

   return os.str();
}
TYPE_TO_XML_CONVERSION(std::vector<std::string>, StringVectorToXML)
//-----------------------------------------------------------------------------

void XMLToStringVector(void* pAddress, std::string sXML)
{
   std::stringstream os;
   os << sXML;

   std::vector<std::string>* pData = (std::vector<std::string>*)(pAddress);
   pData->clear();

   bool bValueTag;
   std::string sTag = ObjectFactory::GetTag(os, bValueTag);
   std::string sValue = ObjectFactory::GetValue(os);

   std::vector<std::string> v;
   Tokenize(sValue, L';', v);

   for (size_t i=0;i<v.size();i++)
   {
      pData->push_back(v[i]);
   }

}
XML_TO_TYPE_CONVERSION(std::vector<std::string>, XMLToStringVector)

//-----------------------------------------------------------------------------
#ifdef ENABLE_UTF8
std::string WStringVectorToXML(void* pAddress, std::string sTag)
{
   std::ostringstream os;

   os << "<" << sTag << ">";

   std::vector<std::wstring>* pData = (std::vector<std::wstring>*)(pAddress);
   std::vector<std::wstring>::iterator it = pData->begin();

   int cnt = 0;
   while (it != pData->end())
   {
      std::wstring s = *it;
      os << std::string(StringUtils::wstring_To_Utf8(s.c_str()).c_str());
      if (cnt!=pData->size()-1) os << ";";
      it++;
      cnt++;
   }

   os << "</" << sTag << ">";

   return os.str();
}
TYPE_TO_XML_CONVERSION(std::vector<std::wstring>, WStringVectorToXML)
#endif
//-----------------------------------------------------------------------------
#ifdef ENABLE_UTF8
void XMLToWStringVector(void* pAddress, std::string sXML)
{
   std::stringstream os;
   os << sXML;

   std::vector<std::wstring>* pData = (std::vector<std::wstring>*)(pAddress);
   pData->clear();

   bool bValueTag;
   std::string sTag = ObjectFactory::GetTag(os, bValueTag);
   std::string sValue = ObjectFactory::GetValue(os);

   std::vector<std::string> v;
   Tokenize(sValue, L';', v);

   for (size_t i=0;i<v.size();i++)
   {
      pData->push_back(StringUtils::Utf8_To_wstring(v[i]));
   }
}
XML_TO_TYPE_CONVERSION(std::vector<std::wstring>, XMLToWStringVector)
#endif