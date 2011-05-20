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
#include <sstream>
#include <cassert>
#include <algorithm>


//! \defgroup xml XML
//! \brief This module contains C++ to XML serialization and deserialization functions.

namespace access
{

   // find variable:

   inline std::list< std::pair<std::string, access::Variable> >::iterator _FindVariable(std::list< std::pair<std::string, access::Variable> >& lst, std::string key)
   {
      std::list< std::pair<std::string, access::Variable> >::iterator it = lst.begin();

      while (it != lst.end())
      {

         if ((*it).first == key)
            return it;

          // special case: class is serialized directly
          if ((*it).first.size() == 0)
          {
               access::Variable var = (*it).second;
               if (var.sClassname == key)
                  return it;
          }

         it++;
      }

      return it;

   }


   //-----------------------------------------------------------------------
   // VARIABLE
   //-----------------------------------------------------------------------
   Variable::Variable()
   {
   }
   //-----------------------------------------------------------------------
   Variable::~Variable()
   {
   }
   //-----------------------------------------------------------------------
   std::string Variable::ToXML(void* pInstance, std::string sTag)
   {
      //--------------------------------------------------------------------
      std::string sTypeString;
      std::string sXML;

      // Check if "Type to String Conversion" function is available:
      std::map<std::string, TypeToStringConversionFunc>::iterator func_it;
      func_it = access::BaseClass::GetStringConversionMap().find(sTypename);

      if (func_it != access::BaseClass::GetStringConversionMap().end())
      {
         // hurray! found a registered type to function converter!!
         sTypeString = (*func_it).second((void*)(size_t(pInstance)+offset));
         sXML = "<" + sTag + ">";
         sXML = sXML + sTypeString;
         sXML = sXML + "</" + sTag + ">";
      }
      else
      {
         // search if Typename is registered...
         std::map<std::string, std::string>::iterator typeit;
         typeit = BaseClass::GetTypeMap().find(sTypename);

         if (typeit != BaseClass::GetTypeMap().end())
         {
            // found class!!
            // Initialize Class (if not yet done..)
            Class::InitClass((*typeit).second);

            // Find Class Members:
            std::map<std::string, Members>::iterator it = Class::GetInstanceMap().find((*typeit).second);
            if (it != Class::GetInstanceMap().end())
            {
               // Write XML:
               if (sTag.size()>0)
               {
                  sXML += "<" + sTag + ">\n";
                  Members::IncreaseXMLAdvance();
                  sXML += (*it).second.ToXML((void*)(size_t(pInstance)+offset), true);
                  Members::DecreaseXMLAdvance();
                  for (int i=0;i<Members::GetXMLadvance();i++)
                  {
                     sXML += " ";
                  }
                  sXML += "</" + sTag + ">";
               }
               else
               {
                  sXML += (*it).second.ToXML((void*)(size_t(pInstance)+offset), false);
               }

              
            }
         }
         else
         {
            // Can't convert type directly to string. Check if "Type to XML Conversion"
            // Is available
            std::map<std::string, TypeToXMLConversionFunc>::iterator xml_func_it;
            xml_func_it = access::BaseClass::GetXMLConversionMap().find(sTypename);

            if (xml_func_it != access::BaseClass::GetXMLConversionMap().end())
            {
               sXML = (*xml_func_it).second((void*)(size_t(pInstance)+offset), sTag);
            }
            /*else if (enumexport::GetEnumMapFromRttiName(sTypename).size() > 0)
            {
               // this is a registered enum value, serialize as
               //<EnumName>EnumString</EnumName>
               std::map<unsigned int,std::string>& enummap = enumexport::GetEnumMapFromRttiName(sTypename);
               assert(false); // currently not yet supported!
            }*/
            else
            {
               std::cout << "<b>**WARNING**</b>: Can't serialize type: " << sTypename << "\n";
               // can't convert this type (sTypename) to xml...
               // you must implement this conversion manually!!
               assert(false);
            }
         } 
      }
      //--------------------------------------------------------------------

      return sXML;

   }

   std::string Variable::ToString(void* pInstance)
   {
      std::ostringstream os;
      std::map<std::string, TypeToStringConversionFunc>::iterator func_it;
      func_it = access::BaseClass::GetStringConversionMap().find(sTypename);

      if (func_it != access::BaseClass::GetStringConversionMap().end())
      {
         // hurray! found a registered type to function converter!!
         return (*func_it).second((void*)(size_t(pInstance)+offset));
      }
      else
      {   
         // can't convert directly to string...
      }

      return os.str();
   }

   //-----------------------------------------------------------------------

   bool Variable::FromString(std::string sValue, void* pInstance)
   {
      std::map<std::string, StringToTypeConversionFunc>::iterator func_it;
      std::map<std::string, StringToTypeConversionFunc>& sMap = access::BaseClass::GetStringToTypeConversionMap();

      func_it = access::BaseClass::GetStringToTypeConversionMap().find(sTypename);

      if (func_it != access::BaseClass::GetStringToTypeConversionMap().end())
      {
         // hurray! found a registered type to function converter!!
         (*func_it).second((void*)(size_t(pInstance)+offset), sValue);
         return true;
      }
      else
      {   
         // can't convert directly to string...
         return false;
      }

   }

   //-----------------------------------------------------------------------

   bool Variable::FromXML(std::string sValue, void* pInstance)
   {
      return false;
   }

   //-----------------------------------------------------------------------
   // MEMBER
   //-----------------------------------------------------------------------
   int Members::_nXMLAdvance = 0;

   Members::Members()
   {
   }

   //-----------------------------------------------------------------------

   Members::~Members()
   {
   }

   //-----------------------------------------------------------------------

   void Members::SetTypeId(std::string sId)
   {
      _sClassTypeId = sId;
   }

   //-----------------------------------------------------------------------

   std::string& Members::GetTypeId()
   {
      return _sClassTypeId;
   }

   //-----------------------------------------------------------------------

   void Members::AddVariable(std::string sVariableName, Variable& oVariable)
   {
      _mapNameOffset.push_back(std::pair<std::string, Variable>(sVariableName, oVariable));
   }

   //-----------------------------------------------------------------------

   void Members::SetClassname(std::string sClassname)
   {
      _sClassName = sClassname;
   }

   //-----------------------------------------------------------------------

   std::string Members::ToXML(void* pInstance, bool bAdvance)
   {
      std::list< std::pair<std::string, access::Variable> >::iterator it = _mapNameOffset.begin();

      std::string sRet;

      if (bAdvance)
      {
         for (int i=0;i<Members::GetXMLadvance();i++)
         {
            sRet += " ";
         }
      }

      sRet += "<" + _sClassName + ">\n";

      Members::IncreaseXMLAdvance();
      while (it != _mapNameOffset.end())
      {
         std::string sNextXML = (*it).second.ToXML(pInstance, (*it).first);
         if (sNextXML.length()>0)
         {
            for (int i=0;i<Members::GetXMLadvance();i++)
            { 
               sRet += " ";
            }               
            sRet += sNextXML;
            sRet += "\n";
         }

         it++;
      }
      Members::DecreaseXMLAdvance();


      for (int i=0;i<Members::GetXMLadvance();i++)
      {
         sRet += " ";
      }


      sRet += "</" + _sClassName + ">";

      if (bAdvance)
         sRet += "\n";

      return sRet;
   }

   //-----------------------------------------------------------------------

   std::string Members::GetValue(std::string sMember, void* pInstance)
   {

      std::list< std::pair<std::string, access::Variable> >::iterator it = _FindVariable(_mapNameOffset, sMember);
      if (it == _mapNameOffset.end())
         return std::string();

      return (*it).second.ToString(pInstance);

   }

   //-----------------------------------------------------------------------

   bool Members::SetValue(std::string sValue, std::string sMember, void* pInstance)
   {
      std::list< std::pair<std::string, access::Variable> >::iterator it = _FindVariable(_mapNameOffset, sMember);
      if (it == _mapNameOffset.end())
         return false;

      return (*it).second.FromString(sValue, pInstance);
   }

   //-----------------------------------------------------------------------
   // CLASS
   //-----------------------------------------------------------------------

   Class::Class()
   {

   }

   //-----------------------------------------------------------------------

   Class::~Class()
   {

   }

   //-----------------------------------------------------------------------

   void Class::AddClass(std::string sClassName, Members& oMemberList)
   {
      GetInstanceMap().insert(std::pair<std::string, Members>(sClassName, oMemberList));         
   }

   //-----------------------------------------------------------------------

   void Class::ToXML(std::ostream& stream, std::string sClassname, void* pInstance, bool bHeader)
   {
      InitClass(sClassname);

      std::map<std::string, Members>::iterator it = GetInstanceMap().find(sClassname);

      if (it == GetInstanceMap().end())
         return;

      std::string sRet;

      sRet += (*it).second.ToXML(pInstance);

      if (bHeader)
      {
         stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << "\n";
         stream << "<!-- Created by i3D OpenGlobe XML Serializer Version 2.1 -->\n";
      }

      stream << sRet;

   }

   //-----------------------------------------------------------------------

   std::string Class::GetValue(std::string sClassname, std::string sMembername, void* pInstance)
   {
      InitClass(sClassname);

      std::map<std::string, Members>::iterator it = GetInstanceMap().find(sClassname);

      if (it == GetInstanceMap().end())
         return std::string();   // return empty value!


      return (*it).second.GetValue(sMembername, pInstance);

   }

   //-----------------------------------------------------------------------

   bool Class::SetValue(std::string sValue, std::string sClassname, std::string sMembername, void* pInstance)
   {
      InitClass(sClassname);

      std::map<std::string, Members>::iterator it = GetInstanceMap().find(sClassname);

      if (it == GetInstanceMap().end())
         return false; 

      return (*it).second.SetValue(sValue, sMembername, pInstance);
   }

   //-----------------------------------------------------------------------

   bool Class::GetMembers(std::string sBaseClass, Members& oMembers)
   {
      InitClass(sBaseClass);

      std::map<std::string, Members>::iterator it = GetInstanceMap().find(sBaseClass);

      if (it == GetInstanceMap().end())
         return false; 

      oMembers = (*it).second;

      return true;
   }

   //-----------------------------------------------------------------------

   std::map<std::string, Members>& Class::GetInstanceMap()
   {
      // construct on first use ideom...
      static std::map<std::string, Members> _mapClassNameClassMember;
      return _mapClassNameClassMember;
   }

   //-----------------------------------------------------------------------

   void Class::InitClass(std::string sBaseClass)
   {
      std::map<std::string, access::BaseClass*>& classmap = access::BaseClass::GetClassMap();
      std::map<std::string, access::BaseClass*>::iterator mapit;

      mapit = classmap.find(sBaseClass);
      if (mapit != classmap.end())
      {
         (*mapit).second->DoInit();
      }
   }

   //-----------------------------------------------------------------------

   bool Class::GetVariable(std::string sBaseClass, std::string sMember, Variable& outVar)
   {
      InitClass(sBaseClass);

      std::map<std::string, Members>::iterator it = GetInstanceMap().find(sBaseClass);

      if (it == GetInstanceMap().end())
         return false; 

      Members& oMembers = (*it).second;

      std::list< std::pair<std::string, access::Variable> >::iterator varit = _FindVariable(oMembers.GetVariableMap(),sMember);
      if (varit == oMembers.GetVariableMap().end())
         return false;

      outVar = (*varit).second;
      return true;
   }


   //-----------------------------------------------------------------------

   void* Class::FromXML(std::istream& stream, std::string sExpectedClass)
   {
      bool bIsValueTag;
      std::string sClassTag = ObjectFactory::GetTag(stream, bIsValueTag);
      std::string sValue = ObjectFactory::GetValue(stream); 
      std::string sClassEnd = std::string("/") + sClassTag;

      if (sClassTag == sExpectedClass)
      {
         void* pObject = types::CreateInstance(sClassTag);

         if (pObject)
         {
            ObjectFactory::Deserialize(stream, sClassTag, sClassEnd, pObject);
         }

         return pObject;
      }

      return 0;
   }

   //-----------------------------------------------------------------------

   std::string Class::GetBaseClassName(std::string ClassName, std::string MemberName)
   {
      Members oMembers;

      if (Class::GetMembers(ClassName, oMembers))
      {
         std::list< std::pair<std::string, access::Variable> >::iterator it;
         it = _FindVariable(oMembers.GetVariableMap(), MemberName);

         if (it != oMembers.GetVariableMap().end())
         {
            Variable V = (*it).second;
            return (*it).second.sClassname;
         }
      }

      return std::string();
   }

   //-----------------------------------------------------------------------

   std::map<std::string, BaseClass*>& BaseClass::GetClassMap()
   {
      static std::map<std::string, BaseClass*> sMap;

      return sMap;
   }

   //-----------------------------------------------------------------------
   // Map Containing TypeId -> C++ Classname
   std::map<std::string, std::string>& BaseClass::GetTypeMap()
   {
      static std::map<std::string, std::string> sMap;

      return sMap;
   }

   //-----------------------------------------------------------------------
   // Map Containing TypeId -> Function To Convert to String
   std::map<std::string, TypeToStringConversionFunc>& BaseClass::GetStringConversionMap()
   {
      static std::map<std::string, TypeToStringConversionFunc> sMap;
      return sMap;
   }


   //-----------------------------------------------------------------------
   // Map Containing TypeId -> Function To Convert to XML
   std::map<std::string, TypeToXMLConversionFunc>& BaseClass::GetXMLConversionMap()
   {
      static std::map<std::string, TypeToXMLConversionFunc> sMap;
      return sMap;
   }
   //-----------------------------------------------------------------------

   std::map<std::string, StringToTypeConversionFunc>& BaseClass::GetStringToTypeConversionMap()
   {
      static std::map<std::string, StringToTypeConversionFunc> sMap;
      return sMap;
   }

   //-----------------------------------------------------------------------

   std::map<std::string, XMLToTypeConversionFunc>& BaseClass::GetXMLToTypeConversionMap()
   {
      static std::map<std::string, XMLToTypeConversionFunc> sMap;
      return sMap;
   }

   //-----------------------------------------------------------------------


   std::string BaseClass::GetBaseClassName(std::string sRtti)
   {
      std::map<std::string, std::string>::iterator typeit;
      typeit = BaseClass::GetTypeMap().find(sRtti);
      if (typeit != BaseClass::GetTypeMap().end())
      {
         return (*typeit).second;
      }

      return std::string();
   }

   //-----------------------------------------------------------------------

}  // namespace


//------------------------------------------------------------------------------
// OBJECT FACTORY --------------------------------------------------------------
//------------------------------------------------------------------------------

//----------------------------------------------------------------------- 
// Recursive Deserialization
void ObjectFactory::Deserialize(std::istream& iStream, const std::string& sClassName, const std::string& sClassEnd, void* pObject, int counter)
{
   std::string sNextTag;
   std::string sNextValue;
   bool bIsValueTag;
   bool bDone = false;

   while (!bDone && iStream.good())
   {
      std::streampos lastpos;

      lastpos = iStream.tellg();
      sNextTag = ObjectFactory::GetTag(iStream, bIsValueTag);
      sNextValue = ObjectFactory::GetValue(iStream);

      if (sNextTag == sClassName)
         counter++;

      if (sNextTag == sClassEnd)
         counter--;

      if (counter == 0)
         bDone = true;

      // Read entire class ?
      if (sClassName == sNextTag)
      {
         lastpos = iStream.tellg();
         sNextTag = ObjectFactory::GetTag(iStream, bIsValueTag);
         sNextValue = ObjectFactory::GetValue(iStream);
      }

      if (bIsValueTag)
      {
         if (!access::Class::SetValue(sNextValue, sClassName, sNextTag, pObject))
         {
            std::string sNewClassName = access::Class::GetBaseClassName(sClassName, sNextTag);
            if (sNewClassName.length() > 0)
            {
               access::Variable outVar;
               if (access::Class::GetVariable(sClassName, sNextTag, outVar))
               {
                  void* newAddress = (void*)(size_t(pObject) + size_t(outVar.offset));
                  std::string sNewClassNameEnd = std::string("/") + sNewClassName;
                  ObjectFactory::Deserialize(iStream, sNewClassName, sNewClassNameEnd, newAddress, 1);
               }
            }
            else
            {    
               // Can't Convert to String...
               // Deserialize until sNextTag is </sNextTag> and try to use custom interpretation!
               //ObjectFactory::PutBackValue(iStream, sNextValue);
               //ObjectFactory::PutBackTag(iStream, sNextTag);
               iStream.seekg(lastpos, std::ios::beg);

               std::string sXML = ObjectFactory::ReadXML(iStream, sNextTag);


               access::Variable outVar;
               if (access::Class::GetVariable(sClassName, sNextTag, outVar))
               {
                  std::map<std::string, access::XMLToTypeConversionFunc>::iterator func_it;

                  func_it = access::BaseClass::GetXMLToTypeConversionMap().find(outVar.sTypename);
                  if (func_it != access::BaseClass::GetXMLToTypeConversionMap().end())
                  {
                     void* newAddress = (void*)(size_t(pObject) + size_t(outVar.offset));
                     (*func_it).second(newAddress, sXML);
                  }
               }
            }
         }
      }
   }
}

// Read XML and store in string until sEndTag is reached.
std::string ObjectFactory::ReadXML(std::istream& iStream, const std::string& sTag)
{
   std::ostringstream os;
   std::string sEndTag = "/" + sTag;
   std::string sNextTag = sTag;
   std::string sValue;

   bool bIsValueTag;

   int counter = 0;

   do 
   {
      sNextTag = ObjectFactory::GetTag(iStream, bIsValueTag);
      if (sNextTag == sTag)
      {   
         counter++;
      }

      if (sNextTag == sEndTag)
      {
         counter--;
      }

      os << "<" << sNextTag << ">";
      if (bIsValueTag)
      {
         sValue = ObjectFactory::GetValue(iStream);
         os << sValue;
      } 

   } while(counter != 0 && iStream.good());


   return os.str();

}

//--------------------------------------------------------------------------

std::string ObjectFactory::GetTag(std::istream& iStream, bool& bIsValueTag, bool ignoreattrib, std::string& attribs)
{
   char ch = 'M';
   bool bSearchStartTag = true;
   std::string sTag;
   bIsValueTag = true;
   bool bStartTag = false;
   attribs = "";

   while (ch != '>' && iStream.good())
   {
      iStream.get(ch);

      if (bStartTag && ch == '?')  // "<?" found, possibly <?xml ....?>
      {
         while (ch != '>' && iStream.good())
         {
            iStream.get(ch);
         }
         iStream.get(ch);

         bSearchStartTag = true;
      }

      if (bStartTag && ch == '!')
      {
         iStream.get(ch);
         if (ch == '-')
         {
            iStream.get(ch);
            if (ch == '-')
            {
               //read until '-->'
               int commentClosed = 0;
               while (commentClosed != 3 && iStream.good())
               {
                  iStream.get(ch);
                  if (ch == '-')
                     commentClosed++;
                  else if (ch == '>' && commentClosed == 2)
                     commentClosed++;
                  else
                     commentClosed = 0;
               }
            }
            else
            {
               iStream.putback('-');
            }
         }
         else
         {
            iStream.putback('-');
         }

         iStream.get(ch);
         bSearchStartTag = true;
      }

      bStartTag = false;
      if (ch == '/')
      {
         bIsValueTag = false;
      }

      if (bSearchStartTag)
      {
         if (ch == '<')
         {
            bSearchStartTag = false;
            bStartTag = true;
         }
      }
      else
      {
         if (ignoreattrib)
         {
            if (ch != '>')
            {   
               sTag += ch;
            }
         }
         else
         {
            if (ch == ' ')
            {
               // space in tag means there are possible attributes
               // Example: <tag attr1="hello" attr2="world"> 
               while (ch != '>' && iStream.good())
               {
                  iStream.get(ch);
                  if (ch != '>')
                     attribs += ch;
               }
            }
            else if (ch != '>')
            {   
               sTag += ch;
            }
         }
      }
   }

   

   // test if tag is open and closed at same time
   // for Example: <Tag/> has the meaning <Tag></Tag>
   // The parser takes <Tag/> from input and automatically creates <Tag></Tag>
   if (sTag.length()>1)
   {
      if (sTag[sTag.length()-1] == '/')
      {
         std::string sNewTag;
         for (size_t i=0;i<sTag.length()-1;i++)
         {
            sNewTag += sTag[i];
         }

         std::string sNewCloseTag = "/" + sNewTag;
         ObjectFactory::PutBackTag(iStream, sNewCloseTag); 
         bIsValueTag = true;
         return sNewTag;
      }
   }

   return sTag;
}

//------------------------------------------------------------------------------

std::vector<std::pair<std::string, std::string> > ObjectFactory::ParseAttribs(const std::string& attribs)
{
   std::vector<std::pair<std::string, std::string> > vRet;

   if (attribs.length()>0)
   {
      std::string key;
      std::string value;
      int parse = 0; // 0: key, 1:value
      bool bstartval = false;
      bool bhyph = false;
      for (size_t i=0;i<attribs.length();i++)
      {
         char cur = attribs[i];
         if (parse == 0) // parsing key
         {
            if (cur == '=')
            {
               bstartval = true;
               parse = 1;
            }
            else if (cur == ' ')
            {
               vRet.push_back(std::pair<std::string, std::string>(key, value));
               key = "";
               value = "";
            }
            else
            {
               key = key+cur;
            }
         }
         else if (parse == 1)
         {
            if (bstartval && cur == '\"')
            {
               bhyph = true;  // ignore opening "
            }
            else
            {
               if (bhyph && cur == '\"')
               {
                  // ignore closing "
                  bhyph = false;
               }
               else
               {  
                  if (cur == ' ' && !bhyph) // finish!
                  {
                     vRet.push_back(std::pair<std::string, std::string>(key, value));
                     key = "";
                     value = "";
                     parse = 0;
                  }
                  else
                  {   
                     value = value + cur;
                  }
               }
            }
            bstartval = false;
         }
      }

      if (key.length()>0)
      {
         vRet.push_back(std::pair<std::string, std::string>(key, value));
         key = "";
         value = "";
      }
   }

   return vRet;
}

//------------------------------------------------------------------------------

std::string ObjectFactory::GetValue(std::istream& iStream)
{
   char ch = '\0';
   std::string sValue;

   while (ch != '<' && iStream.good())
   {
      iStream.get(ch);
      if (ch!='<')
         sValue += ch;
   }

   if (ch=='<') iStream.putback(ch);

   return sValue;
}

//--------------------------------------------------------------------------

void ObjectFactory::PutBackValue(std::istream& iStream, const std::string& sValue)
{
   std::string v = sValue; // copy value to reverse it!

   std::reverse(v.begin(), v.end());

   for (size_t i=0;i<v.length();i++)
   {
      iStream.putback(v[i]);
   }

}

//--------------------------------------------------------------------------

void ObjectFactory::PutBackTag(std::istream& iStream, const std::string& sTag)
{
   std::string realtag = std::string("<") + sTag + std::string(">");

   std::reverse(realtag.begin(), realtag.end());

   for (size_t i=0;i<realtag.length();i++)
   {
      iStream.putback(realtag[i]);
   }
}

//--------------------------------------------------------------------------



//----------------------------------
//XML Encode and XML Decode strings:
//----------------------------------

// xml encodings (xml-encoded entities are preceded with '&')
static const char  AMP = '&';
static const char  rawEntity[] = { '<',   '>',   '&',    '\'',    '\"',    0 };
static const char* xmlEntity[] = { "lt;", "gt;", "amp;", "apos;", "quot;", 0 };
static const int   xmlEntLen[] = { 3,     3,     4,      5,       5 };

// Replace xml-encoded entities with the raw text equivalents.

std::string XMLUtils::Decode(const std::string& encoded)
{
  std::string::size_type iAmp = encoded.find(AMP);
  if (iAmp == std::string::npos)
    return encoded;

  std::string decoded(encoded, 0, iAmp);
  std::string::size_type iSize = encoded.size();
  decoded.reserve(iSize);

  const char* ens = encoded.c_str();
  while (iAmp != iSize) 
  {
    if (encoded[iAmp] == AMP && iAmp+1 < iSize) 
    {
      int iEntity;
      for (iEntity=0; xmlEntity[iEntity] != 0; ++iEntity)
      {
         if (strncmp(ens+iAmp+1, xmlEntity[iEntity], xmlEntLen[iEntity]) == 0)
         {
            decoded += rawEntity[iEntity];
            iAmp += xmlEntLen[iEntity]+1;
            break;
         }
      }
      if (xmlEntity[iEntity] == 0)    // unrecognized sequence
      {  decoded += encoded[iAmp++];
      }

    } 
    else 
    {
      decoded += encoded[iAmp++];
    }
  }
    
  return decoded;
}

// Replace raw text with xml-encoded entities.

std::string XMLUtils::Encode(const std::string& raw)
{
  std::string::size_type iRep = raw.find_first_of(rawEntity);
  if (iRep == std::string::npos)
  {
    return raw;
  }

  std::string encoded(raw, 0, iRep);
  std::string::size_type iSize = raw.size();

  while (iRep != iSize) 
  {
    int iEntity;
    for (iEntity=0; rawEntity[iEntity] != 0; ++iEntity)
    { 
       if (raw[iRep] == rawEntity[iEntity])
       {
          encoded += AMP;
          encoded += xmlEntity[iEntity];
          break;
       }
    }
    if (rawEntity[iEntity] == 0)
    {
      encoded += raw[iRep];
    }
    ++iRep;
  }
  return encoded;
}

//--------------------------------------------------------------------------

std::string XMLUtils::GetRootTag(std::istream& iStream)
{
   bool bIsValueTag;
   return ObjectFactory::GetTag(iStream, bIsValueTag);  
}

//--------------------------------------------------------------------------


