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

#ifndef GREATCIRCLEMATH_HPP_INCLUDED
#define GREATCIRCLEMATH_HPP_INCLUDED

namespace EnjoLib
{
struct Vect3;
struct Geo;
struct Spherical;

//! Contains math for Great Circle calculations
class GreatCircleMath
{
public:
    //! Calculates a geographical point on a great circle
    /*! Calculates a geographical point on a great circle, given ratio (0,1) between starting and ending geographical locations.
    If the distance between them is equal to 90*, you can speed up the calculations by setting
    the last parameter to true
      \param angRatio angular ratio of position between the two defined points (0,1)
      \param geoStart starting geographical position
      \param geoEnd ending geographical position
      \param bQuartDistOptimisation set it to true if distance between positions = 90*, which would speed up calculations
      \return geographical location between starting and ending geographical position
      \sa Geo
      \sa CalcPointOnGCNumerical
    */
    Geo CalcPointOnGC(double angRatio, const Geo & geoStart, const Geo & geoEnd, bool bQuartDistOptimisation);

    //! Calculates an approximate geographical point on a great circle on its portion, smaller or equal than its quarter
    /*! Calculates an approximate geographical point on a great circle, given ratio (0,1) between starting and ending locations.
        The calculation is just a bit faster but approximate so should never be used for other calculations, but rather for displaying data.
        For getting the great circle data, it is wise to loop this method with angRatio being the control variable.
      \param angRatio angular ratio of position between the two defined points (0,1)
      \param v1 starting position
      \param v2 ending position. Usually it should be rotated 90* from v1, and not more
      \return approximate geographical location between starting and ending geographical position
      \sa Vect3
      \sa CalcPointOnGC
    */
    Vect3 CalcPointOnGCNumerical(double angRatio, const Vect3 & v1, const Vect3 & v2);

    //! Calculates radial distance between two geographical locations
    /*! Calculates radial distance between two geographical locations
      \param geoStart starting geographical position
      \param geoEnd ending geographical position
      \return radial distance between two geographical locations
      \sa Geo
    */
    double CalcDistBetweenPoints(const Geo & geoStart, const Geo & geoEnd);

    //! Calculates a geographical point on a great circle
    /*! Calculates a geographical point on a great circle, given ratio (0,1) of the whole distance of the circle
      \param angRatio angular ratio of position between the two defined points (0,1)
      \param geoStart starting geographical position. Could consist of satellite's inclination and longitude of its most northern GC point
      \return geographical location between starting and ending geographical position
      \sa Geo
      \sa CalcPointOnGCNumerical
    */
    Geo CalcPointOnWholeGC(double angRatio, const Geo & geoStart );

private:
};
}

#endif // GREATCIRCLEMATH_HPP_INCLUDED
