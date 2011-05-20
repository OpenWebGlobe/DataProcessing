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

#ifndef _TOKENIZER_H
#define _TOKENIZER_H 
  
#include "og.h"
#include <string>
#include <vector>   
#include "ClassExport.h"

   LIBRARY_API void Tokenize(std::string& sInput, char cSep, std::vector<std::string>& sOut);
#ifdef ENABLE_UTF8   
   LIBRARY_API void Tokenize(std::string& sInputUtf8, wchar_t cSep, std::vector<std::wstring>& sOut);
#endif   
   LIBRARY_API void Tokenize(std::string& sInput, char cSep, std::vector<int>& sOut);
   LIBRARY_API void Tokenize(std::string& sInput, char cSep, std::vector<unsigned int>& sOut);
#ifdef ENABLE_64BIT
   LIBRARY_API void Tokenize(std::string& sInput, char cSep, std::vector<int64>& sOut);
#endif
   LIBRARY_API void Tokenize(std::string& sInput, char cSep, std::vector<long>& sOut);
   LIBRARY_API void Tokenize(std::string& sInput, char cSep, std::vector<unsigned long>& sOut);
   LIBRARY_API void Tokenize(std::string& sInput, char cSep, std::vector<float>& sOut);
   LIBRARY_API void Tokenize(std::string& sInput, char cSep, std::vector<double>& sOut);
       
#endif      