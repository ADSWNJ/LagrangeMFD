#include "RootBrent.hpp"
#include <algorithm>
#include "../GeneralMath.hpp"
#include "RootSubject.hpp"

using namespace EnjoLib;
using namespace std;

RootBrent::RootBrent(double minArg, double maxArg, double eps)
{
    if (minArg > maxArg)
        std::swap(minArg, maxArg);

    this->m_minArg = minArg;
    this->m_maxArg = maxArg;
    this->m_eps = eps;
    this->m_maxIter = GeneralMath().GetMaxIterBinSearchBound(minArg, maxArg, eps);
}

RootBrent::~RootBrent()
{
    //dtor
}

//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    17 July 2011
//
//  Author:
//
//    Original FORTRAN77 version by Richard Brent.
//    C++ version by John Burkardt. https://people.sc.fsu.edu/~jburkardt/cpp_src/brent/brent.html
//    Further C++ modifications by Szymon Ender "Enjo". www.enderspace.de
Result<double> RootBrent::Run( RootSubject & subj ) const
{
    GeneralMath gm;
    const double refValue = subj.GetRefValue();
    double p;
    double q;
    double r;

    double a = m_minArg;
    double b = m_maxArg;
    double faTemp = subj.UpdateGetValue( a ) - refValue;
    if (gm.sign(faTemp) == 0)
    {
        a = (a+b) / 2.0;
    }
//
//  Make local copies of A and B.
//
    double sa = a;
    double sb = b;
    double fa = subj.UpdateGetValue( sa ) - refValue;
    double fb = subj.UpdateGetValue( sb ) - refValue;

    double c = sa;
    double fc = fa;
    double e = sb - sa;
    double d = e;

    double macheps = r8_epsilon ( );
    int i = 0;
    bool bmaxIter = false;
    do
    {
        if ( r8_abs ( fc ) < r8_abs ( fb ) )
        {
            sa = sb;
            sb = c;
            c = sa;
            fa = fb;
            fb = fc;
            fc = fa;
        }

        double tol = 2.0 * macheps * r8_abs ( sb ) + m_eps;
        double m = 0.5 * ( c - sb );

        if ( r8_abs ( m ) <= tol || fb == 0.0 )
        {
            break;
        }

        if ( r8_abs ( e ) < tol || r8_abs ( fa ) <= r8_abs ( fb ) )
        {
            e = m;
            d = e;
        }
        else
        {
            double s = fb / fa;

            if ( sa == c )
            {
                p = 2.0 * m * s;
                q = 1.0 - s;
            }
            else
            {
                q = fa / fc;
                r = fb / fc;
                p = s * ( 2.0 * m * q * ( q - r ) - ( sb - sa ) * ( r - 1.0 ) );
                q = ( q - 1.0 ) * ( r - 1.0 ) * ( s - 1.0 );
            }

            if ( 0.0 < p )
            {
                q = - q;
            }
            else
            {
                p = - p;
            }

            s = e;
            e = d;

            if ( 2.0 * p < 3.0 * m * q - r8_abs ( tol * q ) &&
                    p < r8_abs ( 0.5 * s * q ) )
            {
                d = p / q;
            }
            else
            {
                e = m;
                d = e;
            }
        }
        sa = sb;
        fa = fb;

        if ( tol < r8_abs ( d ) )
        {
            sb = sb + d;
        }
        else if ( 0.0 < m )
        {
            sb = sb + tol;
        }
        else
        {
            sb = sb - tol;
        }

        fb = subj.UpdateGetValue( sb ) - refValue;

        if ( ( 0.0 < fb && 0.0 < fc ) || ( fb <= 0.0 && fc <= 0.0 ) )
        {
            c = sa;
            fc = fa;
            e = sb - sa;
            d = e;
        }
        bmaxIter = ++i == m_maxIter;
    } while( ! bmaxIter );   // Continue searching, until below threshold
    m_numIter = i;
    if ( bmaxIter )
        return Result<double>(m_maxArg, false);
    else
        return Result<double>(sb, subj.IsValid( sb, fb ));
}

int RootBrent::GetNumIterations() const
{
    return m_numIter;
}


double RootBrent::r8_abs ( double x ) const
{
    double value;

    if ( 0.0 <= x )
    {
        value = x;
    }
    else
    {
        value = - x;
    }
    return value;
}
//****************************************************************************80

double RootBrent::r8_epsilon ( ) const
{
    const double value = 2.220446049250313E-016;

    return value;
}
//****************************************************************************80

double RootBrent::r8_max ( double x, double y ) const
{
    double value;

    if ( y < x )
    {
        value = x;
    }
    else
    {
        value = y;
    }
    return value;
}
//****************************************************************************80

double RootBrent::r8_sign ( double x ) const
{
    double value;

    if ( x < 0.0 )
    {
        value = -1.0;
    }
    else
    {
        value = 1.0;
    }
    return value;
}
