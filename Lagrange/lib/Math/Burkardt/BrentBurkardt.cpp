#include "BrentBurkardt.hpp"

#include <cmath>
#include "FuncBase.hpp"
#include "../Opti/OptiSubject.hpp"


using namespace EnjoLib;
using namespace std;

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
Result<double> BrentBurkardt::local_min ( double a, double b, double t, int maxIter, OptiSubject & f,
                                  double &fx )
{
    double d = 0;
//
//  C is the square of the inverse of the golden ratio.
//
    double c = 0.5 * ( 3.0 - std::sqrt ( 5.0 ) );

    double eps = std::sqrt ( r8_epsilon ( ) );

    double sa = a;
    double sb = b;
    double x = sa + c * ( b - a );
    double w = x;
    double v = w;
    double e = sb - x;
    fx = f ( x );
    double fw = fx;
    double fv = fw;

    for (int i = 0; i <= maxIter; ++i)
    {
        if (i == maxIter)
        {
            return Result<double>(0, false);
        }
        double m = 0.5 * ( sa + sb ) ;
        double tol = eps * r8_abs ( x ) + t;
        double t2 = 2.0 * tol;
//
//  Check the stopping criterion.
//
        if ( r8_abs ( x - m ) <= t2 - 0.5 * ( sb - sa ) )
        //if ( r8_abs (sb - sa) <= t )
        {
            break;
        }
//
//  Fit a parabola.
//
        double r = 0.0;
        double q = r;
        double p = q;

        if ( tol < r8_abs ( e ) )
        {
            r = ( x - w ) * ( fx - fv );
            q = ( x - v ) * ( fx - fw );
            p = ( x - v ) * q - ( x - w ) * r;
            q = 2.0 * ( q - r );
            if ( 0.0 < q )
            {
                p = - p;
            }
            q = r8_abs ( q );
            r = e;
            e = d;
        }
        double u;
        if ( r8_abs ( p ) < r8_abs ( 0.5 * q * r ) &&
                q * ( sa - x ) < p &&
                p < q * ( sb - x ) )
        {
//
//  Take the parabolic interpolation step.
//
            d = p / q;
            u = x + d;
//
//  F must not be evaluated too close to A or B.
//
            if ( ( u - sa ) < t2 || ( sb - u ) < t2 )
            {
                if ( x < m )
                    d = tol;
                else
                    d = - tol;
            }
        }
//
//  A golden-section step.
//
        else
        {
            if ( x < m )
                e = sb - x;
            else
                e = sa - x;
            d = c * e;
        }
//
//  F must not be evaluated too close to X.
//
        if ( tol <= r8_abs ( d ) )
        {
            u = x + d;
        }
        else if ( 0.0 < d )
        {
            u = x + tol;
        }
        else
        {
            u = x - tol;
        }

        double fu = f ( u );
//
//  Update A, B, V, W, and X.
//
        if ( fu <= fx )
        {
            if ( u < x )
                sb = x;
            else
                sa = x;
            v = w;
            fv = fw;
            w = x;
            fw = fx;
            x = u;
            fx = fu;
        }
        else
        {
            if ( u < x )
                sa = u;
            else
                sb = u;

            if ( fu <= fw || w == x )
            {
                v = w;
                fv = fw;
                w = u;
                fw = fu;
            }
            else if ( fu <= fv || v == x || v == w )
            {
                v = u;
                fv = fu;
            }
        }
    }
    return Result<double>(x, true);
}


double BrentBurkardt::r8_abs ( double x )
{
  return std::fabs(x);
}
//****************************************************************************80

double BrentBurkardt::r8_epsilon ( )
{
  const double value = 2.220446049250313E-016;

  return value;
}

// ======================================================================
// === Simple wrapper functions
// === for convenience and/or compatibility.
//
// === The three functions are the same as above,
// === except that they take a plain function F
// === instead of a c++ functor.  In all cases, the
// === input and output of F() are of type double.

typedef double DoubleOfDouble (double);

class func_wrapper : public FuncBase {
  DoubleOfDouble* func;
public:
  func_wrapper(DoubleOfDouble* f) {
    func = f;
  }
  virtual double operator() (double x){
    return func(x);
  }
};

//****************************************************************************80
/*
double glomin ( double a, double b, double c, double m, double e,
         double t, double f ( double x ), double &x ){
  func_wrapper foo(f);
  return glomin(a, b, c, m, e, t, foo, x);
}
*/
//****************************************************************************80
/*
double BrentBurkardt::local_min ( double a, double b, double t, double f ( double x ),
  double &x ){
  func_wrapper foo(f);
  return local_min(a, b, t, foo, x);
}
*/
//****************************************************************************80
/*
double zero ( double a, double b, double t, double f ( double x ) ){
  func_wrapper foo(f);
  return zero(a, b, t, foo);
}
*/
