/*
Modified BSD License

This file is a part of Math package and originates from:
http://sf.net/projects/enjomitchsorbit

Copyright (c) 2013, Szymon "Enjo" Ender
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

#include "RootBisection.hpp"
#include "RootSubject.hpp"
#include "../GeneralMath.hpp"
//#include <sstream>
#include <cmath>
//#include <Orbitersdk.h>


using namespace EnjoLib;

RootBisection::RootBisection( double minArg, double maxArg, double eps )
{
    if (minArg > maxArg)
        std::swap(minArg, maxArg);

    this->m_minArg = minArg;
    this->m_maxArg = maxArg;
    this->m_eps = eps;
    this->m_maxIter = GeneralMath().GetMaxIterBinSearchBound(minArg, maxArg, eps);
    m_numIter = 0;
}

RootBisection::~RootBisection()
{}

//#include <orbitersdk.h>
Result<double> RootBisection::Run( RootSubject & subj ) const
{
    // Validation block
    GeneralMath gm;
    const double refValue = subj.GetRefValue();
    double fmin = subj.UpdateGetValue(m_minArg) - refValue;
    double fmax = subj.UpdateGetValue(m_maxArg) - refValue;
    int faSign = gm.sign(fmin);
    int fbSign = gm.sign(fmax);
    if ( faSign == fbSign )
    {
        // The value for maximal argument should have been positive and was negative
        // or it should have been negative while its positive
        //sprintf(oapiDebugString(), "dupa = %lf", fmax);
        return Result<double>(m_maxArg, false);
    }
    // validated!
    double mid, midValue;
    int i = 0;
    double a = m_minArg;
    double b = m_maxArg;
    if (faSign == 0)
    {
        a = (a + b) / 2.0;
        double fa = subj.UpdateGetValue(a) - refValue;
        faSign = gm.sign(fa);
    }
    bool bmaxIter = false;
    do
    {   // Cut the argument in slices until the value (value) is below threshold (binary search)
        mid = (a + b) / 2.0; // Midpoint
        midValue = subj.UpdateGetValue(mid) - refValue;
        int midSign = gm.sign(midValue);  // Cache the sign in a variable for reuse
        if ( faSign == midSign )
        {
            a = mid; // Narrow left border
            // f(a) would need a new evaluation since "a" changes, so store the current value as next iteration's f(a)
            faSign = midSign; // We actually only care about the current sign for the next iteration
        }
        else
        {
            b = mid; // Narrow right border
            // f(a) can be reused from current iteration
        }

        bmaxIter = ++i == m_maxIter;
    } while( (b-a) > m_eps && ! bmaxIter ); // Continue searching, until below threshold
    //sprintf(oapiDebugString(), "i = %d, maxi = %d, mid = %lf, value = %lf, pdiff = %lf",i, m_maxIter, mid, diff, prevDiff);
    m_numIter = i;
    if ( bmaxIter )
        return Result<double>(m_maxArg, false);
    else
        return Result<double>(mid, subj.IsValid( mid, midValue ));
}

int RootBisection::GetNumIterations() const
{
    return m_numIter;
}
