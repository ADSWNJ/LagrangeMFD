#ifndef UTILORBITER_H
#define UTILORBITER_H

#include <OrbiterAPI.h>

namespace EnjoLib
{
class UtilOrbiter
{
    public:
        UtilOrbiter();
        virtual ~UtilOrbiter();
        int GetHandleIndex(OBJHANDLE handle) const;

    protected:

    private:
};
}

#endif // UTILORBITER_H
