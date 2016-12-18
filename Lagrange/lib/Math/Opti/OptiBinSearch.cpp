/*
Modified BSD License

This file is a part of Statistical package and originates from:
http://sf.net/p/enjomitchsorbit

Copyright (c) 2014, Szymon "Enjo" Ender
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

#include "OptiBinSearch.hpp"
#include "OptiSubject.hpp"
#include "../GeneralMath.hpp"
#include <cmath>
#include <utility>

using namespace EnjoLib;

OptiBinSearch::OptiBinSearch(double minArg, double maxArg, double eps)
{
    if (minArg > maxArg)
        std::swap(minArg, maxArg);

    this->m_minArg = minArg;
    this->m_maxArg = maxArg;
    this->m_eps = eps;
    this->m_maxIter = GeneralMath().GetMaxIterBinSearchBound(minArg, maxArg, eps);
}

OptiBinSearch::~OptiBinSearch()
{
}

Result<double> OptiBinSearch::Run( OptiSubject & subj ) const
{
    int i = 0;
    double a = m_minArg;
    double b = m_maxArg;
    bool bmaxIter = false;

    double mid = (a + b) / 2; // Midpoint
    double valMid = subj.UpdateGetValue(mid);
    do
    {   // Cut the argument in slices until the value (value) is below threshold (binary search)
        mid = (a + b) / 2; // Midpoint
        double left = (a + mid) / 2;
        double right = (mid + b) / 2;
        double valLeft = subj.UpdateGetValue(left);
        double valRight = subj.UpdateGetValue(right);
        if (valLeft < valRight && valLeft <= valMid)
        {
            b = mid; // Narrow right border
            valMid = valLeft;
        }
        else if (valRight < valLeft && valRight <= valMid)
        {
            a = mid; // Narrow left border
            valMid = valRight;
        }
        else // Narrow both borders, because the answer is inside
		{
			a = left;
			b = right;
			// value of the middle point stays the same and doesn't need to be recalculated
		}
        bmaxIter = ++i == m_maxIter;
    } while( (b-a)/2 > m_eps && ! bmaxIter ); // Continue searching, until below threshold
    //sprintf(oapiDebugString(), "i = %d, maxi = %d, arg = %lf, value = %lf, pdiff = %lf",i, m_maxIter, arg, diff, prevDiff);
    if ( bmaxIter )
        return Result<double>(m_maxArg, false);
    else
        //return Result<double>(arg, subj.IsValid( arg, value ));
        return Result<double>(mid, true);
}
