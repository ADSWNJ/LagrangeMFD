#include "UtilOrbiter.hpp"

using namespace EnjoLib;

UtilOrbiter::UtilOrbiter(){}
UtilOrbiter::~UtilOrbiter(){}

int UtilOrbiter::GetHandleIndex(OBJHANDLE handle) const
{
    for (unsigned i = 0; i < oapiGetObjectCount(); ++i)
    {
        OBJHANDLE foundObj = oapiGetObjectByIndex(i);
        if (handle == foundObj)
        {
            return i;
        }
    }
    return NULL;
}
