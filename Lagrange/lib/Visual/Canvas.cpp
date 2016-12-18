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

#include <algorithm>
#include "Canvas.hpp"
#include "../Math/GeneralMath.hpp"

using namespace EnjoLib;

Canvas::Canvas( const Point & padding )
    : m_scalePointFunctor(m_zoom)
{
    m_zoom = 1;
}

Canvas::~Canvas()
{
}

const double & Canvas::GetZoom() const
{
    return m_zoom;
}

const Point & Canvas::GetRef() const
{
    return m_refSystem;
}

void Canvas::Center()
{
    if ( IsSymmetricAround00() )
    {
        m_refSystem = - GetWindowSize() / 2;
    }
    else
    {
        const RectangleMy & rectangleData = GetDataToDisplaySize();
        m_refSystem = rectangleData.GetTopLeft() * m_zoom;
        const Point & remainingWindowSize = GetWindowSize() - GetPadding();
        if (remainingWindowSize.x > 0 && remainingWindowSize.y > 0)
            m_refSystem -= GetPadding() / 2;
    }
}

Point Canvas::GetPadding() const
{
    return Point();
}

void Canvas::AutoZoom()
{
    m_zoom = CalculateAutoZoomValue();
    ScaleData();
    Center();
    RefreshClient();
}

double Canvas::CalculateAutoZoomValue() const
{
    const RectangleMy & rectangleData = GetDataToDisplaySize();
    const Point & dataSize = rectangleData.GetSize();
    if ( dataSize.x <= 0 || dataSize.y <= 0 )
        // Invalid data. Return zoom it as it was before
        return m_zoom;

    Point remainingWindowSize = GetWindowSize() - GetPadding();
    if (remainingWindowSize.x <= 0 || remainingWindowSize.y <= 0)
        remainingWindowSize = GetWindowSize(); // Padding was greater than the window. Ignore it.
    // Get ratios of window (available size) vs data to display size
    const double ratioX = remainingWindowSize.x / dataSize.x;
    const double ratioY = remainingWindowSize.y / dataSize.y;
    const double min = std::min( ratioX, ratioY );
    // Do not exceed the shorter dimension ratio
    return min;
    //return ratioY;
}

void Canvas::ScalePointVector(const std::vector<Point> & in, std::vector<Point> * out ) const
{
    if ( in.size() != out->size() )
    {
        // Ensure that sizes of the vector is equal
        *out = std::vector<Point>(in.size());
    }
    std::transform( in.begin(), in.end(), (*out).begin(), m_scalePointFunctor );
}

Point Canvas::ScalePoint( const Point & p ) const
{
    return m_scalePointFunctor( p );
}

Point Canvas::GetPointNatural( const Point & projected ) const
{
    const Point & natural = (projected + m_refSystem) / m_zoom;
    return natural;
}

Point Canvas::GetPointNatural( int x, int y ) const
{
    const Point & natural = GetPointNatural( Point(x,y) );
    return natural;
}

Point Canvas::GetPointProjected( const Point & natural ) const
{
    Point projected = natural * m_zoom - m_refSystem;
    projected.x = GeneralMath().round( projected.x );
    projected.y = GeneralMath().round( projected.y );
    return projected;
}

Point Canvas::GetPointProjected( int x, int y ) const
{
    const Point & natural = GetPointNatural( GetPointProjected( Point(x,y) ) );
    return natural;
}

