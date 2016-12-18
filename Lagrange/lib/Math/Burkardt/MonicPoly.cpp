#include "MonicPoly.hpp"

using namespace EnjoLib;

MonicPoly::~MonicPoly()
{
    //dtor
}

// ======================================================================
// Generally useful functor to evaluate a monic polynomial.
// For details, see class definition in brent.hpp
double MonicPoly::operator()(double x)
{
    double rslt(1);
    for (int ii = coeff.size()-1; ii >= 0; ii--)
    {
        rslt *= x;
        rslt += coeff[ii];
    }
    return rslt;
}

