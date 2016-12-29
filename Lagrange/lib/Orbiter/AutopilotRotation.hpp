#ifndef AUTOPILOT_H
#define AUTOPILOT_H

#include <Systems/Vect3.hpp>
#include <PID/PID.hpp>

class AutopilotRotation
{
    public:
        AutopilotRotation();
        virtual ~AutopilotRotation();
        void Update(double SimDT);
        bool SetTargetVector(const VECTOR3 & targetVector, const OBJHANDLE hRefBody); // Setting _V(0,0,0) disables the AP
        void Disable();
        bool IsEnabled() const;
        void SetVessel(VESSEL * vessel);
    protected:
    private:
        void MECO(VESSEL * vessel);
        void MainEngineOn( VESSEL * vessel, double level );
        void Enable(bool val);
        //void SwitchEnabled();
        VECTOR3 GetVesselAngularAccelerationRatio( const VESSEL * vessel );
        void OnDisabled();
        void OnEnabled();
        VESSEL * GetVessel();
        void SetRot0();

        EnjoLib::PID m_pidAPSpaceX;
        EnjoLib::PID m_pidAPSpaceY;
        EnjoLib::PID m_pidAPSpaceBank;
        VECTOR3 m_targetVector;
        VECTOR3 m_targetVectorUp;
        VECTOR3 m_targetVectorUnit;
        double m_targetLengthPrev;
        bool m_isEnabled;
        const static EnjoLib::Vect3 m_statDeltaGliderRefRotAcc; // reference pitch, yaw and bank angular accelerations
        OBJHANDLE m_controlledVessel;
        VESSEL * m_vessel;
        VECTOR3 m_trX_out;
        VECTOR3 m_trX_plc;
        VECTOR3 m_trX_pro;
        VECTOR3 m_trX_tgt;
        VECTOR3 m_trX_tgtfwd;
        VECTOR3 m_trX_tgtup;
        OBJHANDLE m_hRefBody;

};

#endif // AUTOPILOT_H
