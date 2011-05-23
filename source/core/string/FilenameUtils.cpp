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

#include "FilenameUtils.h"
#include "StringUtils.h"
#include <locale>
#include <iostream>
#include <cmath>


// Implementation for Wide Strings
class FilenameUtilsWide
{
public:
   static std::wstring DelimitPath(const std::wstring& pPath, wchar_t chDelimit = L'\0');
   static std::wstring GetFileRoot(const std::wstring& pFilePath, bool bDelimit = true);
   static bool IsFileOnWeb(const std::wstring& strPath);
   static std::wstring ExtractFileName(const std::wstring& pFilePath);
   static std::wstring ExtractBaseFileName(const std::wstring& pFilePath, wchar_t pChDel = '.');
   static std::wstring ExtractFileExtension(const std::wstring& pFilePath, wchar_t pChDel = L'.');
   static std::wstring MakeHierarchicalFileName(const std::wstring& pFileName, int nDepth);
   static std::wstring UnifyPathDelimiter(const std::wstring& pPath, wchar_t pChDel = L'/');
   static std::wstring GetPathHostID(const std::wstring &strPath, bool bDelimit = true);
   static std::wstring MakeRelativePath(const std::wstring& pPathDst, const std::wstring& pPathSrc, bool bDelimit=false);
   static std::wstring MakeAbsolutePath(const std::wstring& pPathRoot, const std::wstring& pPathRel, bool bDelimit = false);
};

//-----------------------------------------------------------------------------
inline wchar_t _CharToWchar(char c)
{
   return std::use_facet< std::ctype<wchar_t> >(std::cout.getloc()).widen(c);
}


//-----------------------------------------------------------------------------

std::wstring FilenameUtilsWide::DelimitPath(const std::wstring& pPath, wchar_t chDelimit)
{
   std::wstring strRet(pPath);

   if (chDelimit == L'\0')
   {
      int iBS = (int)strRet.rfind(L'\\');
      int iS  = (int)strRet.rfind(L'/');
      chDelimit = (iBS > iS) ? L'\\' : L'/';
   }

   size_t nLen = strRet.size();
   if (nLen > 0 && strRet[nLen-1] != chDelimit)
      strRet += chDelimit;
   return strRet;
}


//-----------------------------------------------------------------------------

std::wstring FilenameUtilsWide::GetFileRoot(const std::wstring& pFilePath, bool bDelimit)
{
   std::wstring strRet;
   // Try to extract the path from the file name
   strRet = pFilePath;
   int iBS = (int)strRet.rfind(L'\\');
   int iS  = (int)strRet.rfind(L'/');
   int inx;
   wchar_t chDel;

   if (iBS == -1 && iS == -1)
      return std::wstring();

   if (iBS > iS)
   {
      inx = iBS;
      chDel = L'\\';
   }
   else
   {
      inx = iS;
      chDel = L'/';
   }

   strRet = StringUtils::Left(strRet, math::Max(iS, iBS));

   if (strRet.size() <= 0)
   {
      // The file name has been passed without path.
      return strRet;
   }

   if (bDelimit)
      strRet = DelimitPath(strRet, chDel);

   return strRet;
}


//-----------------------------------------------------------------------------

bool FilenameUtilsWide::IsFileOnWeb(const std::wstring& strPath)
{
   if (strPath.size() >= 7 && strPath.substr(0,7) == L"http://")
   {
      return true;
   }
   else if (strPath.size() >= 7 && strPath.substr(0,7) == L"file://")
   {
      return true;
   }

   return false;
}


//-----------------------------------------------------------------------------

std::wstring FilenameUtilsWide::ExtractFileName(const std::wstring& pFilePath)
{
   wchar_t pChDel;
   std::wstring strRet(pFilePath);
   int iBS = (int)strRet.rfind(L'\\');
   int iS  = (int)strRet.rfind(L'/');
   int inx = -1;

   if (iBS > iS)
   {
      inx = iBS;
      pChDel = L'\\';
   }
   else if (iS >= 0)
   {
      inx = iS;
      pChDel = L'/';
   }
   else
   {
      pChDel = L'\0';
   }

   strRet = StringUtils::Right(strRet, strRet.size() - inx - 1);
   return strRet;
}

//-----------------------------------------------------------------------------

std::wstring FilenameUtilsWide::ExtractBaseFileName(const std::wstring& pFilePath, wchar_t pChDel)
{
   std::wstring tmp = ExtractFileName(pFilePath);
   int iIndex = (int)tmp.find(pChDel);
   if (iIndex != -1)
      tmp = StringUtils::Left(tmp, iIndex);

   return tmp;
}
//-----------------------------------------------------------------------------

std::wstring FilenameUtilsWide::ExtractFileExtension(const std::wstring& pFilePath, wchar_t pChDel)
{
   std::wstring strRet = pFilePath;
   int iIndex = (int)strRet.rfind(pChDel);
   if (iIndex != -1)
      strRet = StringUtils::Right(strRet, strRet.size() - iIndex - 1);

   return strRet;
}

//-----------------------------------------------------------------------------

std::wstring FilenameUtilsWide::MakeHierarchicalFileName(const std::wstring& pFileName, int nDepth) 
{
   std::wstring fileExtension = ExtractFileExtension(pFileName);
   std::wstring filePath = GetFileRoot(pFileName);
   std::wstring fileBaseName = ExtractBaseFileName(pFileName);
   int nLength = fileBaseName.length();
   int pDirs = (int)floor(double(nLength / nDepth));
   std::wstring pHierarchicalPath;
   for (int i = 0; i < pDirs; i++) {
      pHierarchicalPath.append(fileBaseName.substr(i*nDepth, nDepth));

      if(i != pDirs-1 || nLength % nDepth != 0)
      {
         pHierarchicalPath.append(L"/");
      }
   }
   pHierarchicalPath.append(fileBaseName.substr(pDirs * nDepth, nLength % nDepth));
   if(fileExtension.length() > 0) 
   {
      pHierarchicalPath.append(L"." + fileExtension);
   }

   filePath.append(pHierarchicalPath);
   return filePath;
}

//-----------------------------------------------------------------------------

std::wstring FilenameUtilsWide::UnifyPathDelimiter(const std::wstring& pPath, wchar_t pChDel)
{
   std::wstring strRet(pPath);

   for (size_t i=0;i<strRet.size();i++)
   {
      if (strRet[i] == L'/' ||
         strRet[i] == L'\\')
      {
         strRet[i] = pChDel;
      }
   }
   return strRet;
}

//-----------------------------------------------------------------------------

std::wstring FilenameUtilsWide::GetPathHostID(const std::wstring &strPath, bool bDelimit)
{
   std::wstring strTemp, strRet, strPathLower;
   int inx;
   wchar_t chDel = L'\\';

   if (strPath.size() == 0)
      return strRet;

   strPathLower = strPath;
   //strPathLower.MakeLower();

   // Check for Windows drive specification ("C:\Temp")
   if (strPath.size() >= 2 /*&& cfIsAlpha(strPath[0])*/ && strPath[1] == L':')
   {
      strRet  = strPath[0];
      strRet += L':';
      chDel = L'\\';
   }
   // Check for LAN drive specification ("\\server\Temp")
   else if (strPath.size() > 2 && strPath.substr(0,2) == L"\\\\")
   {
      strTemp = StringUtils::Right(strPath, strPath.size() - 2);
      inx = (int)strTemp.find(L'\\');
      if (inx > 1)
      {
         strRet = L"\\\\";
         strRet += strTemp.substr(0,inx);
         chDel = L'\\';
      }
   }
   // Check for HTTP URL ("http://www.fhnw.ch/Temp")
   else if (strPath.size() >= 7 && strPathLower.substr(0,7) == L"http://")
   {
      inx = (int)strPathLower.find(L'/', 7);
      if (inx >= 7)
      {
         strRet = strPath.substr(0,inx);
         chDel = L'/';
      }
   }
   // Linux/Unix/MacOS Directory
   else if (strPath.size() > 0 && ((strPath[0] == L'\\') || (strPath[0] == '/')))
   {
      strRet = L"/"; //StringUtils::Right(strPath, strPath.size() - 1);
   }
   else
   {
      // At least check the delimiter
      for (size_t i = 0; i < strPath.size(); i++)
         if (strPath[i] == L'/' || strPath[i] == L'\\')
         {
            chDel = strPath[i];
            break;
         }
   }


   if (bDelimit && strPath.size() > 0)
      strRet = DelimitPath(strRet, chDel);

   return strRet;
}

//-----------------------------------------------------------------------

std::wstring FilenameUtilsWide::MakeRelativePath(const std::wstring& pPathDst, const std::wstring& pPathSrc, bool bDelimit)
{
   std::wstring strIDDst, strIDSrc;
   std::wstring strPathDst = pPathDst, strPathSrc = pPathSrc;
   std::wstring strTDst = strPathDst, strTSrc = strPathSrc;
   int i, n, inx;
   int nCase = 0;

   wchar_t chDelSrc = L'/';
   wchar_t chDelDst = L'/';

   // If either path is empty, return the other.
   if (strPathSrc == L"")
   {
      if (bDelimit)
         strPathDst = DelimitPath(strPathDst);
      return strPathDst;
   }
   if (strPathDst == L"")
   {
      if (bDelimit)
         strPathSrc = DelimitPath(strPathSrc);
      return strPathSrc;
   }

   // Get the paths host ids.
   strIDDst = GetPathHostID(strTDst, true);
   strIDSrc = GetPathHostID(strTSrc, true);

   // See if the paths are absolute ("C:\Temp"), absolute without host id ("\temp")
   // or relative ("Temp"). Store a case for a later case decision.
   // First check destination
   if (strIDDst == L"") 
   {
      // The destination path has no host ID. See if it is absolute or relative.
      if (strTDst[0] == chDelDst)
      {
         // It is absolute with no host id. 
         strTDst = StringUtils::Right(strTDst, strTDst.size() - 1);
         nCase = 3;
      }
      else
      {
         // Path is relative
         nCase = 6;
      }
   }
   // Else path is absolute

   // Now check source
   if (strIDSrc == L"") 
   {
      // The source path has no host ID. See if it is absolute or relative.
      if (strTSrc[0] == chDelSrc)
      {
         // It is absolute with no host id. 
         strTSrc = StringUtils::Right(strTSrc, strTSrc.size() - 1);
         nCase += 1;
      }
      else
      {
         // Path is relative
         nCase += 2;
      }
   }

   switch (nCase)
   {
   case 0:
      // Both paths are absolute with host ID. See if both host IDs are equal
      if (strIDSrc != strIDDst)
         return strPathDst;
      // else fall through for further processing
      break;
   case 3:
      // The dest path is absolute but has no host ID. Assume sources Host id is valid.
      // Simply fall through for further processing
      break;
   case 4:
      // Both paths are absolute without host ID. Assume both paths share the same
      // ID.  Simply fall through for further processing.
      break;
   case 8:
      /*
      // Both paths are relative. Simply concatenate them.
      strPathSrc = cfDelimitPath(strPathSrc);
      strPathSrc += strPathDst;
      if (bDelimit)
      strPathSrc = cfDelimitPath(strPathSrc);
      return strPathSrc;
      break;
      */
   case 1:
   case 2:
   case 5:
   case 6:
   case 7:
   default:
      // In all these cases no reasonable relative path can be constructed.
      // Return the (possibly absolute) destination path instead.
      if (bDelimit)
         strPathDst = DelimitPath(strPathDst, chDelDst);
      return strPathDst;
      break;
   }

   // Now we can start parsing the paths and construct a relative path from one to the other.
   // Both paths can be considered absolute without host ID AND without preceding delimiter.
   assert(strTDst.size() > 0);
   assert(strTSrc.size() > 0);
   if (strTDst[0] == chDelDst)
   {
      strTDst = StringUtils::Right(strTDst, strTDst.size() - 1);
   }
   if (strTSrc[0] == chDelSrc)
   {
      strTSrc = StringUtils::Right(strTSrc, strTSrc.size() - 1);
   }

   // Just to make sure: Make both delimiting characters the source's delimiting character
   strTDst = UnifyPathDelimiter(strTDst, chDelSrc);
   strTSrc = UnifyPathDelimiter(strTSrc, chDelSrc);

   // Make sure paths are canonical (no containing \..\)
   //strTDst = PurgePath(strTDst);
   //strTSrc = PurgePath(strTSrc);

   // Find the first differing subdirectory index
   n = (int)math::Min<size_t>(strTDst.size(), strTSrc.size());

   i = -1;
   for (inx = 0; inx < n; inx++)
   {
      if (strTDst[inx] == chDelDst || strTDst[inx] == chDelSrc)
         i = inx;
      if (strTDst[inx] != strTSrc[inx])
         break;
   }

   // Make i point to the character behind the last delimiter
   if (i >= 0)
      i++;

   if (inx == n && n == math::Max<int>((int)strTDst.size(), (int)strTSrc.size()))
   {
      // Paths are identical
      strPathDst = L"";
      return strPathDst;
   }

   // Construct the output path
   strTDst = StringUtils::Right(strTDst, strTDst.size() - i);
   strTSrc = StringUtils::Right(strTSrc, strTSrc.size() - i);
   n -= i;

   // Find next delimiter and insert a "one level up" for each into the 
   // output path.
   strPathDst = L"";
   for (inx = (int)strTSrc.find(chDelSrc); inx > 0; inx = (int)strTSrc.find(chDelSrc))
   {
      assert(StringUtils::Left(strTSrc, inx) != L"..");
      strPathDst += L"..";
      strPathDst += chDelDst;
      strTSrc = StringUtils::Right(strTSrc, strTSrc.size() - inx - 1);
   }

   strPathDst += strTDst;
   if (bDelimit)
      strPathDst = DelimitPath(strPathDst, chDelDst);
   return strPathDst;
}

//-----------------------------------------------------------------------------

std::wstring FilenameUtilsWide::MakeAbsolutePath(const std::wstring& pPathRoot, const std::wstring& pPathRel, bool bDelimit)
{
   std::wstring sPathRoot   = pPathRoot;
   std::wstring sPathRel    = pPathRel;
   // Delimit root path
   sPathRoot = DelimitPath(sPathRoot);

   // Remove delimiter in relative path
   if (sPathRel.size()>=1 && (sPathRel[0] == L'/' || sPathRel[0] == L'\\'))
   {
      sPathRel = sPathRel.substr(1, sPathRel.size()-1);
   }

   // Make an absolute path
   std::wstring sPath = sPathRoot + sPathRel;

   // Unify path delimiter
   sPath = UnifyPathDelimiter(sPath);

   if (bDelimit)
      sPath = DelimitPath(sPath);

   return sPath;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

bool FilenameUtils::IsFileOnWeb(const std::string& strPath)
{
   return FilenameUtilsWide::IsFileOnWeb(StringUtils::Utf8_To_wstring(strPath));
}

//-----------------------------------------------------------------------------

std::string FilenameUtils::DelimitPath(const std::string& pPath, char chDelimit)
{
   std::wstring sResult = FilenameUtilsWide::DelimitPath(StringUtils::Utf8_To_wstring(pPath), _CharToWchar(chDelimit));
   return StringUtils::wstring_To_Utf8(sResult);
}

//-----------------------------------------------------------------------------

std::string FilenameUtils::GetFileRoot(const std::string& pFilePath, bool bDelimit)
{
   std::wstring sResult = FilenameUtilsWide::GetFileRoot(StringUtils::Utf8_To_wstring(pFilePath), bDelimit);
   return StringUtils::wstring_To_Utf8(sResult);
}

//-----------------------------------------------------------------------------

std::string FilenameUtils::ExtractFileName(const std::string& pFilePath)
{
   std::wstring sResult = FilenameUtilsWide::ExtractFileName(StringUtils::Utf8_To_wstring(pFilePath));
   return StringUtils::wstring_To_Utf8(sResult);
}

//-----------------------------------------------------------------------------

std::string FilenameUtils::ExtractBaseFileName(const std::string& pFilePath, char pChDel)
{
   std::wstring sResult = FilenameUtilsWide::ExtractBaseFileName(StringUtils::Utf8_To_wstring(pFilePath), _CharToWchar(pChDel));
   return StringUtils::wstring_To_Utf8(sResult);
}

//-----------------------------------------------------------------------------

std::string FilenameUtils::ExtractFileExtension(const std::string& pFilePath, char pChDel)
{
   std::wstring sResult = FilenameUtilsWide::ExtractFileExtension(StringUtils::Utf8_To_wstring(pFilePath), _CharToWchar(pChDel));
   return StringUtils::wstring_To_Utf8(sResult);
}

//-----------------------------------------------------------------------------

std::string FilenameUtils::MakeHierarchicalFileName(const std::string& pFileName, int nDepth)
{
   std::wstring sResult = FilenameUtilsWide::MakeHierarchicalFileName(StringUtils::Utf8_To_wstring(pFileName), nDepth);
   return StringUtils::wstring_To_Utf8(sResult);
}

//-----------------------------------------------------------------------------

std::string FilenameUtils::UnifyPathDelimiter(const std::string& pPath, char pChDel)
{
   std::wstring sResult = FilenameUtilsWide::UnifyPathDelimiter(StringUtils::Utf8_To_wstring(pPath), _CharToWchar(pChDel));
   return StringUtils::wstring_To_Utf8(sResult);
}

//-----------------------------------------------------------------------------

std::string FilenameUtils::GetPathHostID(const std::string &strPath, bool bDelimit)
{
   std::wstring sResult = FilenameUtilsWide::GetPathHostID(StringUtils::Utf8_To_wstring(strPath), bDelimit);
   return StringUtils::wstring_To_Utf8(sResult);
}

//-----------------------------------------------------------------------------

std::string FilenameUtils::MakeRelativePath(const std::string& pPathDst, const std::string& pPathSrc, bool bDelimit)
{
   std::wstring sResult = FilenameUtilsWide::MakeRelativePath(StringUtils::Utf8_To_wstring(pPathDst), StringUtils::Utf8_To_wstring(pPathSrc), bDelimit);
   return StringUtils::wstring_To_Utf8(sResult);
}

//-----------------------------------------------------------------------------

std::string FilenameUtils::MakeAbsolutePath(const std::string& pPathRoot, const std::string& pPathRel, bool bDelimit)
{
   std::wstring sResult = FilenameUtilsWide::MakeAbsolutePath(StringUtils::Utf8_To_wstring(pPathRoot), StringUtils::Utf8_To_wstring(pPathRel) , bDelimit);
   return StringUtils::wstring_To_Utf8(sResult);
}

//-----------------------------------------------------------------------------
