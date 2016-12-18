/*
Modified BSD License

This file is a part of Math package and originates from:
http://sf.net/projects/enjomitchsorbit

Copyright (c) 2011, Szymon Ender
Copyright (c) ????, Edward Williams
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL SZYMON ENDER BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <cmath>
#include "GreatCircleMath.hpp"
#include "../Systems/Geo.hpp"
#include "../Systems/Vect3.hpp"
#include "Constants.hpp"

using namespace EnjoLib;

Geo GreatCircleMath::CalcPointOnGC(double angRatio, const Geo & geoStart, const Geo & geoEnd, bool bQuartDistOptimisation)
{
    const double & lat1 = geoStart.lat;
    const double & lon1 = geoStart.lon;
    const double & lat2 = geoEnd.lat;
    const double & lon2 = geoEnd.lon;
    // borrowed from http://williams.best.vwh.net/avform.htm#Example
    double d; // distance between points
    if ( bQuartDistOptimisation )
        d = PI/2.0;
    else
        d = CalcDistBetweenPoints(geoStart, geoEnd);
    const double A = sin((1-angRatio)*d)/sin(d);
    const double B = sin(angRatio*d)/sin(d);
    const double x = A*cos(lat1)*cos(lon1) +  B*cos(lat2)*cos(lon2);
    const double y = A*cos(lat1)*sin(lon1) +  B*cos(lat2)*sin(lon2);
    const double z = A*sin(lat1)           +  B*sin(lat2);
    Geo geo;
    geo.lat = atan2(z,sqrt(x*x+y*y));
    geo.lon = atan2(y,x);

    return geo;
}

double GreatCircleMath::CalcDistBetweenPoints(const Geo & geoStart, const Geo & geoEnd)
{
    const double & lat1 = geoStart.lat;
    const double & lon1 = geoStart.lon;
    const double & lat2 = geoEnd.lat;
    const double & lon2 = geoEnd.lon;

    // borrowed from http://williams.best.vwh.net/avform.htm#Example
    double d = 2.0*asin(sqrt((sin((lat1-lat2)/2.0))*sin((lat1-lat2)/2.0)+cos(lat1)*cos(lat2)*sin((lon1-lon2)/2.0)*sin((lon1-lon2)/2.0)));
    return d;
}

Geo GreatCircleMath::CalcPointOnWholeGC(double angRatio, const Geo & geoStart )
{
    const double it = angRatio * 4; // four halves of a pie

    // dividing into 2 quarters greatly inmproves precision
    const double lat1 = geoStart.lat;  // starting point
    const double lat3 = -lat1;          // final point
    const double lat2 = (lat1+lat3)/2.0;              // middle point

    const double lon1 = geoStart.lon; // starting point
    const double lon2 = lon1 + PI/2.0;    // middle point
    const double lon3 = lon1 + PI;      // final point

    const Geo geo1(lat1, lon1);
    const Geo geo2(lat2, lon2);
    const Geo geo3(lat3, lon3);

    int part = (int)floor(it); // which quarter of whole circle

    // calculates point on GC using equations
    const double decimalRatio = it - floor(it);
    Geo geo;
    if ( part % 2 == 0)
        geo = CalcPointOnGC(decimalRatio, geo1, geo2, true);
    else
        geo = CalcPointOnGC(decimalRatio, geo2, geo3, true);

    if ( part >= 2 )
    {
        geo.lat = -geo.lat;
        geo.lon += PI;
    }

    while ( geo.lon > PI )
        geo.lon -= 2*PI;

    return geo;
}


Vect3 GreatCircleMath::CalcPointOnGCNumerical(double angRatio, const Vect3 & v1, const Vect3 & v2)
{
    Vect3 v12 = v1 + (v2 - v1) * angRatio;
    v12 *= v1.len() / v12.len();

    return v12;
}

