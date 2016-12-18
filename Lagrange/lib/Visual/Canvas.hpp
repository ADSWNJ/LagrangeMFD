/*
Modified BSD License

This file is a part of Math package and originates from:
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
DISCLAIMED. IN NO EVENT SHALL SZYMON ENDER BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef CANVAS_HPP_INCLUDED
#define CANVAS_HPP_INCLUDED

#include <vector>
#include "../Systems/RectangleMy.hpp"
#include "../Util/NonCopyable.hpp"
#include "ScalePointFunctor.hpp"

namespace EnjoLib
{
class Canvas : public NonCopyable
{
public:
    Canvas( const Point & padding = Point() );
    virtual ~Canvas();
    virtual void Center();
    virtual void AutoZoom();
    const double & GetZoom() const;
    const Point & GetRef() const;
    Point GetPointNatural( const Point & projected ) const;
    Point GetPointNatural( int x, int y ) const;
    Point GetPointProjected( const Point & natural ) const;
    Point GetPointProjected( int x, int y ) const;

    virtual Point GetWindowSize() const = 0;
    virtual RectangleMy GetDataSize() const = 0;
    virtual RectangleMy GetDataToDisplaySize() const = 0;


protected:
    void ScalePointVector(const std::vector<Point> & in, std::vector<Point> * out ) const;
    Point ScalePoint( const Point & p ) const;
    double CalculateAutoZoomValue() const;

    virtual Point GetPadding() const;
    virtual bool IsSymmetricAround00() const = 0;
    virtual void RefreshClient() = 0;
    /// Insert a value of 1 to one of the axes, to disable scaling in the axis

    virtual void ScaleData() = 0;

    double m_zoom;
    Point m_refSystem;
    ScalePointFunctor m_scalePointFunctor;

private:

};
}

#endif // CANVAS_HPP_INCLUDED
