#ifndef BINSEARCHOPTI2DSUBJECT_H
#define BINSEARCHOPTI2DSUBJECT_H

namespace EnjoLib
{
    struct Point;
    class BinSearchOpti2DSubject
    {
        public:
            BinSearchOpti2DSubject();
            virtual ~BinSearchOpti2DSubject();
            /// Should return the function's value for a given argument.
            virtual double UpdateGetValue( const Point & arg ) = 0;

        protected:
        private:
    };
}
#endif // BINSEARCHOPTI2DSUBJECT_H
