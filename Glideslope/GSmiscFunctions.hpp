
//
// Glideslope Miscellabeous Helper Functions
//
// Purpose ... work on matrices and vectors, linear interpolation, etc
//
// (c) Andrew Stokes (ADSWNJ) 2012-2017
//
// All rights reserved
//



#include "orbitersdk.h"

#ifndef _AZ_RANGE_DEF
#define _AZ_RANGE_DEF
class AZ_RANGE {
  public:
  double az,range;
};
#endif

// Two coordinate systems are used...
// "llr" means latitude, longitude, radius ... defining our 3D point realtive to the planet core as a function of lat and long angles and a radius from core
// "xyz" means ECEF Cartesian coordinates from the core (X-axis is core to 0-lat 0-lon, Y-axis is core to 0-lat 90E-lon, Z-axis is North Pole 90N)
//
// Two relative vector systems are used...
// "ar" means azimuth (i.e. bearing) and range
// "nre" means north, radial, east
//
// Transformations are either direct (e.g. llr to xyz, nre to az) or via a mapping matrix ("m" stands for matrix)


MATRIX3 xyz_to_matrix(VECTOR3 xyz);

VECTOR3 llr_to_xyz(VECTOR3 llr);

VECTOR3 xyz_to_llr(VECTOR3 xyz);

VECTOR3 xyz_to_lladeg(VECTOR3 xyz, double re);

VECTOR3 ar_to_nre(AZ_RANGE a,double rr, double log_north);

VECTOR3 ar_to_nre(AZ_RANGE a,double rr);

AZ_RANGE nre_to_ar(VECTOR3 nre,double rr, double log_north);

AZ_RANGE nre_to_ar(VECTOR3 nre,double rr);

double linterp(double x1, double y1, double x2, double y2, double x, bool cap);

double Vert(VECTOR3 r,VECTOR3 v);

double Horz(VECTOR3 r,VECTOR3 v);

AZ_RANGE _AR(double az,double range);

