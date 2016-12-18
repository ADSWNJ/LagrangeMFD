#ifndef SPACEMATHORBITER_H
#define SPACEMATHORBITER_H

#include <orbitersdk.h>
namespace EnjoLib
{
class SpaceMathOrbiter
{
    public:
        double GetPlaneAngle( const VESSEL * v, const OBJHANDLE hTarget ) const;
        double GetHeadBearing( const VESSEL * v ) const;
        void Crt2Pol (VECTOR3 &pos, VECTOR3 &vel) const;
        void Crt2Pol (VECTOR3 &pos) const;
        double getdeterminant(const MATRIX3 & mat) const;
        MATRIX3 getinvmatrix(const MATRIX3 & mat) const;
        VECTOR3 ToEquatorial( const VECTOR3 & in, const OBJHANDLE hRef ) const;
        VECTOR3 GetPlaneAxis( const OBJHANDLE hObj, const OBJHANDLE hRef ) const;
        void getinvrotmatrix(VECTOR3 arot, MATRIX3 *invrotmatrix) const;//arot not really a vector - see arot defn from vessel struct
        VECTOR3 GetRotationToTarget(VESSEL * vessel, const VECTOR3 & target) const;
        /// Swaps coordinate system
        /** Orbiter uses left hand coordinates because of DirectX legacy. */
        VECTOR3 SwapCoordinateSystem( const VECTOR3 & in ) const;
    protected:
    private:
};
}
#endif // SPACEMATHORBITER_H
