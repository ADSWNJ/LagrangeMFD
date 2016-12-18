#ifndef BOUNDSEARCH_H
#define BOUNDSEARCH_H
#include "../Util/Result.hpp"
#include "OptiSubject.hpp"

namespace EnjoLib
{
class BoundSearch
{
    public:
        BoundSearch();
        virtual ~BoundSearch();

        Result<double> FindUpper(OptiSubject & subj, double start, double minIncrement = 1, double maxIncrement = 0);
        Result<double> FindLower(OptiSubject & subj, double start, double minIncrement = 1, double maxIncrement = 0);
        Result<double> Find(OptiSubject & subj, double start, bool upper, double minIncrement = 1, double maxIncrement = 0);
    protected:
    private:
};
}
#endif // BOUNDSEARCH_H
