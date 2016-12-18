/*
Modified BSD License

This file is a part of Statistical package and originates from:
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
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "EigenAbstract.hpp"
#include <algorithm>
#include <cmath>
#include "EigenValueComparator.hpp"
#include "Assertions.hpp"
#include "Matrix.hpp"

using namespace EnjoLib;

EigenAbstract::EigenAbstract()
{
}

EigenAbstract::~EigenAbstract()
{
}

std::vector<EigenValueVector> EigenAbstract::GetEigenValVec( const Matrix & m, bool sorted ) const
{
    Assertions().Square(m, "EigenAbstract::GetEigenValVec");
    const std::vector<EigenValueVector> & cvec = GetEigenValVecClient( m );
    if ( ! sorted )
        return cvec;
    else
    {
        std::vector<EigenValueVector> vec = cvec;
        std::sort(vec.begin(), vec.end(), EigenValueComparator());
        return vec;
    }
}

Matrix EigenAbstract::GetSortedFeatureVector( const Matrix & m ) const
{
    return GetSortedFeatureVectorFactor(m, 0); // 0 = Take all
}

Matrix EigenAbstract::GetSortedFeatureVectorNumber( const Matrix & m, unsigned number) const
{
    Matrix ret;
    const std::vector<EigenValueVector> & cvec = GetEigenValVec( m, true );
    for (unsigned i = 0; i < cvec.size() && i < number; ++i)
    {
        ret.push_back(cvec[i].GetVector());
    }
    return ret;
}

Matrix EigenAbstract::GetSortedFeatureVectorFactor( const Matrix & m, double leaveFactor ) const
{
    Matrix ret;
    const std::vector<EigenValueVector> & cvec = GetEigenValVec( m, true );
    double maxEigenVal = 1;
    if ( ! cvec.empty() )
        maxEigenVal = cvec.begin()->GetValue();

    for (unsigned i = 0; i < cvec.size() && fabs(cvec[i].GetValue() / maxEigenVal) >= leaveFactor; ++i)
    {
        ret.push_back(cvec[i].GetVector());
    }
    return ret;
}
