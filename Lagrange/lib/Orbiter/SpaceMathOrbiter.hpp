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
        VECTOR3 GetRotationToTarget(const VECTOR3 & target, VECTOR3 *targetFwd, VECTOR3 *targetUp) const;
        void GetTransXTarget(const VESSEL *v, const OBJHANDLE hRefBody, const VECTOR3 & target, VECTOR3 *trX_zhat, VECTOR3 *trX_yhat, VECTOR3 *trX_tgt);
        VECTOR3 GetTransXRot(VESSEL *v, VECTOR3 & vec, VECTOR3 & trX_out, VECTOR3 & trX_plc, VECTOR3 & trX_pro, VECTOR3 *upVec) const;

        /// Swaps coordinate system
        /** Orbiter uses left hand coordinates because of DirectX legacy. */
        VECTOR3 SwapCoordinateSystem( const VECTOR3 & in ) const;
    protected:
    private:
};
}
#endif // SPACEMATHORBITER_H
