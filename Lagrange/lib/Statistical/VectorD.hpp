#ifndef VECTORD_H
#define VECTORD_H

#include "VectorTpl.hpp"

namespace EnjoLib
{
class VectorD : public VectorTpl<double>
{
    public:
        VectorD( const std::vector<double> & init );
        VectorD( int n );
        VectorD();
        virtual ~VectorD();
    protected:
    private:
};
}
#endif // VECTORD_H
