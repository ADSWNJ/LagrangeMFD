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

#include <cmath>
#include "Vect3.hpp"
using namespace EnjoLib;

Vect3::Vect3()
{
    x = y = z = 0;
}

Vect3::Vect3(double x, double y, double z)
{
    this->x = x;
    this->y = y;
    this->z = z;
}

double Vect3::len() const
{
    return sqrt(x * x + y * y + z * z );
}

Vect3 Vect3::norm() const
{
    return Vect3(*this) /= len();
}

Vect3 & Vect3::operator= (const Vect3 & p)
{
    if (this == & p)      // Same object?
        return *this;        // Yes, so skip assignment, and just return *this.
    else
    {
        // Deallocate, allocate new space, copy values...
        x = p.x;
        y = p.y;
        z = p.z;
        return *this;
    }
}

Vect3 & Vect3::operator /= (const double f)
{
    x /= f;
    y /= f;
    z /= f;
    return *this;
}

Vect3 & Vect3::operator *= (const double f)
{
    x *= f;
    y *= f;
    z *= f;
    return *this;
}

Vect3 & Vect3::operator += (const Vect3 & p)
{
    x += p.x;
    y += p.y;
    z += p.z;
    return *this;
}

Vect3 & Vect3::operator -= (const Vect3 & p)
{
    x -= p.x;
    y -= p.y;
    z -= p.z;
    return *this;
}

Vect3 Vect3::operator + (const Vect3 & p) const
{
    return Vect3(*this) += p;
}

Vect3 Vect3::operator - (const Vect3 & p) const
{
    return Vect3(*this) -= p;
}

Vect3 Vect3::operator - () const
{
    Vect3 neg;
    neg.x = -x;
    neg.y = -y;
    neg.z = -z;
    return neg;
}

Vect3 Vect3::operator * (const double f) const
{
    return Vect3(*this) *= f;
}

Vect3 Vect3::operator / (const double f) const
{
    return Vect3(*this) /= f;
}

bool Vect3::operator > (const Vect3 & p) const
{
    return len() > p.len();
}

bool Vect3::operator < (const Vect3 & p) const
{
    return len() < p.len();
}
