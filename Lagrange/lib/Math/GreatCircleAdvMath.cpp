/*
Modified BSD License

This file is a part of Math package and originates from:
http://sf.net/projects/enjomitchsorbit

Copyright (c) 2011, Szymon Ender
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

#include "GreatCircleAdvMath.hpp"
#include "GreatCircleMath.hpp"
#include "../Systems/ThreeSphericals.hpp"
#include "../Systems/SystemsConverter.hpp"
#include "../Systems/Point.hpp"
#include "../Systems/Vect3.hpp"
#include "../Systems/Geo.hpp"
#include "../Systems/Spherical.hpp"
#include "Constants.hpp"
#include "VectorMath.hpp"

using namespace EnjoLib;

ThreeSphericals GreatCircleAdvMath::Calc3SphericalsAroundHalfGlobe( double latStart, double lonStart, double r )
{
    // dividing into 2 quarters greatly inmproves precision
    const double lat1 = latStart;  // starting point
    const double lat3 = -lat1;          // final point
    const double lat2 = (lat1+lat3)/2.0;              // middle point

    const double lon1 = lonStart; // starting point
    const double lon2 = lon1 + PI/2.0;    // middle point
    const double lon3 = lon1 + PI;      // final point

    const double theta1 = lat1 + PI/2.0;
    const double theta2 = lat2 + PI/2.0;
    const double theta3 = lat3 + PI/2.0;

    const double phi1 = 0;
    const double phi2 = lon2 - lon1;
    const double phi3 = lon3 - lon1;

    Spherical sph1(r, theta1, phi1);
    Spherical sph2(r, theta2, phi2);
    Spherical sph3(r, theta3, phi3);

    return ThreeSphericals(sph1, sph2, sph3);
}

std::vector <Point> GreatCircleAdvMath::CalculateGreatCircle( double rad, double latStart, double lonStart, bool numerical, double fixedStep )
{
    if (fixedStep <= 0)
        return std::vector <Point>();

    ThreeSphericals s = Calc3SphericalsAroundHalfGlobe(latStart, lonStart, rad);

    Vect3 v1 = SystemsConverter(s.sph1).GetVector();
    Vect3 v2 = SystemsConverter(s.sph2).GetVector();
    Vect3 v3 = SystemsConverter(s.sph3).GetVector();

    double it = -fixedStep;
    double step = fixedStep;

    std::vector<Geo> vGeo12, vGeo23; // first two quarters
    std::vector<Geo> vGeo34, vGeo41;  // remaining two quarters - mirror
    // Loop numerical great circle calculation
    while ( it <= 1)
    {
        it = it + step;
        // variable precision for numerical method
        if ( numerical && it > 0.3 && it < 0.7 )
            step = fixedStep/2.0;
        else
            step = fixedStep;

        GreatCircleMath gcm;
        Spherical sph12;
        Spherical sph23;
        if ( numerical )
        {
            Vect3 v12 = gcm.CalcPointOnGCNumerical(it, v1, v2);
            Vect3 v23 = gcm.CalcPointOnGCNumerical(it, v2, v3);

            sph12 = SystemsConverter(v12).GetSpherical();
            sph23 = SystemsConverter(v23).GetSpherical();
        }
        else
        {
            Geo g1 = SystemsConverter(v1).GetGeo();
            Geo g2 = SystemsConverter(v2).GetGeo();
            Geo g3 = SystemsConverter(v3).GetGeo();
            Geo g12 = gcm.CalcPointOnGC(it, g1, g2, true);
            Geo g23 = gcm.CalcPointOnGC(it, g2, g3, true);

            sph12 = SystemsConverter(g12, rad).GetSpherical();
            sph23 = SystemsConverter(g23, rad).GetSpherical();
        }

        // Return to real values after calculation
        sph12.phi += lonStart;
        sph23.phi += lonStart;

        Geo g12 = SystemsConverter(sph12).GetGeo();
        Geo g23 = SystemsConverter(sph23).GetGeo();
        // mirror
        Geo g34(-g12.lat, g12.lon + PI );
        Geo g41(-g23.lat, g23.lon + PI );

        vGeo12.push_back(g12);
        vGeo23.push_back(g23);
        vGeo34.push_back(g34);
        vGeo41.push_back(g41);
    }

    std::vector<Point> vRet;
    // Put into vector in a proper order
    PushGeoBackAsPoint( rad, vGeo12, &vRet );
    PushGeoBackAsPoint( rad, vGeo23, &vRet );
    PushGeoBackAsPoint( rad, vGeo34, &vRet );
    PushGeoBackAsPoint( rad, vGeo41, &vRet );

    return vRet;
}

void GreatCircleAdvMath::PushGeoBackAsPoint( double rad, const std::vector<Geo> & vGeo, std::vector<Point> * vRet )
{
    for ( std::vector<Geo>::const_iterator it = vGeo.begin(); it != vGeo.end(); ++it )
        (*vRet).push_back(SystemsConverter(*it, rad).GetPoint());
}
