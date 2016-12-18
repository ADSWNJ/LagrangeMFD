#ifndef OPTIMULTINELDERMEAD_H
#define OPTIMULTINELDERMEAD_H

#include <vector>
#include "../../Util/Result.hpp"

namespace EnjoLib
{
    class OptiMultiSubject;

    class OptiMultiNelderMead
    {
        public:
            OptiMultiNelderMead();
            virtual ~OptiMultiNelderMead();

            Result<std::vector<double> > Run( OptiMultiSubject & subj, double eps = 0.00001, int konvge = 10, int kcount = 500 ) const;
        protected:
        private:
    };
}

#endif // OPTIMULTINELDERMEAD_H
