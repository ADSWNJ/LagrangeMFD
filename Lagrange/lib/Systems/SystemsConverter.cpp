/*
Modified BSD License

This file is a part of Systems package and originates from:
http://sf.net/projects/enjomitchsorbit

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
#include "SystemsConverter.hpp"
#include "../Math/Constants.hpp"

#include "Point.hpp"
#include "Vect3.hpp"
#include "Spherical.hpp"
#include "Geo.hpp"
using namespace EnjoLib;

SystemsConverter::SystemsConverter(const Vect3 & vector)
{
    m_sph = Cartesian2Spherical(vector);
}

SystemsConverter::SystemsConverter(const Spherical & sph)
{
    m_sph = sph;
}

SystemsConverter::SystemsConverter(const Geo & geo, double rad)
{
    m_sph = Geo2Spherical(geo, rad);
}

SystemsConverter::SystemsConverter(const Point & point, double rad)
{
    m_sph = Point2Spherical(point, rad);
}

Vect3 SystemsConverter::GetVector() const
{
    return Spherical2Cartesian(m_sph);
}

Spherical SystemsConverter::GetSpherical() const
{
    return m_sph;
}

Geo SystemsConverter::GetGeo() const
{
    return Spherical2Geo(m_sph);
}

Point SystemsConverter::GetPoint() const
{
    return Spherical2Point(m_sph);
}



Vect3 SystemsConverter::Spherical2Cartesian( const Spherical & sph ) const
{
    Vect3 v;
    v.x = sph.r * sin(sph.theta) * cos(sph.phi);
    v.y = sph.r * sin(sph.theta) * sin(sph.phi);
    v.z = sph.r * cos(sph.theta);

    return v;
}

Spherical SystemsConverter::Cartesian2Spherical( const Vect3 & v ) const
{
    Spherical sph;
    sph.r = v.len();
    sph.theta = acos(v.z / sph.r);
    sph.phi = atan2(v.y, v.x);

    return sph;
}

Geo SystemsConverter::Spherical2Geo( const Spherical & sph ) const
{
    Geo geo;
    geo.lat = sph.theta - PI/2.0;
    geo.lon = sph.phi;

    while ( geo.lon > PI )
        geo.lon -= 2*PI;

    // must keep in range lat = (-90,90) & lon = (-180, 180)
    return geo;
}

Spherical SystemsConverter::Geo2Spherical( const Geo & geo, double rad ) const
{
    Spherical sph;
    sph.theta = geo.lat + PI/2.0;
    sph.phi = geo.lon;
    sph.r = rad;

    return sph;
}

Point SystemsConverter::Spherical2Point(const Spherical & sph ) const
{
    const Geo & geo = Spherical2Geo(sph);
    Point p;
    p.x = geo.lon * sph.r;
    p.y = geo.lat * sph.r;

    return p;
}

Spherical SystemsConverter::Point2Spherical(const Point & p, double rad ) const
{
    const Geo geo(p.y / rad, p.x / rad);
    return Geo2Spherical(geo, rad);
}
