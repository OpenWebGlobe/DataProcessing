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

#ifndef A_VEC2_H
#define A_VEC2_H

#include <cmath>
#include <iostream>
#include <cassert>

//! \class vec2
//! \brief Rational (float, double) vector class with 2 components.
template <typename T>
class vec2
{
public:
   typedef T TYPE;

   vec2() {x=0; y=0; }
   vec2(T xx, T yy) {Set(xx,yy);}
   vec2(const vec2<T>& other) : x(other.x), y(other.y) {}

   void Set(T xx, T yy) {x=xx; y=yy;}

   // Access Coordinates:
   inline operator const T* () const;
   inline operator T* ();
   inline T operator[] (int i) const;
   inline T& operator[] (int i);
   inline T X () const;
   inline T& X ();
   inline T Y () const;
   inline T& Y ();

   // assignment
   inline vec2& operator= (const vec2& oVec);

   // comparison
   bool operator== (const vec2& oVec) const;
   bool operator!= (const vec2& oVec) const;
   bool operator<  (const vec2& oVec) const;
   bool operator<= (const vec2& oVec) const;
   bool operator>  (const vec2& oVec) const;
   bool operator>= (const vec2& oVec) const;

   // arithmetic operations
   inline vec2 operator+ (const vec2& oVec) const;
   inline vec2 operator- (const vec2& oVec) const;
   inline vec2 operator* (T fScalar) const;
   inline vec2 operator/ (T fScalar) const;
   inline vec2 operator- () const;

   // arithmetic updates
   inline vec2& operator+= (const vec2& oVec);
   inline vec2& operator-= (const vec2& oVec);
   inline vec2& operator*= (T fScalar);
   inline vec2& operator/= (T fScalar);

   // vector operations
   inline T Length () const;
   inline T SquaredLength () const;
   inline T Dot (const vec2& oVec) const;
   inline T Normalize();

   union
   {
      struct { T x, y; };
      struct { T xy[2]; };
   };

private:
   inline int CompareArrays (const vec2& oVec) const;

};

//--------------------------------------------------------------------------
template <class T>
inline vec2<T>::operator const T* () const
{
   return xy;
}
//--------------------------------------------------------------------------
template <class T>
inline vec2<T>::operator T* ()
{
   return xy;
}
//--------------------------------------------------------------------------
template <class T>
inline T vec2<T>::operator[] (int i) const
{
   return xy[i];
}
//--------------------------------------------------------------------------
template <class T>
inline T& vec2<T>::operator[] (int i)
{
   return xy[i];
}
//--------------------------------------------------------------------------
template <class T>
inline T vec2<T>::X () const
{
   return xy[0];
}
//--------------------------------------------------------------------------
template <class T>
inline T& vec2<T>::X ()
{
   return xy[0];
}
//--------------------------------------------------------------------------
template <class T>
inline T vec2<T>::Y () const
{
   return xy[1];
}
//--------------------------------------------------------------------------
template <class T>
inline T& vec2<T>::Y ()
{
   return xy[1];
}
//--------------------------------------------------------------------------
template <class T>
inline vec2<T>& vec2<T>::operator= (const vec2& oVec)
{
   xy[0] = oVec.xy[0];
   xy[1] = oVec.xy[1];
   return *this;
}
//--------------------------------------------------------------------------
template <class T>
bool vec2<T>::operator== (const vec2& oVec) const
{
   return CompareArrays(oVec) == 0;
}
//--------------------------------------------------------------------------
template <class T>
bool vec2<T>::operator!= (const vec2& oVec) const
{
   return CompareArrays(oVec) != 0;
}
//--------------------------------------------------------------------------
template <class T>
bool vec2<T>::operator< (const vec2& oVec) const
{
   return CompareArrays(oVec) < 0;
}
//--------------------------------------------------------------------------
template <class T>
bool vec2<T>::operator<= (const vec2& oVec) const
{
   return CompareArrays(oVec) <= 0;
}
//--------------------------------------------------------------------------
template <class T>
bool vec2<T>::operator> (const vec2& oVec) const
{
   return CompareArrays(oVec) > 0;
}
//--------------------------------------------------------------------------
template <class T>
bool vec2<T>::operator>= (const vec2& oVec) const
{
   return CompareArrays(oVec) >= 0;
}
//--------------------------------------------------------------------------
template <class T>
inline vec2<T> vec2<T>::operator+ (const vec2& oVec) const
{
   return vec2(
      xy[0]+oVec.xy[0],
      xy[1]+oVec.xy[1]);
}
//--------------------------------------------------------------------------
template <class T>
inline vec2<T> vec2<T>::operator- (const vec2& oVec) const
{
   return vec2(
      xy[0]-oVec.xy[0],
      xy[1]-oVec.xy[1]);
}
//--------------------------------------------------------------------------
template <class T>
inline vec2<T> vec2<T>::operator* (T fScalar) const
{
   return vec2(
      fScalar*xy[0],
      fScalar*xy[1]);
}
//--------------------------------------------------------------------------
template <class T>
inline vec2<T> vec2<T>::operator/ (T fScalar) const
{
   vec2 kQuot;

   if (fScalar != (T)0.0)
   {
      T fInvScalar = ((T)1.0)/fScalar;
      kQuot.xy[0] = fInvScalar*xy[0];
      kQuot.xy[1] = fInvScalar*xy[1];
   }
   else
   {
      kQuot.xy[0] = Math<T>::MAX_REAL;
      kQuot.xy[1] = Math<T>::MAX_REAL;
   }

   return kQuot;
}
//--------------------------------------------------------------------------
template <class T>
inline vec2<T> vec2<T>::operator- () const
{
   return vec2(
      -xy[0],
      -xy[1]);
}
//--------------------------------------------------------------------------
template <class T>
inline vec2<T> operator* (T fScalar, const vec2<T>& oVec)
{
   return vec2<T>(
      fScalar*oVec[0],
      fScalar*oVec[1]);
}
//--------------------------------------------------------------------------
template <class T>
inline vec2<T>& vec2<T>::operator+= (const vec2& oVec)
{
   xy[0] += oVec.xy[0];
   xy[1] += oVec.xy[1];
   return *this;
}
//--------------------------------------------------------------------------
template <class T>
inline vec2<T>& vec2<T>::operator-= (const vec2& oVec)
{
   xy[0] -= oVec.xy[0];
   xy[1] -= oVec.xy[1];
   return *this;
}
//--------------------------------------------------------------------------
template <class T>
inline vec2<T>& vec2<T>::operator*= (T fScalar)
{
   xy[0] *= fScalar;
   xy[1] *= fScalar;
   return *this;
}
//--------------------------------------------------------------------------
template <class T>
inline vec2<T>& vec2<T>::operator/= (T fScalar)
{
   if (fScalar != (T)0.0)
   {
      T fInvScalar = ((T)1.0)/fScalar;
      xy[0] *= fInvScalar;
      xy[1] *= fInvScalar;
   }
   else
   {
      xy[0] = Math<T>::MAX_REAL;
      xy[1] = Math<T>::MAX_REAL;
   }

   return *this;
}
//--------------------------------------------------------------------------

template <class T>
inline T vec2<T>::Length () const
{
   return Math<T>::Sqrt(xy[0]*xy[0] + xy[1]*xy[1]);
}
//----------------------------------------------------------------------------
template <class T>
inline T vec2<T>::SquaredLength () const
{
   return xy[0]*xy[0] + xy[1]*xy[1];
}
//----------------------------------------------------------------------------
template <class T>
inline T vec2<T>::Dot (const vec2& oVec) const
{
   return xy[0]*oVec.xy[0] + xy[1]*oVec.xy[1];
}
//----------------------------------------------------------------------------
template <class T>
inline T vec2<T>::Normalize()
{
   T fLength = Length();

   if (fLength > Math<T>::ZERO_TOLERANCE)
   {
      T fInvLength = ((T)1.0)/fLength;
      xy[0] *= fInvLength;
      xy[1] *= fInvLength;
   }
   else
   {
      fLength = (T)0.0;
      xy[0] = (T)0.0;
      xy[1] = (T)0.0;
   }

   return fLength;
}
//-----------------------------------------------------------------------------
template <class T>
inline int vec2<T>::CompareArrays (const vec2& oOther) const
{
   return memcmp(xy,oOther.xy,2*sizeof(T));
}
//-----------------------------------------------------------------------------

#endif