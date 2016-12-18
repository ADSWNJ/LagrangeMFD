/*
Modified BSD License

This file is a part of Math package and originates from:
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
#include "VectorMath.hpp"
#include "../Systems/Vect3.hpp"
#include "../Systems/Point.hpp"
using namespace EnjoLib;

double VectorMath::dot(const Vect3 & a, const Vect3 & b )
{
  return a.x*b.x + b.y*a.y + a.z*b.z;
}

Vect3 VectorMath::cross(const Vect3 & a, const Vect3 & b )
{
  Vect3 v;
  v.x = a.y*b.z - b.y*a.z;
  v.y = a.z*b.x - b.z*a.x;
  v.z = a.x*b.y - b.x*a.y;
  return v;
}

double VectorMath::angle(const Vect3 & a, const Vect3 & b )
{
    const Vect3 & aNorm = a.norm();
    const Vect3 & bNorm = b.norm();

    return atan2( cross(aNorm, bNorm).len(), dot(aNorm, bNorm) );
}

double VectorMath::angle(const Point & a, const Point & b )
{
	return atan2(a.y - b.y, a.x - b.x);
}

Point VectorMath::Cast3Donto2D(const Vect3 & in3D, const Vect3 & v1, const Vect3 & v2)
{
    Point ret2D;
    ret2D.x = dot(v1, in3D);
    ret2D.y = dot(v2, in3D);
    return ret2D;
}

Vect3 VectorMath::Cast2Donto3D(const Point & in2D, const Vect3 & v1, const Vect3 & v2)
{
    Vect3 ret3D;
    ret3D.x = v1.x * in2D.x + v2.x * in2D.y;
    ret3D.y = v1.y * in2D.x + v2.y * in2D.y;
    ret3D.z = v1.z * in2D.x + v2.z * in2D.y;
    return ret3D;
}
