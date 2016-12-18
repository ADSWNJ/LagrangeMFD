#include "Poly.hpp"

using namespace EnjoLib;

Poly::~Poly()
{
    //dtor
}

// ======================================================================
// Generally useful functor to evaluate a monic polynomial.
// For details, see class definition in brent.hpp
// Similarly, evaluate a general polynomial (not necessarily monic):
double Poly::operator()(double x)
{
    double rslt(0);
    for (int ii = coeff.size()-1; ii >= 0; ii--)
    {
        rslt *= x;
        rslt += coeff[ii];
    }
    return rslt;
}
