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

#include <cstdlib>
#include <cmath>
#include "Statistical.hpp"
#include "Assertions.hpp"
#include "VectorD.hpp"
#include "Matrix.hpp"

using namespace EnjoLib;

Statistical::Statistical()
{
}

Statistical::~Statistical()
{
}

double Statistical::SumMulDiffMean( const VectorD & v1, const VectorD & v2 ) const
{
    Assertions().SizesEqual(v1, v2, "Statistical::SumSquares");
    double mean1 = v1.Mean();
    double mean2 = v2.Mean();

    double sumSquares = 0;
    for ( size_t i = 0; i < v1.size(); ++i )
    {
        double diff1 = v1[i] - mean1;
        double diff2 = v2[i] - mean2;
        sumSquares += diff1 * diff2;
    }

    return sumSquares;
}

double Statistical::StandardDeviation( const VectorD & v ) const
{
    Assertions().AtLeast2Dimensions(v,"Statistical::StandardDeviation");
    double variance = Variance(v);
    double sd = sqrt(variance);

    return sd;
}

double Statistical::Variance( const VectorD & v ) const
{
    Assertions().AtLeast2Dimensions(v,"Statistical::Variance");
    double diffMean = SumMulDiffMean(v, v);
    double sizeMinus1 = v.size() - 1;
    double var = diffMean / sizeMinus1;

    return var;
}

double Statistical::Covariance( const VectorD & v1, const VectorD & v2 ) const
{
    Assertions().SizesEqual(v1, v2, "Statistical::Covariance");
    Assertions().AtLeast2Dimensions(v1,"Statistical::Covariance");

    double diffMean = SumMulDiffMean(v1, v2);
    double sizeMinus1 = v1.size() - 1;
    double covar = diffMean / sizeMinus1;

    return covar;
}

Matrix Statistical::CovarianceMatrix( const Matrix & data )
{
    unsigned dimensions = data.size();
    Matrix covMat(dimensions);
    for (unsigned i = 0; i < dimensions; ++i)
    {
        const VectorD & vi = data.at(i);
        for (unsigned j = i; j < dimensions; ++j)
        {
            const VectorD & vj = data.at(j);
            double cov = Covariance(vi, vj);
            covMat.at(i).at(j) = cov;
            covMat.at(j).at(i) = cov;
        }
    }
    return covMat;
}

