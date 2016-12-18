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

#ifndef BINSEARCH_H
#define BINSEARCH_H

#include "../../Util/Result.hpp"
#include "IRootAlgo.hpp"

namespace EnjoLib
{
class RootSubject;

/// Searches for an argument of function, that produces a given reference value
class RootBisection : public IRootAlgo
{
    public:
        /// Ctor
        /**
            \param minArg - starting argument
            \param maxArg - maximal argument that will be searched
            \param epsilon - accuracy of the I/O argument (not the function!)
        */
        RootBisection( double minArg, double maxArg, double epsilon );
        virtual ~RootBisection();
        Result<double> Run( RootSubject & subj ) const;
        int GetNumIterations() const;
    protected:
    private:

        double m_minArg, m_maxArg, m_eps;
        bool m_maxArgValPositive;
        int m_maxIter;
        mutable int m_numIter;
};
}

#endif // BINSEARCH_H
