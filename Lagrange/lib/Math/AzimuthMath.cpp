/*
Modified BSD License

This file is a part of Math package and originates from:
http://sf.net/projects/enjomitchsorbit

Copyright (c) 2011, Szymon "Enjo" Ender
Copyright (c) ????, John Sellers
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
#include "AzimuthMath.hpp"
#include "Constants.hpp"
#include "../Systems/Point.hpp"
#include "GeneralMath.hpp"

using namespace EnjoLib;

double AzimuthMath::CalcAzimuth( double latitude, double inclination )
{
    double azimuth;

    if ( fabs(latitude - inclination) > 0.001 && inclination > fabs(latitude) )
        azimuth = asin( cos(inclination) / cos(latitude));
    else
        azimuth = PI/2.0;

    azimuth = GeneralMath().GetIn0_2PIRange(azimuth);
    return azimuth;
}

double AzimuthMath::CalcTrueAzimuth( double latitude, double inclination, const Point & shVel, double mi, double targetRadius )
{
    const double azimuth = CalcAzimuth( latitude, inclination );
    // target orbit's velocity for this azimuth
    const double tgt_orbit_v_module = sqrt(mi/targetRadius);
    const double tgt_orbit_v_y = tgt_orbit_v_module * cos(azimuth); // northern velocity
    const double tgt_orbit_v_x = tgt_orbit_v_module * sin(azimuth); // eastern velocity

    double lnch_v_y = tgt_orbit_v_y - fabs(shVel.y);
    const double lnch_v_x = tgt_orbit_v_x - shVel.x;

    if (lnch_v_y == 0) lnch_v_y = 0.01; //div by zero protection
    double true_azimuth = atan2( lnch_v_x, lnch_v_y );

    true_azimuth = GeneralMath().GetIn0_2PIRange(true_azimuth);
    return true_azimuth;
}
