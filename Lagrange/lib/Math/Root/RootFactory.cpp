#include "RootFactory.hpp"
#include <stdexcept>
#include "RootBisection.hpp"
#include "RootBrent.hpp"

using namespace EnjoLib;

RootFactory::RootFactory(){}
RootFactory::~RootFactory(){}

std::unique_ptr<IRootAlgo> RootFactory::Create(RootType type, double minArg, double maxArg, double eps)
{
    switch (type)
    {
    case ROOT_BIN_SEARCH:
        return std::unique_ptr<IRootAlgo>(new RootBisection(minArg, maxArg, eps));
    case ROOT_BRENT:
        return std::unique_ptr<IRootAlgo>(new RootBrent(minArg, maxArg, eps));
    }

    throw std::invalid_argument("Not handled RootType");
}
