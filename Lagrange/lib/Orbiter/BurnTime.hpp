#ifndef BURNTIME_H
#define BURNTIME_H

#include <Orbitersdk.h>
#include <vector>

class BurnTime
{
    public:
        BurnTime();
        virtual ~BurnTime();

        double GetBurnStart(VESSEL *vessel, THGROUP_TYPE thGroupType, double instantaneousBurnTime, double deltaV);
        double GetBurnTimeVariadic(VESSEL *vessel, THGROUP_TYPE thGroupType, double deltaV);
        double GetBurnTime(VESSEL *vessel, double deltaV);
        double GetStackMass(VESSEL *vessel);
        void AddVesselToStack(VESSEL *vessel, std::vector<VESSEL*> &stack);
    protected:
    private:
};

#endif // BURNTIME_H
