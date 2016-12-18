#include <Orbitersdk.h>
#include "SpaceMathKOST.hpp"
#include "SpaceMathOrbiter.hpp"

using namespace EnjoLib;

SpaceMathKOST::SpaceMathKOST()
{}

SpaceMathKOST::~SpaceMathKOST()
{}

// ElementsOrbitParam constructor
SpaceMathKOST::ElementsOrbitParam::ElementsOrbitParam( const kostElements & pel, const kostOrbitParam & pop )
{
    el = pel;
    op = pop;
}

// ElementsOrbitParam constructor
SpaceMathKOST::ElementsOrbitParam::ElementsOrbitParam()
{}

SpaceMathKOST::ElementsOrbitParam SpaceMathKOST::GetElements(
                    const OBJHANDLE hObj, const OBJHANDLE hRef, int frame ) const
{
    kostStateVector stateVec = GetRelativeStateVector( hObj, hRef );
    if ( frame == FRAME_EQU ) // Equatorial frame chosen
        stateVec = ToEquatorial(stateVec, hRef);
    // else - already in ecliptic frame
	// Orbiter uses left hand coordinate system. Accommodate to KOST.
	stateVec = SwapCoordinateSystem( stateVec );
	const double mi = oapiGetMass(hRef) * GGRAV;
	kostElements el; kostOrbitParam op;
    kostStateVector2Elements(mi, &stateVec, &el, &op);

    return ElementsOrbitParam( el, op );
}

kostStateVector SpaceMathKOST::ToEquatorial( const kostStateVector & in, const OBJHANDLE hRef ) const
{
    kostStateVector out;
    out.pos = SpaceMathOrbiter().ToEquatorial(in.pos, hRef);
    out.vel = SpaceMathOrbiter().ToEquatorial(in.vel, hRef);
    return out;
}

VECTOR3 SpaceMathKOST::ToEquatorial( const VECTOR3 & in, const OBJHANDLE hRef ) const
{
    return SpaceMathOrbiter().ToEquatorial(in, hRef);
}

kostStateVector SpaceMathKOST::SwapCoordinateSystem( const kostStateVector & in ) const
{
    kostStateVector out;
    out.pos = SpaceMathOrbiter().SwapCoordinateSystem(in.pos);
    out.vel = SpaceMathOrbiter().SwapCoordinateSystem(in.vel);
    return out;
}

kostStateVector SpaceMathKOST::GetRelativeStateVector( const OBJHANDLE hObj, const OBJHANDLE hRef ) const
{
    kostStateVector out;
    oapiGetRelativePos(hObj, hRef, &out.pos);
    oapiGetRelativeVel(hObj, hRef, &out.vel);
    return out;
}
