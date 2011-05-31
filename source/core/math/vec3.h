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

#ifndef A_VEC3_H
#define A_VEC3_H

#include "og.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
//#include "math/mathdef.h"

#define vec3f vec3<float>
#define vec3d vec3<double>

#ifndef MIN                                                                      
#define MIN(a,b) ((a)<(b)?(a):(b))  
#endif 
#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))                                               
#endif

#define AGEPI       3.1415926535897932384626433832795028841971693993751
#define AGEHALFPI   1.5707963267948966192313216916395
#define rad2deg(x)  ((180.0*x)/AGEPI)
#define deg2rad(x)  ((x*AGEPI)/180.0)
#define AGE_EPSILON  1E-24  
#define AGE_ROUNDING_ERROR 1E-24 
#define AGE_GRAD_PI  (180.0 / AGEPI);
#define AGE_GRAD_PI2 (AGEPI / 180.0)

//-----------------------------------------------------------------------------

template<typename T>
class Math {
   static const T ZERO_TOLERANCE;
};

//-----------------------------------------------------------------------------

enum Axis { XAXIS=0, YAXIS=1, ZAXIS=2 }; 

//-----------------------------------------------------------------------------


//! \ingroup math linalg
template <typename T>
inline bool equals(T a, T b)
{
   return (a + AGE_ROUNDING_ERROR > b) && (a - AGE_ROUNDING_ERROR < b);
}

template <typename T>
class vec3;

//-----------------------------------------------------------------------------
// Prototypes:
template <typename T> inline T       length(const vec3<T>& v);
template <typename T> inline vec3<T> normalize_ret(const vec3<T>& v);
template <typename T> inline vec3<T> cross(const vec3<T>& v1, const vec3<T>& v2);
template <typename T> inline T        dot(const vec3<T>& a, const vec3<T>& b);
  
template <typename T> inline vec3<T> reflect(const vec3<T>& a,  const vec3<T>& b);
template <typename T> inline vec3<T> abs(const vec3<T>& v);


//-----------------------------------------------------------------------------
//! \class vec3
//! \brief Vector class with 3 components.
//! \author Martin Christen, martin.christen@fhnw.ch
//! \ingroup math
template <typename T>
class vec3
{
public:
   typedef T TYPE;

   //! Constructor. Initializes x,y and z to 0.
   inline vec3() : x(0), y(0), z(0) {}
   //! Constructor.
   //! \param xx value for x-component
   //! \param yy value for y-component
   //! \param zz value for z-component
   inline vec3(T xx, T yy, T zz) : x(xx), y(yy), z(zz) {}

   //! Copy Constructor.
   //! \param other copy from this vec3
   inline vec3(const vec3<T>& other) : x(other.x), y(other.y), z(other.z) {}

   //! Set values from 3 components.
   //! \param xx value for x-component
   //! \param yy value for y-component
   //! \param zz value for z-component
   inline void Set(const T xx, const T yy, const T zz) {x=xx, y=yy, z=zz;}

   //! Set value from other vector.
   //! \param p vec3 to copy from
   inline void Set(const vec3<T>& p) { x=p.x; y=p.y; z=p.z;}


   //! coordinate access
   inline operator const T* () const;

   //! coordinate access
   inline operator T* ();
   
   //! coordinate access
   inline T operator[] (int i) const;

   //! coordinate access
   inline T& operator[] (int i);

   //! access first component
   inline T X () const;

   //! access first component
   inline T& X ();

   //! access second component
   inline T Y () const;

   //! access second component
   inline T& Y ();

   //! access third component
   inline T Z () const;

   //! access third component
   inline T& Z ();


   // comparison
   inline bool operator== (const vec3& oOther) const;
   inline bool operator!= (const vec3& oOther) const;
   inline bool operator<  (const vec3& oOther) const;
   inline bool operator<= (const vec3& oOther) const;
   inline bool operator>  (const vec3& oOther) const;
   inline bool operator>= (const vec3& oOther) const;


   //! Invert sign operator. Inverses sign of all 3 components. (x,y,z) -> (-x, -y, -z).
   inline vec3<T> operator-() const { return vec3<T>(-x, -y, -z);   }

   //! Assign operator.
   inline vec3<T>& operator=(const vec3<T>& other)	{ x = other.x; y = other.y; z = other.z; return *this; }

   //! Add vector operator.
   inline vec3<T> operator+(const vec3<T>& other) const { return vec3<T>(x + other.x, y + other.y, z + other.z);	}
   
   //! Add and assign operator.
   inline vec3<T>& operator+=(const vec3<T>& other)	{ x+=other.x; y+=other.y; z+=other.z; return *this; }

   //! Subtract vector operator.
   inline vec3<T> operator-(const vec3<T>& other) const { return vec3<T>(x - other.x, y - other.y, z - other.z);	}
   
   //! Subtract vector and assign operator.
   inline vec3<T>& operator-=(const vec3<T>& other)	{ x-=other.x; y-=other.y; z-=other.z; return *this; }

   //! Component-wise multiplication operator.
   inline vec3<T> operator*(const vec3<T>& other) const { return vec3<T>(x * other.x, y * other.y, z * other.z);	}
   
   //! Component-wise multiplication and assignment.
   inline vec3<T>& operator*=(const vec3<T>& other)	{ x*=other.x; y*=other.y; z*=other.z; return *this; }
   
   //! Mulitplication with scalar operator.
   inline vec3<T> operator*(const T v) const { return vec3<T>(x * v, y * v, z * v);	}
   
   //! Multiplication with scalar and assign operator.
   inline vec3<T>& operator*=(const T v) { x*=v; y*=v; z*=v; return *this; }

   //! Component wise division operator.
   inline vec3<T> operator/(const vec3<T>& other) const { return vec3<T>(x / other.x, y / other.y, z / other.z);	}
   
   //! Component wise division and assignement.
   inline vec3<T>& operator/=(const vec3<T>& other)	{ x/=other.x; y/=other.y; z/=other.z; return *this; }
   
   //! Scalar division operator.
   inline vec3<T> operator/(const T v) const { T i=(T)1.0/v; return vec3<T>(x * i, y * i, z * i);	}
   
   //! Scalar division and assign operator.
   inline vec3<T>& operator/=(const T v) { T i=(T)1.0/v; x*=i; y*=i; z*=i; return *this; }


   //! Returns the dot product with another vector.
   inline T Dot(const vec3<T>& other) const
   {
      return x*other.x + y*other.y + z*other.z;
   }

   //! Calculates the cross product with another vector
   //! \param p: vector to multiply with.
   //! \return Crossproduct of this vector with p.
   inline vec3<T> Cross(const vec3<T>& p) const;

   //! Calculates the cross product with another vector and normalizes the result.
   //! \param p: vector to multiply with.
   //! \return Crossproduct of this vector with p.
   inline vec3<T> UnitCross(const vec3<T>& p) const;

   //! Test if all components are near zero
   inline bool  IsZero(void) const
   {
      return (fabs(x)<AGE_EPSILON && fabs(y)<AGE_EPSILON && fabs(z)<AGE_EPSILON);
   }

   //! Normalizes the vector. Returns previous length.
   inline T Normalize();

   //! Returns squared length of the vector.
   inline T Length() const { return sqrt(x*x + y*y + z*z);}

   //! Returns squared length of the vector.
   inline T SquaredLength() const { return x*x + y*y + z*z; }

 
   //! Returns distance from an other point.
   inline T GetDistanceFrom(const vec3<T>& other) const
   {
      return vec3<T>(x - other.x, y - other.y, z - other.z).Length();
   }

   //! Returns squared distance from an other point. 
   inline T GetDistanceFromSQ(const vec3<T>& other) const
   {
      return vec3<T>(x - other.x, y - other.y, z - other.z).SquaredLength();
   }

   //! Sets the length of the vector to a new value
   inline void SetLength(T newlength)
   {
      Normalize();
      *this *= newlength;
   }

   //! Inverts the vector.
   inline void Invert()
   {
      x *= -1.0f;
      y *= -1.0f;
      z *= -1.0f;
   }
                                                           
   union
   {
      struct { T x, y, z; };
      struct { T r, g, b; };
      struct { T xyz[3]; };
   };

private:
   int CompareArrays (const vec3& oOther) const;

};

//----------------------------------------------------------------------------
template <class T>
inline vec3<T>::operator const T* () const
{
   return xyz;
}
//----------------------------------------------------------------------------
template <class T>
inline vec3<T>::operator T* ()
{
   return xyz;
}
//----------------------------------------------------------------------------
template <class T>
inline T vec3<T>::operator[] (int i) const
{
   return xyz[i];
}
//----------------------------------------------------------------------------
template <class T>
inline T& vec3<T>::operator[] (int i)
{
   return xyz[i];
}
//----------------------------------------------------------------------------
template <class T>
inline T vec3<T>::X () const
{
   return xyz[0];
}
//----------------------------------------------------------------------------
template <class T>
inline T& vec3<T>::X ()
{
   return xyz[0];
}
//----------------------------------------------------------------------------
template <class T>
inline T vec3<T>::Y () const
{
   return xyz[1];
}
//----------------------------------------------------------------------------
template <class T>
inline T& vec3<T>::Y ()
{
   return xyz[1];
}
//----------------------------------------------------------------------------
template <class T>
inline T vec3<T>::Z () const
{
   return xyz[2];
}
//----------------------------------------------------------------------------
template <class T>
inline T& vec3<T>::Z ()
{
   return xyz[2];
}
//----------------------------------------------------------------------------
template <class T>
int vec3<T>::CompareArrays (const vec3& oOther) const
{
   return memcmp(xyz,oOther.xyz,3*sizeof(T));
}
//----------------------------------------------------------------------------
template <class T>
inline bool vec3<T>::operator== (const vec3& oOther) const
{
   return CompareArrays(oOther) == 0;
}
//----------------------------------------------------------------------------
template <class T>
inline bool vec3<T>::operator!= (const vec3& oOther) const
{
   return CompareArrays(oOther) != 0;
}
//----------------------------------------------------------------------------
template <class T>
inline bool vec3<T>::operator< (const vec3& oOther) const
{
   return CompareArrays(oOther) < 0;
}
//----------------------------------------------------------------------------
template <class T>
inline bool vec3<T>::operator<= (const vec3& oOther) const
{
   return CompareArrays(oOther) <= 0;
}
//----------------------------------------------------------------------------
template <class T>
inline bool vec3<T>::operator> (const vec3& oOther) const
{
   return CompareArrays(oOther) > 0;
}
//----------------------------------------------------------------------------
template <class T>
inline bool vec3<T>::operator>= (const vec3& oOther) const
{
   return CompareArrays(oOther) >= 0;
}
//----------------------------------------------------------------------------

template <class T>
inline vec3<T> vec3<T>::Cross (const vec3<T>& oOther) const
{
   return vec3(
      xyz[1]*oOther.xyz[2] - xyz[2]*oOther.xyz[1],
      xyz[2]*oOther.xyz[0] - xyz[0]*oOther.xyz[2],
      xyz[0]*oOther.xyz[1] - xyz[1]*oOther.xyz[0]);
}
//----------------------------------------------------------------------------
template <class T>
inline vec3<T> vec3<T>::UnitCross (const vec3<T>& oOther) const
{
   vec3 kCross(
      xyz[1]*oOther.xyz[2] - xyz[2]*oOther.xyz[1],
      xyz[2]*oOther.xyz[0] - xyz[0]*oOther.xyz[2],
      xyz[0]*oOther.xyz[1] - xyz[1]*oOther.xyz[0]);
   kCross.Normalize();
   return kCross;
}

//----------------------------------------------------------------------------
template <class T>
inline T vec3<T>::Normalize ()
{
   T fLength = Length();

   if (fLength > Math<T>::ZERO_TOLERANCE)
   {
      T fInvLength = ((T)1.0)/fLength;
      xyz[0] *= fInvLength;
      xyz[1] *= fInvLength;
      xyz[2] *= fInvLength;
   }
   else
   {
      fLength = (T)0.0;
      xyz[0] = (T)0.0;
      xyz[1] = (T)0.0;
      xyz[2] = (T)0.0;
   }

   return fLength;
}
//----------------------------------------------------------------------------

// operator*
/*template <typename T>
vec3<T> operator* (const vec3<T>& v, const T s)
{
      return vec3<T>(
         v.x * s,
         v.y * s,
         v.z * s);
}

template <typename T> 
inline vec3<T> operator* (const T s, const vec3<T>& v)
{
   return vec3<T>(
      s * v.x,
      s * v.y,
      s * v.z);
}
*/

//-----------------------------------------------------------------------------
// Some useful macros to avoid stack / memory allocation
#define vec3_set(p, X, Y, Z) (p).x = (X),  (p).y = (Y),  (p).z = (Z)
#define vec3_add(a,b,result)  (result).x = (a).x + (b).x, (result).y = (a).y + (b).y, (result).z = (a).z + (b).z
#define vec3_sub(a,b,result)    (result).x = (a).x - (b).x, (result).y = (a).y - (b).y, (result).z = (a).z - (b).z
#define vec3_scale(a,s)  (a).x *= (s), (a).y *= (s), (a).z *= (s)
#define vec3_dot(a,b)    ((a).x * (b).x + (a).y * (b).y + (a).z * (b).z)
#define vec3_cross(a,b,result)  (result).x = (a).y * (b).z  -  (a).z * (b).y, (result).y = (a).z * (b).x  -  (a).x * (b).z, (result).z = (a).x * (b).y  -  (a).y * (b).x
#define vec3_changesign(p)  (p).x = (-(p).x),  (p).y = (-(p).y),  (p).z = (-(p).z)
#define vec3_sscale(s,a,r)  (r).x=(s)*(a).x,(r).y=(s)*(a).y,(r).z=(s)*(a).z
#define vec3_addscaled(v1,s,v2,r) (r).x = (v1).x + (s)*(v2).x, (r).y = (v1).y + (s)*(v2).y, (r).z = (v1).z + (s)*(v2).z
//-----------------------------------------------------------------------------
//Useful functions
//-----------------------------------------------------------------------------
// Returns normalized vector, without changing original. Only use if really necessary!
//! \ingroup math
template <typename T>
inline vec3<T> normalize_ret(const vec3<T>& v)
{ 
   vec3<T>  r;
   T L=sqrt(v.x*v.x + v.y*v.y + v.z*v.z); 
   if (L!=T(0))
   { 
      r.x = v.x/L; r.y = v.y/L; r.z = v.z/L; 
   } 
   else 
      return v;
   return r;
}  
//-----------------------------------------------------------------------------
//! \ingroup math
template <typename T>
inline vec3<T> cross(const vec3<T>& v1, const vec3<T>& v2) 
{  
   vec3<T> v;
   v.x = v1.y*v2.z - v1.z*v2.y; 
   v.y = v1.z*v2.x - v1.x*v2.z;
   v.z = v1.x*v2.y - v1.y*v2.x;
   return v;
}
//-----------------------------------------------------------------------------
//! \ingroup math
template <typename T>
inline T dot(const vec3<T>& a, const vec3<T>& b) 
{
   return vec3_dot(a,b);
}
//-----------------------------------------------------------------------------
// Reflect (GLSL; HLSL; Renderman)
//! \ingroup math
template <typename T>
inline vec3<T> reflect(const vec3<T>& N, const vec3<T>& I)
{
   return I + N*(-2.0f*vec3_dot(N,I));
}
//-----------------------------------------------------------------------------
// Faceforward (Renderman version)
//! \ingroup math
template <typename T>
inline vec3<T> faceforward(const vec3<T>& N, const vec3<T>& I)
{
   if (vec3_dot(N,I) < 0) return N;
   return -N;
}
//-----------------------------------------------------------------------------
// Refract Funktion (Renderman; HLSL)
//! \ingroup math
template <typename T>
inline vec3<T> refract(const vec3<T>& I, const vec3<T>& N, T eta)
{
   T dot = vec3_dot(I,N);
   T k = 1.0f - eta*eta*(1.0f-dot*dot);
   if (k<0) return I;
   return I*eta - N*(eta*dot+sqrt(k));
}
//-----------------------------------------------------------------------------
// Abstand zweier Punkte
//inline T distance(vec3& A, vec3& B)
//{
//  return length(B-A); 
//}
//-----------------------------------------------------------------------------
// Alle Komponenten werden positiv
//! \ingroup math
template <typename T>
inline vec3<T> abs(const vec3<T>& v)
{  vec3<T> r;
r.x = fabs(v.x); r.y = fabs(v.y); r.z = fabs(v.z);
return r;
}
//-----------------------------------------------------------------------------
template <typename T>
inline vec3<T> product(const vec3<T> &a, const vec3<T> &b)                           
{ 
   return vec3<T>( a.x * b.x, a.y * b.y, a.z * b.z ); 
}
//-----------------------------------------------------------------------------
template <class T>
inline vec3<T> operator-(const vec3<T> &v)                                         
{ 
   return vec3<T>(-v.x,-v.y,-v.z); 
};                                             
//-----------------------------------------------------------------------------
template <class T>
inline T length(const vec3<T> &v)                                            
{ 
   return sqrt(dot(v,v)); 
};                                                    
//-----------------------------------------------------------------------------
template <class T>
inline vec3<T> operator*(const T f, const vec3<T> &v)                          
{ 
   return vec3<T>(f*v.x, f*v.y, f*v.z); 
};                                        
//-----------------------------------------------------------------------------
template <class T>
inline vec3<T> operator*(const vec3<T> &v, const T f)                          
{ 
   return vec3<T>(f*v.x, f*v.y, f*v.z); 
};                                       
//-----------------------------------------------------------------------------
template <class T>
inline void operator*=(vec3<T> &v, const T f)                                
{ 
   v.x *= f; v.y *= f; v.z *= f; 
};                                             
//-----------------------------------------------------------------------------
template <class T>
inline void operator*=(vec3<T> &v, const vec3<T> &f)                               
{ 
   v.x *= f.x; v.y *= f.y; v.z *= f.z; 
};                                       
//-----------------------------------------------------------------------------
template <class T>
inline vec3<T> operator/(const vec3<T> &v, const T f)                          
{ 
   return (1/f)*v; 
};                                                           
//-----------------------------------------------------------------------------
template <class T>
inline void operator/=(vec3<T> &v, const T f)                                
{ 
   v *= (1/f); 
};                                                               
//-----------------------------------------------------------------------------
template <class T>
inline vec3<T> operator+(const vec3<T> &a, const vec3<T> &b)                         
{ 
   return vec3<T>(a.x+b.x, a.y+b.y, a.z+b.z); 
};                                  
//-----------------------------------------------------------------------------
template <class T>
inline vec3<T>& operator+=(vec3<T> &a, const vec3<T> &b)                             
{                                                                              
   a.x += b.x;                                                            
   a.y += b.y;                                                            
   a.z += b.z;                                                            
   return a;                                                              
}                                                                              
//-----------------------------------------------------------------------------
template <class T>
inline bool operator==(const vec3<T>& a, const vec3<T>& b)                         
{                                                                              
   return a.x==b.x && a.y==b.y && a.z==b.z;                               
}                                                                              
//-----------------------------------------------------------------------------
template <class T>
inline vec3<T> operator^(const vec3<T> &a, const vec3<T> &b)                         
{                                                                              
   return vec3<T>(a.y*b.z-a.z*b.y,                                          
      a.z*b.x-a.x*b.z,                              
      a.x*b.y-a.y*b.x);                             
}                                                                             
//-----------------------------------------------------------------------------
template <class T>
inline vec3<T> operator-(const vec3<T> &a, const vec3<T> &b)                         
{ 
   return vec3<T>(a.x-b.x, a.y-b.y, a.z-b.z); 
}                                  
//-----------------------------------------------------------------------------
template <class T>
inline void normalize(vec3<T> &v)                                                
{ 
   v *= (1.0f/length<T>(v)); 
}                                                    
//-----------------------------------------------------------------------------
template <class T>
inline vec3<T> GetMin(const vec3<T> a, const vec3<T> b)                                 
{
   return vec3<T>(MIN(a.x,b.x),MIN(a.y,b.y),MIN(a.z,b.z)); 
}                      
//-----------------------------------------------------------------------------
template <class T>
inline vec3<T> GetMax(const vec3<T> a, const vec3<T> b)                                 
{ 
   return vec3<T>(MAX(a.x,b.x),MAX(a.y,b.y),MAX(a.z,b.z)); 
}                      
//-----------------------------------------------------------------------------
template <class T>
inline T maxValue(const vec3<T> a)                                           
{                                                                              
   return a[a.maxDim()];                                                  
}                                                                              
//-----------------------------------------------------------------------------
/*template <class T>
inline vec3<T> abs(const vec3<T> &a)                                               
{                                                                              
   return vec3<T>(fabs(a.x),fabs(a.y),fabs(a.z));                           
}*/
//-----------------------------------------------------------------------------

#define abs_dbl            abs<double>
#define refract_dbl        refract<double>
#define faceforward_dbl    faceforward<double>
#define reflect_dbl        reflect<double>
#define dot_dbl            dot<double>
#define cross_dbl          cross<double>
#define normalize_dbl      normalize_ret<double>
#define length_dbl         length<double>

#define abs_flt            abs<float>
#define refract_flt        refract<float>
#define faceforward_flt    faceforward<float>
#define reflect_flt        reflect<float>
#define dot_flt            dot<float>
#define cross_flt          cross<float>
#define normalize_flt      normalize_ret<float>
#define length_flt         length<float>



//}

#endif
