#include "OptiMultiNelderMead.hpp"
#include "../Burkardt/NelderMeadBurkardt.hpp"
#include "OptiMultiSubject.hpp"
//#include "../../Statistical/Assertions.hpp"
//#include "../../Statistical/VectorTpl.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>

using namespace std;
using namespace EnjoLib;

OptiMultiNelderMead::OptiMultiNelderMead()
{
    //ctor
}

OptiMultiNelderMead::~OptiMultiNelderMead(){}

Result<std::vector<double> > OptiMultiNelderMead::Run( OptiMultiSubject & subj, double eps, int konvge, int kcount ) const
{
    bool verbose = false;
    //verbose = true;
    std::vector<double> xret;

    const std::vector<double> sta = subj.GetStart();
    const std::vector<double> ste = subj.GetStep();

    //Assertions().SizesEqual(EnjoLib::VectorTpl<double>(sta), EnjoLib::VectorTpl<double>(ste), "start, step");

    // TODO: assertions!
    int i;
    int icount;
    int ifault;
    int n;
    int numres;
    double *start;
    double *step;
    double *xmin;
    double ynewlo;

    n = sta.size();

    start = new double[n];
    step = new double[n];
    xmin = new double[n];

    for (i = 0; i < n; ++i)
    {
       start[i] = sta[i];
       step[i] = ste[i];
    }

    ynewlo = subj.Get( start, n );

    ostringstream oss;
    if (verbose)
    {
        oss << "\n";
        oss << "TEST01\n";
        oss << "  Apply NELMIN to ROSENBROCK function.\n";


        oss << "\n";
        oss << "  Starting point X:\n";
        oss << "\n";
        for ( i = 0; i < n; i++ )
        {
            oss << "  " << setw(14) << start[i] << "\n";
        }
        oss << "\n";
        oss << "  F(X) = " << ynewlo << "\n";
    }

    NelderMeadBurkardt::nelmin ( subj, n, start, xmin, &ynewlo, eps, step,
                                 konvge, kcount, &icount, &numres, &ifault );

    if (verbose)
    {
        oss << "\n";
        oss << "  Return code IFAULT = " << ifault << "\n";
        oss << "\n";
        oss << "  Estimate of minimizing value X*:\n";
        oss << "\n";
        for ( i = 0; i < n; i++ )
        {
            oss << "  " << setw(14) << xmin[i] << "\n";
        }

        oss << "\n";
        oss << "  F(X*) = " << ynewlo << "\n";

        oss << "\n";
        oss << "  Number of iterations = " << icount << "\n";
        oss << "  Number of restarts =   " << numres << "\n";
        cout << oss.str();
    }
    delete [] start;
    delete [] step;


    for ( i = 0; i < n; i++ )
    {
        xret.push_back(xmin[i]);
    }
    Result<std::vector<double> > ret(xret, ifault == 0);

    delete [] xmin;
    return ret;
}
