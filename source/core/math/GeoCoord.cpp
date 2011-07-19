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

#include "GeoCoord.h"
#include <cmath>
#include <sstream>
#include <float.h>

//-----------------------------------------------------------------------------
// WGS84 (Quelle: Bundesamt fuer Landestopographie)

const double WGS84_a =           6378137.0;          // grosse Halbachse [m]
const double WGS84_b =           6356752.314245;     // kleine Halbachse [m]
const double WGS84_F_INV =       298.257223563;      // Abplattung 1/f
const double WGS84_E_SQUARED =   0.006694379990197;  // erste numerische Exzentrizität im Quadrat
const double WGS84_E =           0.081819190842961775161887117288255; // erste numerische Exzentrizität
const double WGS84_E_SQUARED2 =  0.006739496742;     // zweite numerische Exzentrizität im Quadrat
const double WGS84_RN_POLE =     6.399593625758673e+006; // Radius am Pol
//-----------------------------------------------------------------------------
// CH1903  (Quelle: Bundesamt für Landestopographie)
const double CH1903_a =          6377397.155;
const double CH1903_E_SQUARED =  0.006674372230614;
const double CH1903_PHI_0 =      46.952404;          //  46?57' 08.66" : geogr. Breite Nullpunkt (Bern)
const double CH1903_LAMDA_0 =    7.439583;           //   7?26' 22.50" : geogr. Länge Nullpunkt (Bern)
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Degree Converter

inline double fract_(double x)           //!< x-floor(x) or mod(x,1);
{
   return x-floor(x);
}

double  DegreeConverter::FromDMS(double deg, double min, double sec)
{
    return deg+min/60.0+sec/3600.0; 
}

void    DegreeConverter::ToDMS(double fDecimal, double* fDeg, double* fMin, double* fSec)
{
   double ff = fract_(fDecimal);
   *fDeg = fDecimal - ff;
   *fMin = 60*ff;
   *fSec = 60.0*(fract_(*fMin));
   *fMin = *fMin - fract_(*fMin);
}

void    DegreeConverter::ToDMSString(double value, std::wstring& strDMS)
{  
   double fD, fM, fS;
   ToDMS(value, &fD, &fM, &fS);

   std::wostringstream strout(std::wostringstream::out);
   strout << fD << "¡ã" << fM << "'" << fS << "\"";
   strDMS = strout.str();
}


//-----------------------------------------------------------------------------

vec3<double> GeoCoord::NORTHPOLE = vec3<double>(0,0,(1-WGS84_E_SQUARED)*WGS84_RN_POLE*CARTESIAN_SCALE_INV);
vec3<double> GeoCoord::SOUTHPOLE = vec3<double>(0,0,-(1-WGS84_E_SQUARED)*WGS84_RN_POLE*CARTESIAN_SCALE_INV);

GeoCoord::GeoCoord(double lng, double lat, double ellipsoidH)
{
   _latitude = lat;
   _longitude = lng;
   _ellipsoid_height = ellipsoidH;
   ToCartesian(&_fCartesianX, &_fCartesianY, &_fCartesianZ);
   _bCartesian = true;
}

//-----------------------------------------------------------------------------

GeoCoord::~GeoCoord()
{
}

//-----------------------------------------------------------------------------
double GeoCoord::GetRadius()
{
   double x,y,z;
   double sinlat = sin(DEG2RAD(_latitude));    // sin der Breite
   double coslat = cos(DEG2RAD(_latitude));    // cos der Breite
   double sinlong = sin(DEG2RAD(_longitude));   // sin der Länge
   double coslong = cos(DEG2RAD(_longitude));   // cos der Länge

   double Rn = WGS84_a / sqrt(1.0-WGS84_E_SQUARED*sinlat*sinlat);

   x = (Rn + _ellipsoid_height) * coslat * coslong;
   y = (Rn + _ellipsoid_height) * coslat * sinlong;
   z = ((1-WGS84_E_SQUARED)*Rn + _ellipsoid_height) * sinlat; 

   return sqrt(x*x+y*y+z*z)/CARTESIAN_SCALE;
}
//-----------------------------------------------------------------------------
void GeoCoord::ToCartesian(double* x, double* y, double* z) const
{   
   double sinlat = sin(DEG2RAD(_latitude));    // sin der Breite
   double coslat = cos(DEG2RAD(_latitude));    // cos der Breite
   double sinlong = sin(DEG2RAD(_longitude));   // sin der Länge
   double coslong = cos(DEG2RAD(_longitude));   // cos der Länge

   // Normalkrümmungsradius Rn:
   double Rn = WGS84_a / sqrt(1.0-WGS84_E_SQUARED*sinlat*sinlat) ;
   //double Rn = WGS84_a * WGS84_a / (sqrt(WGS84_a*WGS84_a*coslat*coslat+WGS84_b*WGS84_b*sinlat*sinlat));


   *x = (Rn + _ellipsoid_height) * coslat * coslong;
   *y = (Rn + _ellipsoid_height) * coslat * sinlong;
   *z = ((1-WGS84_E_SQUARED)*Rn + _ellipsoid_height) * sinlat;

   *x *= CARTESIAN_SCALE_INV;
   *y *= CARTESIAN_SCALE_INV;
   *z *= CARTESIAN_SCALE_INV;
}
//-----------------------------------------------------------------------------
void GeoCoord::FromCartesian(double x, double y, double z, bool bFast)
{
   double Rn;
   double sinlat2, coslat;
   coslat = 0;

   x *= CARTESIAN_SCALE;
   y *= CARTESIAN_SCALE;
   z *= CARTESIAN_SCALE;

/*
   // H. Vermeille: "Direct transformation from geocentric coordinates to geodetic coordinates"

   double ra2 = 1 / (WGS84_a * WGS84_a);

   double X = x;
   double Y = y;
   double Z = z;
   double e2 = WGS84_E_SQUARED2;
   double e4 = e2 * e2;

   double XXpYY = X * X + Y * Y;
   double sqrtXXpYY = sqrt(XXpYY);
   double p = XXpYY * ra2;
   double q = Z * Z * (1 - e2) * ra2;
   double r = 1 / 6.0 * (p + q - e4);
   double s = e4 * p * q / (4 * r * r * r);
   double t = pow(1 + s + sqrt(s * (2 + s)), 1 / 3.0);
   double u = r * (1 + t + 1 / t);
   double v = sqrt(u * u + e4 * q);
   double w = e2 * (u + v - q) / (2 * v);
   double k = sqrt(u + v + w * w) - w;
   double D = k * sqrtXXpYY / (k + e2);
   double lon = 2 * atan2(Y, X + sqrtXXpYY);
   double sqrtDDpZZ = sqrt(D * D + Z * Z);
   double lat = 2 * atan2(Z, D + sqrtDDpZZ);
   double elevation = (k + e2 - 1) * sqrtDDpZZ / k;

   _latitude = RAD2DEG(lat);
   _longitude = RAD2DEG(lon);
   _ellipsoid_height = elevation;

*/

    // ITERATIV
   
   _longitude = atan2(y,x);

   // erster Näherungswert:
   _latitude = atan2(z,sqrt(x*x+y*y));

   // Iterativer Prozess
   if (bFast)
   {
      for (int i=0;i<10;i++) // better: while (abs(cos(_latitude) - coslat) > DBL_EPSILON) 
      {
         sinlat2 = sin(_latitude); sinlat2 *= sinlat2;
         coslat = cos(_latitude);
         Rn =  WGS84_a / sqrt(1-WGS84_E_SQUARED*sinlat2);
         _ellipsoid_height = sqrt(x*x+y*y) / coslat - Rn;
         _latitude = atan2(z/sqrt(x*x+y*y), 1-(Rn*WGS84_E_SQUARED)/(Rn+_ellipsoid_height)); 
      }
   }
   else
   {
      while (abs(cos(_latitude) - coslat) > DBL_EPSILON)
      {
         sinlat2 = sin(_latitude); sinlat2 *= sinlat2;
         coslat = cos(_latitude);
         Rn =  WGS84_a / sqrt(1-WGS84_E_SQUARED*sinlat2);
         _ellipsoid_height = sqrt(x*x+y*y) / coslat - Rn;
         _latitude = atan2(z/sqrt(x*x+y*y), 1-(Rn*WGS84_E_SQUARED)/(Rn+_ellipsoid_height)); 
      }
   }

   _latitude = RAD2DEG(_latitude);
   _longitude = RAD2DEG(_longitude);

   _fCartesianX = x;
   _fCartesianY = y;
   _fCartesianZ = z;

   _bCartesian = true;

}

//-----------------------------------------------------------------------------

void GeoCoord::CalcNormal(double* nX, double* nY, double* nZ)
{
   double sinlat = sin(DEG2RAD(_latitude));   
   double coslat = cos(DEG2RAD(_latitude));   
   double sinlong = sin(DEG2RAD(_longitude));  
   double coslong = cos(DEG2RAD(_longitude));

   *nX = coslat * coslong;
   *nY = coslat * sinlong;
   *nZ = sinlat;
}

//-----------------------------------------------------------------------------

void GeoCoord::CalcTangent(double* tX, double* tY, double* tZ)
{
   double sinlat = sin(DEG2RAD(_latitude));   
   double coslat = cos(DEG2RAD(_latitude));   
   double sinlong = sin(DEG2RAD(_longitude));  
   double coslong = cos(DEG2RAD(_longitude));

   *tX = -sinlat * coslong;
   *tY = -sinlat * sinlong;
   *tZ = -sinlat;
}

//-----------------------------------------------------------------------------

void GeoCoord::CalcBinormal(double* bX, double* bY, double* bZ)
{
   double sinlong = sin(DEG2RAD(_longitude));
   double coslong = cos(DEG2RAD(_longitude));

   *bX = -sinlong;
   *bY = coslong;
   *bZ = 0;
}
//-----------------------------------------------------------------------------

double GeoCoord::CalcEllipsoidDistanceTo(GeoCoord& oCoordTarget)
{
   double fLat1 = _latitude;
   double fLon1 = _longitude;
   double fLat2 = oCoordTarget.GetLatitude();
   double fLon2 = oCoordTarget.GetLongitude();
   double fDistance = 0.0;
   double fFaz;
   double fBaz;
   double fR = 1.0 - 1.000000/WGS84_F_INV;
   double tu1, tu2, cu1, su1, cu2, x, sx, cx, sy, cy, y, sa, c2a, cz, e, c, d;
   double fCosy1;
   double fCosy2;
   fDistance = 0.0;

   if((fLon1 == fLon2) && (fLat1 == fLat2))
      return fDistance;
   fLon1 = DEG2RAD(fLon1);
   fLon2 = DEG2RAD(fLon2);
   fLat1 = DEG2RAD(fLat1);
   fLat2 = DEG2RAD(fLat2);

   fCosy1 = cos(fLat1);
   fCosy2 = cos(fLat2);

   if(fCosy1 == 0.0)
      fCosy1 = 0.0000000001;
   if(fCosy2 == 0.0)
      fCosy2 = 0.0000000001;

   tu1 = fR * sin(fLat1) / fCosy1;
   tu2 = fR * sin(fLat2) / fCosy2;
   cu1 = 1.0 / sqrt(tu1 * tu1 + 1.0);
   su1 = cu1 * tu1;
   cu2 = 1.0 / sqrt(tu2 * tu2 + 1.0);
   x = fLon2 - fLon1;

   fDistance = cu1 * cu2;
   fBaz = fDistance * tu2;
   fFaz = fBaz * tu1;

   do
   {
      sx = sin(x);
      cx = cos(x);
      tu1 = cu2 * sx;
      tu2 = fBaz - su1 * cu2 * cx;
      sy = sqrt(tu1 * tu1 + tu2 * tu2);
      cy = fDistance * cx + fFaz;
      y = atan2(sy, cy);
      sa = fDistance * sx / sy;
      c2a = -sa * sa + 1.0;
      cz = fFaz + fFaz;
      if(c2a > 0.0) cz = -cz / c2a + cy;
      e = cz * cz * 2. - 1.0;
      c = ((-3.0 * c2a + 4.0) * 1.000000/WGS84_F_INV + 4.0) * c2a * 1.000000/WGS84_F_INV / 16.0;
      d = x;
      x = ((e * cy * c + cz) * sy * c + y) * sa;
      x = (1.0 - c) * x * 1.000000/WGS84_F_INV + fLon2 - fLon1;
   } while(fabs(d - x) > 0.000000000005);

   x = sqrt((1.0 / fR / fR - 1.0) * c2a + 1.0) + 1.0;
   x = (x - 2.0) / x;
   c = 1.0 - x;
   c = (x * x / 4.0 + 1.0) / c;
   d = (0.375 * x * x - 1.0) * x;
   x = e * cy;
   fDistance = 1.0 - e - e;
   fDistance = ((((sy * sy * 4.0 - 3.0) *
      fDistance * cz * d / 6.0 - x) * d / 4.0 + cz) * sy * d + y) * c * 1.0/*GEO::ERAD*/ * fR;

   return fDistance;
}
//-----------------------------------------------------------------------------

double GeoCoord::CalcAzimuthTo(GeoCoord& oCoordTarget)
{
   double fResult = 0.0;
   double fLat1 = _latitude;
   double fLon1 = _longitude;
   double fLat2 = oCoordTarget.GetLatitude();
   double fLon2 = oCoordTarget.GetLongitude();

   int ilat1 = (int)(0.50 + fLat1 * 360000.0);
   int ilat2 = (int)(0.50 + fLat2 * 360000.0);
   int ilon1 = (int)(0.50 + fLon1 * 360000.0);
   int ilon2 = (int)(0.50 + fLon2 * 360000.0);

   fLon1 = DEG2RAD(fLon1);
   fLon2 = DEG2RAD(fLon2);
   fLat1 = DEG2RAD(fLat1);
   fLat2 = DEG2RAD(fLat2);

   if ((ilat1 == ilat2) && (ilon1 == ilon2))
   {
      return fResult;
   }
   else if (ilon1 == ilon2)
   {
      if (ilat1 > ilat2)
         fResult = AGEPI;
   }
   else
   {
      double c = acos(sin(fLat2)*sin(fLat1) + cos(fLat2)*cos(fLat1)*cos((fLon2-fLon1)));
      fResult = asin(cos(fLat2)*sin((fLon2-fLon1))/sin(c));

      if ((ilat2 > ilat1) && (ilon2 > ilon1))
      {
      }
      else if ((ilat2 < ilat1) && (ilon2 < ilon1))
      {
         //			fResult = AGEPI - fResult;
         fResult = atan(cos((fLat1-fLat2)/2.0f)/sin((fLat1+fLat2)/2.0f)/tan((fLon2-fLon1)/2.0f)) + atan(sin((fLat1-fLat2)/2.0f)/cos((fLat1+fLat2)/2.0f)/tan((fLon2-fLon1)/2.0f));
      }
      else if ((ilat2 < ilat1) && (ilon2 > ilon1))
      {
         if (abs(fLon2 - fLon1) < AGEPI/2.0f)
            fResult = AGEPI - fResult;
         else
            int nBreak = 0;
      }
      else if ((ilat2 > ilat1) && (ilon2 < ilon1))
      {
         fResult += 2*AGEPI;
      }
   }

   return fResult;
}
//-----------------------------------------------------------------------------

void GeoCoord::Recalc()
{
   ToCartesian(&_fCartesianX, &_fCartesianY, &_fCartesianZ);
   _bCartesian = true;
}

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

bool GeoCoord::FromLineIntersection(const vec3<double>& vPosIn, const vec3<double>& vDirIn)
{
   /* Warning: vDir should never be (0,0,0) */

   double b2 = WGS84_b*WGS84_b;
   double a2 = WGS84_a*WGS84_a;
   double t0,t1,N,h;

   vec3<double> vPos = vPosIn*CARTESIAN_SCALE;
   vec3<double> vDir = vDirIn*CARTESIAN_SCALE;  // not really required

   double D = 2*b2*vPos.y*vDir.y*vPos.x*vDir.x +
      2*a2*vPos.y*vDir.y*vPos.z*vDir.z +
      2*a2*vPos.x*vDir.x*vPos.z*vDir.z +
      a2*b2*vDir.x*vDir.x -
      a2*vDir.x*vDir.x*vPos.z*vPos.z -
      b2*vDir.x*vDir.x*vPos.y*vPos.y -
      a2*vDir.z*vDir.z*vPos.x*vPos.x +
      a2*a2*vDir.z*vDir.z -
      a2*vDir.z*vDir.z*vPos.y*vPos.y -
      b2*vDir.y*vDir.y*vPos.x*vPos.x +
      b2*a2*vDir.y*vDir.y -
      a2*vDir.y*vDir.y*vPos.z*vPos.z;

   if (D<0)
   {
      //cfout << "WARNING::OUTSIDE\n";
      return false;     // NO SOLUTION, LINE DOESN'T INTERSECT ELLIPSOID
   }

   N  = b2*vDir.x*vDir.x + a2*vDir.z*vDir.z + b2*vDir.y*vDir.y;

   h = -b2*vPos.y*vDir.y
      -b2*vPos.x*vDir.x
      -a2*vPos.z*vDir.z;

   t0 = (h + WGS84_b*sqrt(D)) / N;
   t1 = (h - WGS84_b*sqrt(D)) / N;

   vec3<double> P1 = vPos + vDir*t0;
   vec3<double> P0 = vPos + vDir*t1;

   double dx2, dy2, dz2;

   dx2 = P1.x-vPos.x; dx2*=dx2;
   dy2 = P1.y-vPos.y; dy2*=dy2;
   dz2 = P1.z-vPos.z; dz2*=dz2;
   double mindist1 = sqrt(dx2 + dy2 + dz2);

   dx2 = P0.x-vPos.x; dx2*=dx2;
   dy2 = P0.y-vPos.y; dy2*=dy2;
   dz2 = P0.z-vPos.z; dz2*=dz2;
   double mindist0 = sqrt(dx2 + dy2 + dz2);

   if (mindist0<mindist1)
   {
      if (t0>=0)
         FromCartesian(P0.x/CARTESIAN_SCALE, P0.y/CARTESIAN_SCALE, P0.z/CARTESIAN_SCALE);
      else
      {
         //cout << "BEHIND!\n";
         return false;
      }
   }
   else
   {
      if (t0>=0)
         FromCartesian(P1.x/CARTESIAN_SCALE, P1.y/CARTESIAN_SCALE, P1.z/CARTESIAN_SCALE);
      else
      {
         //cout << "BEHIND!\n";
         return false;
      }
   }

   return true;
}


double GeoCoord::CalcAzimuthTo(double lng, double lat) 
{
   double fResult = 0.0;
   double fLat1 = _latitude;
   double fLon1 = _longitude;
   double fLat2 = lat;
   double fLon2 = lng;

   int ilat1 = (int)(0.50 + fLat1 * 360000.0);
   int ilat2 = (int)(0.50 + fLat2 * 360000.0);
   int ilon1 = (int)(0.50 + fLon1 * 360000.0);
   int ilon2 = (int)(0.50 + fLon2 * 360000.0);

   fLon1 = DEG2RAD(fLon1);
   fLon2 = DEG2RAD(fLon2);
   fLat1 = DEG2RAD(fLat1);
   fLat2 = DEG2RAD(fLat2);

   if ((ilat1 == ilat2) && (ilon1 == ilon2))
   {
      return fResult;
   }
   else if (ilon1 == ilon2)
   {
      if (ilat1 > ilat2)
         fResult = AGEPI;
   }
   else
   {
      double c = acos(sin(fLat2)*sin(fLat1) + cos(fLat2)*cos(fLat1)*cos((fLon2-fLon1)));
      fResult = asin(cos(fLat2)*sin((fLon2-fLon1))/sin(c));

      if ((ilat2 > ilat1) && (ilon2 > ilon1))
      {
      }
      else if ((ilat2 < ilat1) && (ilon2 < ilon1))
      {
         //			fResult = AGEPI - fResult;
         fResult = atan(cos((fLat1-fLat2)/2.0f)/sin((fLat1+fLat2)/2.0f)/tan((fLon2-fLon1)/2.0f)) + atan(sin((fLat1-fLat2)/2.0f)/cos((fLat1+fLat2)/2.0f)/tan((fLon2-fLon1)/2.0f));
      }
      else if ((ilat2 < ilat1) && (ilon2 > ilon1))
      {
         if (abs(fLon2 - fLon1) < AGEHALFPI)
            fResult = AGEPI - fResult;
         else
            int nBreak = 0;
      }
      else if ((ilat2 > ilat1) && (ilon2 < ilon1))
      {
         fResult += 2*AGEPI;
      }
   }

   return fResult;
}



double GeoCoord::CalcArea(double lng0, double lat0, double lng1, double lat1)
{
   lng0 = DEG2RAD(lng0);
   lng1 = DEG2RAD(lng1);
   lat0 = DEG2RAD(lat0);
   lat1 = DEG2RAD(lat1);

   return (lng1-lng0)*WGS84_a*WGS84_a*(1.0-WGS84_E*WGS84_E)*(-log(WGS84_E*sin(lat1)-1.0)+log(1.0+WGS84_E*sin(lat1))+log(WGS84_E*sin(lat0)-1.0)-log(1.0+WGS84_E*sin(lat0)))/WGS84_E/2.0;
}

//-----------------------------------------------------------------------------


