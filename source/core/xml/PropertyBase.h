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

#ifndef PROPERTY_BASE_H
#define PROPERTY_BASE_H

#include <string>
#include <sstream>
#include <vector>
#include <list>
#include <map>
#include <ostream>
#include <typeinfo>

//-----------------------------------------------------------------------------
// One way to use offsetof is the C-offsetof for POD data on a class. This is
// dangerous because it is not Standard-C to use it in a non-POD class.
// It works with MSVC and gcc. But gcc gives warnings.
//#define offsetof_cpp(type, mem) offsetof(type,mem)
//-----------------------------------------------------------------------------
// To avoid warnings when compiling with gcc it is possible to use this hack:
//#define offsetof_cpp(c,m) ((size_t)&(((c*)1)->m)-1)
//-----------------------------------------------------------------------------
// Because PropertyBase.h should be fully portable, there is a nice way to
// calculate offset in a portable way.
// Please note, it is calculated during runtime.
// In the macro "c" is ignored and this offsetof_cpp has to
// called from within the class (which is always the case in PropetyBase macro)
#define offsetof_cpp(c,m) ((int) ((char *) &this->m   - (char *) this))

//-----------------------------------------------------------------------------
// Macros to simplify Syntax to define intrusive class access:
// Example:
//
//  BeginPropertyMap(MyDemoClass)
//    XMLProperty(MyDemoClass, "Name", _sName)
//    XMLProperty(MyDemoClass, "Int", _nInteger)
//    XMLProperty(MyDemoClass, "Double", _dDouble)
//  EndPropertyMap(MyDemoClass)
//-----------------------------------------------------------------------------
#define BeginPropertyMap(classname) \
   namespace internal { \
class API_EXPORT _Accessor_##classname : public classname, public Access::BaseClass \
      { public:  static bool _bClassInit;                                  \
      virtual ~_Accessor_##classname(){}                                \
      _Accessor_##classname() \
         { \
         Access::BaseClass::GetClassMap().insert(std::pair<std::string, Access::BaseClass*>(std::string(""#classname), this));\
         Access::BaseClass::GetTypeMap().insert(std::pair<std::string, std::string>(std::string(typeid(classname).name()),std::string(""#classname)));\
         } \
         virtual void DoInit() \
         { \
         if (!_bClassInit) \
            { \
            Access::Members M; \
            Access::Variable V; \
            size_t nOffset; \
            size_t nSize; \
            std::string sTypeIdName; \
            M.SetClassname(""#classname); \
            M.SetTypeId(typeid(classname).name());  

#define EndPropertyMap(classname) Access::Class::AddClass(std::string(""#classname), M); \
           } \
           _bClassInit = true; \
         } \
      };\
      volatile _Accessor_##classname _gInstance_##classname; \
      bool _Accessor_##classname::_bClassInit = false; \
    } \
 CLASS_EXPORT(classname)

#define XMLProperty(classname, varname, var) nOffset = offsetof_cpp(_Accessor_##classname, var); \
   nSize = sizeof(var); \
   sTypeIdName = typeid(var).name();\
   V.offset = nOffset;\
   V.sClassname = Access::BaseClass::GetBaseClassName(sTypeIdName);  \
   V.size = nSize; \
   V.readonly = false; \
   V.sTypename = sTypeIdName; \
   M.AddVariable(std::string(varname),V);

#define XMLPropertyReadOnly(classname, varname, var) nOffset = offsetof_cpp(_Accessor_##classname, var); \
   nSize = sizeof(var); \
   sTypeIdName = typeid(var).name();\
   V.offset = nOffset;\
   V.sClassname = Access::BaseClass::GetBaseClassName(sTypeIdName);  \
   V.size = nSize; \
   V.readonly = true; \
   V.sTypename = sTypeIdName; \
   M.AddVariable(std::string(varname),V);


#define Inherits(classname) Access::Members m; \
   if (Access::Class::GetMembers(std::string(""#classname), m)) \
   { \
   std::list< std::pair<std::string, Access::Variable> >::iterator it = m.GetVariableMap().begin(); \
   while (it != m.GetVariableMap().end()) \
      { \
      M.AddVariable((*it).first,(*it).second); \
      it++; \
      } \
   } \

//-----------------------------------------------------------------------------
// Macro to define a function to convert a new type to string...
#define TYPE_TO_STRING_CONVERSION(type, func) namespace internal { class API_EXPORT _TypeToStringConversion_##func \
{ public: \
   _TypeToStringConversion_##func(){ Access::BaseClass::GetStringConversionMap().insert(std::pair<std::string, Access::TypeToStringConversionFunc>(typeid(type).name(), func));} \
   virtual ~_TypeToStringConversion_##func(){} }; _TypeToStringConversion_##func g_CreateInstance_##func; }  

// Macro to define a function to convert a new type to XML
#define TYPE_TO_XML_CONVERSION(type, func)  namespace internal { class API_EXPORT _TypeToXMLConversion_##func \
{ public: \
   _TypeToXMLConversion_##func(){ Access::BaseClass::GetXMLConversionMap().insert(std::pair<std::string, Access::TypeToXMLConversionFunc>(typeid(type).name(), func));} \
   virtual ~_TypeToXMLConversion_##func(){} }; _TypeToXMLConversion_##func g_CreateInstanceXML_##func; } 

//-----------------------------------------------------------------------------
// Macro to define a function to convert string to type
#define STRING_TO_TYPE_CONVERSION(type, func)  namespace internal { class API_EXPORT _StringToTypeConversion_##func \
{ public: \
   _StringToTypeConversion_##func(){ Access::BaseClass::GetStringToTypeConversionMap().insert(std::pair<std::string, Access::StringToTypeConversionFunc>(typeid(type).name(), func));} \
   virtual ~_StringToTypeConversion_##func(){} }; _StringToTypeConversion_##func g_CreateInstanceString2Type_##func; } 

//-----------------------------------------------------------------------------
// Macro to define a function to convert xml to type
#define XML_TO_TYPE_CONVERSION(type, func)  namespace internal { class API_EXPORT _XMLToTypeConversion_##func \
{ public: \
   _XMLToTypeConversion_##func(){ Access::BaseClass::GetXMLToTypeConversionMap().insert(std::pair<std::string, \
   Access::XMLToTypeConversionFunc>(typeid(type).name(), func));} \
   virtual ~_XMLToTypeConversion_##func(){} }; _XMLToTypeConversion_##func g_CreateInstanceXML2Type_##func; } 
//-----------------------------------------------------------------------------

   namespace Access
   {
      typedef std::string(*TypeToStringConversionFunc)(void*);
      typedef std::string(*TypeToXMLConversionFunc)(void*, std::string);

      typedef void(*StringToTypeConversionFunc)(void*, std::string);
      typedef void(*XMLToTypeConversionFunc)(void*, std::string);

      //------------------------------------------------------------------------
      //! \ingroup xml
      //! \author Martin Christen, martin.christen@gmail.com
      class LIBRARY_API BaseClass
      {
      public:
         virtual void DoInit() = 0;

         static std::string GetBaseClassName(std::string sRtti);

         static std::map<std::string, Access::BaseClass*>& GetClassMap();
         static std::map<std::string, std::string>& GetTypeMap();

         // Type -> String or XML
         static std::map<std::string, TypeToStringConversionFunc>& GetStringConversionMap();
         static std::map<std::string, TypeToXMLConversionFunc>& GetXMLConversionMap();

         // String or XML -> Type
         static std::map<std::string, StringToTypeConversionFunc>& GetStringToTypeConversionMap();
         static std::map<std::string, XMLToTypeConversionFunc>& GetXMLToTypeConversionMap();
      };

      //------------------------------------------------------------------------
      //! \ingroup xml
      //! \author Martin Christen, martin.christen@gmail.com
      class LIBRARY_API Variable
      {
      public:
         Variable();
         virtual ~Variable();

         bool readonly;          // true if this varible has read-only flag


         std::string sTypename;  // type-name
         std::string sClassname; // real class name
         size_t offset;
         size_t size;

         // Convert variable Value to String (not always possible!!)
         std::string ToString(void* pInstance);

         // Convert Variable Value to XML
         std::string ToXML(void* pInstance, std::string sTag);

         // Set variable value by string. Returns false if failed.
         bool        FromString(std::string sValue, void* pInstance);

         // Set variable value by XML. Returns false if failed.
         bool        FromXML(std::string sXML, void* pInstance);
      };

      //------------------------------------------------------------------------
      //! \ingroup xml
      //! \author Martin Christen, martin.christen@gmail.com
      class LIBRARY_API Members
      {
         friend class Variable;
      public:
         Members();
         virtual ~Members();

         void SetTypeId(std::string sId);
         void SetClassname(std::string sClassname);
         void AddVariable(std::string sVariableName, Variable& oVariable);

         std::string ToXML(void* pInstance, bool bAdvance = true);

         std::string GetValue(std::string sMember, void* pInstance);

         bool        SetValue(std::string sValue, std::string sMember, void* pInstance);

         std::string& GetTypeId();

         std::list< std::pair<std::string, Access::Variable> >& GetVariableMap(){return _mapNameOffset;}

         std::string GetClassname(){return _sClassName;}


         static void IncreaseXMLAdvance() {_nXMLAdvance += 3;}
         static void DecreaseXMLAdvance() {_nXMLAdvance -= 3;}
         static int  GetXMLadvance(){return _nXMLAdvance;}

      protected:
         std::string _sClassTypeId;
         std::string _sClassName;
         std::list< std::pair<std::string, Access::Variable> > _mapNameOffset;
         static int _nXMLAdvance;
      };

      //------------------------------------------------------------------------
      //! \ingroup xml
      //! \author Martin Christen, martin.christen@gmail.com
      class LIBRARY_API Class
      {
      public:
         Class();
         virtual ~Class();

         static void AddClass(std::string sClassName, Members& oMemberList);

         static void ToXML(std::ostream& stream, std::string sClassname, void* pInstance, bool bHeader = true);

         static void* FromXML(std::istream& stream, std::string sExpectedClass);

         static std::string GetValue(std::string sClassname, std::string sMembername, void* pInstance);

         static bool SetValue(std::string sValue, std::string sClassname, std::string sMembername, void* pInstance);

         static bool GetMembers(std::string BaseClass, Members& oMembers);

         static std::string GetBaseClassName(std::string ClassName, std::string MemberName);

         static std::map<std::string, Members>& GetInstanceMap();

         static void InitClass(std::string sBaseClass);

         static bool GetVariable(std::string sBaseClass, std::string sMember, Variable& outVar);

      };
   }

   //---------------------------------------------------------------------------
   //! \ingroup xml
   //! \author Martin Christen, martin.christen@gmail.com
   class LIBRARY_API ObjectFactory
   {
   public:
      static void Deserialize(std::istream& iStream, const std::string& sClassName, const std::string& sClassEnd, void* pObject, int counter = 1);
      static std::string GetTag(std::istream& iStream, bool& bIsValueTag, bool ignoreattrib = true, std::string attribs = std::string());
      static std::vector<std::pair<std::string, std::string> > ParseAttribs(const std::string& attribs);
      static std::string GetValue(std::istream& iStream);
      static void PutBackTag(std::istream& iStream, const std::string& sTag);
      static void PutBackValue(std::istream& iStream, const std::string& sValue);
      // Read XML and store in string until sEndTag is reached.
      static std::string ReadXML(std::istream& iStream, const std::string& sTag);
   };

   //---------------------------------------------------------------------------
   //! \ingroup xml
   //! \author Martin Christen, martin.christen@gmail.com
   class LIBRARY_API XMLUtils
   {
   public:
      //! Encode string to XML (Example: &quot;, &lt;, ...)
      //! This must be implemented when writing strings!
      static std::string Encode(const std::string& raw);

      //! Decode XML string 
      //! This mus be used when reading strings
      static std::string Decode(const std::string& encoded);

      //! Retrieves root tag of an XML document
      static std::string GetRootTag(std::istream& iStream);
   };


//-----------------------------------------------------------------------------
// Serialization groups (list, vector)
//-----------------------------------------------------------------------------

// Serialize group of serializable Type:
// After property definition type (in .cpp file)
//
//    SerializeVector(TYPENAME);     // for std::vector<TYPENAME>
//    SerializeList(TYPENAME);       // for std::list<TYPENAME>
//
// Typename can't special chars like '<' or '>' (template). 
// If you really want to use templates you have to use a typedef before!
// for example: SerializeVector(boost::shared_ptr<int>) is NOT allowed and results
// in cryptic compiler errors.

   namespace internal
   {
      namespace xml
      {
         //---------------------
         // EXPORT STD::VECTOR<T>
         //---------------------

         // XML Serialization of a std::vector of a specified class:
         template <class T>
         std::string VecToXML(void* pAddress, std::string sTag)
         {
            std::ostringstream os;

            std::vector<T>* pData = (std::vector<T>*)(pAddress); 

            // only write children node-xml if there are members in list!
            if ((*pData).size()>0)
            {
               os << "<" <<  sTag << ">\n";
               Access::Members::IncreaseXMLAdvance();

               std::string sClassName = types::GetClassNameFromTypeIdName(typeid(T).name());

               for (size_t i=0;i<(*pData).size();i++)
               {
                  std::ostringstream sOutXML;

                  Access::Class::ToXML(sOutXML, sClassName, &((*pData)[i]), false);
                  os << sOutXML.str();
               }

               Access::Members::DecreaseXMLAdvance();
               for (int i=0;i<Access::Members::GetXMLadvance();i++)
                  os << " ";
               os << "</" << sTag << ">";
            }

            return os.str();
         }

         //---------------------
         // EXPORT STD::LIST<T>
         //---------------------
         // XML Serialization of a std::list of a specified class:
         template <class T>
         std::string ListToXML(void* pAddress, std::string sTag)
         {
            std::ostringstream os;

            std::list<T>* pData = (std::list<T>*)(pAddress); 

            // only write children node-xml if there are members in list!
            if ((*pData).size()>0)
            {
               os << "<" <<  sTag << ">\n";
               Access::Members::IncreaseXMLAdvance();

               std::string sClassName = types::GetClassNameFromTypeIdName(typeid(T).name());

               T* it = pData->begin();
               while (it!=pData->end())
               {
                  std::ostringstream sPropertyXML;

                  Access::Class::ToXML(sPropertyXML, sClassName, &(*it), false);
                  os << sPropertyXML.str();
                  it++;
               }

               Access::Members::DecreaseXMLAdvance();
               for (int i=0;i<Access::Members::GetXMLadvance();i++)
                  os << " ";
               os << "</" << sTag << ">";
            }

            return os.str();
         }

         //--------------------------------------------------------------------------

         //-------------------------------
         // IMPORT STD::VECTOR<T> from XML
         //-------------------------------

         template<class T>
         void XMLToVec(void* pAddress, std::string sXML)
         {
            std::stringstream os;
            os << sXML;

            std::vector<T>* pData = (std::vector<T>*)(pAddress);
            pData->clear();

            bool bValueTag;
            std::string sTag = ObjectFactory::GetTag(os, bValueTag);
            std::string sValue = ObjectFactory::GetValue(os);
            std::string sTagEnd = "/" + sTag;

            std::string sClassName = types::GetClassNameFromTypeIdName(typeid(T).name());

            do 
            {
               sTag = ObjectFactory::GetTag(os, bValueTag);
               sValue = ObjectFactory::GetValue(os);

               if (sTag != sTagEnd)
               {
                  if (sTag == sClassName)
                  {
                     ObjectFactory::PutBackValue(os, sValue);
                     ObjectFactory::PutBackTag(os, sTag);
                     std::string sNewNodeXML = ObjectFactory::ReadXML(os, sTag);
                     std::istringstream sPropertyXML(sNewNodeXML);
                     T* p = (T*)Access::Class::FromXML(sPropertyXML, sClassName);

                     if (p)
                     {
                        pData->push_back(*p);
                        delete p;
                     }
                  }
               }

            } while (sTag != sTagEnd && os.good());
         }

         //-------------------------------
         // IMPORT STD::LIST<T> from XML
         //-------------------------------

         template<class T>
         void XMLToList(void* pAddress, std::string sXML)
         {
            std::stringstream os;
            os << sXML;

            std::list<T>* pData = (std::list<T>*)(pAddress);
            pData->clear();

            bool bValueTag;
            std::string sTag = ObjectFactory::GetTag(os, bValueTag);
            std::string sValue = ObjectFactory::GetValue(os);
            std::string sTagEnd = "/" + sTag;

            std::string sClassName = types::GetClassNameFromTypeIdName(typeid(T).name());

            do 
            {
               sTag = ObjectFactory::GetTag(os, bValueTag);
               sValue = ObjectFactory::GetValue(os);

               if (sTag != sTagEnd)
               {
                  if (sTag == sClassName)
                  {
                     ObjectFactory::PutBackValue(os, sValue);
                     ObjectFactory::PutBackTag(os, sTag);
                     std::string sNewNodeXML = ObjectFactory::ReadXML(os, sTag);
                     std::istringstream sPropertyXML(sNewNodeXML);
                     T* p = (T*)Access::Class::FromXML(sPropertyXML, sClassName);

                     if (p)
                     {
                        pData->push_back(*p);
                        delete p;
                     }
                  }
               }

            } while (sTag != sTagEnd && os.good());
         }


      }  // namespace xml
   } // namespace internal
//-----------------------------------------------------------------------------

#define SerializeVector(classname) \
   std::string function_vecinstance_write_##classname(void* pAddress, std::string sTag) \
{ return internal::xml::VecToXML<classname>(pAddress, sTag); } \
   TYPE_TO_XML_CONVERSION(std::vector<classname>, function_vecinstance_write_##classname) \
   void function_vecinstance_read_##classname(void* pAddress, std::string sXML) \
{ internal::xml::XMLToVec<classname>(pAddress, sXML); } \
   XML_TO_TYPE_CONVERSION(std::vector<classname>, function_vecinstance_read_##classname)

//-----------------------------------------------------------------------------

#define SerializeList(classname) \
   std::string function_lstinstance_write_##classname(void* pAddress, std::string sTag) \
{ return internal::xml::ListToXML<classname>(pAddress, sTag); } \
   TYPE_TO_XML_CONVERSION(std::list<classname>, function_lstinstance_write_##classname) \
   void function_lstinstance_read_##classname(void* pAddress, std::string sXML) \
{ internal::xml::XMLToList<classname>(pAddress, sXML); } \
   XML_TO_TYPE_CONVERSION(std::list<classname>, function_lstinstance_read_##classname)

//-----------------------------------------------------------------------------




#endif
