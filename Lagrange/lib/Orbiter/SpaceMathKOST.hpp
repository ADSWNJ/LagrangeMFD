// Due to KOST dependency - licensed under LGPL

#ifndef SPACEMATHORBITERKOST_H
#define SPACEMATHORBITERKOST_H

#include <kost.h>
namespace EnjoLib
{
class SpaceMathKOST
{
    public:
        SpaceMathKOST();
        virtual ~SpaceMathKOST();
        struct ElementsOrbitParam;
        ElementsOrbitParam GetElements( const OBJHANDLE hObj, const OBJHANDLE hRef, int frame ) const;
        kostStateVector SwapCoordinateSystem( const kostStateVector & in ) const;
        kostStateVector GetRelativeStateVector( const OBJHANDLE hObj, const OBJHANDLE hRef ) const;
        kostStateVector ToEquatorial( const kostStateVector & in, const OBJHANDLE hRef ) const;
        VECTOR3 ToEquatorial( const VECTOR3 & in, const OBJHANDLE hRef ) const;

    protected:
    private:
};
struct SpaceMathKOST::ElementsOrbitParam
{
    ElementsOrbitParam( const kostElements & pel, const kostOrbitParam & pop );
    ElementsOrbitParam();
    kostElements el;
    kostOrbitParam op;
};
}
#endif // SPACEMATHORBITERKOST_H
