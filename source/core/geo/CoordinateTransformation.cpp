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

#include <cmath>
#include <gdalwarper.h>
#include <ogr_spatialref.h>
#include <cpl_conv.h>
#include <string>
#include <cassert>

#include "CoordinateTransformation.h"


#define AGEPI       3.1415926535897932384626433832795028841971693993751
#define AGEHALFPI   1.5707963267948966192313216916395
#define RAD2DEG(x)  ((180.0*x)/AGEPI)
#define DEG2RAD(x)  ((x*AGEPI)/180.0)

#define WGS84_a            6378137.0          
#define WGS84_b            6356752.314245     
#define WGS84_F_INV        298.257223563  
#define WGS84_E_SQUARED    0.006694379990197 
#define WGS84_E            0.081819190842961775161887117288255
#define WGS84_E_SQUARED2   0.006739496742
#define WGS84_RN_POLE      6.399593625758673e+006


CoordinateTransformation::~CoordinateTransformation()
{
   if (_pCT)
   {
      OCTDestroyCoordinateTransformation((OGRCoordinateTransformation*) _pCT);   
      _pCT = 0;
   }

   if (_pCTBack)
   {
      OCTDestroyCoordinateTransformation((OGRCoordinateTransformation*) _pCTBack); 
      _pCTBack = 0;
   }
}

CoordinateTransformation::CoordinateTransformation(unsigned int nSourceEPSG, unsigned int nDestEPSG)
{
   _nDest2     = 0;
   _pCT        = 0;
   _pCTBack    = 0;

   if (nDestEPSG == 3395) // Ellipsoid Mercator
   {
      nDestEPSG = 4326;
      _nDest2 = 3395;
   }
   else if (nDestEPSG == 3785) // Spherical "Web-Mercator"
   {
      nDestEPSG = 4326;
      _nDest2 = 3785;
   }

   Init(nSourceEPSG, nDestEPSG);
}

void CoordinateTransformation::Init(unsigned int nSourceEPSG, unsigned int nDestEPSG)
{
   _nSourceEPSG = nSourceEPSG;
   _nDestEPSG = nDestEPSG;

   _bIdentity = false;

   if (_nSourceEPSG == 0 || _nDestEPSG == 0 || nSourceEPSG == _nDest2)
      _bIdentity = true;
   
   if (_pCT)
   {
      OCTDestroyCoordinateTransformation((OGRCoordinateTransformation*) _pCT);   
      _pCT = 0;
   }
   
   if (_pCTBack)
   {
      OCTDestroyCoordinateTransformation((OGRCoordinateTransformation*) _pCTBack); 
      _pCTBack = 0;
   }

   if (!_bIdentity)
   {
      OGRSpatialReference srcref;
      OGRSpatialReference dstref;

      OGRErr err;
      err = srcref.importFromEPSG(_nSourceEPSG);
      err = dstref.importFromEPSG(_nDestEPSG);
      
      _pCT        = (void*)OGRCreateCoordinateTransformation(&srcref, &dstref);
      _pCTBack    = (void*)OGRCreateCoordinateTransformation(&dstref, &srcref);
   }
}

bool CoordinateTransformation::Transform(double* dX, double* dY)
{
   if (_bIdentity)   // no transformation required, source is dest
   {
      return true;
   }

   if (!_pCT)
      return false;
   
   if(!((OGRCoordinateTransformation*)_pCT)->Transform(1, dX, dY))
      return false;

   if (_nDest2 == 3395)
   {
      double out_x;
      double out_y;
      Mercator::Forward(*dX, *dY, out_x, out_y);
      if (out_y > 1.0) out_y = 1.0;
      if (out_y < -1.0) out_y = -1.0;
      *dX = out_x;
      *dY = out_y;
   }
   else if (_nDest2 == 3785) // Web Mercator
   {
      double out_x;
      double out_y;
      Mercator::ForwardCustom(*dX, *dY, out_x, out_y, 0);
      if (out_y > 1.0) out_y = 1.0;
      if (out_y < -1.0) out_y = -1.0;
      *dX = out_x;
      *dY = out_y;
   }
   
   return true;
}

//-----------------------------------------------------------------------------

bool CoordinateTransformation::TransformBackwards(double* dX, double* dY)
{
   if (_bIdentity)   // no transformation required, source is dest
   {
      return true;
   }

   if (!_pCTBack)
      return false;


   if (_nDest2 == 3395)
   {
      double out_x;
      double out_y;
      Mercator::Reverse(*dX, *dY, out_x, out_y);
      *dX = out_x;
      *dY = out_y;
      //dZ = dZ;
   }
   else if (_nDest2 == 3785) // Web Mercator
   {
      double out_x;
      double out_y;
      Mercator::ReverseCustom(*dX, *dY, out_x, out_y, 0);
      *dX = out_x;
      *dY = out_y;
      //dZ = dZ;
   }


   if(!((OGRCoordinateTransformation*)_pCTBack)->Transform(1, dX, dY))
      return false;

   

   return true;
}

//-----------------------------------------------------------------------------


bool CoordinateTransformation::Transform(double* dX, double* dY, double* dZ)
{
   if (_bIdentity)   // no transformation required, source is dest
   {
      return true;
   }


   if (!_pCT)
      return false;

   if(!((OGRCoordinateTransformation*)_pCT)->Transform(1, dX, dY, dZ))
      return false;

   if (_nDest2 == 3395)
   {
      double out_x;
      double out_y;
      Mercator::Forward(*dX, *dY, out_x, out_y);
      if (out_y > AGEPI) out_y = AGEPI;
      if (out_y < -AGEPI) out_y = -AGEPI;
      *dX = out_x;
      *dY = out_y;
      //dZ = dZ;
   }
   else if (_nDest2 == 3785) // Web Mercator
   {
      double out_x;
      double out_y;
      Mercator::ForwardCustom(*dX, *dY, out_x, out_y, 0);
      if (out_y > AGEPI) out_y = AGEPI;
      if (out_y < -AGEPI) out_y = -AGEPI;
      *dX = out_x;
      *dY = out_y;
      //dZ = dZ;
   }

   return true;
}

//-----------------------------------------------------------------------------

bool CoordinateTransformation::TransformBackwards(double* dX, double* dY, double* dZ)
{
   if (_bIdentity)   // no transformation required, source is dest
   {
      return true;
   }

   if (!_pCTBack)
      return false;

   if (_nDest2 == 3395)
   {
      double out_x;
      double out_y;
      Mercator::Reverse(*dX, *dY, out_x, out_y);
      *dX = out_x;
      *dY = out_y;
      //dZ = dZ;
   }
   else if (_nDest2 == 3785) // Web Mercator
   {
      double out_x;
      double out_y;
      Mercator::ReverseCustom(*dX, *dY, out_x, out_y, 0);
      *dX = out_x;
      *dY = out_y;
      //dZ = dZ;
   }

   if(!((OGRCoordinateTransformation*)_pCTBack)->Transform(1, dX, dY, dZ))
      return false;

   return true;
}


//-----------------------------------------------------------------------------

Mercator::Mercator()
{

}

Mercator::~Mercator()
{
}

bool Mercator::Forward(const double lng, const double lat, double& out_x, double& out_y)
{
   const double WGS84_a_uniform = 1.0;
   const double lngRad0 = 0;

   double lngRad = DEG2RAD(lng);
   double latRad = DEG2RAD(lat);

   // #Todo: make sure lng/lat is in correct range!

   out_x = WGS84_a_uniform*(lngRad-lngRad0);
   out_y = WGS84_a_uniform*log(tan(AGEPI/4.0+latRad/2.0)*pow((1.0-WGS84_E*sin(latRad))/(1.0+WGS84_E*sin(latRad)),0.5*WGS84_E));

   out_x /= AGEPI;
   out_y /= AGEPI;

   return true;
}  

//-----------------------------------------------------------------------------

bool Mercator::Reverse(const double&  xx, const double& yy, double& out_lng, double& out_lat)
{
   double x = xx * AGEPI;
   double y = yy * AGEPI;

   const double WGS84_a_uniform = 1.0;
   const double lngRad0 = 0;

   double t = exp(-y/WGS84_a_uniform);
   out_lat = AGEHALFPI-2.0*atan(t);  // initial value for iteration...

   for (int i=0;i<10;i++)
   {
      double F= pow((1.0-WGS84_E*sin(out_lat))/(1.0+WGS84_E*sin(out_lat)),0.5*WGS84_E);
      out_lat = AGEHALFPI - 2.0*atan(t*F);
   }

   out_lng = x / WGS84_a_uniform + lngRad0;

   out_lat = RAD2DEG(out_lat);
   out_lng = RAD2DEG(out_lng);

   while (out_lng>180)
      out_lng -=180;

   while (out_lng<-180)
      out_lng +=180;

   while (out_lat>90)
      out_lat-=180;

   while (out_lat<-90)
      out_lat+=180;

   return true;
}

//-----------------------------------------------------------------------------

bool Mercator::ForwardCustom(const double lng, const double lat, double& out_x, double& out_y, const double e)
{
   const double a_uniform = 1.0;
   const double lngRad0 = 0;

   double lngRad = DEG2RAD(lng);
   double latRad = DEG2RAD(lat);

   // #Todo: make sure lng/lat is in correct range!


   if (e==0.0)
   {
      out_x = a_uniform*(lngRad-lngRad0);
      out_y = log(tan(AGEPI/4.0 + latRad/2));
   }
   else
   {
      out_x = a_uniform*(lngRad-lngRad0);
      out_y = a_uniform*log(tan(AGEPI/4.0+latRad/2.0)*pow((1.0-e*sin(latRad))/(1.0+e*sin(latRad)),0.5*e));
   }

   out_x /= AGEPI;
   out_y /= AGEPI;

   return true;
}

//-----------------------------------------------------------------------------

bool Mercator::ReverseCustom(const double&  xx, const double& yy, double& out_lng, double& out_lat, const double e)
{
   double x = xx * AGEPI;
   double y = yy * AGEPI;

   const double a_uniform = 1.0;
   const double lngRad0 = 0;

   double t = exp(-y/a_uniform);
   out_lat = AGEHALFPI-2.0*atan(t);  // initial value for iteration...

   if (e==0.0)
   {
      out_lat = AGEHALFPI - 2 * atan(t);     
   }
   else
   { 
      for (int i=0;i<10;i++)
      {
         double F= pow((1.0-e*sin(out_lat))/(1.0+e*sin(out_lat)),0.5*e);
         out_lat = AGEHALFPI - 2.0*atan(t*F);
      }
   }

   out_lng = x / a_uniform + lngRad0;

   out_lat = RAD2DEG(out_lat);
   out_lng = RAD2DEG(out_lng);

   while (out_lng>180)
      out_lng -=180;

   while (out_lng<-180)
      out_lng +=180;

   while (out_lat>90)
      out_lat-=180;

   while (out_lat<-90)
      out_lat+=180;

   return true;
}

//-----------------------------------------------------------------------------
