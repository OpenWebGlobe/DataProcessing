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


#ifndef _GEOCOORD_H_
#define _GEOCOORD_H_
  
#include "og.h"

#ifndef I3D_USE_PRECOMPILED_HEADERS 
#include <string>
#include "vec3.h"
#endif

//-----------------------------------------------------------------------------
// WGS84 (Quelle: Bundesamt für Landestopographie)

extern OPENGLOBE_API const double WGS84_a;            // grosse Halbachse [m]
extern OPENGLOBE_API const double WGS84_b;            // kleine Halbachse [m]
extern OPENGLOBE_API const double WGS84_F_INV;        // Abplattung 1/f
extern OPENGLOBE_API const double WGS84_E;            // erste numerische Exzentrizität
extern OPENGLOBE_API const double WGS84_E_SQUARED;    // erste numerische Exzentrizität im Quadrat
extern OPENGLOBE_API const double WGS84_E_SQUARED2;   // zweite numerische Exzentrizität im Quadrat
extern OPENGLOBE_API const double WGS84_RN_POLE;      // Radius am Pol
//-----------------------------------------------------------------------------
// CH1903  (Quelle: Bundesamt für Landestopographie)
extern const double CH1903_a;
extern const double CH1903_E_SQUARED;
extern const double CH1903_PHI_0;       //  46g 57' 08.66" : geogr. Breite Nullpunkt (Bern)
extern const double CH1903_LAMDA_0;     //   7g 26' 22.50" : geogr. Länge Nullpunkt (Bern)
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
#define DEG2RAD(x) ((x)*0.017453292519943295769236907684886)	//3.14159265358979323846/180.0)
#define RAD2DEG(x) ((x)*57.295779513082320876798154814105)		//180/3.14159265358979323846)
//-----------------------------------------------------------------------------
#define CARTESIAN_SCALE			(8388607.0) // 2^23-1
#define CARTESIAN_SCALE_INV	(1.1920930376163765926810017443897e-7)

#define GetMeters(x) ((x)*CARTESIAN_SCALE_INV)
//Deg2Meter_WGS84: (6378137.0*2.0*AGEPI/360.0)
#define Deg2Meter (1.113194907932736e+005) 

//-----------------------------------------------------------------------------
   //! \class DegreeConverter
   //! \brief Convert a degree value from/to DMS (Degree Minute Second) representation
   //! \author Martin Christen, martin.christen@fhnw.ch
   //! \ingroup math geotoolbox
   class OPENGLOBE_API DegreeConverter
   {
   public:
      //! Convert DMS value to degrees.
      //! \param deg Degree value ("integer")
      //! \param min Minute value ("integer")
      //! \param sec Seconds value ("double")
      static double  FromDMS(double deg, double min, double sec);

      //! Convert a degree value to DMS.
      //! \param value [in] value in degree
      //! \param deg [out] pointer to double that will hold degree
      //! \param min [out] pointer to double that will hold minutes
      //! \param sec [out] pointer to double that will hold seconds
      static void    ToDMS(double value, double* deg, double* min, double* sec);

      //! Convert degree value to a formatted unicode string containing DMS.
      static void    ToDMSString(double value, std::wstring& strDMS);
   };
   //--------------------------------------------------------------------------

   //! \brief Class containing a 3d point in WGS84 coordinates. Holds degree and cartesian representation.
   //! \author Martin Christen, martin.christen@fhnw.ch
   //! \ingroup math geotoolbox
   class OPENGLOBE_API GeoCoord
   {
   public:
      //! \brief Default constructor. 
      //! Sets latitude, longitude and ellipsoid height to 0.
      GeoCoord() { _latitude = 0; _longitude=0; _ellipsoid_height=0;}

      //! \brief Constructor.
      //! \param lng Longitude [DEG]
      //! \param lat Latitude [DEG]
      //! \param ellipsoidH Ellipsoid Height [m]
      GeoCoord(double lng, double lat, double ellipsoidH = 0.0);

      //! \brief Destructor.
      virtual ~GeoCoord(void);

      //! \brief Retrieve Cartesian representation of this point.
      //! \param x Cartesian x-value
      //! \param y Cartesian y-value
      //! \param z Cartesian z-value
		void        ToCartesian(double* x, double* y, double* z) const;

      //! \brief Set GeoCoord using cartesian coordinates.
      //! \param x Cartesian x-value
      //! \param y Cartesian y-value
      //! \param z Cartesian z-value
      //! \param bFast if set to true, a fast approximation is used.
      void        FromCartesian(double x, double y, double z, bool bFast = true);

      //! \brief Construct GeoCoord from ray-ellipsoid intersection.
      //! The ray is given in World Coordinates, closest intersection is returned. 
      //! Returns false if there is no intersection, in this case long/lat/elevation is set to 0.
      //! \param vPosIn Start Position of ray
      //! \param vDirIn Direction of ray
      //! \return true if there is an intersection.
      bool  FromLineIntersection(const vec3<double>& vPosIn, const vec3<double>& vDirIn);


      // Normal, Binormal, Tangent:

      //! \brief Calculate surface normal of current GeoCoord
      void        CalcNormal(double* nX, double* nY, double* nZ);
      //! \brief Calculate surface normal of current GeoCoord
      void        CalcNormal(vec3<double>& vNormal) {CalcNormal(&vNormal.x,&vNormal.y,&vNormal.z);}
      //! \brief Calculate surface tangent of current GeoCoord
      void        CalcTangent(double* tX, double* tY, double* tZ);
      //! \brief Calculate surface tangent of current GeoCoord
      void        CalcTangent(vec3<double>& vTangent) {CalcTangent(&vTangent.x,&vTangent.y,&vTangent.z);}
      //! \brief Calculate surface binormal of current GeoCoord
      void        CalcBinormal(double* bX, double* bY, double* bZ);
      //! \brief Calculate surface binormal of current GeoCoord
      void        CalcBinormal(vec3<double>& vBinormal) {CalcBinormal(&vBinormal.x,&vBinormal.y,&vBinormal.z);}

      //! \brief Retrieve Latitude [DEG].
      double      GetLatitude(){ return _latitude;}

      //! \brief Retrieve Longitude [DEG].
      double      GetLongitude(){ return _longitude; }
      
      //! \brief Retrieve Ellipsoid Height [m].
      double      GetEllipsoidHeight() { return _ellipsoid_height; }

      //! \brief Retrieve Radius to current position [m].
		double      GetRadius();

      //! \brief Set Latitude [DEG]
      //! \param latitude The latitude [DEG] to set.
      void        SetLatitude(double latitude){_latitude = latitude; _bCartesian=false;}
      
      //! \brief Set Longitude [DEG]
      //! \param longitude The longitude [DEG] to set
      void        SetLongitude(double longitude){ _longitude = longitude; _bCartesian=false;}
      
      //! \brief Set Ellipsoid Height [m]
      //! \param ellh The Ellipsoid height [m] to set.
      void        SetEllipsoidHeight(double ellh) {_ellipsoid_height = ellh; _bCartesian=false;}

      //! \brief Print info,
      //! \deprecated do not use anymore. Will be removed soon.
      void        Info();

      //! \brief Scalar multiplication
      GeoCoord operator* (double d) { return GeoCoord(_longitude*d, _latitude*d, _ellipsoid_height);}
      
      //! \brief Add two coordinates
      GeoCoord operator+ (GeoCoord v) { return GeoCoord(v._longitude + _longitude, v._latitude + _latitude, _ellipsoid_height);}
      
      //! \brief Subtract two coordinates
      GeoCoord operator- (GeoCoord v) { return GeoCoord(_longitude - v._longitude, _latitude - v._latitude, _ellipsoid_height);}
     
      //! \brief Calculate Ellipsoidical distance to target point
      //! \param oCoordTarget Target GeoCoord
      //! \return distance in [m]
		double CalcEllipsoidDistanceTo(GeoCoord& oCoordTarget);

      //! \brief Calculate azimuth to target coord
      //! \param oCoordTarget Target GeoCoord
      //! \return azimuth in [RAD]
		double CalcAzimuthTo(GeoCoord& oCoordTarget);

      //! \brief Calculate azimuth to target coord specified in longitude, latitude
      //! \param lng Target Longitude
      //! \param lat Target Latitude
      //! \return azimuth in [RAD]
      double CalcAzimuthTo(double lng, double lat);

      //! \brief Calculate exact area on ellipsoid, given two lng/lat pairs.
      static double CalcArea(double lng0, double lat0, double lng1, double lat1);

      //! \brief Return latitude
      //! \deprecated use GetLatitude()
      double latitude() {return _latitude;}

      //! \brief Return longitude
      //! \deprecated use GetLongitude()
      double longitude() {return _longitude;}

      //! \brief Return ellipsoid height
      //! \deprecated use GetEllipsoidHeight
      double elliposid_height(){return _ellipsoid_height;}

      //! \brief Recalculate Cartesian Representation.
      //! This mus be called before using x(), y() or z()
      void   RecalcCartesian(){Recalc();}

      //! \brief Retrieve x-value of cartesian coordinate. 
      //! RecalcCartesian must have been called previously.
      //! \return cartesian x-coord
      double x() {return _fCartesianX;}

      //! \brief Retrieve y-value of cartesian coordinate. 
      //! RecalcCartesian must have been called previously.
      //! \return cartesian y-coord
      double y() {return _fCartesianY;}

      //! \brief Retrieve z-value of cartesian coordinate. 
      //! RecalcCartesian must have been called previously.
      //! \return cartesian z-coord
      double z() {return _fCartesianZ;}

      //! \brief Retrieve cartesian coordinate and store in a vec3<double>.
      //! \param v Vector to store cartesian coordinate
      void   GetCartesian(vec3<double>& v) {v.x = _fCartesianX; v.y = _fCartesianY; v.z = _fCartesianZ;}

      //! \brief Cartesian coordinate ("World Coordinate") of north pole
      static vec3<double> NORTHPOLE;

      //! \brief Cartesian coordinate ("World Coordinate") of south pole
      static vec3<double> SOUTHPOLE;

   private:
      void     Recalc();
      double   _latitude; 
      double   _longitude;
      double   _ellipsoid_height;

      // optimization:
      double   _fCartesianX, _fCartesianY, _fCartesianZ;
      bool     _bCartesian;
   };

//-----------------------------------------------------------------------------

#endif
