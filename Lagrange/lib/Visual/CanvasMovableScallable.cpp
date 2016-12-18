#include "CanvasMovableScallable.hpp"
#include "../Math/GeneralMath.hpp"

using namespace EnjoLib;

CanvasMovableScallable::CanvasMovableScallable()
{
    m_doMoveRefSystem = false;
}

CanvasMovableScallable::~CanvasMovableScallable()
{

}

double CanvasMovableScallable::GetScrollChange() const
{
    return 0.05;
}

void CanvasMovableScallable::ScaleMouseWheel( Point newPos, int wheelDelta )
{
    const double change = GetScrollChange();
    const Point & savedRefSystem = saveRefSystem( newPos );
    m_zoom *= (1 + (wheelDelta > 0 ? change : -change));
    loadRefSystem( newPos, savedRefSystem );
    ScaleData();
    RefreshClient();
}

Point CanvasMovableScallable::saveRefSystem( Point newPos )
{
    const double x = (m_refSystem.x + newPos.x) / m_zoom;
    const double y = (m_refSystem.y + newPos.y) / m_zoom;

    return Point(x, y);
}

void CanvasMovableScallable::loadRefSystem( Point newPos, Point savedRefSystem )
{
	GeneralMath gm;
	int x = (int)(gm.round(savedRefSystem.x * m_zoom) - newPos.x);
    int y = (int)(gm.round(savedRefSystem.y * m_zoom) - newPos.y);

    m_refSystem.x = x;
    m_refSystem.y = y;
}


void CanvasMovableScallable::OnRightDown( Point newPos )
{
    m_doMoveRefSystem = true;
    m_storedPos = newPos;
}

void CanvasMovableScallable::OnRightUp()
{
    m_doMoveRefSystem = false;
}

void CanvasMovableScallable::OnMove( Point newPos )
{
    if ( m_doMoveRefSystem )
    {
        m_refSystem.x += (m_storedPos.x - newPos.x);
        m_refSystem.y += (m_storedPos.y - newPos.y);
        m_storedPos = newPos;
        RefreshClient();
    }
}
