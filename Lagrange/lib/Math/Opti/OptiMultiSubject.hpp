#ifndef OPTIMULTISUBJECT_H
#define OPTIMULTISUBJECT_H

#include <vector>

namespace EnjoLib
{
    class OptiMultiSubject
    {
        public:
            OptiMultiSubject();
            virtual ~OptiMultiSubject();

            //virtual double Get(const std::vector<double> & in) = 0;
            virtual double Get(const double * in, int n) = 0;

            virtual std::vector<double> GetStart() const = 0;
            virtual std::vector<double> GetStep() const = 0;


        protected:
        private:
    };
}

#endif // OPTIMULTISUBJECT_H
