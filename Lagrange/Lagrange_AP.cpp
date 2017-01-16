// ==============================================================
//
//	Lagrange (AutoPilot)
//	==================================
//
//	Copyright (C) 2016-2017	Andrew (ADSWNJ) Stokes
//                   All rights reserved
//
//	See Lagrange.cpp
//
// ==============================================================

#include "Lagrange_AP.hpp"
#include <assert.h>

Lagrange_AP::Lagrange_AP()
  : m_targetVector(_V(0.0, 0.0, 0.0))
  , m_targetVectorUp(_V(0.0, 0.0, 0.0))
  , m_targetVectorUnit(_V(0.0, 0.0, 0.0))
  , m_targetLengthPrev(0.0)
  , m_isEnabled(false)
  , m_vessel(NULL)
  , m_trxOut(_V(0.0, 0.0, 0.0))
  , m_trxPlc(_V(0.0, 0.0, 0.0))
  , m_trxPro(_V(0.0, 0.0, 0.0))
  , m_hRefBody(NULL)
  , m_dampspin(false)
  , m_aVelLast(_V(0.0,0.0,0.0))
{}

Lagrange_AP::~Lagrange_AP() {}

void Lagrange_AP::SetRefBody(const OBJHANDLE hRefBody) {
  m_hRefBody = hRefBody;
}

void  Lagrange_AP::GetTransXTarget(const VECTOR3 & trxVec)
{
  // Find TransX style coord system axes in ship coords
  VECTOR3 rpos;
  VECTOR3 gvel;
  VECTOR3 gPos;
  m_vessel->GetRelativePos(m_hRefBody, rpos);
  m_vessel->GetGlobalVel(gvel);
  m_vessel->GetGlobalPos(gPos);

  VECTOR3 gPro = unit(gvel);
  VECTOR3 gPlc = unit(crossp(gvel, rpos));
  VECTOR3 gOut = unit(crossp(gPlc, gPro));
  VECTOR3 gTgt = gPro * trxVec.x + gPlc * trxVec.y + gOut * trxVec.z;
  m_vessel->Global2Local(gPro + gPos, m_trxPro);
  m_vessel->Global2Local(gPlc + gPos, m_trxPlc);
  m_vessel->Global2Local(gTgt + gPos, m_targetVector);
  m_targetVectorUnit = unit(m_targetVector);
}

void Lagrange_AP::SetTargetVector(const VECTOR3 & targetVector)
{
  if (length(targetVector) < 1e-6) {
    m_targetVector = _V(0.0, 0.0, 0.0);
    m_targetVectorUnit = _V(0.0, 0.0, 0.0);
    m_targetVectorFwd = _V(0.0, 0.0, 0.0);
    m_targetVectorUp = _V(0.0, 0.0, 0.0);
    return;
  }
  assert(m_vessel != NULL);
  assert(m_hRefBody != NULL);
  GetTransXTarget(targetVector);
}


void Lagrange_AP::SetVessel(VESSEL* vessel) {
  m_vessel = vessel;
  for (int i = 0; i < 3; i++) {
//    ap[i].Enable(i, m_vessel, i==2?true:false);
    one_ap[i].SetDeadband(0.001);
    one_ap[i].SetAccParams(0.01, 0.5, 0.8);
  }
}


VECTOR3 Lagrange_AP::GetRotationToTarget(const VECTOR3 & target, VECTOR3 *targetFwd, VECTOR3 *targetUp) const
{
  VECTOR3 tgtFwd = *targetFwd;
  VECTOR3 tgtUp = *targetUp;

  double ang_g = atan2(tgtUp.x, tgtUp.y);
  if (abs(tgtUp.x) < 0.001 && abs(tgtUp.y) < 0.001) ang_g = 0.0; // Damps behavior on the cardinal point
  MATRIX3 rot_g = _M(cos(ang_g), -sin(ang_g), 0.0,
    sin(ang_g), cos(ang_g), 0.0,
    0.0, 0.0, 1.0);
  VECTOR3 tgt2 = mul(rot_g, tgtFwd);
  VECTOR3 tgtUp2 = mul(rot_g, tgtUp);

  double ang_a = atan2(-target.y, target.z);
  if (abs(target.y) < 0.001 && abs(target.z) < 0.001) ang_a = 0.0;
  MATRIX3 rot_a = _M(1.0, 0.0, 0.0,
    0.0, cos(ang_a), -sin(ang_a),
    0.0, sin(ang_a), cos(ang_a));
  VECTOR3 tgt3 = mul(rot_a, tgt2);
  VECTOR3 tgtUp3 = mul(rot_a, tgtUp2);

  double ang_b = atan2(-target.x, target.z);
  if (abs(target.x) < 0.001 && abs(target.z) < 0.001) ang_b = 0.0;
  MATRIX3 rot_b = _M(cos(ang_b), 0.0, -sin(ang_b),
    0.0, 1.0, 0.0,
    sin(ang_b), 0.0, cos(ang_b));

  *targetFwd = mul(rot_b, tgt3);
  *targetUp = mul(rot_b, tgtUp3);

  return _V(-ang_a, ang_b, ang_g);
}

void Lagrange_AP::Update(double SimT, double SimDT)
{

  VESSELSTATUS vs;
  m_vessel->GetStatus(vs);

  /*if (oapiGetTimeAcceleration() > 100)
  {
    VECTOR3 gblTgtUnit;
    VECTOR3 gblPos;
    m_vessel->GetGlobalPos(gblPos);
    m_vessel->Local2Global(m_targetVectorUnit, gblTgtUnit);
    gblTgtUnit -= gblPos;
    m_vessel->SetGlobalOrientation(gblTgtUnit);
    m_vessel->SetAngularVel(_V(0.0, 0.0, 0.0));
    return;
  }*/

  //vessel->DeactivateNavmode( NAVMODE_KILLROT );
  m_vessel->DeactivateNavmode(NAVMODE_PROGRADE);
  m_vessel->DeactivateNavmode(NAVMODE_RETROGRADE);
  m_vessel->DeactivateNavmode(NAVMODE_NORMAL);
  m_vessel->DeactivateNavmode(NAVMODE_ANTINORMAL);
  m_vessel->DeactivateNavmode(NAVMODE_HLEVEL);
  m_vessel->DeactivateNavmode(NAVMODE_HOLDALT);


  m_targetVectorFwd = _V(0.0, 0.0, 1.0) * length(m_targetVector);
  m_targetVectorUp = m_trxPlc;
  m_angleToTarget =  GetRotationToTarget(m_targetVector, &m_targetVectorFwd, &m_targetVectorUp);

  if (IsEnabled()) ExecuteRotAP(m_angleToTarget, SimT, SimDT);
  m_vessel->GetAngularVel(m_aVel);
/*  sprintf(oapiDebugString(), "Pro:{%.2f,%.2f,%.2f} PLC:{%.2f,%.2f,%.2f} Tgt:{%.2f,%.2f,%.2f} Ang:{%.2f,%.2f,%.2f} VRot:{%.2f,%.2f,%.2f} AVel:{%.2f,%.2f,%.2f}",
    m_trxPro.x, m_trxPro.y, m_trxPro.z,
    m_trxPlc.x, m_trxPlc.y, m_trxPlc.z,
    m_targetVector.x, m_targetVector.y, m_targetVector.z,
    m_angleToTarget.x * 180.0 / PI, m_angleToTarget.y * 180.0 / PI, m_angleToTarget.z * 180.0 / PI,
    vs.vrot.x * 180.0 / PI, vs.vrot.y * 180.0 / PI, vs.vrot.z * 180.0 / PI,
    m_aVel.data[0]*DEG, m_aVel.data[1] * DEG, m_aVel.data[2] * DEG
  );
*/
}


void Lagrange_AP::ExecuteRotAP(const VECTOR3 angleToTarget, const double SimT, const double SimDT) {
  int minaxis = 0; // set to 0 after debugging
  VECTOR3 aVel;
  VECTOR3 ang = angleToTarget;
  VECTOR3 rot{ 0.0,0.0,0.0 };
  m_dumpIx++;

  // Determines the angular acceleration in the last SimDT, and then calculates the Acc:Thrust ratio
  // From this, the average Thrust Reference Value is calculated each of the first 10 cycles, then updating each 10 cycles
  // (this removes the bumps from any one-off burn anomalies). 
  m_vessel->GetAngularVel(aVel);
  m_aVelLast = aVel;
  for (int axis = minaxis; axis < 3; axis++) {
    one_ap[axis].CalcCali(aVel.data[axis] * DEG, SimT, SimDT);
  }

  //If we are spinning too much ... damp it first
  if (length(aVel)*DEG > 100.0 || m_dampspin) {
    SetRot0();
    m_vessel->ActivateNavmode( NAVMODE_KILLROT );
    if (m_dumping) fprintf(m_dumpFile, "%d, %.2f, %.3f, AvelLen = %.2f, KILLROT\n", m_dumpIx, oapiGetSimTime(), SimDT, length(aVel) * DEG);
    m_dampspin = (length(aVel) > 1.0 * RAD);
    return;
  } else {
    m_dampspin = false;
    m_vessel->DeactivateNavmode(NAVMODE_KILLROT);
  }

// Focus on z first, then y+z, then move x gently at the end
  if (abs(ang.z) * DEG > 20.0) {
    ang.x = ang.y = 0.0;
  } else if (abs(ang.y) * DEG > 20.0) {
    ang.x = 0.0;
  }

  //rot = _V(0.0, 0.0, 0.0);
  rot.data[0] = one_ap[0].CalcThrust(-ang.x * DEG, aVel.x * DEG, SimT, SimDT);
  rot.data[1] = one_ap[1].CalcThrust(-ang.y * DEG, aVel.y * DEG, SimT, SimDT);
  rot.data[2] = one_ap[2].CalcThrust(-ang.z * DEG, aVel.z * DEG, SimT, SimDT);

  //If we are at target, use KILLROT to keep us stable
  if (length(ang)*DEG < 0.001 && length(aVel)*DEG < 0.0001) {
    m_vessel->ActivateNavmode(NAVMODE_KILLROT);
  } else {
    m_vessel->DeactivateNavmode(NAVMODE_KILLROT);
  }

  for (int axis = minaxis; axis < 3; axis++) {
    one_ap[axis].ExecuteThrust();
  }
}

bool Lagrange_AP::IsEnabled() const
{
  return m_isEnabled;
}

void Lagrange_AP::Enable()
{
  m_isEnabled = true;

  if (fopen_s(&m_dumpFile, ".\\Config\\MFD\\Lagrange\\Diags\\AP.csv", "w") != 0) {
    m_dumping = false;
  } else {
    m_dumping = true;
    m_dumpIx = 0;
    m_dumpTotT = 0.0;
    assert(m_dumpfile != NULL);
    fprintf(m_dumpFile, "Ix, SimT, SimDT, TotT,");
    for (char c = 'X'; c <= 'Z'; c++) {
      fprintf(m_dumpFile, "%c_RefIx, %c_AngToTgt, %c_AACC, %c_AVEL, %c_CycParam, %c_CycToHit, %c_aVelRatio, %c_IdealThr, %c_ActThr, %c_ThRefVal",
                            c,        c,           c,       c,       c,           c,           c,            c,           c,         c);
      if (c!= 'Z') fprintf(m_dumpFile, ",");
    }
    fprintf(m_dumpFile, "\n");
  }
  for (int i = 0; i < 3; i++) {
    one_ap[i].Enable(i, m_vessel, true);
  }
}

void Lagrange_AP::Disable()
{
  m_vessel->ActivateNavmode(NAVMODE_KILLROT);
  SetTargetVector(_V(0.0, 0.0, 0.0));
  SetRot0();
  for (int i = 0; i < 3; i++) {
    one_ap[i].Disable();
  }
  m_isEnabled = false;
  if (m_dumping) {
    fclose(m_dumpFile);
    m_dumping = false;
  }
}

void Lagrange_AP::SetRot0() {
  m_vessel->SetAttitudeRotLevel(0, 0.0);
  m_vessel->SetAttitudeRotLevel(1, 0.0);
  m_vessel->SetAttitudeRotLevel(2, 0.0);
}

VECTOR3 Lagrange_AP::GetATT() {
  return m_angleToTarget;
}

VECTOR3 Lagrange_AP::GetAVel() {
  return m_aVel;
}

OBJHANDLE Lagrange_AP::GetRefBody() {
  return m_hRefBody;
}

/*void AutopilotRotation::MainEngineOn(VESSEL * vessel, double level)
{
THGROUP_HANDLE h = v->GetThrusterGroupHandle( THGROUP_MAIN );

  if (h == NULL)
    return;
  if (level > 1.0)
    level = 1.0;
  else if (level < 0.0)
    level = 0.0;
  vessel->SetThrusterGroupLevel(h, level);
}*/