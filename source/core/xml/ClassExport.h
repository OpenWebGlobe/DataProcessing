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

#ifndef _CLASSEXPORT_H
#define _CLASSEXPORT_H

#include "og.h"
#include <map>
#include <string>

// If compiled as DLL, export / Import could be defined in LIBRARY_API
#define LIBRARY_API OPENGLOBE_API
#define API_EXPORT

// you can enable utf8 and 64bit support if appropriate libraries are available
#define ENABLE_64BIT
#define ENABLE_UTF8

//------------------------------------------------------------------------------
// 64bit can only be enabled if the type int64 and uint64 was previously defined
// utf8 requires "strings/StringUtils.h", available from Martin Christen, martin.christen@gmail.com
//------------------------------------------------------------------------------

namespace internal
{
   typedef void*(*funcCreateInstance)();

   class InstantiateBase
   {
   public:
      InstantiateBase(){}
      virtual ~InstantiateBase(){}

      // Map: ClassName -> Function to create instance
      static std::map<std::string, funcCreateInstance >& GetInstanceMap()
      {
         // Construct On First Use Idiom 
         // also see: http://www.parashift.com/c++-faq-lite/ctors.html#faq-10.14
         static std::map<std::string, funcCreateInstance > _mapInstanceMap;
         return _mapInstanceMap;
      }

      // Map: ClassName -> typeof name
      static std::map<std::string,std::string>& GetNameMap()
      {
         // Construct On First Use Idiom
         // also see: http://www.parashift.com/c++-faq-lite/ctors.html#faq-10.14
         static std::map<std::string, std::string > _mapNameMap;
         return _mapNameMap;
      }
   };
}

//------------------------------------------------------------------------------

namespace types
{
   //! \brief Create instance of a registered class
   LIBRARY_API void* CreateInstance(std::string sClassName);

   //! \brief returns true if specified class is registered
   //! \param sClass Name of registered class
   LIBRARY_API bool ClassExists(std::string sClass);        

   //! \brief List registered classes to std::cout
   LIBRARY_API void ListRegisteredClasses();

   // Get Class name by using (platform specific) typeid name.
   // Example: 
   // std::string sClassname;
   // sClassname = cwc::types::GetClassNameFromTypeIdName(typeid(myInstance).name());
   LIBRARY_API std::string GetClassNameFromTypeIdName(std::string sTypeIdName);
}

//------------------------------------------------------------------------------

#define CLASS_EXPORT(classname) namespace internal \
{                                                  \
class LIBRARY_API Instantiate_##classname : public InstantiateBase \
{                                                  \
public:                                            \
   Instantiate_##classname()                       \
{  InstantiateBase::GetInstanceMap().insert(       \
   std::pair<std::string, funcCreateInstance >(    \
   std::string(""#classname), &CreateInstance));   \
   InstantiateBase::GetNameMap().insert(std::pair<std::string, std::string>( \
   std::string(typeid(classname).name()), std::string(""#classname))); \
   }                                               \
   virtual ~Instantiate_##classname()              \
{ }                                                \
                                                   \
   static void* CreateInstance(){return new classname;} \
};                                                 \
   volatile Instantiate_##classname  g_oInstance_##classname; \
}                                                  \

//------------------------------------------------------------------------------

#endif