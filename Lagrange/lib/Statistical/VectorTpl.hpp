#ifndef VECTORTPL_H
#define VECTORTPL_H


#include <vector>
#include <string>
namespace EnjoLib
{
template<class T>
class VectorTpl : public std::vector<T>
{
    public:
        std::string Print() const; // TODO
        std::string PrintScilab( const char * varName ) const;
        std::string PrintPython( const char * varName ) const;

        VectorTpl( const std::vector<T> & init );
        VectorTpl( int n );
        VectorTpl();
        virtual ~VectorTpl();

        //! Length of vector
        T Len() const;
        //! Normalised copy of vector
        VectorTpl Norm() const;
        T SumSquares() const;
        T Sum() const;
        T Mean() const;
        VectorTpl AdjustMean() const;

        VectorTpl & operator += (const VectorTpl & par);
        VectorTpl & operator -= (const VectorTpl & par);
        VectorTpl & operator /= (const T f);
        VectorTpl & operator *= (const T f);
        VectorTpl & operator += (const T f);
        VectorTpl & operator -= (const T f);

        VectorTpl operator + (const VectorTpl & par) const;
        VectorTpl operator - (const VectorTpl & par) const;
        VectorTpl operator - () const;
        VectorTpl operator + (const T f) const;
        VectorTpl operator - (const T f) const;
        VectorTpl operator * (const T f) const;
        VectorTpl operator / (const T f) const;
        bool operator > (const VectorTpl & par) const;
        bool operator < (const VectorTpl & par) const;



    protected:
    private:
        void SizesEqual( const VectorTpl & par, const char * functionName ) const;

        typedef typename std::vector<T>::const_iterator CIt;
        typedef typename std::vector<T>::iterator It;
};
}

#endif // VECTORTPL_H
