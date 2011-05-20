#include "math/mathutils.h"

//------------------------------------------------------------------------------

inline void ImageObject::ReadPixel4(int x, int y, unsigned char& r, unsigned char& g, unsigned char& b, unsigned char& a)
{
   size_t adr;

   // Validate Range
#ifdef _DEBUG
   if (x>(int)_width-1) x = (int)_width-1;
   if (y>(int)_height-1) y = (int)_height-1;
   if (x<0) x = 0;
   if (y<0) y = 0;
#endif

   adr = 4*y*_width+4*x;
   r = _qData.get()[adr];
   g = _qData.get()[adr+1];
   b = _qData.get()[adr+2];
   a = _qData.get()[adr+3];
}

//---------------------------------------------------------------------------

inline void ImageObject::ReadPixelBilinear4(double x, double y, unsigned char& r, unsigned char& g, unsigned char& b, unsigned char& a)
{
   double uf = math::Fract<double>(x);
   double vf = math::Fract<double>(y);

   int nPixelX = int(x);
   int nPixelY = int(y);

   int u00,v00,u10,v10,u01,v01,u11,v11;
   u00 = nPixelX; v00 = nPixelY;
   u10 = nPixelX+1; v10 = nPixelY;
   u01 = nPixelX; v01 = nPixelY+1;
   u11 = nPixelX+1; v11 = nPixelY+1;

   if (u00<0) u00 = 0;
   if (v00<0) v00 = 0;
   if (u10<0) u10 = 0;
   if (v10<0) v10 = 0;
   if (u01<0) u01 = 0;
   if (v01<0) v01 = 0;
   if (u11<0) u11 = 0;
   if (v11<0) v11 = 0;

   if (u00>(int)_width-1) u00 = (int)_width-1;
   if (v00>(int)_height-1) v00 = (int)_height-1;
   if (u10>(int)_width-1) u10 = (int)_width-1;
   if (v10>(int)_height-1) v10 = (int)_height-1;
   if (u01>(int)_width-1) u01 = (int)_width-1;
   if (v01>(int)_height-1) v01 = (int)_height-1;
   if (u11>(int)_width-1) u11 = (int)_width-1;
   if (v11>(int)_height-1) v11 = (int)_height-1;

   unsigned char r00,g00,b00,a00;
   unsigned char r10,g10,b10,a10;
   unsigned char r01,g01,b01,a01;
   unsigned char r11,g11,b11,a11;

   // 
   ReadPixel4(u00, v00, r00, g00, b00, a00);
   ReadPixel4(u10, v10, r10, g10, b10, a10);
   ReadPixel4(u01, v01, r01, g01, b01, a01);
   ReadPixel4(u11, v11, r11, g11, b11, a11);
   //
   double rd, gd, bd, ad;
   rd = (double(r00)*(1-uf)*(1-vf)+double(r10)*uf*(1-vf)+double(r01)*(1-uf)*vf+double(r11)*uf*vf)+0.5;
   gd = (double(g00)*(1-uf)*(1-vf)+double(g10)*uf*(1-vf)+double(g01)*(1-uf)*vf+double(g11)*uf*vf)+0.5;
   bd = (double(b00)*(1-uf)*(1-vf)+double(b10)*uf*(1-vf)+double(b01)*(1-uf)*vf+double(b11)*uf*vf)+0.5;
   ad = (double(a00)*(1-uf)*(1-vf)+double(a10)*uf*(1-vf)+double(a01)*(1-uf)*vf+double(a11)*uf*vf)+0.5;

   rd = math::Clamp<double>(rd, 0.0, 255.0);
   gd = math::Clamp<double>(gd, 0.0, 255.0);
   bd = math::Clamp<double>(bd, 0.0, 255.0);
   ad = math::Clamp<double>(ad, 0.0, 255.0);

   r = (unsigned char) rd;
   g = (unsigned char) gd;
   b = (unsigned char) bd;
   a = (unsigned char) ad;
}

//------------------------------------------------------------------------------


