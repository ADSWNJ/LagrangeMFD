#ifndef IOPTIALGO_H
#define IOPTIALGO_H

#include "../../Util/Result.hpp"

namespace EnjoLib
{
    class OptiSubject;

    class IOptiAlgo
    {
        public:
            IOptiAlgo();
            virtual ~IOptiAlgo();
            virtual Result<double> Run( OptiSubject & subj ) const = 0;

        protected:
        private:
    };
}

#endif // IOPTIALGO_H
