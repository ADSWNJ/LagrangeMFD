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
  , m_targetLengthPrev(0.0)
  , m_isEnabled(false)
  , m_dampspin(false)
  , m_aVelLast(_V(0.0,0.0,0.0))
{
  for (int i = 0; i < 3; i++) {
    one_ap[i].SetDeadband(0.001);
    one_ap[i].SetAccParams(0.01, 0.5, 0.8);
  }
  for (int i = 3; i < 6; i++) {
    one_ap[i].SetDeadband(10.0);
    one_ap[i].SetAccParams(0.01, 0.2, 0.8);
  }
}

Lagrange_AP::~Lagrange_AP() {}


/*
 * Convert TransX Target
 * ---------------------
 * Converts TransX targeting information into global targeting data, w.r.t. the relative pos and vel of vessel to reference body
 */
VECTOR3 Lagrange_AP::ConvertTransXTarget(VESSEL *v, const VECTOR3 &trxVec, const VECTOR3 &Qr, const VECTOR3 &Pr) //  , VECTOR3 &proHat, VECTOR3 &plcHat, VECTOR3 &outHat)
{
  VECTOR3 proHat;
  VECTOR3 plcHat;
  VECTOR3 outHat;
  VECTOR3 tgtVector;
  VECTOR3 gTgt;
  VECTOR3 tgtVectorLocal;

  if (length(trxVec) == 0.0) {
    proHat = _V(0.0, 0.0, 1.0);
    plcHat = _V(0.0, 1.0, 0.0);
    outHat = _V(1.0, 0.0, 0.0);
    tgtVector = _V(0.0, 0.0, 0.0);
  } else {
    proHat = unit(Pr);
    plcHat = crossp(proHat, unit(Qr));
    outHat = crossp(plcHat, proHat);
    tgtVector = unit(proHat * trxVec.x + plcHat * trxVec.y + outHat * trxVec.z);

  }

  v->GetGlobalPos(gTgt);
  gTgt += tgtVector;
  v->Global2Local(gTgt, tgtVectorLocal);
  return tgtVectorLocal;
}

VECTOR3 Lagrange_AP::GetRotationToTarget(const VECTOR3 &target) const
{
  return _V(atan2(target.y, target.z), atan2(target.x, target.z), 0.0);
}


/*
 * Autopilot Plan Mode Update
 * ==========================
 * Params:
 * v is the vessel handle
 * apState is 0 for INACTIVE, 1 for ARMED, 2 for POINT, 3 for POINT+BURN
 * SimT is the current simulation time
 * SimDT is the last simulation time step
 * BurnSimT is the instantaneous burn point
 * targetVector is a TransX-style PRO/OUT/PLC vector
 * Qr is the global position of the vessel relative to the reference celestial body
 * Pr is the global velocity of the vessel relative to the reference celestial body
 *
 */
void Lagrange_AP::Update_PlanMode(VESSEL* v, const int apState, const double SimT, const double SimDT, const double burnSimT, const VECTOR3 &targetVector, const VECTOR3 &Qr, const VECTOR3 &Pr) {

  double lenQr = length(Qr);
  double lenPr = length(Pr);

  VECTOR3 QrU = Qr;
  VECTOR3 PrU = Pr;

  VESSELSTATUS vs;
  v->GetStatus(vs);

  m_targetVectorLocal = ConvertTransXTarget(v, targetVector, Qr, Pr); //  , m_targetProAxis, m_targetPlcAxis, m_targetOutAxis);


  char buf[256];
  sprintf_s(buf, 256, "TV:{%.2f,%.2f,%.2f} Rot:{%.1f,%.1f,%.1f} LTGT:{%.2f,%.2f,%.2f} ATAN2:{%.2f %.2f}  BST:%f",
    targetVector.x, targetVector.y, targetVector.z,
    vs.arot.x*DEG, vs.arot.y*DEG, vs.arot.z*DEG,
    m_targetVectorLocal.x, m_targetVectorLocal.y, m_targetVectorLocal.z,
    atan2(m_targetVectorLocal.y, m_targetVectorLocal.z)*DEG, atan2(m_targetVectorLocal.x, m_targetVectorLocal.z)*DEG,
    burnSimT
  );
  strcpy(oapiDebugString(), buf);
  m_angleToTarget = GetRotationToTarget(m_targetVectorLocal);

  if (apState > 1) {
    if (!IsEnabled()) Enable(v);
    double totT = ExecuteRotAP(v, m_angleToTarget, SimT, SimDT);
  } else {
    Disable(v);
  }
}

/*
* Autopilot Hold Mode Update
* ==========================
* Params:
* v is the vessel handle
* apState is 0 for INACTIVE, 1 for ARMED, 2 for POINT, 3 for POINT+BURN
* SimT is the current simulation time
* SimDT is the last simulation time step
* QLPr is the global position of the vessel relative to the LP
* PLPr is the global velocity of the vessel relative to the LP
*
*/
void Lagrange_AP::Update_HoldMode(VESSEL* v, const int apState, const double SimT, const double SimDT, const VECTOR3 &Qr, const VECTOR3 &Pr, const VECTOR3 &QLPr, const VECTOR3 &PLPr) {
  if (apState < 3) {
    Disable(v);
    return;
  }
  if (!IsEnabled()) Enable(v);

  m_targetVectorLocal = ConvertTransXTarget(v, _V(1.0, 0.0, 0.0), Qr, Pr); //  , m_targetProAxis, m_targetPlcAxis, m_targetOutAxis);
  m_angleToTarget = GetRotationToTarget(m_targetVectorLocal);
  double warp = oapiGetTimeAcceleration();
  v->GetAngularVel(m_aVel);

  double l_ang = length(m_angleToTarget)*DEG;
  double l_aVel = length(m_aVel)*DEG;

  if (ExecuteRotAP(v, m_angleToTarget, SimT, SimDT)>0.01) return;
  ExecuteLinAP(v, QLPr, PLPr, SimT, SimDT);
}

double Lagrange_AP::ExecuteRotAP(VESSEL* v, const VECTOR3 angleToTarget, const double SimT, const double SimDT) {
  VECTOR3 ang = angleToTarget;
  VECTOR3 rot{ 0.0,0.0,0.0 };
  double warp = oapiGetTimeAcceleration();
  bool atWarp = warp > 10.0;
  m_dumpIx++;
  v->GetAngularVel(m_aVel);

  double spinVdeg = length(m_aVel)*DEG;

  //If we are spinning too much ... damp it first
  if (spinVdeg > 100.0 || m_dampspin) {
    m_dampspin = (spinVdeg > 0.05 / SimDT);
    SetRot0(v);
    if (atWarp) {
      WarpKillRot(v,'a');
      if (m_dumping) fprintf(m_dumpFile, "%d, %.2f, %.3f, spinVdeg = %.2f, simDT = %.2f, spinPerDT = %.2f, WARPKILLROT, warp_factor %.0f\n", m_dumpIx, oapiGetSimTime(), SimDT, spinVdeg, SimDT, spinVdeg * SimDT, warp);

    } else {
      v->ActivateNavmode(NAVMODE_KILLROT);
      if (m_dumping) fprintf(m_dumpFile, "%d, %.2f, %.3f, spinVdeg = %.2f, f, simDT = %.2f, spinPerDT = %.2f, KILLROT\n", m_dumpIx, oapiGetSimTime(), SimDT, spinVdeg, SimDT, spinVdeg * SimDT);
    }
    return true;
  } else {
    if (m_dampspin & m_dumping) fprintf(m_dumpFile, "%d, %.2f, %.3f, spinVdeg = %.2f, f, simDT = %.2f, spinPerDT = %.2f, KILLEXIT\n", m_dumpIx, oapiGetSimTime(), SimDT, spinVdeg, SimDT, spinVdeg * SimDT);
    m_dampspin = false;
    v->DeactivateNavmode(NAVMODE_KILLROT);
  }
  m_aVelLast = m_aVel;


  // Determines the angular acceleration in the last SimDT, and then calculates the Acc:Thrust ratio
  // From this, the average Thrust Reference Value is calculated each of the first 10 cycles, then updating each 10 cycles
  // (this removes the bumps from any one-off burn anomalies). 

  for (int axis = 0; axis < 3; axis++) {
    one_ap[axis].CalcCali(m_aVel.data[axis] * DEG, SimT, SimDT);
  }

// Focus on z first, then y+z, then move x gently at the end
  if (abs(ang.z) * DEG > 10.0) {
    ang.x = ang.y = 0.0;
  } else if (abs(ang.y) * DEG > 40.0) {
    ang.x = 0.0;
  }

  rot = _V(0.0, 0.0, 0.0);
  rot.data[0] = one_ap[0].CalcThrust(-ang.x * DEG, m_aVel.x * DEG, SimT, SimDT);
  rot.data[1] = one_ap[1].CalcThrust(ang.y * DEG, m_aVel.y * DEG, SimT, SimDT);
  rot.data[2] = one_ap[2].CalcThrust(ang.z * DEG, m_aVel.z * DEG, SimT, SimDT);

  double totT = abs(rot.x) + abs(rot.y) + abs(rot.z);
  if (m_dumping) fprintf(m_dumpFile, "%d, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f\n",
    m_dumpIx, SimT, SimDT, spinVdeg, 
    -ang.x * DEG, m_aVel.x * DEG, rot.data[0], 
    -ang.y * DEG, m_aVel.y * DEG, rot.data[1],
    -ang.z * DEG, m_aVel.z * DEG, rot.data[2] );

  if (atWarp) WarpKillRot(v, 'z'); 

  if (length(ang)*DEG < 0.01 / warp && length(m_aVel)*DEG < 0.001 / warp) {
    if (atWarp) {
      WarpKillRot(v, 'a');
    } else {
      v->ActivateNavmode(NAVMODE_KILLROT);
    }
    return 0.0;
  }
  v->DeactivateNavmode(NAVMODE_PROGRADE);
  v->DeactivateNavmode(NAVMODE_RETROGRADE);
  v->DeactivateNavmode(NAVMODE_NORMAL);
  v->DeactivateNavmode(NAVMODE_ANTINORMAL);
  v->DeactivateNavmode(NAVMODE_HLEVEL);
  v->DeactivateNavmode(NAVMODE_HOLDALT);
  for (int axis = 0; axis < 3; axis++) {
    one_ap[axis].ExecuteThrust(v);
  }
  return totT;
}

void Lagrange_AP::ExecuteLinAP(VESSEL* v, const VECTOR3 &QLPr, const VECTOR3 &PLPr, const double SimT, const double SimDT) {
  double warp = oapiGetTimeAcceleration();
  bool atWarp = warp > 10.0;
  m_dumpIx++;

  v->DeactivateNavmode(NAVMODE_PROGRADE);
  v->DeactivateNavmode(NAVMODE_RETROGRADE);
  v->DeactivateNavmode(NAVMODE_NORMAL);
  v->DeactivateNavmode(NAVMODE_ANTINORMAL);
  v->DeactivateNavmode(NAVMODE_HLEVEL);
  v->DeactivateNavmode(NAVMODE_HOLDALT);

  // Determine the Thrust Reference Value is calculated each of the first 10 cycles, then updating each 10 cycles
  for (int axis = 3; axis < 6; axis++) {
    one_ap[axis].CalcCali(PLPr.data[axis-3], SimT, SimDT);
  }

  VECTOR3 lin; 
  lin = _V(0.0, 0.0, 0.0);
  lin.data[0] = one_ap[3].CalcThrust(QLPr.x, PLPr.x, SimT, SimDT);
  lin.data[1] = one_ap[4].CalcThrust(QLPr.y, PLPr.y, SimT, SimDT);
  lin.data[2] = one_ap[5].CalcThrust(QLPr.z, PLPr.z, SimT, SimDT);

  if (m_dumping) fprintf(m_dumpFile, "%d, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f\n",
    m_dumpIx, SimT, SimDT, 0.0,
    QLPr.x, PLPr.x, lin.data[0],
    QLPr.y, PLPr.y, lin.data[1],
    QLPr.z, PLPr.z, lin.data[2]);

  for (int axis = 3; axis < 6; axis++) {
    one_ap[axis].ExecuteThrust(v);
  }
}

bool Lagrange_AP::IsEnabled() const
{
  return m_isEnabled;
}

void Lagrange_AP::Enable(VESSEL* v)
{
  if (!m_isEnabled) m_dampspin = true; // initial damping
  m_isEnabled = true;

  m_dumpFile = NULL;
  errno_t err = fopen_s(&m_dumpFile, ".\\Config\\MFD\\Lagrange\\Diags\\AP.csv", "w");
  if (err != 0 || m_dumpFile == NULL) {
    m_dumping = false;
  } else {
    m_dumping = true;
    m_dumpIx = 0;
    m_dumpTotT = 0.0;
    fprintf(m_dumpFile, "Ix, SimT, SimDT, spinVdeg, ");
    for (char c = 'X'; c <= 'Z'; c++) {
      fprintf(m_dumpFile, "%c_Ang, %c_Vel, %c_Thr",
                            c,        c,    c);
      if (c!= 'Z') fprintf(m_dumpFile, ",");
    }
    fprintf(m_dumpFile, "\n");
  }
  for (int i = 0; i < 6; i++) {
    one_ap[i].Enable(v, i, true);
  }
}

void Lagrange_AP::Disable(VESSEL* v)
{
  v->ActivateNavmode(NAVMODE_KILLROT);
  m_targetVector = _V(0.0, 0.0, 0.0);
  SetRot0(v);
  for (int i = 0; i < 3; i++) {
    one_ap[i].Disable();
  }
  m_isEnabled = false;
  if (m_dumping) {
    fclose(m_dumpFile);
    m_dumping = false;
  }
}

void Lagrange_AP::SetRot0(VESSEL* v) {
  v->SetAttitudeRotLevel(0, 0.0);
  v->SetAttitudeRotLevel(1, 0.0);
  v->SetAttitudeRotLevel(2, 0.0);
}

VECTOR3 Lagrange_AP::GetATT() const {
  return m_angleToTarget;
}

VECTOR3 Lagrange_AP::GetAVel() const {
  return m_aVel;
}

void Lagrange_AP::WarpKillRot(VESSEL *v, char axis) {
  VESSELSTATUS vs;
  v->GetStatus(vs);
  if (axis == 'a' || axis == 'x') vs.vrot.x = 0.0;
  if (axis == 'a' || axis == 'y') vs.vrot.y = 0.0;
  if (axis == 'a' || axis == 'z') vs.vrot.z = 0.0;
  v->DefSetState(&vs);
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