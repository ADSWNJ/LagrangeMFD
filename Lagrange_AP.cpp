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
  , m_burnFrozen(false)
  , m_dampspin(false)
  , m_aVelLast(_V(0.0,0.0,0.0))
{
  for (int i = 0; i < 3; i++) {
    one_ap[i].SetDeadband(0.001, 0.001, true);
    one_ap[i].SetAccParams(0.01, 0.5, 0.8);
  }
  for (int i = 3; i < 6; i++) {
    one_ap[i].SetDeadband(10.0, 1.0, false);
    one_ap[i].SetAccParams(0.01, 0.05, 0.2);
  }
}

Lagrange_AP::~Lagrange_AP() {}


/*
 * Convert TransX Target
 * ---------------------
 * Converts TransX targeting information into vessel-local targeting data, w.r.t. the relative pos and vel of vessel to reference body
 */
VECTOR3 Lagrange_AP::ConvertTransXTarget(VESSEL *v, const VECTOR3 &trxVec, const VECTOR3 &Qr, const VECTOR3 &Pr, VECTOR3 &trxGbl, VECTOR3 &upLcl, const bool burnFrozen) 
{
  VECTOR3 proHat;
  VECTOR3 plcHat;
  VECTOR3 outHat;
  VECTOR3 gTgt;
  VECTOR3 tgtVectorLocal;

  if (!burnFrozen) {
    if (length(trxVec) == 0.0) {
      proHat = _V(0.0, 0.0, 1.0);
      plcHat = _V(0.0, -1.0, 0.0);
      outHat = _V(1.0, 0.0, 0.0);
      trxGbl = _V(0.0, 0.0, 0.0);
    } else {
      proHat = unit(Pr);
      plcHat = crossp(unit(Qr), proHat);
      outHat = crossp(proHat, plcHat);
      trxGbl = unit(proHat * trxVec.x + plcHat * trxVec.z + outHat * trxVec.y);
    }
  }

  v->GetGlobalPos(gTgt);
  gTgt += trxGbl;
  v->Global2Local(gTgt, tgtVectorLocal);
  v->GetGlobalPos(gTgt);
  if (!burnFrozen) {
    gTgt -= plcHat;
    v->Global2Local(gTgt, upLcl);
  }
  return tgtVectorLocal;
}

VECTOR3 Lagrange_AP::GetRotationToTarget(const VECTOR3 &target, const VECTOR3 &up) const
{
  double upxyLen2 = abs(up.x*up.x + up.y*up.y);
  if (upxyLen2 < 1e-6) return _V(atan2(target.y, target.z), atan2(target.x, target.z), 0.0);
  return _V(atan2(target.y, target.z), atan2(target.x, target.z), atan2(-up.x, up.y));
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

  if (!m_burnFrozen) {
    m_targetVector = targetVector;
    m_deltaV = length(m_targetVector);
  }
  m_targetVectorLocal = ConvertTransXTarget(v, m_targetVector, Qr, Pr, m_targetVectorGlobal, m_upVectorLocal, m_burnFrozen); 
  if (!m_burnFrozen) {
    m_tgtV = length(vs.rvel + m_targetVectorLocal);
  }
  m_angleToTarget = GetRotationToTarget(m_targetVectorLocal, m_upVectorLocal);

  /*char buf[256];
  sprintf_s(buf, 256, "ALPHA DBG: burnFrozen:%d G-tgt:{%+.4f,%+.4f,%+.4f} L-tgt:{%+.4f,%+.4f,%+.4f}  Angle-tgt:{%+.4f,%+.4f,%+.4f}  UpVector:{%+.4f,%+.4f,%+.4f}",
    (m_burnFrozen?1:0),
    m_targetVectorGlobal.x, m_targetVectorGlobal.y, m_targetVectorGlobal.z,
    m_targetVectorLocal.x, m_targetVectorLocal.y, m_targetVectorLocal.z,
    m_angleToTarget.x*DEG, m_angleToTarget.y*DEG, m_angleToTarget.z*DEG,
    m_upVectorLocal.x, m_upVectorLocal.y, m_upVectorLocal.z
  );
  strcpy(oapiDebugString(), buf);*/

  if (apState > 1) { // point enabled
    if (!IsEnabled()) Enable(v);
    double warp = oapiGetTimeAcceleration();
    while (SimT <= m_startBurnTime && (SimT + warp*2.0 > m_startBurnTime) && warp > 1.0) {
      warp /= 10.0;
      oapiSetTimeAcceleration(warp); // within 2 warp-cycles of the start burn, reduce warp gracefully
    }
    double totT = ExecuteRotAP(v, m_angleToTarget, SimT, SimDT);
    if (!m_burnFrozen) GetVesselBurnData(v, burnSimT);
    if (apState > 2) { // burn enabled
      if (!m_burnFrozen) {
        if (SimT <= m_startBurnTime && (SimT + 2.5 > m_startBurnTime)) {
          m_burnFrozen = true;
        }
      }
      if (m_burnFrozen) {
        ExecuteBurn(v, SimT, SimDT);
      }
    }
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

  m_targetVectorLocal = ConvertTransXTarget(v, _V(1.0, 0.0, 0.0), Qr, Pr, m_targetVectorGlobal, m_upVectorLocal, false); //  , m_targetProAxis, m_targetPlcAxis, m_targetOutAxis);
  m_angleToTarget = GetRotationToTarget(m_targetVectorLocal, m_upVectorLocal);
  double warp = oapiGetTimeAcceleration();
  v->GetAngularVel(m_aVel);

  double l_ang = length(m_angleToTarget)*DEG;
  double l_aVel = length(m_aVel)*DEG;
  ExecuteRotAP(v, m_angleToTarget, SimT, SimDT);
  if (abs(m_angleToTarget.x) * DEG > 0.01) return;
  if (abs(m_angleToTarget.y) * DEG > 0.01) return;
  if (abs(m_angleToTarget.z) * DEG > 0.01) return;
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

// Focus on y first, then y+x, then move z gently at the end
  if (abs(ang.y) * DEG > 30.0) {
    ang.x = ang.z = 0.0;
  } else if (abs(ang.x) * DEG > 5.0 || abs(ang.y) * DEG > 5.0) {
    ang.z = 0.0;
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
    WarpKillRot(v, 'a');
    return 0.0;
  }
  v->DeactivateNavmode(NAVMODE_KILLROT);
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

  v->DeactivateNavmode(NAVMODE_KILLROT);
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
  if (!m_isEnabled) {
    m_dampspin = true; // initial damping
    m_burnFrozen = false; // accept targeting data for the burn until the actual burn time is imminent
  }
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
    one_ap[i].Enable(v, i, false);    // Set true for AP debug
  }
}

void Lagrange_AP::Disable(VESSEL* v)
{
  if (!m_isEnabled) return;
  v->ActivateNavmode(NAVMODE_KILLROT);
  m_targetVector = _V(0.0, 0.0, 0.0);
  SetRot0(v);
  WarpKillRot(v, 'a');
  for (int i = 0; i < 3; i++) {
    one_ap[i].Disable();
  }
  m_isEnabled = false;
  m_burnFrozen = false;
  if (m_dumping) {
    fclose(m_dumpFile);
    m_dumping = false;
  }
  strcpy(oapiDebugString(), "");
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

void Lagrange_AP::ExecuteBurn(VESSEL *v, double SimT, double SimDT) {
  THGROUP_HANDLE thgm = v->GetThrusterGroupHandle(THGROUP_MAIN);
  if (thgm == NULL) {
    m_burnFrozen = false;
    return; // Um ... no main engine?!
  }

  if (SimT > m_endBurnTime) {                                  // finished burn: MECO time
    v->SetThrusterGroupLevel(thgm, 0.0);
    m_burnFrozen = false;
    Disable(v);
  } else if (SimT + SimDT > m_endBurnTime) {                   // last puff needed
    double fractionalBurn = (m_endBurnTime - SimT) / SimDT;
    v->SetThrusterGroupLevel(thgm, fractionalBurn);
  } else if (SimT> m_startBurnTime) {                          // regular burn
    v->SetThrusterGroupLevel(thgm, 1.0);
  } else if (SimT + SimDT> m_startBurnTime) {                  // first DT of burn
    double fractionalBurn = (SimT + SimDT - m_startBurnTime) / SimDT;
    v->SetThrusterGroupLevel(thgm, fractionalBurn);
  } else {
    v->SetThrusterGroupLevel(thgm, 0.0);                       // Pre-burn
  }

  return;
}

void Lagrange_AP::GetVesselBurnData(VESSEL *v, double burnSimT) {
  // Original code concepts courtesy of Enjo: https://sourceforge.net/p/enjomitchsorbit/codeHG/ci/default/tree/lib/Orbiter/BurnTime.cpp#l84
  std::vector<VESSEL*> stack;
  AddVesselToStack(v, stack);
  m_stackMass = 0.0;
  for (auto it = stack.begin(); it != stack.end(); it++) {
    m_stackMass += (*it)->GetMass();
  }

  m_thrust = 0.0;
  m_ISP = 0.0;
  unsigned int numTh = v->GetGroupThrusterCount(THGROUP_MAIN);
  for (unsigned int i = 0; i < numTh; i++) {
    THRUSTER_HANDLE thh = v->GetGroupThruster(THGROUP_MAIN, i);
    m_thrust += v->GetThrusterMax0(thh);
    m_ISP += v->GetThrusterIsp0(thh);
  }
  m_ISP /= (double)numTh;

  double a_s = m_thrust / m_stackMass; // Eq. 1 ... starting acceleration
  double m_dot = m_thrust / m_ISP; // Eq. 2 ... mass flow rate
  double j = m_thrust * m_dot / (m_stackMass * m_stackMass); // Eq. 3 "jerk" = rate of change of acceleration as mass burns off
  double t_d = -(m_ISP * m_stackMass * (exp(-m_deltaV / m_ISP) - 1.0)) / m_thrust; // Eq. 4 ... thrust duration

  m_startBurnTime = burnSimT + (a_s * t_d) / (2.0 * a_s + j * t_d) - t_d; // Eq.5 ... actual start time (before the instantaneous burn time by roughly 1/2 t_d)
  m_endBurnTime = m_startBurnTime + t_d;

}

void Lagrange_AP::AddVesselToStack(VESSEL *v, std::vector<VESSEL*> &stack) {
  for (auto it = stack.begin(); it != stack.end(); it++)
    if (*it == v) return;  // Already found this vessel, so ignore recursion here
  if (v) {
    stack.push_back(v); // stack this vessel, and go lookign for docked and attached vessels
    for (unsigned int i = 0; i < v->DockCount(); i++) {
      if (OBJHANDLE dockedVessel = v->GetDockStatus(v->GetDockHandle(i))) {
        AddVesselToStack(oapiGetVesselInterface(dockedVessel), stack);
      }
    }
  }
}

bool Lagrange_AP::IsBurnFrozen() const {
  return m_burnFrozen;
}

double Lagrange_AP::GetBurnStart() const {
  return m_startBurnTime;
}

double Lagrange_AP::GetBurnEnd() const {
  return m_endBurnTime;
}

double Lagrange_AP::GetBurnDV() const {
  return m_deltaV;
}

double Lagrange_AP::GetBurnTgtV() const {
  return m_tgtV;
}
