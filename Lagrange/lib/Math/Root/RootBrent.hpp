#ifndef ROOTBRENT_HPP
#define ROOTBRENT_HPP

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


#include "IRootAlgo.hpp"

namespace EnjoLib
{
class RootBrent : public IRootAlgo
{
    public:
        RootBrent(double minArg, double maxArg, double epsilon);
        virtual ~RootBrent();
        Result<double> Run( RootSubject & subj ) const;
        int GetNumIterations() const;
    protected:
    private:
        double r8_abs ( double x ) const;
        double r8_epsilon ( ) const;
        double r8_max ( double x, double y ) const;
        double r8_sign ( double x ) const;

                double m_minArg, m_maxArg, m_eps;
        bool m_maxArgValPositive;
        int m_maxIter;
        mutable int m_numIter;
};
}
#endif // ROOTBRENT_HPP
