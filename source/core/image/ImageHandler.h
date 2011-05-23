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

#ifndef _IMAGEHANDLER_H
#define _IMAGEHANDLER_H

//----------------------------------------------------------------------------
#include "og.h"
#include "boost/shared_array.hpp"
#include <string>
#include <vector>

namespace Img
{
   enum PixelFormat
   {
      PixelFormat_RGB,     // RGB Data, 8 bits per channel
      PixelFormat_BGR,     // BGR Data, 8 bits per channel
      PixelFormat_RGBA,    // RGBA Data, 8 bits per channel
      PixelFormat_BGRA,    // BGRA Data, 8 bits per channel
      
      PixelFormat_unknown, // Unspecified Pixel Format -> error!
   };
}

//----------------------------------------------------------------------------
class OPENGLOBE_API ImageObject
{
public:
   ImageObject();
   virtual ~ImageObject();
   
   //! \brief Allocate Image Data
   void AllocateImage(unsigned int w, unsigned int h, Img::PixelFormat ePixelFormat);   
   
   //! \brief Retrieve Pixel Format
   Img::PixelFormat GetPixelFormat() { return _ePixelFormat;}

   //! \brief Get number of Channels (3 or 4)
   int GetNumChannels();
   
   unsigned int   GetWidth(){return _width;}
   unsigned int   GetHeight(){return _height;}
   
   // Create image using source RGBA buffer
   void FillFromRGBA(unsigned char*  input);

   // Create image from source RGB buffer
   void FillFromRGB(unsigned char* input);
   
   // Retrieve Raw Data (RGB, RGBA, BGR, BGRA according to PixelFormat)
   boost::shared_array<unsigned char> GetRawData() { return _qData;}
   
   // Export image to PPM. This is mainly for testing purposes.
   void SavePPM(std::string sFilename);

   inline void ReadPixel4(int x, int y, unsigned char& r, unsigned char& g, unsigned char& b, unsigned char& a);
   inline void ReadPixelBilinear4(double x, double y, unsigned char& r, unsigned char& g, unsigned char& b, unsigned char& a);
  
protected:
   int _bpp();

   void _RGB_RGBA(unsigned char*  input);
   void _RGB_RGB(unsigned char*  input);
   void _RGBA_RGBA(unsigned char*  input);
   void _RGBA_RGB(unsigned char*  input);

   void _RGBA_BGRA(unsigned char*  input);
   void _RGBA_BGR(unsigned char*  input);
   void _RGB_BGR(unsigned char*  input); 
   void _RGB_BGRA(unsigned char*  input);

   
   boost::shared_array<unsigned char> _qData; 
   Img::PixelFormat _ePixelFormat; 
   unsigned int _width;
   unsigned int _height; 
};


#include "image/ImageHandler.inl"

//------------------------------------------------------------------------------

#endif
