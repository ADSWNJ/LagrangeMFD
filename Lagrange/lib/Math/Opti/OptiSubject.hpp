#ifndef BINSEARCHOPTISUBJECT_H
#define BINSEARCHOPTISUBJECT_H

namespace EnjoLib
{
    class OptiSubject
    {
        public:
            OptiSubject();
            virtual ~OptiSubject();

            /// Should return the function's value for a given argument.
            virtual double UpdateGetValue( double arg ) = 0;

            double operator() (double x){return UpdateGetValue(x);}

        protected:
        private:
    };
}

#endif // BINSEARCHOPTISUBJECT_H
