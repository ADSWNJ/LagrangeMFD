#ifndef OPTIFACTORY_H
#define OPTIFACTORY_H

#include <memory>
#include "IOptiAlgo.hpp"
#include "OptiType.hpp"

#include <Util/AutoPtr.hpp>

namespace EnjoLib
{
    class OptiFactory
    {
        public:
            OptiFactory();
            virtual ~OptiFactory();

            static std::unique_ptr<IOptiAlgo> Create(OptiType type, double minArg, double maxArg, double eps);

        protected:
        private:
    };
}

#endif // OPTIFACTORY_H
