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

#ifndef VECTORMATH_HPP_INCLUDED
#define VECTORMATH_HPP_INCLUDED

namespace EnjoLib
{
struct Vect3;
struct Point;
class VectorMath
{
    public:
         //! Dot product
        /*! Calculates dot product of two vectors
          \param a 1st vector
          \param a 2nd vector
          \return dot product
        */
        double dot(const Vect3 & a, const Vect3 & b );

        //! Cross product
        /*! Calculates cross product of two vectors
          \param a 1st vector
          \param a 2nd vector
          \return cross product
        */
        Vect3 cross(const Vect3 & a, const Vect3 & b );

        //! Angle between vectors
        /*! Calculates angle between two vectors
          \param a 1st vector
          \param a 2nd vector
          \return angle between vectors
        */
        double angle(const Vect3 & a, const Vect3 & b );

		double angle(const Point & a, const Point & b );

        Point Cast3Donto2D(const Vect3 & in3D, const Vect3 & v1, const Vect3 & v2);
        Vect3 Cast2Donto3D(const Point & in2D, const Vect3 & v1, const Vect3 & v2);
};
}

#endif // VECTORMATH_HPP_INCLUDED
