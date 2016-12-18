#ifndef BINSEARCHOPTI2D_H
#define BINSEARCHOPTI2D_H

#include "../Util/Result.hpp"
#include "../Systems/Point.hpp"

namespace EnjoLib
{
    class BinSearchOpti2DSubject;
    class BinSearchOpti2D
    {
        public:
            BinSearchOpti2D(const Point & minArgs, const Point & maxArgs, double eps);
            virtual ~BinSearchOpti2D();
            Result<Point> Run( BinSearchOpti2DSubject & subj ) const;
        protected:
        private:

            Point m_minArgs, m_maxArgs;
            double m_eps;
            int m_maxIter;
    };
}
#endif // BINSEARCHOPTI2D_H
