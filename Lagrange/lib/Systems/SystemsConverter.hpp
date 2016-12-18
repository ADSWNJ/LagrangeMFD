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

#ifndef SYSTEMSCONVERTER_HPP_INCLUDED
#define SYSTEMSCONVERTER_HPP_INCLUDED

#include "Spherical.hpp"
namespace EnjoLib
{
struct Point;
struct Vect3;
struct Spherical;
struct Geo;

//! Converts between coordinate systems
/*!
Converts between coordinate systems given a specific input
*/
class SystemsConverter
{
public:
    //! Ctor
    /*!
    Gets input
    */
    SystemsConverter( const Vect3 & vector );
    //! Ctor
    /*!
    Gets input
    */
    SystemsConverter( const Spherical & sph );
    //! Ctor
    /*!
    Gets input
    */
    SystemsConverter( const Geo & geo, double radius );
    //! Ctor
    /*!
    Gets input
    */
    SystemsConverter( const Point & point, double radius );

    Vect3 GetVector() const;
    Spherical GetSpherical() const;
    Geo GetGeo() const;
    Point GetPoint() const;

private:
    Point Spherical2Point( const Spherical & sph ) const;
    Vect3 Spherical2Cartesian( const Spherical & sph ) const;
    Spherical Cartesian2Spherical( const Vect3 & cart ) const;
    Geo Spherical2Geo( const Spherical & sph ) const;
    Spherical Geo2Spherical( const Geo & geo, double radius ) const;
    Spherical Point2Spherical( const Point & point, double radius ) const;

    Spherical m_sph;
};
}

#endif // SYSTEMSCONVERTER_HPP_INCLUDED
