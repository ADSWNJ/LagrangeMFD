/*
Modified BSD License

This file is a part of Math package and originates from:
http://sf.net/projects/enjomitchsorbit

Copyright (c) 2011-2012, Szymon "Enjo" Ender
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
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <cmath>
#include "Constants.hpp"
#include "GeneralMath.hpp"
#include "SimpsonFunctor.hpp"
#include "../Systems/Point.hpp"

using namespace EnjoLib;
// Simpson integration
// a - starting argument of function
// b - ending argument of function
// function pointer (define your own function)
// epsilon - specific preision that you may want to set
double GeneralMath::SimpsonInt(double a, double b, const SimpsonFunctor & function, double epsilon) const
{
    double h, integ = 0, integ_prev = 1;
    int m = 2;
    do
    {
        double sum = 0;
        h = (b-a) / (double)(2.0*m);
        for ( int i = 1; i < 2*m; ++i )
        {
            if (i % 2) sum = sum + 4.0 * function(a + h*i);
            else sum = sum + 2.0 * function(a + h*i);
        }
        integ_prev = integ;
        integ = (h/3.0) * ( function(a) + sum + function(b) );
        m = m * 2;
    }
    while ( fabs(integ - integ_prev) > epsilon );
    return integ;
}

int GeneralMath::round(double r) const
{
	return static_cast<int>((r > 0.0) ? std::floor(r + 0.5) : std::ceil(r - 0.5));
}

int GeneralMath::sign( double x ) const
{
    if (x > 0) return 1;
    if (x < 0) return -1;
    return 0;
}

double GeneralMath::Log2( double n ) const
{
    return log(n) / log(2.0);
}

double GeneralMath::LinearInterpol( double x, const Point & p1, const Point & p2 ) const
{
    // y = ax + b
    const Point & pDiff = p2 - p1;
    if ( pDiff.x == 0 )
        return p1.y;

    const double a = pDiff.y / pDiff.x;
    const double b = p1.y - a * p1.x;
    const double y = a * x + b;
    return y;
}

double GeneralMath::GetInPIRange( double angle ) const
{
    while ( angle > PI )  angle -= 2*PI;
    while ( angle < -PI ) angle += 2*PI;

    return angle;
}

double GeneralMath::GetIn2PIRange( double angle ) const
{
    angle = fmod(angle, 2*PI);
    return angle;
}

double GeneralMath::GetIn0_2PIRange( double angle ) const
{
    angle = GetIn2PIRange(angle);
    if (angle < 0) angle += 2*PI;
    return angle;
}

/**
Bound binary seach should finish in log2(n) iterations. Let's allow for max 2 logs.
*/
int GeneralMath::GetMaxIterBinSearchBound(double minArg, double maxArg, double eps) const
{
    const double numSlices = this->round( fabs(maxArg-minArg) / eps);
    int maxIter = 2 * this->round(this->Log2(numSlices));
    return maxIter;
}
