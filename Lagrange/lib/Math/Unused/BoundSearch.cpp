#include "BoundSearch.hpp"
//#include "GeneralMath.hpp"
#include <cmath>


using namespace EnjoLib;

BoundSearch::BoundSearch()
{
}

BoundSearch::~BoundSearch()
{
}

Result<double> BoundSearch::FindUpper(OptiSubject & subj, double start, double minIncrement, double maxIncrement)
{
    return Find(subj, start, true, minIncrement, maxIncrement);
}
Result<double> BoundSearch::FindLower(OptiSubject & subj, double start, double minIncrement, double maxIncrement)
{
    return Find(subj, start, false, minIncrement, maxIncrement);
}

Result<double> BoundSearch::Find(OptiSubject & subj, double start, bool upper, double minIncrement, double maxIncrement)
{
    // Input data preprocessing - determine search direction and adjust increments
    int sign = upper ? 1 : -1;
    minIncrement = sign * fabs(minIncrement);
    maxIncrement = sign * fabs(maxIncrement);
    // Initial conditions
    double testArg = start;
    double value = subj.UpdateGetValue(testArg);
    double valuePrev;
    double incr = minIncrement;
    bool incrExceeded = false;
    do
    {
        if (incrExceeded == false)
        {
            incr = incr * 2;
            if (maxIncrement != 0 && fabs(incr) > fabs(maxIncrement))
            {
                incrExceeded = true;
                incr = maxIncrement; // Don't increase the increment anymore
            }
        }
        testArg += incr;
        valuePrev = value; // Store previous value before updating it
        value = subj.UpdateGetValue(testArg); // Test new argument

    } while(value < valuePrev);

    return Result<double>(testArg, true);
}
