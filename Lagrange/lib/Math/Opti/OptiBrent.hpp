#ifndef BRENTOPTI_H
#define BRENTOPTI_H

#include "IOptiAlgo.hpp"

namespace EnjoLib
{
    class OptiBrent : public IOptiAlgo
    {
        public:
            OptiBrent(double minArg, double maxArg, double eps);
            virtual ~OptiBrent();

            Result<double> Run( OptiSubject & subj ) const;

        protected:
        private:

            double m_minArg, m_maxArg, m_eps;
            bool m_maxArgValPositive;
            int m_maxIter;
    };
}

#endif // BRENTOPTI_H
