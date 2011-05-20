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


#ifndef _GN_COORDINATETRANSFORMATION_H_
#define _GN_COORDINATETRANSFORMATION_H_

#include <string>

//! \class CoordinateTransformation
//! \author Martin Christen, martin.christen@fhnw.ch
class CoordinateTransformation
{
public:
   //! Constructor using Source/Dest EPSG code
   CoordinateTransformation(unsigned int nSourceEPSG, unsigned int nDestEPSG);

   //! Destructor
   virtual ~CoordinateTransformation();
   
   //! Returns current EPSG Code for Source Coordinate
   unsigned int GetSourceEPSG(){return _nSourceEPSG;}

   //! Returns current EPSG Code for Dest Coordinate
   unsigned int GetDestEPSG(){return _nDestEPSG;}

   //! (Re)Init EPSG codes.
   void Init(unsigned int nSourceEPSG, unsigned int nDestEPSG);


   //! 2D Transformation
   bool Transform(double* dX, double* dY);
   bool TransformBackwards(double* dX, double* dY);
   
   //! 3D Transformation
   bool Transform(double* dX, double* dY, double* dZ);
   bool TransformBackwards(double* dX, double* dY, double* dZ);

protected:      
   unsigned int                  _nSourceEPSG;
   unsigned int                  _nDestEPSG;
   unsigned int                  _nDest2;
   bool                          _bIdentity;

private:
   CoordinateTransformation(){}
   void*                         _pCT;       // hidden type: OGRCoordinateTransformation*
   void*                         _pCTBack;   // hidden type: OGRCoordinateTransformation*
   
};

//! \class Mercator
//! \author Martin Christen, martin.christen@fhnw.ch
class Mercator
{
public:
   Mercator();
   virtual ~Mercator();

   //! Forward Projection on WGS84 Ellipsoid
   static bool Forward(const double   lng,  const double lat,   double& out_x,   double& out_y);

   //! Reverse Projection on WGS84 Ellipsoid
   static bool Reverse(const double&  in_x, const double& in_y, double& out_lng, double& out_lat);

   //! Forward Projection using Ellipsoid with paramater e
   static bool ForwardCustom(const double   lng,  const double lat,   double& out_x,   double& out_y, const double e);

   //! Forward Projection using Ellipsoid with paramater e
   static bool ReverseCustom(const double&  in_x, const double& in_y, double& out_lng, double& out_lat, const double e);
};

#endif

