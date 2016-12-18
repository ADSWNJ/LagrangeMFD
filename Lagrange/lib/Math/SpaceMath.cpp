/*
Modified BSD License

This file is a part of Math package and originates from:
http://sf.net/projects/enjomitchsorbit

Copyright (c) 2010, "Face"
Copyright (c) 2011, Szymon "Enjo" Ender
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
#include "SpaceMath.hpp"
#include "VectorMath.hpp"
#include "GreatCircleAdvMath.hpp"
#include "../Systems/Geo.hpp"
#include "../Systems/Vect3.hpp"
#include "../Systems/ThreeSphericals.hpp"
#include "../Systems/SystemsConverter.hpp"
#include "Constants.hpp"
using namespace EnjoLib;

double SpaceMath::CalcAngleToSatelliteNodeOnGround(const Geo & shipPos, double satIncl, double satLAN, bool northernHeading)
{
    // the following is:
    // lat=atan((sin(lat1)*cos(lat2)*sin(lon-lon2)-sin(lat2)*cos(lat1)*sin(lon-lon1))/(cos(lat1)*cos(lat2)*sin(lon1-lon2)))
    // from http://williams.best.vwh.net/avform.htm#Int
    // solved for lon instead of lat, using sinusoid formula:
    // a * sin(x) + b * cos(x) = sqrt(a*a + b*b) * sin( x + atan(b/a) )
    // thanks, Face!
    //double satLonAtGivenLat = asin(tan(shipPos.lat) * cos(satIncl) / sin(satIncl) );
    const double satLonAtGivenLat = asin(tan(shipPos.lat) / tan(satIncl) );
    const double diffBetweenShipsPosAndSatLAN = satLAN - shipPos.lon;

    double angle;
    if ( northernHeading )
        angle = diffBetweenShipsPosAndSatLAN + satLonAtGivenLat;
    else
        angle = PI + diffBetweenShipsPosAndSatLAN - satLonAtGivenLat;

    if (angle > 2*PI)
        angle -= 2*PI;

    return angle;
}

Vect3 SpaceMath::CalcPlaneMomentum( double latStart, double lonStart )
{
    double rad = 1; // doesn't matter, since we're normalising the returned vector
    const ThreeSphericals & s = GreatCircleAdvMath().Calc3SphericalsAroundHalfGlobe(latStart, lonStart, rad);

    Spherical satSph1(s.sph1);
    Spherical satSph2(s.sph2);
    satSph1.phi += lonStart;
    satSph2.phi += lonStart;

    const Vect3 & realv1 = SystemsConverter( satSph1 ).GetVector();
    const Vect3 & realv2 = SystemsConverter( satSph2 ).GetVector();

    return VectorMath().cross( realv1, realv2 ).norm();
}

// Thanks to Kwan and Topper
double SpaceMath::RocketEqnT(double dv, double m, double F, double isp)
{
  return ( dv * m / (2.0 * F ) ) * ( 1 + exp( -1.0 * dv / isp ) );
}

