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

#ifndef _IMAGELOADER_H
#define _IMAGELOADER_H

#include "ImageHandler.h"

/*
   Example Code:
   -------------
   
   unsigned char* pData = { compressed image }
   unsigned int nSize = { size of compressed image }
   
   ImageLoader loader;
   ImageObject data;
   
   loader.ImageFromMemory(Img::Format_PNG, pData, nSize, PixelFormat_RGB, data);
   
   // ... now data contains RGB data of image...
   

*/


//----------------------------------------------------------------------
namespace Img
{

   enum FileFormat
   {
      Format_PNG,    // PNG File Format
      Format_JPG,    // JPG File Format (baseline only)
      Format_TGA,    // TGA File Format
      Format_GIF,    // GIF File Format
   };
}

//------------------------------------------------------------------------------

/*static*/ class OPENGLOBE_API ImageLoader
{
public:
   ImageLoader() {}
   virtual ~ImageLoader() {}
   // synchrousous loading from disk
   static bool LoadFromDisk(Img::FileFormat eFormat, const std::string& sFilename, Img::PixelFormat eDestPixelFormat, ImageObject& outputimage);
   
   // decompress from memory
   static bool LoadFromMemory(Img::FileFormat eFormat, const unsigned char* pData, const unsigned int nSize, Img::PixelFormat eDestPixelFormat, ImageObject& outputimage);   
   
};

//------------------------------------------------------------------------------

#endif