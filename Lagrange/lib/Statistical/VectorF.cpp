#include "VectorF.hpp"

using namespace EnjoLib;

VectorF::VectorF( const std::vector<float> & init )
: VectorTpl(init)
{}

VectorF::VectorF( int n )
: VectorTpl(n)
{}

VectorF::VectorF(){}

VectorF::~VectorF(){}
