#ifndef BINSEARCHRESULT_H
#define BINSEARCHRESULT_H

namespace EnjoLib
{
template <class T>
class Result
{
    public:
        Result( const T & value, bool isSuccess );

        const T value;
        const bool isSuccess;
};

template <class T>
Result<T>::Result( const T & value, bool isSuccess )
: value(value)
, isSuccess(isSuccess)
{
}
}

#endif // BINSEARCHRESULT_H
