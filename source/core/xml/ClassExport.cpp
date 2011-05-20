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
#include <iostream>

namespace types
{

//------------------------------------------------------------------------------
   
void* CreateInstance(std::string sClassName)
{
   std::map<std::string, internal::funcCreateInstance >::iterator i;

   i = internal::InstantiateBase::GetInstanceMap().find(sClassName);
   if (i != internal::InstantiateBase::GetInstanceMap().end())
   {
      internal::funcCreateInstance oFunction = (*i).second;
      return oFunction();
   }
   return 0; // classname doesn't exist

}
   
//------------------------------------------------------------------------------

bool ClassExists(std::string sClass)
{
   std::map<std::string, internal::funcCreateInstance >::iterator i;
   i = internal::InstantiateBase::GetInstanceMap().begin();
   while (i!=internal::InstantiateBase::GetInstanceMap().end())
   {
      if ((*i).first == sClass)
      {
         return true;
      }
      i++;
   }
   return false;
}


//------------------------------------------------------------------------------

std::string GetClassNameFromTypeIdName(std::string sTypeIdName)
{
   std::map<std::string, std::string>::iterator i;
   i = internal::InstantiateBase::GetNameMap().find(sTypeIdName);

   if (i == internal::InstantiateBase::GetNameMap().end())
   {
      return std::string("Unnamed Class");
   }
   else
   {
      return (*i).second;
   }
}


//------------------------------------------------------------------------------

void ListRegisteredClasses()
{
   std::cout << "************************\n";
   std::cout << "**** Registered Classes:\n";
   unsigned int nCount = 0;
   std::map<std::string, internal::funcCreateInstance >::iterator i;
   i = internal::InstantiateBase::GetInstanceMap().begin();
   while (i!=internal::InstantiateBase::GetInstanceMap().end())
   {
      std::cout << "**** " << (*i).first << "\n";
      nCount++;
      i++;
   }
   std::cout << "************************\n";
   std::cout << "Total: " << nCount << "\n";
   std::cout << "************************\n";

   std::cout << "************************\n";
   std::cout << "Type Map:\n";
   std::map<std::string, std::string>::iterator j;
   j = internal::InstantiateBase::GetNameMap().begin();
   while (j!=internal::InstantiateBase::GetNameMap().end())
   {
      std::cout << "Typeid \"" << (*j).first << "\" is classname \"" << (*j).second << "\"\n"; 
      j++;
   }
   std::cout << "************************\n";
}
   

} // namespace types



