/*******************************************************************************
Project       : i3D
Purpose       : Math Utils.
Creation Date : 01.03.2007
Author        : Martin Christen
Copyright     : This source code file is copyright (c) FHNW 2007
*******************************************************************************/


#ifndef _MATHUTILS_H
#define _MATHUTILS_H

#include "og.h"
#include <cmath>
#include <float.h>
#include <vector>

namespace math
{
   //! \ingroup math shader
   template<typename T>
   inline T Rand(void)
   {
      return (T)rand()/(T)RAND_MAX;
   }

   //! \ingroup math shader
   template<typename T>
   inline T Rand01(void)
   {
      return (T)(::abs(rand()))/(T)RAND_MAX;
   }

   //--------------------------------------------------------------------------
   //! \ingroup math shader
   template<typename T>
   inline T Abs(T x)             //!< returns x if x>=0, otherwise returns -x
   {
      return (x < 0 ? -x : x);
   }

   template<typename T>
   inline bool IsZero( T a );

   template<>
   inline bool IsZero<double>(double a)
   {
      return ( math::Abs<double>(a) < DBL_EPSILON);
   }   

   template<>
   inline bool IsZero<float>(float a)
   {
      return ( math::Abs<float>(a) < FLT_EPSILON);
   }


   //--------------------------------------------------------------------------
   //! \ingroup math shader
   template<typename T>
   inline T Sign(T x)            //!< returns 1 if x > 0, returns 0 if x=0, returns -1 if x < 0
   {
      if (x>0) return 1;
      if (x<0) return -1;
      return 0;
   }
   //--------------------------------------------------------------------------
   //! \ingroup math shader
   template<typename T>
   inline T Floor(T x)           //!< value equal to nearest Integer that is less than or equal to x
   {
      //return (T)((int)x - ((x < 0 && x != (int)(x))));
      return (T)floor((T)x);
   }

   //--------------------------------------------------------------------------
   //! \ingroup math shader
   template<typename T>
   inline double Ceil(T x)            //!< value equal to nearest Integer that is greater than or equal to x
   {
      //return (T)((int)x + ((x > 0 && x != (int)(x))));
      return (T)ceil(x);
   }

   //--------------------------------------------------------------------------
   //! \ingroup math shader
   template<typename T>
   inline void Round(T &x, int nDecimal)     // round values   
   {
      if (nDecimal < 0)
         return;

      // move the decimal place nDecimal number of times
      x *= (T)std::pow((T)10, (T)nDecimal);

      T fDiff;
      if (x > 0)       // check for -ve or +ve
      {
         fDiff = x - std::floor(x);
         if (fDiff >= 0.5)
            x = std::ceil(x);      // e.g. 2.6 -> 3.0
         else
            x = std::floor(x);     // e.g. 2.2 -> 2.0
      }
      else
      {
         fDiff = std::fabs(x) - std::floor(std::fabs(x));
         if (fDiff >= 0.5)
            x = std::floor(x);    // e.g. -2.6 -> -3.0
         else
            x = std::ceil(x);     // e.g. -2.2 -> -2.0
      }

      // move back the decimal place to the original position
      x /= (T)std::pow((double)10, (T)nDecimal);
   }

   

   //--------------------------------------------------------------------------
   //! \ingroup math shader
   template<typename T>
   inline T PrevInt(T x)    //!< returns previous Integer, example: -3.1 -> -4 and 1.1 -> 1
   {
      T r = Floor<T>(x);
      return r;
   }
     
   //! \ingroup math shader
   template<typename T>
   inline T NextInt(T x)       //!< returns next Integer, example: -3.1 -> -3 and 1.1 -> 2
   {
      T r = Ceil<T>(x);
      return r;
   }
   
   //--------------------------------------------------------------------------
   //! \ingroup math shader
   template<typename T>
   inline T Fract(T x)           //!< x-floor(x) or mod(x,1);
   {
      return x-Floor<T>(x);
   }

   //--------------------------------------------------------------------------

   template <class T> void Swap(T &a, T &b)
   {
      T temp;
      temp = a;
      a = b;
      b = temp;
   }

   //--------------------------------------------------------------------------
   //! \ingroup math shader
   template<typename T>
   inline T Mod(T x, T y)    //!< x-y*floor(x/y)
   {
      return x-y*Floor<T>(x/y);   
   }  
  
   //--------------------------------------------------------------------------
   template <class T>
   inline T Clip(T n, T minValue, T maxValue)
   {
      return math::Min<T>(math::Max<T>(n, minValue), maxValue);
   }

   //--------------------------------------------------------------------------
   //! \ingroup math shader
   template<typename T>
   inline T Clamp(T x, T minval, T maxval)
   {
      return (x < minval ? minval : (x > maxval ? maxval : x));
   }
   //--------------------------------------------------------------------------
   //! \ingroup math shader
   template<typename T>
   inline T Mix(T x, T y, T a)  //!< linear blend: x*(1-a) + y*a 
   {
      return (1-a)*x + a*y;
   }

   //--------------------------------------------------------------------------
   //! \ingroup math shader
   template<typename T>
   inline T Step(T edge, T x)
   {
      return (T) (x >= edge);
   }

   //--------------------------------------------------------------------------
   //! \ingroup math shader
   template<typename T>
   inline T Lerp(T x, T a, T b)
   {
      return (a + x * (b - a));
   }
   
   //--------------------------------------------------------------------------
   
   //! \ingroup math shader
   template<typename T>
	inline T SCurve(T x)
   {
      return ( x * x * (3.0 - 2.0 * x));
   }

   /*!
    * \brief This function calculates bilinear interpolation (2x2)
    * \param tx input coordinate in x direction
    * \param ty input coordinate in y direction
    * \param P00 input value at (0,0)
    * \param P01 input value at (0,1)
    * \param P10 input value at (1,0)
    * \param P11 input value at (1,1)
    * \return returns bilinear interpolation
    * \ingroup math shader
    */
   template <typename T> 
   inline T Bilinear(
      T tx, T ty,
      T P00, T P01, T P10, T P11)
   {
      T u = Fract<T>(tx);
      T v = Fract<T>(ty);

      return (1-u)*(1-v)*P01 + u*(1-v)*P11 + (1-u)*v * P00 + u*v*P10;
   }
   
   /*!
    * \brief clamps a value [0,inf]
    * \param x The value to be zero clamped 
    * \return clamped value
    * \ingroup math shader
    */
   template<typename T>
   inline T ClampZero(T x)
   {
      return (x>0.0)?x:0.0;
   }
   /*!
    * \brief calculates binomial for bicubic interpolation 
    * \param x value
    * \return bicubic binomial
    * \ingroup math shader
    */
   template<typename T>
   inline T BCurve(T x)
   {
      T p0=ClampZero<T>(x+2);
      T p1=ClampZero<T>(x+1);
      T p2=ClampZero<T>(x);
      T p3=ClampZero<T>(x-1);
      return ((p0*p0*p0) - 4*(p1*p1*p1) + 6*(p2*p2*p2) - 4*(p3*p3*p3) ) / 6.0; 
   }

   /*!
    * \brief This function calculates bicubic interpolation (4x4)
    * \param tx input coordinate in x direction
    * \param ty input coordinate in y direction
    * \param P00 input value at (0,0) 
    * \param P01 input value at (0,1)
    * \param P02 input value at (0,2)
    * \param P03 input value at (0,3)
    * \param P10 input value at (1,0)
    * \param P11 input value at (1,1)
    * \param P12 input value at (1,2)
    * \param P13 input value at (1,3)
    * \param P20 input value at (2,0)
    * \param P21 input value at (2,1)
    * \param P22 input value at (2,2)
    * \param P23 input value at (2,3)
    * \param P30 input value at (3,0)
    * \param P31 input value at (3,1)
    * \param P32 input value at (3,2)
    * \param P33 input value at (3,3)
    * \return
    * \ingroup math shader
    */
   template <typename T>
   inline T BiCubic(
      T tx, T ty,
      T P00, T P01, T P02, T P03,
      T P10, T P11, T P12, T P13,
      T P20, T P21, T P22, T P23,
      T P30, T P31, T P32, T P33)
   {
      T u=Fract<T>(tx);
      T v=Fract<T>(ty);

      T Rdx[4],Rdy[4];
      for(int n=0;n<=3;n++)
      {
         Rdx[n]=BCurve<T>(n-1-u);
         Rdy[n]=BCurve<T>(n-1-v);
      }

      float s;

      s =P00*Rdx[0]*Rdy[0];
      s+=P01*Rdx[1]*Rdy[0];
      s+=P02*Rdx[2]*Rdy[0];
      s+=P03*Rdx[3]*Rdy[0];

      s+=P10*Rdx[0]*Rdy[1];
      s+=P11*Rdx[1]*Rdy[1];
      s+=P12*Rdx[2]*Rdy[1];
      s+=P13*Rdx[3]*Rdy[1];

      s+=P20*Rdx[0]*Rdy[2];
      s+=P21*Rdx[1]*Rdy[2];
      s+=P22*Rdx[2]*Rdy[2];
      s+=P23*Rdx[3]*Rdy[2];

      s+=P30*Rdx[0]*Rdy[3];
      s+=P31*Rdx[1]*Rdy[3];
      s+=P32*Rdx[2]*Rdy[3];
      s+=P33*Rdx[3]*Rdy[3];

      return s; 
   }

   //--------------------------------------------------------------------------

   //! \ingroup math shader
   template<typename T>
   inline T Smoothstep(T a, T b, T x) //!< 3*x^2 - 2*x^3 Cubic Approximation, crashes if a==b
   {
      if (x<a) return 0.0;
      if (x>=b) return 1.0;
      x = (x-a) / (b-a);
      return SCurve<T>(x);    
   }

   //--------------------------------------------------------------------------
  
   //! \ingroup math shader
   template<typename T>
   inline T Pulse(T a, T b, T x) //!< step(a,x) - step(b,x)
   {
      return (Step<T>(a,x) - Step<T>(b,x));
   }

   //--------------------------------------------------------------------------

   //! \ingroup math shader
   template<typename T>
   inline T Gammacorrect(T gamma, T x)
   {
      return (T)pow((T)x, (T)1.0f/gamma);
   }

   //--------------------------------------------------------------------------
   //! \ingroup math shader
   template<typename T>
   inline T Bias(T b, T x)
   {
      return (T)pow(x,(T)log(b)/(T)log(0.5));
   }

   //--------------------------------------------------------------------------
   //! \ingroup math shader
   template<typename T>
   inline T Gain(T a, T b)
   {
      T p = log(1.0 - b) / log(0.5);
     
      if (a < 0.001)
         return 0.0;
      else if (a > 0.999)
         return 1.0;
      if (a < 0.5)
         return (T)pow((T)2.0 * a, (T)p) / 2.0;
      else
         return 1.0 - (T)pow((float)2.0 * (1.0 - a), (T)p) / 2.0;
   }

   //--------------------------------------------------------------------------
   //! \ingroup math shader
   inline unsigned long Pow2(int k)
   {
      return	(unsigned long(1) << (k));
   }
   //--------------------------------------------------------------------------
   //! \ingroup math shader
   inline int Nextpow2(int n) //!< next power of 2: 2^A of n.
   {
      int p2=1; if (n==0) return 1; for (; p2<n; p2<<=1); return p2;
   }
   //--------------------------------------------------------------------------
   //! \ingroup math shader
   template<typename T>
   bool IsPower2(T i)
   {
      int	nExp;
      double mant = frexp((double)i, &nExp);
      return (mant == 0.5);
   }


} // namespace math


#endif