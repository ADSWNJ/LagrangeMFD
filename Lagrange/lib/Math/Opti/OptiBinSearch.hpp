#ifndef BINSEARCHOPTI_H
#define BINSEARCHOPTI_H

#include "../../Util/Result.hpp"
#include "IOptiAlgo.hpp"

namespace EnjoLib
{
    class OptiSubject;
    class OptiBinSearch : public IOptiAlgo
    {
        public:
            OptiBinSearch(double minArg, double maxArg, double eps);
            virtual ~OptiBinSearch();
            Result<double> Run( OptiSubject & subj ) const;
        protected:
        private:

            double m_minArg, m_maxArg, m_eps;
            bool m_maxArgValPositive;
            int m_maxIter;
    };
}
#endif // BINSEARCHOPTI_H
