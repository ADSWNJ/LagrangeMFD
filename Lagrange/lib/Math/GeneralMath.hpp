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
DISCLAIMED. IN NO EVENT SHALL SZYMON ENDER BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GENERALMATH_HPP_INCLUDED
#define GENERALMATH_HPP_INCLUDED

namespace EnjoLib
{
class SimpsonFunctor;
struct Point;

struct GeneralMath
{
/// Simpson integration
/**
 a - starting argument of function
 b - ending argument of function
 function - define your own function by deriving SimpsonFunctor and passing it
 epsilon - specific precision that you may want to set
*/
    double SimpsonInt(double a, double b, const SimpsonFunctor & function, double epsilon = 0.001) const;
    /// Linear interpolation
    double LinearInterpol( double x, const Point & p1, const Point & p2 ) const;
	int round(double r) const;
	double Log2(double n) const;
	/// Returns angle in <-PI, PI> range
	double GetInPIRange( double angle ) const;
	/// Returns angle in <-2PI, 2PI> range
	double GetIn2PIRange( double angle ) const;
	/// Returns angle in <0, 2PI> range
	double GetIn0_2PIRange( double angle ) const;
	/// Returns sign of argument: 1, -1 or 0, if x == 0
	int sign( double x ) const;
	/// Iteration limit for bound binary search
	int GetMaxIterBinSearchBound(double minArg, double maxArg, double eps) const;
};
}

#endif // GENERALMATH_HPP_INCLUDED
