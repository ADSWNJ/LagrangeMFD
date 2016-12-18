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

#include <algorithm>
#include <cmath>
#include <sstream>

#include "VectorTpl.hpp"
#include "Assertions.hpp"

using namespace EnjoLib;

template class VectorTpl<float>;
template class VectorTpl<double>;

template<class T>
VectorTpl<T>::VectorTpl( const std::vector<T> & init )
{
    for (CIt it = init.begin(); it != init.end(); ++it)
        this->push_back(*it);
}

template<class T>
VectorTpl<T>::VectorTpl( int n )
{
    for (int i = 0; i < n; ++i)
        this->push_back(0);
}

template<class T>
VectorTpl<T>::VectorTpl(){}

template<class T>
VectorTpl<T>::~VectorTpl(){}

template<class T>
std::string VectorTpl<T>::Print() const
{
    std::ostringstream ss;
    for (CIt cit = this->begin(); cit != this->end(); ++cit)
    {
        ss << *cit;
        if ( cit != this->end() - 1)
            ss << ", ";
    }

    return ss.str();
}

template<class T>
std::string VectorTpl<T>::PrintPython( const char * varName ) const
{
    std::ostringstream ss;
    ss << varName << " = [ ";
    for (CIt cit = this->begin(); cit != this->end(); ++cit)
        ss << *cit << ", ";
    ss << " ];\n";
    return ss.str();
}

template<class T>
std::string VectorTpl<T>::PrintScilab( const char * varName ) const
{
    std::ostringstream ss;
    ss << "\n" << varName << " = [ ";
    for (CIt cit = this->begin(); cit != this->end(); ++cit)
        ss << *cit << " ";
    ss << " ];\n";
    return ss.str();
}

template<class T>
T VectorTpl<T>::Len() const
{
    return sqrt( SumSquares() );
}

template<class T>
VectorTpl<T> VectorTpl<T>::Norm() const
{
    T len = Len();
    if ( len == 0 )
        return VectorTpl();
    return VectorTpl(*this) /= Len();
}

template<class T>
T VectorTpl<T>::SumSquares() const
{
    T sumSquares = 0;
    for (CIt cit = this->begin(); cit != this->end(); ++cit )
        sumSquares += (*cit) * (*cit);

    return sumSquares;
}

template<class T>
T VectorTpl<T>::Mean() const
{
    if ( this->empty() )
        return 0;
    T mean = Sum() / (T) this->size();
    return mean;
}

template<class T>
VectorTpl<T> VectorTpl<T>::AdjustMean() const
{
    return operator - (Mean());
}

template<class T>
T VectorTpl<T>::Sum() const
{
    return std::accumulate(this->begin(), this->end(), 0.0);
}

template<class T>
VectorTpl<T> & VectorTpl<T>::operator += (const T f)
{
    for (It it = this->begin(); it != this->end(); ++it)
        (*it) += f;

    return *this;
}

template<class T>
VectorTpl<T> & VectorTpl<T>::operator -= (const T f)
{
    for (It it = this->begin(); it != this->end(); ++it)
        (*it) -= f;

    return *this;
}

template<class T>
VectorTpl<T> & VectorTpl<T>::operator /= (const T f)
{
    for (It it = this->begin(); it != this->end(); ++it)
        (*it) /= f;

    return *this;
}

template<class T>
VectorTpl<T> & VectorTpl<T>::operator *= (const T f)
{
    for (It it = this->begin(); it != this->end(); ++it)
        (*it) *= f;

    return *this;
}

template<class T>
VectorTpl<T> & VectorTpl<T>::operator += (const VectorTpl<T> &  p)
{
    Assertions().SizesEqual(*this, p, "VectorTpl::+=");
    for (size_t i = 0; i < this->size(); ++i)
        this->at(i) += p[i];

    return *this;
}

template<class T>
VectorTpl<T> & VectorTpl<T>::operator -= (const VectorTpl<T> &  p)
{
    Assertions().SizesEqual(*this, p, "VectorTpl::-=");
    for (size_t i = 0; i < this->size(); ++i)
        this->at(i) -= p[i];

    return *this;
}

template<class T>
VectorTpl<T> VectorTpl<T>::operator + (const VectorTpl<T> &  p) const
{
    Assertions().SizesEqual(*this, p, "VectorTpl::+");
    return VectorTpl(*this) += p;
}

template<class T>
VectorTpl<T> VectorTpl<T>::operator - (const VectorTpl<T> &  p) const
{
    Assertions().SizesEqual(*this, p, "VectorTpl::-");
    return VectorTpl(*this) -= p;
}

template<class T>
VectorTpl<T> VectorTpl<T>::operator - () const
{
    VectorTpl neg(*this);
    for (It it = neg.begin(); it != neg.end(); ++it)
        (*it) = -(*it);
    return neg;
}

template<class T>
VectorTpl<T> VectorTpl<T>::operator - (const T f) const
{
    return VectorTpl(*this) -= f;
}

template<class T>
VectorTpl<T> VectorTpl<T>::operator + (const T f) const
{
    return VectorTpl(*this) += f;
}

template<class T>
VectorTpl<T> VectorTpl<T>::operator * (const T f) const
{
    return VectorTpl(*this) *= f;
}

template<class T>
VectorTpl<T> VectorTpl<T>::operator / (const T f) const
{
    return VectorTpl(*this) /= f;
}

template<class T>
bool VectorTpl<T>::operator > (const VectorTpl<T> &  p) const
{
    return Len() > p.Len();
}

template<class T>
bool VectorTpl<T>::operator < (const VectorTpl<T> &  p) const
{
    return Len() < p.Len();
}
