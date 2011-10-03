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

#ifndef _MAT4_H
#define _MAT4_H

#include "vec3.h"

#define mat4f mat4<float>
#define mat4d mat4<double>

//namespace cwc
//{

//! \class mat4
//! \brief Matrix class for 4x4 matrices (with focus on computer graphics)
//! \author Martin Christen, martin.christen@fhnw.ch
//! \ingroup math linalg
template<typename T>
class mat4
{
public:      
   typedef T TYPE;

   //! \brief Default Constructor
   mat4<T>();

   //! \brief Constructor to fill matrix by value
   mat4<T>(T a, T b, T c, T d,
      T e, T f, T g, T h,
      T i, T j, T k, T l,
      T m, T n, T o, T p);

   //! \brief Constructor fill fill matrix by an array of data
   mat4<T>(const T* array);

   //! \brief Copy constructor
   mat4<T>(const mat4& m);

   // Copy and cast
   mat4<T>& operator=(const mat4& m);
   mat4<T>& operator=(const T* array);

   operator const T*() const;

   // Mutators/accessors

   //! \brief Set matrix by values
   void Set(T a, T b, T c, T d,
      T e, T f, T g, T h,
      T i, T j, T k, T l,
      T m, T n, T o, T p);

   //! \brief Set single value
   void SetVal(short row, short col, T val);

   //! \brief Retrieve single value
   T GetVal(short row, short col) const;

   //! \brief Set value using OpenGL style
   void SetVal(short pos, T val);

   //! \brief Retrieve value using OpenGL style
   T GetVal(short pos) const;

   //! \brief Set matrix to identity
   void SetIdentity();

   //! \brief Set matrix to zero-matrix
   void SetZero();

   //! \brief Test if matrix is identity.
   //! \return true if matrix is identity matrix, false otherwise.
   bool IsIdentity() const;

   //! \brief Test if matrix is zero matrix.
   //! \return true if matrix is zero matrix, false otherwise.
   bool IsZero() const;

   //! \brief Set matrix to rotation matrix.
   //! \param x x-value of rotation axis
   //! \param y y-value of rotation axis
   //! \param z z-value of rotation axis
   //! \param angle Angle in [RAD]
   void SetRotation(T x, T y, T z, T angle);

   //! \brief Set matrix to translation matrix
   //! \param x x-value of translation
   //! \param y y-value of translation
   //! \param z z-value of translation
   void SetTranslation(T x,T y,T z);

   //! \brief Set matrix to scale matrix
   //! \param x Value to scale x-axis
   //! \param y Value to scale y-axis
   //! \param z Value to scale z-axis
   void SetScale(T x,T y,T z);

   //! \brief Set matrix to scale matrix using uniform scaling.
   //! \param s Value to scale x-axis, y-axis and z-axis
   void SetScale(T s);

   void SetRotation(const vec3<T>& axis,T angle);
   void SetRotationX(T fTheta);
   void SetRotationY(T fTheta);
   void SetRotationZ(T fTheta);

   void SetTranslation(const vec3<T>& v);
   void SetScale(const vec3<T>& v);

   // Airplane Navigation (any Ellipsoid)
   void CalcNavigationFrame(double lng, double lat);
   void CalcBodyFrame(double yaw, double pitch, double roll);
   void CalcLocalHorizon(double lng, double lat);

   // Transforms axes to the given vectors.
   void SetAxisTransformation(const vec3<T>& x,const vec3<T>& y,const vec3<T>& z);

   // Retrieve Translation      
   void ExtractTranslation(T &x, T &y, T &z);
   void ExtractTranslation(vec3<T>& vOutTranslation);

   vec3<T> ExtractViewDirection();

   void RemoveTranslation();

   void SetViewTransform(const vec3<T>& vPosition, const vec3<T>& vLookAt, const vec3<T>& vUp);

   // Operators
   bool operator==(const mat4<T>& op2) const;
   bool operator!=(const mat4<T>& op2) const;

   void mult(const mat4<T>& op2);


   const mat4<T> operator*(T op2) const;
   const mat4<T> operator/(T op2) const;
   mat4<T>& operator*=(T op2);
   mat4<T>& operator/=(T op2);

   const mat4<T> operator-() const;

   void           SetInverse(const mat4<T>& original);
   const mat4<T>  Inverse() const;
   void           SetTranspose(const mat4<T>& original);
   const mat4<T>  Transpose() const;
   void           SetCofactors(const mat4<T>& original);
   const mat4<T>  Cofactors() const;
   T              Determinant() const;

   void       GetGLMatrix(T* mat) const;
   const T*   GetGLMatrix() const;
   void       SetGLMatrix(const T* mat);

   // Apply matrix to vector
   const vec3<T> operator*(const vec3<T>& v);
   const vec3<T> vec3mul(const vec3<T>& v) const;

   // Matrix add/subtract/multiply to self operators
   const mat4<T> operator+(const mat4<T>& op2) const;
   const mat4<T> operator-(const mat4<T>& op2) const;
   const mat4<T> operator*(const mat4<T>& op2) const;
   mat4<T>& operator+=(const mat4<T>& op2);
   mat4<T>& operator-=(const mat4<T>& op2);
   mat4<T>& operator*=(const mat4<T>& op2);

   void  print();

   //protected:
   T _vals[4][4];

private:
   void ScaleRow(short row,T scale);
   void SwapRows(short row1,short row2);
   void AddToRow(short rowToAdd, T scale, short rowToChange);
   T    CofactorElem(short i, short j) const;
};

//--------------------------------------------------------------------------

template<typename T>
mat4<T>::mat4()
{
   SetIdentity();
}

template<typename T>
mat4<T>::mat4(  T a, T b, T c, T d,
              T e, T f, T g, T h,
              T i, T j, T k, T l,
              T m, T n, T o, T p)
{
   _vals[0][0] = a; _vals[1][0] = b; _vals[2][0] = c; _vals[3][0] = d;
   _vals[0][1] = e; _vals[1][1] = f; _vals[2][1] = g; _vals[3][1] = h;
   _vals[0][2] = i; _vals[1][2] = j; _vals[2][2] = k; _vals[3][2] = l;
   _vals[0][3] = m; _vals[1][3] = n; _vals[2][3] = o; _vals[3][3] = p;
}


template <typename T>
mat4<T>::mat4(const T* array)
{
   T* ptr = (T*)_vals;
   for (short i=0; i<16; ++i)
   {
      ptr[i] = array[i];
   }
}


template <typename T>
mat4<T>::mat4(const mat4<T>& m)
{
   T* ptr = (T*)_vals;
   const T* ptr2 = (T*)(m._vals);
   for (short i=0; i<16; ++i)
   {
      ptr[i] = ptr2[i];
   }
}

//-----------------------------------------------------------------------------
// Matrix operator =
//-----------------------------------------------------------------------------

template <typename T>
mat4<T>& mat4<T>::operator=(const mat4<T>& m)
{
   T* ptr = (T*)_vals;
   const T* ptr2 = (T*)(m._vals);
   for (short i=0; i<16; ++i)
   {
      ptr[i] = ptr2[i];
   }
   return *this;
}


template <typename T>
mat4<T>& mat4<T>::operator=(const T* array)
{
   T* ptr = (T*)_vals;
   for (short i=0; i<16; ++i)
   {
      ptr[i] = array[i];
   }
   return *this;
}


//-----------------------------------------------------------------------------
// Matrix cast
//-----------------------------------------------------------------------------

template <typename T>
mat4<T>::operator const T*() const
{
   return (const T*)_vals;
}


//-----------------------------------------------------------------------------
// Set matrix
//-----------------------------------------------------------------------------

template <typename T>
void mat4<T>::Set( T a, T b, T c, T d,
                  T e, T f, T g, T h,
                  T i, T j, T k, T l,
                  T m, T n, T o, T p)
{
   _vals[0][0] = a;
   _vals[1][0] = b;
   _vals[2][0] = c;
   _vals[3][0] = d;
   _vals[0][1] = e;
   _vals[1][1] = f;
   _vals[2][1] = g;
   _vals[3][1] = h;
   _vals[0][2] = i;
   _vals[1][2] = j;
   _vals[2][2] = k;
   _vals[3][2] = l;
   _vals[0][3] = m;
   _vals[1][3] = n;
   _vals[2][3] = o;
   _vals[3][3] = p;
}


//-----------------------------------------------------------------------------
// Indexed access
//-----------------------------------------------------------------------------

template <typename T>
void mat4<T>::SetVal(short row,short col,T val)
{
   _vals[col][row] = val;
}


template <typename T>
T mat4<T>::GetVal(short row,short col) const
{
   return _vals[col][row];
}

template <typename T>
void mat4<T>::SetVal(short pos, T val)
{
   ((double*)_vals)[pos] = val;
}

template <typename T>
T mat4<T>::GetVal(short pos) const
{
   return ((double*)_vals)[pos];
}


//-----------------------------------------------------------------------------
// Identity and zero matrices
//-----------------------------------------------------------------------------

template <typename T>
void mat4<T>::SetIdentity()
{
   for (short j=0; j<4; j++)
      for (short i=0; i<4; i++)
      {
         _vals[i][j] = (i==j) ? 1.0f : 0.0f;
      }
}


template <typename T>
void mat4<T>::SetZero()
{
   for (short j=0; j<4; j++)
      for (short i=0; i<4; i++)
      {
         _vals[i][j] = 0;
      }
}


template <typename T>
bool mat4<T>::IsIdentity() const
{
   for (short j=0; j<4; j++)
      for (short i=0; i<4; i++)
      {
         if ((i != j && _vals[i][j] != 0) ||
            (i == j && _vals[i][j] != T(1)))
            return false;
      }
      return true;
}


template <typename T>
bool mat4<T>::IsZero() const
{
   for (short j=0; j<4; j++)
      for (short i=0; i<4; i++)
      {
         if (_vals[i][j] != 0)
            return false;
      }
      return true;
}


//-----------------------------------------------------------------------------
// Extract an OpenGL matrix, please note that this only makes sense
// with types supported by OpenGL.
//-----------------------------------------------------------------------------

template <typename T>
void mat4<T>::GetGLMatrix(T* mat) const
{
   const T* ptr = (T*)_vals;
   for (short i=0; i<16; ++i)
   {
      mat[i] = ptr[i];
   }
}


template <typename T>
const T* mat4<T>::GetGLMatrix() const
{
   return (const T*)_vals;
}


template <typename T>
void mat4<T>::SetGLMatrix(const T* mat)
{
   T* ptr = (T*)_vals;
   for (short i=0; i<16; ++i)
   {
      ptr[i] = mat[i];
   }
}


//-----------------------------------------------------------------------------
// Create transforms
//-----------------------------------------------------------------------------

template <typename T>
void mat4<T>::SetScale(T s)
{
   SetScale(s, s, s);
}


template <typename T>
void mat4<T>::SetRotation(const vec3<T>& axis, T angle)
{
   SetRotation(axis.x, axis.y, axis.z, angle);
}


template <typename T>
void mat4<T>::SetTranslation(const vec3<T> &v)
{
   SetTranslation(v.x, v.y, v.z);
}

template <typename T>
void mat4<T>::SetScale(const vec3<T> &v)
{
   SetScale(v.x, v.y, v.z);
}

//-----------------------------------------------------------------------------
// Extract translation components
//-----------------------------------------------------------------------------

template <typename T>
void mat4<T>::ExtractTranslation(vec3<T>& vOutTranslation)
{
   vOutTranslation.x = _vals[3][0]; 
   vOutTranslation.y = _vals[3][1]; 
   vOutTranslation.z = _vals[3][2];
}

template <typename T>
void mat4<T>::ExtractTranslation(T &x, T &y, T &z)
{
   x = _vals[3][0]; 
   y = _vals[3][1]; 
   z = _vals[3][2];
}

template <typename T>
vec3<T> mat4<T>::ExtractViewDirection()
{
   vec3<T> vResult(-_vals[2][0],-_vals[2][1],-_vals[2][2]);
   return vResult;

   // (_vals[2][0],  _vals[2][1], _vals[2][2])   Left-handed column oriented: get z row.
   // (_vals[0][2],  _vals[1][2], _vals[2][2])   Left-handed row oriented: get z column.
   // (-_vals[2][0],-_vals[2][1], -_vals[2][2])  OpenGL style matrix (right-handed, column oriented), get the -z row.
   // (-_vals[0][2],-_vals[1][2], -_vals[2][2])  Right-handed row oriented: get -z column.
}


template <typename T>
void mat4<T>::CalcNavigationFrame(double lng, double lat)
{
   Set(
      -sin(lat)*cos(lng),  -sin(lng),  -cos(lat)*cos(lng), 0,
      -sin(lat)*sin(lng),  cos(lng),   -cos(lat)*sin(lng), 0,
      cos(lat),            0,          -sin(lat),          0,
      0,                   0,          0,                 1);
}

template <typename T>
void mat4<T>::CalcLocalHorizon(double lng, double lat)
{
   Set(
      -sin(lat)*cos(lng),  -sin(lng),  cos(lat)*cos(lng), 0,
      -sin(lat)*sin(lng),  cos(lng),   cos(lat)*sin(lng), 0,
      cos(lat),            0,          sin(lat),          0,
      0,                   0,          0,                 1);
}


template <typename T>
void mat4<T>::CalcBodyFrame(double yaw, double pitch, double roll)
{
   double cosPitch = cos(pitch);
   double cosRoll = cos(roll);
   double cosYaw = cos(yaw);
   double sinPitch = sin(pitch);
   double sinRoll = sin(roll);
   double sinYaw = sin(yaw);

   Set(
      cosPitch*cosYaw, -cosRoll*sinYaw+sinRoll*sinPitch*cosYaw, sinRoll*sinYaw+cosRoll*sinPitch*cosYaw, 0,
      cosPitch*sinYaw, cosRoll*cosYaw+sinRoll*sinPitch*sinYaw, -sinRoll*cosYaw+cosRoll*sinPitch*sinYaw, 0,
      -sinPitch, sinRoll*cosPitch, cosRoll*cosPitch, 0,
      0,0,0,1);
}


//-----------------------------------------------------------------------------
// Remove translation component
//-----------------------------------------------------------------------------

template <typename T>
void mat4<T>::RemoveTranslation()
{
   _vals[3][0] = 0;
   _vals[3][1] = 0;
   _vals[3][2] = 0;
}

//-----------------------------------------------------------------------------
// Create View Matrix
//-----------------------------------------------------------------------------

template <typename T>
void mat4<T>::SetViewTransform(const vec3<T>& vEye, const vec3<T>& vCenter, const vec3<T>& vUp)
{
   vec3<T> f = vCenter - vEye;
   vec3<T> vUpNorm = vUp;
   f.Normalize();

   vUpNorm.Normalize();

   vec3<T> s = cross<T>(f, vUpNorm);
   vec3<T> u = cross<T>(s, f);

   Set(s.x,  s.y,  s.z,  0,
      u.x,  u.y,  u.z,  0,
      -f.x, -f.y, -f.z,  0,
      0,    0,    0,    1);

   mat4<T> Tmp;
   Tmp.SetTranslation(-vEye.x, -vEye.y, -vEye.z);
   mult(Tmp);

}

//-----------------------------------------------------------------------------
// Gauss-Jordan helper: Scale a row
//-----------------------------------------------------------------------------

template <typename T>
void mat4<T>::ScaleRow(short row, T scale)
{
   for (short i=0; i<4; i++)
   {
      _vals[i][row] *= scale;
   }
}


//-----------------------------------------------------------------------------
// Gauss-Jordan helper: Swap two rows
//-----------------------------------------------------------------------------

template <typename T>
void mat4<T>::SwapRows(short row1,short row2)
{
   for (short i=0; i<4; i++)
   {
      T temp = _vals[i][row1];
      _vals[i][row1] = _vals[i][row2];
      _vals[i][row2] = temp;
   }
}


//-----------------------------------------------------------------------------
// Gauss-Jordan helper: Add a row to another
//-----------------------------------------------------------------------------

template <typename T>
void mat4<T>::AddToRow(short rowToAdd,T scale,short rowToChange)
{
   for (short i=0; i<4; i++)
   {
      _vals[i][rowToChange] += _vals[i][rowToAdd]*scale;
   }
}


//-----------------------------------------------------------------------------
// Find cofactor element at given location
//-----------------------------------------------------------------------------

template <typename T>
T mat4<T>::CofactorElem(short a, short b) const
{
   short p[3], q[3];

   for (short i=0; i<3; i++)
   {
      p[i] = (i<a) ? i : i+1;
      q[i] = (i<b) ? i : i+1;
   }

   T ret = _vals[p[0]][q[0]] * _vals[p[1]][q[1]] * _vals[p[2]][q[2]] +
      _vals[p[1]][q[0]] * _vals[p[2]][q[1]] * _vals[p[0]][q[2]] +
      _vals[p[2]][q[0]] * _vals[p[0]][q[1]] * _vals[p[1]][q[2]] -
      _vals[p[2]][q[0]] * _vals[p[1]][q[1]] * _vals[p[0]][q[2]] -
      _vals[p[1]][q[0]] * _vals[p[0]][q[1]] * _vals[p[2]][q[2]] -
      _vals[p[0]][q[0]] * _vals[p[2]][q[1]] * _vals[p[1]][q[2]];

   return ((((a+b)%2) != 0) ? -ret : ret);
}


//-----------------------------------------------------------------------------
// Comparisons
//-----------------------------------------------------------------------------

template <typename T>
bool mat4<T>::operator==(const mat4<T>& op2) const
{
   for (short j=0; j<4; j++)
      for (short i=0; i<4; i++)
      {
         if (_vals[i][j] != op2._vals[i][j])
            return false;
      }

      return true;
}


template <typename T>
bool mat4<T>::operator!=(const mat4<T>& op2) const
{
   for (short j=0; j<4; j++)
      for (short i=0; i<4; i++)
      {
         if (_vals[i][j] != op2._vals[i][j])
            return true;
      }

      return false;
}


//-----------------------------------------------------------------------------
// Self-multiplication and division by a scalar
//-----------------------------------------------------------------------------

template <typename T>
mat4<T>& mat4<T>::operator*=(T op2)
{
   for (short j=0; j<4; j++)
      for (short i=0; i<4; i++)
      {
         _vals[i][j] *= op2;
      }

      return *this;
}

template <typename T>
mat4<T>& mat4<T>::operator/=(T op2)
{
   for (short j=0; j<4; j++)
      for (short i=0; i<4; i++)
      {
         _vals[i][j] /= op2;
      }

      return *this;
}


//-----------------------------------------------------------------------------
// Matrix self-add and self-subtract
//-----------------------------------------------------------------------------

template <typename T>
mat4<T>& mat4<T>::operator+=(const mat4<T>& op2)
{
   for (short j=0; j<4; j++)
      for (short i=0; i<4; i++)
      {
         _vals[j][i] += op2._vals[j][i];
      }

      return *this;
}


template <typename T>
mat4<T>& mat4<T>::operator-=(const mat4<T>& op2)
{
   for (short j=0; j<4; j++)
      for (short i=0; i<4; i++)
      {
         _vals[j][i] -= op2._vals[j][i];
      }

      return *this;
}

//-----------------------------------------------------------------------------
template <typename T>
void mat4<T>::print()
{
   for (short j=0; j<4; j++)
   {
      std::cout << " | ";
      for (short i=0; i<4; i++)
      {
         std::cout << _vals[j][i] << "\t | ";
      }
      std::cout << "\n";
   }
}

//-----------------------------------------------------------------------------

template <typename T>
const mat4<T> mat4<T>::operator*(T op2) const
{
   mat4<T> result;

   for (short j=0; j<4; j++)
      for (short i=0; i<4; i++)
      {
         result._vals[i][j] = _vals[i][j] * op2;
      }

      return result;
}

//-----------------------------------------------------------------------------

template <typename T>
void mat4<T>::mult(const mat4<T>& op2)
{
   mat4<T> result;

   for (short j=0; j<4; j++)
      for (short i=0; i<4; i++)
      {
         result._vals[i][j] =
            _vals[0][j]*op2._vals[i][0] +
            _vals[1][j]*op2._vals[i][1] +
            _vals[2][j]*op2._vals[i][2] +
            _vals[3][j]*op2._vals[i][3];
      }

      for (short j=0; j<4; j++)
         for (short i=0; i<4; i++)
            _vals[i][j] = result._vals[i][j];
}

//-----------------------------------------------------------------------------

template <typename T>
const mat4<T> operator*(T op1,const mat4<T>& op2)
{
   mat4<T> result;

   for (short j=0; j<4; j++)
      for (short i=0; i<4; i++)
      {
         result.SetVal(i, j, op1 * op2.GetVal(i, j));
      }

      return result;
}

//-----------------------------------------------------------------------------

template <typename T>
const mat4<T> mat4<T>::operator/(T op2) const
{
   mat4<T> result;

   for (short j=0; j<4; j++)
      for (short i=0; i<4; i++)
      {
         result._vals[i][j] = _vals[i][j] / op2;
      }

      return result;
}

//-----------------------------------------------------------------------------

template <typename T>
const mat4<T> mat4<T>::operator-() const
{
   mat4<T> result;

   for (short j=0; j<4; j++)
      for (short i=0; i<4; i++)
      {
         result._vals[i][j] = -_vals[i][j];
      }

      return result;
}

//-----------------------------------------------------------------------------
// Matrix inverse by Gauss-Jordan elimination
//-----------------------------------------------------------------------------

template <typename T>
void mat4<T>::SetInverse(const mat4<T>& orig)
{
   mat4<T> temp = orig;
   int i, j;
   T fac;
   T pivot, pivot2;
   int pivrow;

   SetIdentity();
   for (j=0; j<4; j++)
   {
      // Find largest pivot
      pivrow = j;
      pivot = fabs(temp._vals[j][j]);
      for (i=j+1; i<4; i++)
      {
         pivot2 = fabs(temp._vals[j][i]);
         if (pivot2 > pivot)
         {
            pivot = pivot2;
            pivrow = i;
         }
      }

      // Check for singular matrix
      if (pivot == T(0))
      {
         SetZero();
         return;
      }

      // Get pivot into jth row
      if (pivrow != j)
      {
         temp.SwapRows(pivrow, j);
         SwapRows(pivrow, j);
      }

      // Turn the pivot into 1
      fac = T(1)/temp._vals[j][j];
      temp.ScaleRow(j, fac);
      ScaleRow(j, fac);

      // Zero out this element in other rows
      for (i=0; i<4; i++)
      {
         if (j != i && temp._vals[j][i] != T(0))
         {
            fac = -(temp._vals[j][i]);
            temp.AddToRow(j, fac, i);
            AddToRow(j, fac, i);
         }
      }
   }
}

//-----------------------------------------------------------------------------

template <typename T>
const mat4<T> mat4<T>::Inverse() const
{
   mat4<T> res;
   res.SetInverse(*this);
   return res;
}


//-----------------------------------------------------------------------------
// Matrix cofactors
//-----------------------------------------------------------------------------

template <typename T>
void mat4<T>::SetCofactors(const mat4<T>& orig)
{
   for (int j=0; j<4; j++)
      for (int i=0; i<4; i++)
      {
         _vals[i][j] = orig.CofactorElem(i, j);
      }
}

//-----------------------------------------------------------------------------

template <typename T>
const mat4<T> mat4<T>::Cofactors() const
{
   mat4<T> result;

   for (int j=0; j<4; j++)
      for (int i=0; i<4; i++)
      {
         result._vals[i][j] = CofactorElem(i, j);
      }

      return result;
}

//-----------------------------------------------------------------------------
// Matrix determinant
//-----------------------------------------------------------------------------

template <typename T>
T mat4<T>::Determinant() const
{
   T result = 0;

   for (int i=0; i<4; i++)
   {
      result += CofactorElem(i, 0) * GetVal(0, i);
   }

   return result;
}


//-----------------------------------------------------------------------------
// Matrix transpose
//-----------------------------------------------------------------------------
template <typename T>
void mat4<T>::SetTranspose(const mat4<T>& orig)
{
   for (int j=0; j<4; j++)
      for (int i=0; i<4; i++)
      {
         _vals[i][j] = orig._vals[j][i];
      }
}

//-----------------------------------------------------------------------------
template <typename T>
const mat4<T> mat4<T>::Transpose() const
{
   mat4<T> result;

   for (int j=0; j<4; j++)
      for (int i=0; i<4; i++)
      {
         result._vals[i][j] = _vals[j][i];
      }

      return result;
}


//-----------------------------------------------------------------------------
// Set a rotation matrix given axis as three values, and angle
//-----------------------------------------------------------------------------
template <typename T>
void mat4<T>::SetRotation(T x,T y,T z,T angle)
{
   _vals[0][0] = x*x+(T(1)-x*x)*cos(angle);
   _vals[1][0] = x*y*(T(1)-cos(angle))-z*sin(angle);
   _vals[2][0] = z*x*(T(1)-cos(angle))+y*sin(angle);
   _vals[3][0] = T(0);

   _vals[0][1] = x*y*(T(1)-cos(angle))+z*sin(angle);
   _vals[1][1] = y*y+(T(1)-y*y)*cos(angle);
   _vals[2][1] = y*z*(T(1)-cos(angle))-x*sin(angle);
   _vals[3][1] = T(0);

   _vals[0][2] = z*x*(T(1)-cos(angle))-y*sin(angle);
   _vals[1][2] = y*z*(T(1)-cos(angle))+x*sin(angle);
   _vals[2][2] = z*z+(T(1)-z*z)*cos(angle);
   _vals[3][2] = T(0);

   _vals[0][3] = T(0);
   _vals[1][3] = T(0);
   _vals[2][3] = T(0);
   _vals[3][3] = T(1);
}

template <typename T>
void  mat4<T>::SetRotationX(T fTheta)
{
   T fSin = T(sin(fTheta));
   T fCos = T(cos(fTheta));
   _vals[0][0] = 1; _vals[0][1] = 0;		_vals[0][2] = 0;		_vals[0][3] = 0;
   _vals[1][0] = 0; _vals[1][1] = fCos;	_vals[1][2] = -fSin;  _vals[1][3] = 0;
   _vals[2][0] = 0; _vals[2][1] = fSin;	_vals[2][2] = fCos;	_vals[2][3] = 0;
   _vals[3][0] = 0; _vals[3][1] = 0;		_vals[3][2] = 0;		_vals[3][3] = 1;
}

template <typename T>
void  mat4<T>::SetRotationY(T fTheta)
{
   T fSin = T(sin(fTheta));
   T fCos = T(cos(fTheta));
   _vals[0][0] = fCos;		_vals[0][1] = 0;   _vals[0][2] = fSin;	_vals[0][3] = 0;
   _vals[1][0] = 0;			_vals[1][1] = 1;   _vals[1][2] = 0;		_vals[1][3] = 0;
   _vals[2][0] = -fSin;	   _vals[2][1] = 0;   _vals[2][2] = fCos;	_vals[2][3] = 0;
   _vals[3][0] = 0;			_vals[3][1] = 0;   _vals[3][2] = 0;		_vals[3][3] = 1;
}

template <typename T>
void  mat4<T>::SetRotationZ(T fTheta)
{
   T fSin = T(sin(fTheta));
   T fCos = T(cos(fTheta));
   _vals[0][0] = fCos;		_vals[0][1] = -fSin; _vals[0][2] = 0;     _vals[0][3] = 0;
   _vals[1][0] = fSin;	   _vals[1][1] = fCos; _vals[1][2] = 0;     _vals[1][3] = 0;
   _vals[2][0] = 0;			_vals[2][1] = 0;    _vals[2][2] = 1;     _vals[2][3] = 0;
   _vals[3][0] = 0;			_vals[3][1] = 0;    _vals[3][2] = 0;     _vals[3][3] = 1;
}

/*
template <typename T>
void  mat4<T>::SetRotationZ_LeftHanded(T fTheta)
{
T fSin = T(sin(fTheta));
T fCos = T(cos(fTheta));
_vals[0,0] = fCos;		_vals[0,1] = fSin; _vals[0,2] = 0;     _vals[0,3] = 0;
_vals[1,0] = -fSin;	   _vals[1,1] = fCos; _vals[1,2] = 0;     _vals[1,3] = 0;
_vals[2,0] = 0;			_vals[2,1] = 0;    _vals[2,2] = 1;     _vals[2,3] = 0;
_vals[3,0] = 0;			_vals[3,1] = 0;    _vals[3,2] = 0;     _vals[3,3] = 1;
}
*/



//-----------------------------------------------------------------------------
// Create a translation transform
//-----------------------------------------------------------------------------
template <typename T>
void mat4<T>::SetTranslation(T x,T y,T z)
{
   _vals[0][0] = T(1);
   _vals[1][0] = T(0);
   _vals[2][0] = T(0);
   _vals[3][0] = x;
   _vals[0][1] = T(0);
   _vals[1][1] = T(1);
   _vals[2][1] = T(0);
   _vals[3][1] = y;
   _vals[0][2] = T(0);
   _vals[1][2] = T(0);
   _vals[2][2] = T(1);
   _vals[3][2] = z;
   _vals[0][3] = T(0);
   _vals[1][3] = T(0);
   _vals[2][3] = T(0);
   _vals[3][3] = T(1);
}


//-----------------------------------------------------------------------------
// Create a scaling transform
//-----------------------------------------------------------------------------
template <typename T>
void mat4<T>::SetScale(T x, T y, T z)
{
   _vals[0][0] = x;
   _vals[1][0] = T(0);
   _vals[2][0] = T(0);
   _vals[3][0] = T(0);
   _vals[0][1] = T(0);
   _vals[1][1] = y;
   _vals[2][1] = T(0);
   _vals[3][1] = T(0);
   _vals[0][2] = T(0);
   _vals[1][2] = T(0);
   _vals[2][2] = z;
   _vals[3][2] = T(0);
   _vals[0][3] = T(0);
   _vals[1][3] = T(0);
   _vals[2][3] = T(0);
   _vals[3][3] = T(1);
}


//-----------------------------------------------------------------------------
// Set axis transformation matrix
//-----------------------------------------------------------------------------
template <typename T>
void mat4<T>::SetAxisTransformation(const vec3<T>& x,const vec3<T>& y,const vec3<T>& z)
{
   _vals[0][0] = x.x;
   _vals[1][0] = y.x;
   _vals[2][0] = z.x;
   _vals[3][0] = 0.0f;
   _vals[0][1] = x.y;
   _vals[1][1] = y.y;
   _vals[2][1] = z.y;
   _vals[3][1] = 0.0f;
   _vals[0][2] = x.z;
   _vals[1][2] = y.z;
   _vals[2][2] = z.z;
   _vals[3][2] = 0.0f;
   _vals[0][3] = 0.0f;
   _vals[1][3] = 0.0f;
   _vals[2][3] = 0.0f;
   _vals[3][3] = 1.0f;
}


//-----------------------------------------------------------------------------
// Apply matrix transform to a vector
//-----------------------------------------------------------------------------
template <typename T>
const vec3<T> mat4<T>::operator*(const vec3<T>& v)
{
   return vec3<T>(
      v.x*T(_vals[0][0]) + v.y*T(_vals[1][0]) +
      v.z*T(_vals[2][0]) + T(_vals[3][0]),
      v.x*T(_vals[0][1]) + v.y*T(_vals[1][1]) +
      v.z*T(_vals[2][1]) + T(_vals[3][1]),
      v.x*T(_vals[0][2]) + v.y*T(_vals[1][2]) +
      v.z*T(_vals[2][2]) + T(_vals[3][2])) /
      T(v.x*T(_vals[0][3]) + v.y*T(_vals[1][3]) +
      v.z*T(_vals[2][3]) + T(_vals[3][3]));
}

template <typename T>
const vec3<T> mat4<T>::vec3mul(const vec3<T>& v) const
{
   return vec3<T>(
      v.x*T(_vals[0][0]) + v.y*T(_vals[1][0]) +
      v.z*T(_vals[2][0]) + T(_vals[3][0]),
      v.x*T(_vals[0][1]) + v.y*T(_vals[1][1]) +
      v.z*T(_vals[2][1]) + T(_vals[3][1]),
      v.x*T(_vals[0][2]) + v.y*T(_vals[1][2]) +
      v.z*T(_vals[2][2]) + T(_vals[3][2])) /
      T(v.x*T(_vals[0][3]) + v.y*T(_vals[1][3]) +
      v.z*T(_vals[2][3]) + T(_vals[3][3]));
}


//-----------------------------------------------------------------------------
// Matrix binary operations
//-----------------------------------------------------------------------------
template <typename T>
mat4<T>& mat4<T>::operator*=(const mat4<T>& op2)
{
   mat4<T> result;

   for (short j=0; j<4; j++)
      for (short i=0; i<4; i++)
      {
         result._vals[i][j] =
            _vals[0][j]*op2._vals[i][0] +
            _vals[1][j]*op2._vals[i][1] +
            _vals[2][j]*op2._vals[i][2] +
            _vals[3][j]*op2._vals[i][3];
      }

      *this = result;

      return *this;
}

//-----------------------------------------------------------------------------
template <typename T>
const mat4<T> mat4<T>::operator+(const mat4<T>& op2) const
{
   mat4<T> result;

   for (short j=0; j<4; j++)
      for (short i=0; i<4; i++)
      {
         result._vals[j][i] = _vals[j][i] + op2._vals[j][i];
      }

      return result;
}


//-----------------------------------------------------------------------------
// Matrix subtract
//-----------------------------------------------------------------------------
template <typename T>
const mat4<T> mat4<T>::operator-(const mat4<T>& op2) const
{
   mat4<T> result;

   for (short j=0; j<4; j++)
      for (short i=0; i<4; i++)
      {
         result._vals[j][i] = _vals[j][i] - op2._vals[j][i];
      }

      return result;
}


//-----------------------------------------------------------------------------
// Matrix multiply
//-----------------------------------------------------------------------------
template <typename T>
const mat4<T> mat4<T>::operator*(const mat4<T>& op2) const
{
   mat4<T> result;

   for (short j=0; j<4; j++)
      for (short i=0; i<4; i++)
      {
         result._vals[i][j] =
            _vals[0][j]*op2._vals[i][0] +
            _vals[1][j]*op2._vals[i][1] +
            _vals[2][j]*op2._vals[i][2] +
            _vals[3][j]*op2._vals[i][3];
      }

      return result;
}

//-----------------------------------------------------------------------------

//} // namespace

#endif  // include-guard

