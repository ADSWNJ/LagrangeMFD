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

#include "RectangleMy.hpp"
using namespace EnjoLib;

RectangleMy::RectangleMy( const Point & topLeft, const Point & bottomRight )
    : m_topLeft(topLeft)
    , m_bottomRight(bottomRight)
{
    if ( bottomRight.x < topLeft.x )
        m_bottomRight.x = topLeft.x;

    if ( bottomRight.y < topLeft.y )
        m_bottomRight.y = topLeft.y;
}

RectangleMy::RectangleMy( const Point & pos, int width )
{
    double w2 = width/2.0;
    double h2 = width/2.0;
    m_topLeft = Point( pos.x - w2, pos.y - h2 );
    m_bottomRight = Point( pos.x + w2, pos.y + h2 );
}

RectangleMy::~RectangleMy()
{
}

Point RectangleMy::GetSize() const
{
    return Point( m_bottomRight.x - m_topLeft.x, m_bottomRight.y - m_topLeft.y );
}

Point RectangleMy::GetPos() const
{
    Point sz = GetSize();

    return m_topLeft + sz / 2.0;
}

void RectangleMy::SetPos( const Point & pos )
{
    double w2 = GetSize().x/2.0;
    double h2 = GetSize().y/2.0;

    m_topLeft = Point( pos.x - w2, pos.y - h2 );
    m_bottomRight = Point( pos.x + w2, pos.y + h2 );
}

const Point & RectangleMy::GetTopLeft() const
{
    return m_topLeft;
}
const Point & RectangleMy::GetBottomRight() const
{
    return m_bottomRight;
}
Point RectangleMy::GetBottomLeft() const
{
    return Point( m_topLeft.x, m_bottomRight.y );
}
Point RectangleMy::GetTopRight() const
{
    return Point( m_bottomRight.x, m_topLeft.y );
}

void RectangleMy::SetTopLeft(const Point & p)
{
    m_topLeft = p;
}

void RectangleMy::SetBottomRight(const Point & p)
{
    m_bottomRight = p;
}

void RectangleMy::SetSize(int w, int h)
{
    double w2 = w/2.0;
    double h2 = h/2.0;

    Point pos = GetPos();

    m_topLeft = Point( pos.x - w2, pos.y - h2 );
    m_bottomRight = Point( pos.x + w2, pos.y + h2 );
}
