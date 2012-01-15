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

#ifndef _FILESYSTEM_H_
#define _FILESYSTEM_H_

#include "og.h"
#include <cassert>
#include <vector>
#include <string>
#include <boost/shared_array.hpp>
#include <stdint.h>

//! File utilities like creating directories, removing files, renaming, retrieving modification time etc.
//! This is based on boost::filesystem (previous versions used wxWidgets and Qt)
//! \author Martin Christen, martin.christen@fhnw.ch
/*static*/ class OPENGLOBE_API FileSystem
{
public:
   //---------------------------------------------------------------------------
   /*!
   * \brief Returns time of last modification of given file.
   * \param sFile path to file
   * \return -1 if error occured (file doesn't exist), time_t otherwise.
   */
   static time_t modificationtime(const std::string& sFile);
   //---------------------------------------------------------------------------
   /*!
   * \brief Create a directory using ascii string
   * \param sPath path to be created
   */
   static bool makedir(const std::string& sPath);
   //---------------------------------------------------------------------------
   //! \brief creates all subdirs in a given filename if they don't exist yet
   static bool makeallsubdirs(const std::string& sFilename);
   //---------------------------------------------------------------------------
   /*!
   * \brief Remove (Delete) a file using ascii filename.
   * \param sFile path to file
   */
   static bool rm(const std::string& sFile);
   //---------------------------------------------------------------------------
   /*!
   * \brief Remove (Delete) an ascii directory with all contents. WARNING: use with caution!
   * \param sFile path to directory
   */
   static bool rm_all(const std::string& sDirectory);
   //---------------------------------------------------------------------------
   /*!
   * \brief Renames ascii sFile1 to ascii sFile2.
   * \param sFile1 File to be renamed
   * \param sFile2 New Name for the File
   */
   static bool rename(const std::string& sFile1, const std::string& sFile2);
   //---------------------------------------------------------------------------
   /*!
   * \brief Returns true if the ascii file exists and is a plain file.
   * \param sFile path to file
   * \return true or false
   */
   static bool FileExists(const std::string& sFile);
   //---------------------------------------------------------------------------
   /*!
   * \brief Returns true if the directory exists and really is a directory.
   * \param sDir path to directory
   * \return true or false
   */
   static bool DirExists(const std::string& sDir);
   //---------------------------------------------------------------------------
   // Get Disk Space (of disk being used at specified path)
   static void GetDiskSpace(const std::string& sPath, uintmax_t& capacity, uintmax_t& free, uintmax_t& available);
   //---------------------------------------------------------------------------
   /*!
   * \brief Lists all files in a directory and returns them in a vector. std::wstring
   */
   static std::vector<std::string> GetFilesInDirectory(const std::string& sDir, const std::string& sExtension = "");
   //---------------------------------------------------------------------------
   /*!
   * \brief Lists all filenames in a directory and returns them in a vector. std::string
   */
   static std::vector<std::string> GetFileNamesInDirectory(const std::string& sDir, const std::string& sExtension = "");
   //---------------------------------------------------------------------------
   /*!
   * \brief Lists all subdirectorynames in a directory and returns them in a vector. std::string
   */
   static std::vector<std::string> GetSubdirsInDirectory(const std::string& sDir);
   //---------------------------------------------------------------------------
   //! \brief Copy File to memory. Returns true if successful. vResult contains the data.
   static bool FileToMemory(const std::string& sPath, std::vector<unsigned char>& vResult);
   //---------------------------------------------------------------------------
   //! \brief Copy File to memory. Alloc memory.
   static bool FileToAllocedMemory(const std::string& sPath, unsigned char*& pResult, size_t& nSize);
   //---------------------------------------------------------------------------
   //! \brief free the memory that was previously alloced by "FileToAllocedMemory".
   static void FreeAllocedMemory(unsigned char* memory);
   //---------------------------------------------------------------------------
   //! \brief Copy File to memory. Alloc memory by using memory manager. (todo)
   //static unsigned char* FileToMemoryBlock(const std::string& sPath, MemoryManager& memmanager);
   //---------------------------------------------------------------------------
   //! \brief Read first line of a text file and return string. A line is limited to 1024 chars.
   static std::string ReadLine(const std::string& sPath);
   //---------------------------------------------------------------------------
   //! \brief Read all lines of a text file and return vector of string per line. A line is limited to 1024 chars.
   static std::vector<std::string> LinesToVector(const std::string& sPath);
   //---------------------------------------------------------------------------
   /*!
   * \brief Exclusively locks a file by creating an exclusive locking file. 
   * The file may not exist yet when this function is called! In this case it locks the "future" file.
   * If file is already locked, waits until the file can be accessed.
   * This function can be used on clusters.
   * \param file the filename of the file to be locked.
   * \return handle
   */
   static   int Lock(const std::string& file);
   //---------------------------------------------------------------------------
   /*!
   * \brief unlocks a previously locked file. Other computers / processes / threads can access the file again.
   * \param file the filename
   * \param handle the handle created by Lock
   */
   static   void Unlock(const std::string& file, int handle);
   //---------------------------------------------------------------------------
   //! \brief Retrieve current working directory
   static std::string GetCWD();
};


#endif

