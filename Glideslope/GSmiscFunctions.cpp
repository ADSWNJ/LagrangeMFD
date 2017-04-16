
//
// Glideslope Miscellabeous Helper Functions
//
// Purpose ... work on matrices and vectors, linear interpolation, etc
//
// (c) Andrew Stokes (ADSWNJ) 2012-2017
//
// All rights reserved
//

#include "GlideslopeCore.hpp"

// Two coordinate systems are used...
// "llr" means latitude, longitude, radius ... defining our 3D point realtive to the planet core as a function of lat and long angles and a radius from core
// "xyz" means ECEF Cartesian coordinates from the core (X-axis is core to 0-lat 0-lon, Y-axis is core to 0-lat 90E-lon, Z-axis is North Pole 90N)
//
// Two relative vector systems are used...
// "ar" means azimuth (i.e. bearing) and range
// "nre" means north, radial, east
//
// Transformations are either direct (e.g. llr to xyz, nre to az) or via a mapping matrix ("m" stands for matrix)
// Planet is treated as a simple sphere (good enough close to the center, but will be wrong at the poles for a place like Earth)


MATRIX3 xyz_to_matrix(VECTOR3 xyz) {                   // Use this to set up a transform matrix from a fixed XYZ point
  VECTOR3 r=xyz/length(xyz);                          // ... MUL an XYZ into an NRE relative to the fix point
  VECTOR3 np=_V(0,0,1);                               // ... TMUL an NRE fron the fix point into an XYZ coordinate
  VECTOR3 e=crossp(np,r);
  e=e/length(e);
  VECTOR3 n=crossp(r,e);
  n=n/length(n);
  return _M(n.x,n.y,n.z,r.x,r.y,r.z,e.x,e.y,e.z);
}

VECTOR3 llr_to_xyz(VECTOR3 llr) {                     // Converts Lat Long Radius into XYZ ECEF coords
  double lat=llr.x;
  double lon=llr.y;
  double r=llr.z;
  return _V(cos(lat)*cos(lon),cos(lat)*sin(lon),sin(lat))*r;
}

VECTOR3 xyz_to_llr(VECTOR3 xyz) {                     // Converts XYZ ECEF back into Lat Long (in Radians) and Radius (in m)
  double len = length(xyz); 
  double lat = asin(xyz.z / len);
//  double lon = asin(xyz.y/(len*cos(lat)));
  double lon = atan2(xyz.y / len,xyz.x / len);
  return _V(lon,lat,len);
}

VECTOR3 xyz_to_lladeg(VECTOR3 xyz, double re) {       // Converts XYZ to Lat/Long in DEGREES and Altitude for debug output
  double len = length(xyz); 
  double lat = asin(xyz.z / len);

  VECTOR3 xyz_over_len = xyz / len;
  double cos_lat = cos(lat);
//  double lon = asin(xyz.y/(len*cos(lat)));
  double lon = atan2(xyz_over_len.y, xyz_over_len.x);
  return _V(floor(lat*RAD2DEG*1000000.0 + 0.5)/1000000.0,floor(lon*RAD2DEG*1000000.0+0.5)/1000000.0,floor((len-re)*1000000.0+0.5)/1000000.0);
}

VECTOR3 ar_to_nre(AZ_RANGE a,double rr, double log_north) { // Convert Azimuth & Range into NRE
  double range_angle=a.range/rr;
  return _V(cos(a.az+log_north)*sin(range_angle),cos(range_angle),sin(a.az+log_north)*sin(range_angle))*rr;
}

VECTOR3 ar_to_nre(AZ_RANGE a,double rr) {               // Convert Azimuth & Range into NRE assuming North = 0
  return ar_to_nre(a,rr,0);
}

AZ_RANGE nre_to_ar(VECTOR3 nre,double rr, double log_north) { // Convert NRE into Azimuth & Range
  double rr2=length(nre);
  double n=nre.x/rr2;
  double r=nre.y/rr2;
  double e=nre.z/rr2;
  double srange_angle=sqrt(n*n+e*e);
  double crange_angle=r;
  double range_angle=atan2(srange_angle,crange_angle);
  double range=range_angle*rr;
  AZ_RANGE result;
  result.az=atan2(e,n)-log_north;
  result.range=range;
  return result;
}

AZ_RANGE nre_to_ar(VECTOR3 nre,double rr) {                 // Convert NRE to Azimuth & Range assuming North = 0
  return nre_to_ar(nre,rr,0);
}

AZ_RANGE _AR(double az,double range) {                            // Return an _AR structure
  AZ_RANGE result;
  result.az=az;
  result.range=range;
  return result;
}

double Vert(VECTOR3 r,VECTOR3 v) {
  return dotp(r,v)/length(r);
}

double Horz(VECTOR3 r,VECTOR3 v) {
  double vv=Vert(r,v);
  return sqrt(dotp(v,v)-vv*vv);
}

double linterp(double x1, double y1, double x2, double y2, double x, bool cap) {
// Linear interpolation: find y, given X1-X2 and Y2-Y2 and X distance along slope
// If capped, lock Y to Y1 if X goes below X1, and to Y2 if X goes above X2
  double deltax=(x2-x1);
  double deltay=(y2-y1);
  double xcap = ((x<x1)? x1 : (x>x2)? x2: x);
  double frac= (deltax == 0.0) ? 1.0 : (xcap-x1)/deltax;
  return y1+deltay*frac;
}
