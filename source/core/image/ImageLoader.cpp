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

#include "og.h"
#include "image/ImageLoader.h"
#include "lodepng/lodepng.h"
#include "string/StringUtils.h"
#include <fstream>
#include <cassert>

#define STBI_NO_HDR
#include "stb_image.c"

#define LODEPNG_COMPILE_ZLIB
#define LODEPNG_COMPILE_ENCODER
#include "lodepng/lodepng.cpp"

//------------------------------------------------------------------------------

bool ImageLoader::LoadFromDisk(Img::FileFormat eFormat, const std::string& sFilename, Img::PixelFormat eDestPixelFormat, ImageObject& outputimage)
{
   std::vector<unsigned char> vecData;
   std::ifstream ifs;
   
#ifdef OS_WINDOWS
   std::wstring sFilenameW = StringUtils::Utf8_To_wstring(sFilename);
   ifs.open(sFilenameW.c_str(), std::ios::in | std::ios::binary);
#else
   ifs.open(sFilename.c_str(), std::ios::in | std::ios::binary);
#endif
   if (ifs.good())
   {
      unsigned char s;
      while (!ifs.eof())
      {  
         ifs.read((char*)&s, 1);
         vecData.push_back(s);  
      }
   }
   else
   {
      return false;
   }
   
   return ImageLoader::LoadFromMemory(eFormat, &vecData[0], vecData.size(), eDestPixelFormat, outputimage);
}

//------------------------------------------------------------------------------

bool ImageLoader::LoadRaw32FromDisk(const std::string& sFilename, int w, int h,  Raw32ImageObject& outputdata)
{
   std::ifstream ifs;
   
#ifdef OS_WINDOWS
   std::wstring sFilenameW = StringUtils::Utf8_To_wstring(sFilename);
   ifs.open(sFilenameW.c_str(), std::ios::binary);
#else
   ifs.open(sFilename.c_str(), std::ios::binary);
#endif
   outputdata.AllocateImage(w,h);
      int offset = 0;
      if (ifs.good())
      {
         while (!ifs.eof())
         {
                     
            float value;
            ifs.read((char*)&(value), sizeof(float));
            if (!ifs.eof())
            {
               outputdata.SetValue(offset, value);
            }
            offset++;
         }
         ifs.close();
         return true;
      }
      else
      {
         ifs.close();
         return false;
      }
}

//------------------------------------------------------------------------------

bool ImageLoader::LoadFromMemory(Img::FileFormat eFormat, const unsigned char* pData, const unsigned int nSize, Img::PixelFormat eDestPixelFormat, ImageObject& outputimage)
{
   unsigned int w,h;

   switch(eFormat)
   {
      case Img::Format_PNG:
         {
            std::vector<unsigned char> out;
            if (LodePNG::decode(out, w, h, pData, nSize) == 0)
            {
               outputimage.AllocateImage(w,h, eDestPixelFormat);
               outputimage.FillFromRGBA(&(out[0]));
               return true;
            }
            else
            {     
               return false; // failed!
            }
         }
         break;
      case Img::Format_JPG:
      case Img::Format_GIF:
      case Img::Format_TGA:
      {
         int x,y,comp;
         unsigned char* pImage = stbi_load_from_memory((unsigned char*)pData, (int)nSize, &x, &y, &comp, 0);
         if (x>0 && y>0)
         {
            if (comp == 3)
            {
               outputimage.AllocateImage(x,y, eDestPixelFormat);
               outputimage.FillFromRGB(pImage);
               stbi_image_free(pImage);
               return true;
            }
            else if (comp == 4)
            {
               outputimage.AllocateImage(x,y, eDestPixelFormat);
               outputimage.FillFromRGBA(pImage);
               stbi_image_free(pImage);
               return true;
            }
         }
         break;
      }
      case Img::Format_RAW32:
      {
         int x,y;
         float comp;

         break;
      }
      default:
         assert(false);
   }

   return false;
}

//------------------------------------------------------------------------------
