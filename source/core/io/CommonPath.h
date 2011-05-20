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

#ifndef _IO_COMMONPATH_H_
#define _IO_COMMONPATH_H_

#include "og.h"
#include <cassert>
#include <string>

//-----------------------------------------------------------------------------
//! \class CommonPath
//! \brief Common directories
//! \author Martin Christen, martin.christen@fhnw.ch
//! \todo add more common paths (like user directory etc.)
//! \ingroup file
class CommonPath
{
public:
   //! \brief Retrieve full path to current working directory
   //! \return current working directory 
   static std::string GetCwd();

   //! \brief Set current working directory.
   static void SetCwd(std::string& sPath);

   //! \brief Return platform specific config directory.
   //! The config directory is platform specific and can be:
   //!   - Windows: "C:/Documents and Settings/All Users/Application Data"
   //!   - Linux: "/etc"
   //!   - MacOS: "/Library/Preferences"
   //! \return full path to config directory
   //static std::wstring GetConfigDir();

   //! \brief Convert a directory containing a wildcard
   //! Allowed wildcards are: {cwd} current working directory and
   //! {scene} which holds the directory of the current scene.
   //! \param sPath the directory/path which can have a wildcard
   //! \return the full valid path without wildcards.
   static std::string ConvertWildcard(std::string& sPath);

   //! \brief Test if path contains a wildcard
   //! \return true if there is a wildcard in path,.
   static bool HasWildcard(std::string& sPath);

   //! \brief Set scene root.
   static void SetSceneRoot(std::string& sSceneRoot){_sSceneRoot = sSceneRoot;}

   //! \brief Retrieve current scene root.
   static std::string GetSceneRoot();

   //! \brief Retrieve application directory (platform dependent location, under windows it is %appdata%/appname)
   static std::string GetApplicationDirectory(const std::string& appname);

private:
   static std::string _sSceneRoot;

};





#endif