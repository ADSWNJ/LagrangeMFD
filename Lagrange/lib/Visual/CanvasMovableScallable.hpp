#ifndef CANVASMOVABLESCALLABLE_HPP_INCLUDED
#define CANVASMOVABLESCALLABLE_HPP_INCLUDED

#include "Canvas.hpp"

namespace EnjoLib
{
class CanvasMovableScallable : public Canvas
{
public:
    CanvasMovableScallable();
    virtual ~CanvasMovableScallable();

protected:
    virtual double GetScrollChange() const;

    virtual void ScaleMouseWheel(Point newPos, int wheelDelta);
    virtual void OnRightDown(Point newPos);
    virtual void OnRightUp();
    virtual void OnMove(Point newPos);

private:
    Point saveRefSystem( Point newPos );
    void loadRefSystem( Point newPos, Point savedRefSystem );

    Point m_storedPos;
    bool m_doMoveRefSystem;
};
}

#endif // CANVASMOVABLESCALLABLE_HPP_INCLUDED
