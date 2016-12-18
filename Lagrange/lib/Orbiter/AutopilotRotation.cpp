#include <orbitersdk.h>
#include <Orbiter/AutopilotRotation.hpp>
#include <Orbiter/AngularAcc.hpp>
#include <Orbiter/SpaceMathOrbiter.hpp>
#include <Orbiter/SystemsConverterOrbiter.hpp>
#include <Orbiter/VesselCapabilities.hpp>

using namespace EnjoLib;
const Vect3 AutopilotRotation::m_statDeltaGliderRefRotAcc(0.125, 0.066, 0.189);

AutopilotRotation::AutopilotRotation()
: m_targetVector(_V(0,0,0))
, m_targetVectorUnit(_V(0,0,0))
, m_targetLengthPrev(0)
, m_pidAPSpaceX(0.8, 5)
, m_pidAPSpaceY(m_pidAPSpaceX)
, m_pidAPSpaceBank(1, 12)
, m_controlledVessel(NULL)
{
    m_isEnabled = false;
}

AutopilotRotation::~AutopilotRotation(){}

bool AutopilotRotation::SetTargetVector(const VECTOR3 & targetVector)
{
    m_targetVector = targetVector;
    m_targetVectorUnit = unit(targetVector);
    double targetLength = length(m_targetVector);
    bool consideredComplete = false;
    if (targetLength != 0 && m_targetLengthPrev != 0)
    {
        bool isDVVerySmall = targetLength < 1.0; // Prevent rolling 180 degrees around
        bool isDVIncreasing = targetLength > m_targetLengthPrev; // dV starts increasing = burn complete
        if (isDVIncreasing || isDVVerySmall)
        {
            MECO(GetVessel());
            consideredComplete = true;
        }
    }
    m_targetLengthPrev = targetLength;
    return consideredComplete;
}

void AutopilotRotation::Disable()
{
    SetTargetVector(_V(0,0,0));
}

VESSEL * AutopilotRotation::GetVessel()
{
    return m_controlledVessel == NULL ? NULL : oapiGetVesselInterface(m_controlledVessel);
}

void AutopilotRotation::Update(double SimDT)
{
    bool isZeroVector = m_targetVector.x + m_targetVector.y + m_targetVector.z == 0;
    if (isZeroVector)
    {
        if (IsEnabled())
            Enable(false);
        //sprintf(oapiDebugString(), "TransX: AUTO rotation disabled. Press SHIFT+C in target view to enable");
        return;
    }

    if (!IsEnabled())
    {
        Enable(true);
    }
    VESSEL * vessel = GetVessel();
    if (!vessel)
        return;
    if (vessel != oapiGetFocusInterface())
        return; // trying to control other ship (the focused one, but the one which was programmed)

    if (oapiGetTimeAcceleration() > 100)
    {
        SetRot0(); // Adds stability
        return;
    }


    vessel->DeactivateNavmode( NAVMODE_PROGRADE );
    vessel->DeactivateNavmode( NAVMODE_RETROGRADE );
    vessel->DeactivateNavmode( NAVMODE_NORMAL );
    vessel->DeactivateNavmode( NAVMODE_ANTINORMAL );
    vessel->DeactivateNavmode( NAVMODE_HLEVEL );
    vessel->DeactivateNavmode( NAVMODE_HOLDALT );
    //sprintf(oapiDebugString(), "TransX: AUTO rotation ENABLED!");

    const VECTOR3 & angleToTarget = SpaceMathOrbiter().GetRotationToTarget(vessel, m_targetVectorUnit);
    const VECTOR3 & accRatio = GetVesselAngularAccelerationRatio(vessel);

    const double inputBank = (vessel->GetBank() - PI/2.0) / PI; // Targeting 90* = PI/2, like the Prograde autopilot
    const double b = accRatio.z * m_pidAPSpaceBank.Update( inputBank, SimDT );
    const double x = accRatio.x * m_pidAPSpaceX.Update( angleToTarget.x, SimDT );
    const double y = accRatio.y * m_pidAPSpaceY.Update( angleToTarget.y, SimDT );

    vessel->SetAttitudeRotLevel( 2, b );
    vessel->SetAttitudeRotLevel( 1, -x );
    vessel->SetAttitudeRotLevel( 0, y );
}

void AutopilotRotation::Enable(bool val)
{
    m_isEnabled = val;
    OnDisabled();
    OnEnabled();
}
/*
void AutopilotRotation::SwitchEnabled()
{
    m_isEnabled = !m_isEnabled;
    OnDisabled();
}
*/
bool AutopilotRotation::IsEnabled() const
{
    return m_isEnabled;
}

void AutopilotRotation::OnEnabled()
{
    if (!IsEnabled())
        return;
    //oapiGetVesselInterface
    VESSEL * vessel = oapiGetFocusInterface();
    m_controlledVessel = vessel == NULL ? NULL : vessel->GetHandle();
}
void AutopilotRotation::OnDisabled()
{
    if (IsEnabled())
        return;

    VESSEL * vessel = GetVessel();
    if (!vessel)
        return;

    SetRot0();
    vessel->ActivateNavmode( NAVMODE_KILLROT );
}

void AutopilotRotation::SetRot0()
{
    VESSEL * vessel = GetVessel();
    if (!vessel)
        return;
    vessel->SetAttitudeRotLevel( _V(0, 0, 0) );
    // Orbiter 2016 workaround for non working SetAttitudeRotLevel():
    GetVessel()->SetAttitudeRotLevel(0, 0);
    GetVessel()->SetAttitudeRotLevel(1, 0);
    GetVessel()->SetAttitudeRotLevel(2, 0);
}

VECTOR3 AutopilotRotation::GetVesselAngularAccelerationRatio( const VESSEL * vessel )
{
    VECTOR3 accRatio;
    //if ( m_rotationRatioNeeded )
    {
        const VECTOR3 & refMaxAngAcc = SystemsConverterOrbiter(m_statDeltaGliderRefRotAcc).GetVECTOR3();
        const VECTOR3 & currMaxAngAcc = AngularAcc().GetMaxAngAcc( vessel );
        accRatio.x = refMaxAngAcc.x / currMaxAngAcc.x;
        accRatio.y = refMaxAngAcc.y / currMaxAngAcc.y;
        accRatio.z = refMaxAngAcc.z / currMaxAngAcc.z;
    }
   // else
    {
    //    accRatio.x = accRatio.y = accRatio.z = 1;
    }

    return accRatio;
}

void AutopilotRotation::MECO(VESSEL * vessel)
{
    MainEngineOn(vessel, 0);
}

void AutopilotRotation::MainEngineOn( VESSEL * vessel, double level )
{
    THGROUP_HANDLE h = VesselCapabilities().GetMainEnginesHandle(vessel);
    if ( h == NULL )
        return;
    if ( level > 1 )
        level = 1;
    else if ( level < 0 )
        level = 0;
    vessel->SetThrusterGroupLevel( h, level );
}
