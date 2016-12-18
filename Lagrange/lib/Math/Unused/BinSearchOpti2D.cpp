#include "BinSearchOpti2D.hpp"
#include "BinSearchOpti2DSubject.hpp"
#include "GeneralMath.hpp"
#include <cmath>
#include <utility>

using namespace EnjoLib;

BinSearchOpti2D::BinSearchOpti2D(const Point & minArgs, const Point & maxArgs, double eps)
{
    this->m_minArgs = minArgs;
    this->m_maxArgs = maxArgs;
    this->m_eps = eps;
    // Bound binary seach should finish in log2(n) iterations. Let's allow for max 2 logs.
	GeneralMath gm;
    const double numSlices = gm.round( (maxArgs-minArgs).len() / eps);
    this->m_maxIter = 2 * gm.round(gm.Log2(numSlices));
}

BinSearchOpti2D::~BinSearchOpti2D()
{
}

Result<Point> BinSearchOpti2D::Run( BinSearchOpti2DSubject & subj ) const
{
//    GeneralMath gm;
    Point mid;
    double valMid;
    bool valMidDirty = true; // Caching
    int i = 0;
    Point a = m_minArgs;
    Point b = m_maxArgs;
    bool bmaxIter = false;
    do
    {   // Cut the argument in slices until the value (value) is below threshold (binary search)
        Point ab = Point(a.x, b.y);
        Point ba = Point(b.x, a.y);
        mid = (a + b) / 2; // Midpoint
        Point botLeft = (a + mid) / 2;
        Point topRigh = (b + mid) / 2;
        Point botRigh = (ba + mid) / 2;
        Point topLeft = (ab + mid) / 2;
        double valBotLeft = subj.UpdateGetValue(botLeft);
        double valTopRigh = subj.UpdateGetValue(topRigh);
        double valBotRigh = subj.UpdateGetValue(botRigh);
        double valTopLeft = subj.UpdateGetValue(topLeft);
        if (valMidDirty)
            valMid = subj.UpdateGetValue(mid);
        if (valBotLeft < valTopRigh && valBotLeft < valBotRigh && valBotLeft < valTopLeft && valBotLeft <= valMid)
        {
            b = mid; // Narrow right border
            valMidDirty = true;
        }
        else if (valTopRigh < valBotLeft && valTopRigh < valBotRigh && valTopRigh < valTopLeft && valTopRigh <= valMid)
        {
            a = mid; // Narrow left border
            valMidDirty = true;
        }
        else if (valBotRigh < valBotLeft && valBotRigh < valTopRigh && valBotRigh < valTopLeft && valBotRigh <= valMid)
        {
            a.x = mid.x;
            b.y = mid.y;
            valMidDirty = true;
        }
        else if (valTopLeft < valBotLeft && valTopLeft < valTopRigh && valTopLeft < valBotRigh && valTopLeft <= valMid)
        {
            b.x = mid.x;
            a.y = mid.y;
            valMidDirty = true;
        }
        else // Narrow both borders, because the answer is inside
		{
			a = botLeft;
			b = topRigh;
			// value of the middle point stays the same and doesn't need to be recalculated
		}
        bmaxIter = ++i == m_maxIter;
    } while( (b-a).len()/2 > m_eps && ! bmaxIter ); // Continue searching, until below threshold
    //sprintf(oapiDebugString(), "i = %d, maxi = %d, arg = %lf, value = %lf, pdiff = %lf",i, m_maxIter, arg, diff, prevDiff);
    if ( bmaxIter )
        return Result<Point>(m_maxArgs, false);
    else
        //return Result<double>(arg, subj.IsValid( arg, value ));
        return Result<Point>(mid, true);
}
