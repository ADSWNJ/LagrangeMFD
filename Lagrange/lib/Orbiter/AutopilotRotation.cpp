#include <orbitersdk.h>
#include <Orbiter/AutopilotRotation.hpp>
#include <Orbiter/AngularAcc.hpp>
#include <Orbiter/SpaceMathOrbiter.hpp>
#include <Orbiter/SystemsConverterOrbiter.hpp>
#include <Orbiter/VesselCapabilities.hpp>

using namespace EnjoLib;
const Vect3 AutopilotRotation::m_statDeltaGliderRefRotAcc(0.125, 0.066, 0.189);
//const Vect3 AutopilotRotation::m_statDeltaGliderRefRotAcc(0.066, 0.066, 0.189);

AutopilotRotation::AutopilotRotation()
: m_targetVector(_V(0,0,0))
, m_targetVectorUnit(_V(0,0,0))
, m_targetLengthPrev(0)
//, m_pidAPSpaceX(0.8, 5)
, m_pidAPSpaceX(0.1, 12)
, m_pidAPSpaceY(m_pidAPSpaceX)
//, m_pidAPSpaceBank(1, 12)
, m_pidAPSpaceBank(.5, 8)
, m_controlledVessel(NULL)
, m_vessel(NULL)
{
    m_isEnabled = false;
}

AutopilotRotation::~AutopilotRotation(){}

bool AutopilotRotation::SetTargetVector(const VECTOR3 & targetVector, const OBJHANDLE hRefBody)
{
    m_hRefBody = hRefBody;
    if (length(targetVector) == 0.0) {
      m_targetVector = _V(0.0, 0.0, 0.0);
      m_targetVectorUp = _V(0.0, 0.0, 0.0);
      return true;
    }
    SpaceMathOrbiter().GetTransXTarget(m_vessel, hRefBody, targetVector, &m_trX_pro, &m_trX_plc, &m_trX_tgt);

    m_targetVector = m_trX_pro * length(m_trX_tgt);
    m_targetVectorUp = m_trX_plc;


    m_targetVectorUnit = unit(m_targetVector);
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
    SetTargetVector(_V(0,0,0), m_hRefBody);
}

VESSEL * AutopilotRotation::GetVessel()
{
    return m_vessel;
}

void AutopilotRotation::SetVessel(VESSEL * vessel) {
  m_vessel = vessel;
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

    if (oapiGetTimeAcceleration() > 100)
    {
        SetRot0(); // Adds stability
        return;
    }
    //vessel->DeactivateNavmode( NAVMODE_KILLROT );
    vessel->DeactivateNavmode( NAVMODE_PROGRADE );
    vessel->DeactivateNavmode( NAVMODE_RETROGRADE );
    vessel->DeactivateNavmode( NAVMODE_NORMAL );
    vessel->DeactivateNavmode( NAVMODE_ANTINORMAL );
    vessel->DeactivateNavmode( NAVMODE_HLEVEL );
    vessel->DeactivateNavmode( NAVMODE_HOLDALT );

    VECTOR3 angleToTarget  = SpaceMathOrbiter().GetRotationToTarget(m_trX_tgt, &m_targetVector, &m_targetVectorUp);


    const VECTOR3 & accRatio = GetVesselAngularAccelerationRatio(vessel);

    const double degree = 2.0 * PI / 360.0;

    if (abs(angleToTarget.z) > 10.0 * degree) {
      angleToTarget = _V(0.0, 0.0, angleToTarget.z);
    } else if (abs(angleToTarget.x) > 45.0 * degree) {
      angleToTarget = _V(angleToTarget.x, 0.0, angleToTarget.z);
    } 
    
    const double damp = 0.1;
    const double b = damp * accRatio.z * m_pidAPSpaceBank.Update(angleToTarget.z, SimDT );
    const double x = damp * accRatio.x * m_pidAPSpaceX.Update( angleToTarget.x, SimDT );
    const double y = damp * accRatio.y * m_pidAPSpaceY.Update( angleToTarget.y, SimDT );

    VESSELSTATUS vs;
    vessel->GetStatus(vs);
    sprintf(oapiDebugString(), "Pro:{%.2f,%.2f,%.2f} PLC:{%.2f,%.2f,%.2f} Tgt:{%.2f,%.2f,%.2f} Ang:{%.2f,%.2f,%.2f} VRot:{%.2f,%.2f,%.2f}",
      m_trX_pro.x, m_trX_pro.y, m_trX_pro.z,
      m_trX_plc.x, m_trX_plc.y, m_trX_plc.z,
      m_trX_tgt.x, m_trX_tgt.y, m_trX_tgt.z,
      angleToTarget.x * 180.0 / PI, angleToTarget.y * 180.0 / PI, angleToTarget.z * 180.0 / PI,
      vs.vrot.x * 180.0 / PI, vs.vrot.y * 180.0 / PI, vs.vrot.z * 180.0 / PI);

    //vessel->SetAttitudeRotLevel( 2, b );
    //vessel->SetAttitudeRotLevel( 1, x );
    //vessel->SetAttitudeRotLevel( 0, y );
    for (int i = 0; i < 3; i++) {
      double att = angleToTarget.data[i];
      double vax = vs.vrot.data[i];

    }
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
    vessel->SetAttitudeRotLevel(0, 0);
    vessel->SetAttitudeRotLevel(1, 0);
    vessel->SetAttitudeRotLevel(2, 0);
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
    MainEngineOn(vessel, 0.0);
}

void AutopilotRotation::MainEngineOn( VESSEL * vessel, double level )
{
    THGROUP_HANDLE h = VesselCapabilities().GetMainEnginesHandle(vessel);
    if ( h == NULL )
        return;
    if ( level > 1.0 )
        level = 1.0;
    else if ( level < 0.0 )
        level = 0.0;
    vessel->SetThrusterGroupLevel( h, level );
}
