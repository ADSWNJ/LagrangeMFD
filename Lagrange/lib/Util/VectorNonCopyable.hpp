#ifndef VectorNonCopyable_H
#define VectorNonCopyable_H

#include <vector>
#include "NonCopyable.hpp"

template <class T>
class VectorNonCopyable : public std::vector<T>, public EnjoLib::NonCopyable
{
    public:

    protected:

    private:

};


#endif // VectorNonCopyable_H
