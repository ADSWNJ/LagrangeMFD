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

#ifndef VECTORUTIL_H
#define VECTORUTIL_H

#include <stdexcept>

namespace EnjoLib
{
template<class T> class VectorTpl;
class Matrix;

class Assertions
{
    public:
        Assertions();
        virtual ~Assertions();
        template<class T>
        void SizesEqual( const VectorTpl<T> & v1, const VectorTpl<T> & v2, const char * identifier ) const;
        template<class T>
        void AtLeast2Dimensions( const VectorTpl<T> & v, const char * identifier ) const;
        void Square( const Matrix & m, const char * identifier ) const;
        void CanMultiply( const Matrix & m1, const Matrix & m2, const char * identifier  ) const;

    protected:
    private:
};

    template<class T>
    void Assertions::SizesEqual( const VectorTpl<T> & v1, const VectorTpl<T> & v2, const char * identifier ) const
    {
        if ( v1.size() != v2.size() )
            throw std::invalid_argument( std::string("Incompatible sizes at\n") + identifier + "()\n");
    }

    template<class T>
    void Assertions::AtLeast2Dimensions( const VectorTpl<T> & v, const char * identifier ) const
    {
        if ( v.size() < 2 )
            throw std::invalid_argument( std::string("Dimension must be at least 2\n") + identifier + "()\n");
    }
}
#endif // VECTORUTIL_H
