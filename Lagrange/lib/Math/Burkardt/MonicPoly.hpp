#ifndef MONICPOLY_H
#define MONICPOLY_H

#include "FuncBase.hpp"
#include <vector>

namespace EnjoLib {

class MonicPoly : public FuncBase
{
public:
    virtual ~MonicPoly();
    // constructors:
    MonicPoly(const std::size_t degree)
        : coeff(degree) {}
    MonicPoly(const std::vector<double>& v)
        : coeff(v) {}
    MonicPoly(const double* c, std::size_t degree)
        : coeff(std::vector<double>(c, c+degree)) {}

    virtual double operator() (double x);

private:
    std::vector<double> coeff;
};
}

#endif // MONICPOLY_H
