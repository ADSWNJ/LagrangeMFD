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

#ifndef AZIMUTHMATH_HPP_INCLUDED
#define AZIMUTHMATH_HPP_INCLUDED

namespace EnjoLib
{
struct Point;
class AzimuthMath
{
public:
//! Calculates azimuth, without taking current ship's velocity into account.
    /*!
      \param latitude current ship's latitude
      \param inclination required final inclination
      \return azimuth required for this inclination
      \sa CalcTrueAzimuth()
    */
    double CalcAzimuth( double latitude, double inclination );

//! Calculates azimuth, taking current ship's velocity into account
    /*! Uses the simple azimuth which doesn't take velocity into account for initial calculations
    and figures out true required azimuth by comparing current ship's and target's velocities.
      \param latitude current ship's latitude
      \param inclination required final inclination
      \param shVel current ship's velocity
      \param mi Standard gravitational parameter = G * body_mass
      \param targetRadius radius of target or just a given target orbit
      \return true azimuth required for this inclination
      \sa CalcAzimuth(), Point
    */
    double CalcTrueAzimuth( double latitude, double inclination, const Point & shVel, double mi, double targetRadius );

private:
};
}

#endif // AZIMUTHMATH_HPP_INCLUDED
