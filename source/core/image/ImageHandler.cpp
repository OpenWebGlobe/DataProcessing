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

#include "ImageHandler.h"
#include <cassert>
#include <iostream>
#include <fstream>

//----------------------------------------------------------------------------

ImageObject::ImageObject()
{
   _ePixelFormat = Img::PixelFormat_unknown; 
   _width = 0;
   _height = 0;
}

//----------------------------------------------------------------------------

ImageObject::~ImageObject()
{

}

//------------------------------------------------------------------------------

int ImageObject::_bpp()
{
   int bpp = 0;

   switch (_ePixelFormat)
   {
   case Img::PixelFormat_BGR:
   case Img::PixelFormat_RGB:
      bpp = 3;
      break;
   case Img::PixelFormat_BGRA:
   case Img::PixelFormat_RGBA:
      bpp = 4;
      break; 
   default:
      assert(false);
      return 0;
   }
   
   return bpp;
}  

//------------------------------------------------------------------------------

void ImageObject::AllocateImage(unsigned int w, unsigned int h, Img::PixelFormat ePixelFormat)
{
   unsigned int bpp;
   _width = w;
   _height = h;
   _ePixelFormat = ePixelFormat;
   
   bpp = _bpp();
   
   _qData = boost::shared_array<unsigned char>(new unsigned char[w*h*bpp]); 
}
//------------------------------------------------------------------------------
void ImageObject::_RGB_RGBA(unsigned char*  input)
{
   unsigned char* dst = _qData.get();
      
   for (size_t y=0;y<_height;y++)
   {
      for (size_t x=0;x<_width;x++)
      {
         size_t adr3=3*_width*y+3*x;
         size_t adrbpp=4*_width*y+4*x;

         dst[adrbpp+0] = input[adr3+0]; // R 
         dst[adrbpp+1] = input[adr3+1]; // G
         dst[adrbpp+2] = input[adr3+2]; // B
         dst[adrbpp+3] = 0; // A 
      }
   }
}
//------------------------------------------------------------------------------
void ImageObject::_RGB_RGB(unsigned char*  input)
{
   unsigned char* src = input;
   unsigned char* dst = _qData.get();
   memcpy(dst, src, _width*_height*3);
}
//------------------------------------------------------------------------------
void ImageObject::_RGBA_RGBA(unsigned char*  input)
{
   unsigned char* src = input;
   unsigned char* dst = _qData.get();
   memcpy(dst, src, _width*_height*4);
}
//------------------------------------------------------------------------------
void ImageObject::_RGBA_RGB(unsigned char*  input)
{
      unsigned char* dst = _qData.get();
      
      for (size_t y=0;y<_height;y++)
      {
         for (size_t x=0;x<_width;x++)
         {
            size_t adr4=4*_width*y+4*x;
            size_t adrbpp=3*_width*y+3*x;

            for (int i=0;i<3;i++)
            {
               dst[adrbpp+i] = input[adr4+i];
            }
         }
      }
}
//------------------------------------------------------------------------------
void ImageObject::_RGBA_BGRA(unsigned char*  input)
{
   unsigned char* dst = _qData.get();
      
   for (size_t y=0;y<_height;y++)
   {
      for (size_t x=0;x<_width;x++)
      {
         size_t adr4=4*_width*y+4*x;
         size_t adrbpp=4*_width*y+4*x;

         dst[adrbpp+2] = input[adr4+0]; // R -> B
         dst[adrbpp+1] = input[adr4+1]; // G -> G
         dst[adrbpp+0] = input[adr4+2]; // B -> R
         dst[adrbpp+3] = input[adr4+3]; // A -> A
      }
   }
}
//------------------------------------------------------------------------------
void ImageObject::_RGBA_BGR(unsigned char*  input)
{
   unsigned char* dst = _qData.get();
      
   for (size_t y=0;y<_height;y++)
   {
      for (size_t x=0;x<_width;x++)
      {
         size_t adr4=4*_width*y+4*x;
         size_t adrbpp=3*_width*y+3*x;

         dst[adrbpp+2] = input[adr4+0]; // R -> B
         dst[adrbpp+1] = input[adr4+1]; // G -> G
         dst[adrbpp+0] = input[adr4+2]; // B -> R
      }
   }
}
//------------------------------------------------------------------------------
void ImageObject::_RGB_BGR(unsigned char*  input)
{
   unsigned char* dst = _qData.get();
      
   for (size_t y=0;y<_height;y++)
   {
      for (size_t x=0;x<_width;x++)
      {
         size_t adr3=3*_width*y+3*x;
         size_t adrbpp=3*_width*y+3*x;

         dst[adrbpp+2] = input[adr3+0]; // R -> B
         dst[adrbpp+1] = input[adr3+1]; // G -> G
         dst[adrbpp+0] = input[adr3+2]; // B -> R
      }
   }
}
//------------------------------------------------------------------------------
void ImageObject::_RGB_BGRA(unsigned char*  input)
{
   unsigned char* dst = _qData.get();
      
   for (size_t y=0;y<_height;y++)
   {
      for (size_t x=0;x<_width;x++)
      {
         size_t adr3=3*_width*y+3*x;
         size_t adrbpp=4*_width*y+4*x;

         dst[adrbpp+2] = input[adr3+0]; // R -> B
         dst[adrbpp+1] = input[adr3+1]; // G -> G
         dst[adrbpp+0] = input[adr3+2]; // B -> R
         dst[adrbpp+3] = 0;
      }
   }
}
//------------------------------------------------------------------------------

void ImageObject::FillFromRGBA(unsigned char*  input)
{
   if (_qData)
   {
      switch(_ePixelFormat)
      {
         case Img::PixelFormat_RGB:
            _RGBA_RGB(input);
            return;
         break;
         case Img::PixelFormat_RGBA:
            _RGBA_RGBA(input);
         break;
         case Img::PixelFormat_BGR:
            _RGBA_BGR(input);
         break;
         case Img::PixelFormat_BGRA:
            _RGBA_BGRA(input);
         break;
         default:
            assert(false);
      }
   }
   else
   {
      assert(false); // no image data allocated. Call AllocateImage(...) first! 
   }
}

//------------------------------------------------------------------------------

void ImageObject::FillFromRGB(unsigned char* input)
{
    if (_qData)
   {
      switch(_ePixelFormat)
      {
         case Img::PixelFormat_RGB:
            _RGB_RGB(input);
            return;
         break;
         case Img::PixelFormat_RGBA:
            _RGB_RGBA(input);
         break;
         case Img::PixelFormat_BGR:
            _RGB_BGR(input);
         break;
         case Img::PixelFormat_BGRA:
            _RGB_BGRA(input);
         break;
         default:
            assert(false);
      }

   }
   else
   {
      assert(false); // no image data allocated. Call AllocateImage(...) first! 
   }
}

//------------------------------------------------------------------------------

void ImageObject::SavePPM(std::string sFilename)
{
   if (!_qData)
      return;
      
    
   unsigned char* pData;
   
   if (_bpp() == 3)
   {
      pData = _qData.get();   

      std::ofstream ofp;
      ofp.open(sFilename.c_str(), std::ios::out | std::ios::binary);

      if (!ofp) 
      {
         std::cout << "Can't open file: " << sFilename.c_str() << std::endl;
         return;
      }

      ofp << "P6" << std::endl;
      ofp << _width << " " << _height << std::endl;
      ofp << 255 << std::endl;

      ofp.write( reinterpret_cast<char *>(pData), (3*_width*_height)*sizeof(unsigned char));

      if (ofp.fail()) 
      {
         std::cout << "Can't write image " << sFilename.c_str() << std::endl;
         return;
      }

      ofp.close();   
   }
   else if (_bpp() == 4)
   {
      pData = _qData.get();   

      std::ofstream ofp;
      ofp.open(sFilename.c_str(), std::ios::out | std::ios::binary);

      if (!ofp) 
      {
         std::cout << "Can't open file: " << sFilename.c_str() << std::endl;
         return;
      }

      ofp << "P6" << std::endl;
      ofp << _width << " " << _height << std::endl;
      ofp << 255 << std::endl;
      
      for (size_t y = 0;y<_height;y++)
      {
         for (size_t x = 0; x<_width;x++)
         {
            size_t adr4 = 4*_width*y+4*x;
            unsigned char r = pData[adr4];
            unsigned char g = pData[adr4+1];
            unsigned char b = pData[adr4+2];
            
            ofp.write((char*)&r, 1);
            ofp.write((char*)&g, 1);
            ofp.write((char*)&b, 1);
         }
      }
      
      if (ofp.fail()) 
      {
         std::cout << "Can't write image " << sFilename.c_str() << std::endl;
         return;
      }

      ofp.close();   
   } 
}

 //---------------------------------------------------------------------------

 int ImageObject::GetNumChannels()
 {
   switch (_ePixelFormat)
   {
      case Img::PixelFormat_RGB:     
      case Img::PixelFormat_BGR: 
         return 3;    
      case Img::PixelFormat_RGBA:    
      case Img::PixelFormat_BGRA:
         return 4;
   }

   return 0;
 }


 void ImageObject::Convert(Img::PixelFormat newPixelFormat, ImageObject& out)
 {
      out.AllocateImage(GetWidth(), GetHeight(), newPixelFormat);

      if (_ePixelFormat == Img::PixelFormat_RGB)
      {
         out.FillFromRGB(this->GetRawData().get());
      }
      else if (_ePixelFormat == Img::PixelFormat_BGR)
      {
        // #todo
        //out.FillFromBGR(this->GetRawData().get()); 
      }
      else if (_ePixelFormat == Img::PixelFormat_RGBA)
      {
          out.FillFromRGBA(this->GetRawData().get());
      }
      else if (_ePixelFormat == Img::PixelFormat_BGRA)
      {
         // #todo
         //out.FillFromBGRA(this->GetRawData().get()); 
      }
 }