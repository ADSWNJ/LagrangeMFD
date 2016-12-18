#ifndef STATEVECT3_H
#define STATEVECT3_H

#include "Vect3.hpp"

namespace EnjoLib
{
struct StateVect3
{
    StateVect3( const Vect3 & pos = Vect3(), const Vect3 & vel = Vect3() )
    : pos(pos), vel(vel)
    {

    }

    Vect3 pos, vel;
};
}
#endif // STATEVECT3_H
