/*
Modified BSD License

This file is a part of Systems package and originates from:
http://sf.net/projects/enjomitchsorbit

Copyright (c) 2011, Szymon "Enjo" Ender
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

#ifndef VECT3_HPP_INCLUDED
#define VECT3_HPP_INCLUDED

namespace EnjoLib
{
struct Vect3
{
    //! Default Ctor
    Vect3();
    //! Ctor
    /*!
      \param x coord
      \param y coord
      \param z coord
      */
    Vect3(double x, double y, double z);

    //! Length of vector
    double len() const;
    //! Normalised copy of vector
    Vect3 norm() const;

    Vect3 & operator = (const Vect3 & par);
    Vect3 & operator += (const Vect3 & par);
    Vect3 & operator -= (const Vect3 & par);
    Vect3 & operator /= (const double par);
    Vect3 & operator *= (const double par);

    Vect3 operator + (const Vect3 & par) const;
    Vect3 operator - (const Vect3 & par) const;
    Vect3 operator - () const;
    Vect3 operator * (const double f) const;
    Vect3 operator / (const double f) const;
    bool operator > (const Vect3 & par) const;
    bool operator < (const Vect3 & par) const;

    //! x coord
    double x;
    //! y coord
    double y;
    //! z coord
    double z;
};
}

#endif // VECT3_HPP_INCLUDED
