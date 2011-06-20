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

#include "ImageWriter.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "image/JPEGHandler.h"

#include <fstream>

//------------------------------------------------------------------------------

ImageWriter::ImageWriter()
{
}

//------------------------------------------------------------------------------

ImageWriter::~ImageWriter()
{
}

//------------------------------------------------------------------------------


bool ImageWriter::WritePNG(const std::string& sFilename, unsigned char* buffer_rbga, int width, int height)
{
   return (stbi_write_png(sFilename.c_str(), width, height, 4, buffer_rbga, 4*width) == 0);
}

//------------------------------------------------------------------------------

bool ImageWriter::WritePNG(const std::string& sFilename, ImageObject& image)
{
   if (image.GetNumChannels() != 4)
   {
      return false;
   }

   return WritePNG(sFilename, image.GetRawData().get(), image.GetWidth(), image.GetHeight());
}

//------------------------------------------------------------------------------

bool ImageWriter::WriteJPG(const std::string& sFilename, ImageObject& image, int quality)
{
   unsigned char* pInput = 0;
   ImageObject rgbimage;

   if (image.GetPixelFormat() != Img::PixelFormat_RGB)
   {
      image.Convert(Img::PixelFormat_RGB, rgbimage);
      pInput = rgbimage.GetRawData().get();
   }
   else
   {
      pInput = image.GetRawData().get();
   }

   boost::shared_array<unsigned char> outjpg;
   int len;
   if (JPEGHandler::RGBToJpeg(pInput, image.GetWidth(), image.GetHeight(), quality, outjpg, len))
   {
      std::fstream off(sFilename.c_str(), std::ios::binary);
      if (off.good())
      {
         off.write((char*)outjpg.get(), (std::streamsize)len);
         off.close();
         return true;
      }
   }

   return false;

}
