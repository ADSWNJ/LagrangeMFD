#include "OptiBrent.hpp"
#include "OptiSubject.hpp"
#include "../GeneralMath.hpp"
#include <cmath>
#include <utility>
#include "../Burkardt/BrentBurkardt.hpp"

using namespace EnjoLib;

OptiBrent::OptiBrent(double minArg, double maxArg, double eps)
{
    if (minArg > maxArg)
        std::swap(minArg, maxArg);

    this->m_minArg = minArg;
    this->m_maxArg = maxArg;
    this->m_eps = eps;
    this->m_maxIter = GeneralMath().GetMaxIterBinSearchBound(minArg, maxArg, eps);
}

OptiBrent::~OptiBrent(){}

Result<double> OptiBrent::Run( OptiSubject & subj ) const
{
    BrentBurkardt bkrd;
    double y = 0;
    return bkrd.local_min(m_minArg, m_maxArg, m_eps, m_maxIter, subj, y);
}
