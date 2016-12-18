#ifndef FUNCBASE_H
#define FUNCBASE_H

namespace EnjoLib {

class FuncBase
{
    public:
        FuncBase();
        virtual ~FuncBase();

        virtual double operator() (double) = 0;
    protected:
    private:
};
}
#endif // FUNCBASE_H
