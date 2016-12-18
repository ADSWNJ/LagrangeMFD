#ifndef POLY_H
#define POLY_H

#include "FuncBase.hpp"
#include <vector>

namespace EnjoLib {

class Poly : public FuncBase
{
public:

    virtual ~Poly();

    // constructors:
    Poly(const std::size_t degree)
        : coeff(1+degree) {}
    Poly(const std::vector<double>& v)
        : coeff(v) {}
    Poly(const double* c, std::size_t degree)
        : coeff(std::vector<double>(c, 1+c+degree)) {}

    virtual double operator() (double x);

protected:
private:
    std::vector<double> coeff;    // a vector of size nterms i.e. 1+degree
};
}

#endif // POLY_H
