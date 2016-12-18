#ifndef SOLVERQUARTIC_H
#define SOLVERQUARTIC_H

#include <complex>
#include <array>

class SolverQuartic
{
    public:
        SolverQuartic();
        virtual ~SolverQuartic();

        typedef std::array<std::complex<double>, 4> Roots;
        Roots Run(double a, double b, double c, double d, double e);

    protected:
    private:
};

#endif // SOLVERQUARTIC_H
