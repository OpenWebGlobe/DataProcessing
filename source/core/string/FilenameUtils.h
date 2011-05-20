/*******************************************************************************
Project       : i3D OpenGlobe SDK - Reference Implementation
Version       : 1.0
Author        : Martin Christen, martin.christen@fhnw.ch
Copyright     : (c) 2006-2010 by FHNW/IVGI. All Rights Reserved

$License$
*******************************************************************************/

#ifndef _FILENAMEUTILS_H_
#define _FILENAMEUTILS_H_

#include "og.h"
#include <cassert>
#include <string>

//! \brief File Utilities
//! A collection of useful routines for file handling like path delimiting, extracting file root or file extension or making a file path relative.
//! These utilities are basically string management for files. All filenames are utf8 encoded.
//! \ingroup file string
//! \author Martin Christen, martin.christen@fhnw.ch
class FilenameUtils
{
public:
   //! \brief Append delimiter to a path
   //! Append a delimiter to end of path if it doesn't exist yet.
   //! A deliminiter is usually the '/' sign, on Windows it is '\'.
   //! \param pPath path to be delimited
   //! \param chDelimit the delimit character, which is usually '\' or '/'. If \0 is specified, then the delimiter char which exists in the path is used.
   //! \return the delimited path
   static std::string  DelimitPath(const std::string& pPath, char chDelimit = '\0');

   //! \brief Extract file root.
   //! Extracts the file root (path) from a given file. For example c:/data/bla.tif would
   //! return C:/data/.
   //! \param pFilePath file with absolute directory
   //! \param bDelimit true if the returned path should have a delimiter (/ or \)
   //! \return the file root
   static std::string GetFileRoot(const std::string& pFilePath, bool bDelimit = true);


   //! \brief test if file path is a "web reference".
   //! Returns true if file path starts with 'http://' or 'file://'.
   //! \param strPath the path to be tested
   //! \return true if patch is a web reference, false otherwise
   static bool IsFileOnWeb(const std::string& strPath);

   //! \brief extract file name from an ascii based path.
   //! Extracts the file name from an ascii based path, for example '/usr/home/a/image.tif' would return 'image.tif'.
   //! \param pFilePath the file path 
   //! \return the extracted file name 
   static std::string ExtractFileName(const std::string& pFilePath);

   //! \brief extract file name without extension from a path.
   //! Extracts the file name from a path, for example '/usr/home/a/image.tif' would return 'image'.
   //! \param pFilePath the file path
   //! \return the extracted file name
   static std::string ExtractBaseFileName(const std::string& pFilePath, char pChDel = '.');

   //! \brief Extract file extension.
   //! Extracts the file extension from a path. For example '/home/a/image.tif' would return 'tif'
   //! Optionally an extension delimiter char can be specified, usually this is '.'.
   //! \param pFilePath the path pointing to a file
   //! \param pChDel the delimiting character
   //! \return string containing file extension
   static std::string ExtractFileExtension(const std::string& pFilePath, char pChDel = '.');


   //! \brief Generate hierarchical file path
   //! Creates a path of hierarchical directories from filename according to the hierarchical depth.
   //! \param pFileName a filename without path
   //! \param nDepth is the depth of hierarchy
   //! \return string containing the relative path to the file, subdivided in directories
   static std::string  MakeHierarchicalFileName(const std::string& pFileName, int nDepth);


   //! \brief clean up wrong delimiters.
   //! Looks for all occurences of / or \ and changes all delimiters to the same one specified.
   //! \param pPath absolute path
   //! \param pChDel delimiter character which is used to unify.
   //! \return the path with unified delimiter
   static std::string UnifyPathDelimiter(const std::string& pPath, char pChDel = '/');

   //! \brief Get Host Identifier
   //! If the path is absolute, return the host identifier. This can be
   //! a drive name ("C:"), a network name ("\\server") or, for future internet
   //! paths, a protocol and server ("http://www.fhnw.ch"). If the path
   //! is relative, an empty string is returned.
   //! \param strPath absolute path
   //! \param bDelimit if true, the returned path is delimited with '\' or '/'.
   //! \return host id string
   static std::string GetPathHostID(const std::string &strPath, bool bDelimit = true);

   //! \brief Make an absolute path relative.
   //! An absolute path is made relative to another.
   //! \param pPathDst Destination Path, e.g.  'c:\data\bla'
   //! \param pPathSrc SourcePatch, e.g. 'c:\data\bla\demo'
   //! \param bDelimit true if result should be delimited
   //! \return the relative path, e.g. 'demo'
   static std::string MakeRelativePath(const std::string& pPathDst, const std::string& pPathSrc, bool bDelimit=false);

   //! \brief Make path Absolute.
   //! A relative path is made absolute.
   //! \param pPathRoot Root path (e.g. 'c:/data')
   //! \param pPathRel Relative path (e.g. 'bla/test.tif')
   //! \param bDelimit true if result should be delimited
   //! \return the absolute path (e.g. 'c:/data/bla/test.tif')
   static std::string MakeAbsolutePath(const std::string& pPathRoot, const std::string& pPathRel, bool bDelimit = false);
};


#endif

