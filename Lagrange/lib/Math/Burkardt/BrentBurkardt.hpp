#ifndef BRENTBURKARDT_H
#define BRENTBURKARDT_H

#include <vector>
#include "../../Util/Result.hpp"

namespace EnjoLib {

class FuncBase;
class OptiSubject;

class BrentBurkardt
{
    public:
        double glomin ( double a, double b, double c, double m, double e, double t,
          FuncBase & f, double &x );
        Result<double> local_min ( double a, double b, double t, int maxIter, OptiSubject & f,
          double &x );
        double local_min_rc ( double &a, double &b, int &status, double value );
        double r8_abs ( double x );
        double r8_epsilon ( );
        double r8_max ( double x, double y );
        double r8_sign ( double x );
        void timestamp ( );
        double zero ( double a, double b, double t, FuncBase & f );
        void zero_rc ( double a, double b, double t, double &arg, int &status,
          double value );

        // === simple wrapper functions
        // === for convenience and/or compatibility
        double glomin ( double a, double b, double c, double m, double e, double t,
          double f ( double x ), double &x );
        double local_min ( double a, double b, double t, double f ( double x ),
          double &x );
        double zero ( double a, double b, double t, double f ( double x ) );
};

}

#endif // BRENTBURKARDT_H
