/*
Modified BSD License

This file is a part of Math package and originates from:
http://sf.net/projects/enjomitchsorbit

Copyright (c) 2012, Szymon "Enjo" Ender
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

#include <algorithm>
#include <cmath>
#include "SpaceMathBody.hpp"
#include "Constants.hpp"
using namespace EnjoLib;

SpaceMathBody::SpaceMathBody(double mass, double radius)
: m_mass(mass)
, m_radius(radius)
, m_mi(CalcMi(mass))
{}

SpaceMathBody::~SpaceMathBody()
{}

double SpaceMathBody::GetHohmannDVCircularise( double r1, double r2) const
{
    PrepareData(r1, r2);

    const double dv = sqrt(m_mi/r2) * ( 1 - sqrt(2*r1 / (r1+r2)) );
    return dv;
}

double SpaceMathBody::GetHohmannDVExtend( double r1, double r2 ) const
{
    PrepareData(r1, r2);

    const double dv = sqrt(m_mi/r1) * ( sqrt(2*r2 / (r1+r2)) - 1 );
    return dv;
}

double SpaceMathBody::CalcA( double rp, double ra) const
{
	const double e = CalcEcc(rp, ra);
	PrepareData(rp, ra);
	const double a = rp / (1 - e);

	return a;
}

double SpaceMathBody::CalcEcc( double rp, double ra ) const
{
    PrepareData(rp, ra);
	const double e = (ra-rp)/(rp+ra);

	return e;
}

double SpaceMathBody::CalcH( double rp, double ra ) const
{
    const double e = CalcEcc(rp, ra);
    const double a = CalcA(rp, ra);
    const double h = sqrt(m_mi * a * ( 1 - (e * e)));

	return h;
}

double SpaceMathBody::CalcVelAtRadius( double rp, double ra, double rTest ) const
{
    const double h = CalcH(rp, ra);
    rTest += m_radius;
    const double vel = h / rTest;

    return vel;
}

double SpaceMathBody::CalcMi( double mass ) const
{
    const double mi = mass * GGRAV_;
    return mi;
}

double SpaceMathBody::CalcPeriod( double PeA, double ApA ) const
{
    const double a = CalcA(PeA, ApA);
	const double T = 2*PI * sqrt(a*a*a / m_mi);

	return T;
}

void SpaceMathBody::PrepareData(double & alt1, double & alt2) const
{
    alt1 += m_radius;
    alt2 += m_radius;
    if (alt1 > alt2)
        std::swap(alt1, alt2);
}

double SpaceMathBody::CalcEnergyFromVelRadius( double vel, double r ) const
{
    const double energy = vel*vel / 2.0 - m_mi / (r + m_radius);
    return energy;
}
double SpaceMathBody::CalcEnergyFromRadii( double rp, double ra ) const
{
    const double a = CalcA(rp, ra);
    const double energy = - m_mi / ( 2 * a );
    return energy;
}

double SpaceMathBody::GetMi() const
{
    return m_mi;
}
