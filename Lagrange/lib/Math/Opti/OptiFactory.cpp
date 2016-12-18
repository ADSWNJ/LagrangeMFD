#include "OptiFactory.hpp"
#include <stdexcept>
#include "OptiBinSearch.hpp"
#include "OptiBrent.hpp"

using namespace EnjoLib;

OptiFactory::OptiFactory(){}
OptiFactory::~OptiFactory(){}

std::unique_ptr<IOptiAlgo> OptiFactory::Create(OptiType type, double minArg, double maxArg, double eps)
{
    switch (type)
    {
    case OPTI_BIN_SEARCH:
        return std::unique_ptr<IOptiAlgo>(new OptiBinSearch(minArg, maxArg, eps));
    case OPTI_BRENT:
        return std::unique_ptr<IOptiAlgo>(new OptiBrent(minArg, maxArg, eps));
    }

    throw std::invalid_argument("Not handled OptiType");
}
