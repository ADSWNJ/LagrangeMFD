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
  , m_thrustRefIx{ -1, -1, -1 }
  , m_thrustRefVal(_V(1.0, 1.0, 1.0))
  , m_thrustLastTime{ false, false, false }
  , m_fullCycle{ false, false, false }
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
}


VECTOR3 Lagrange_AP::GetRotationToTarget(const VECTOR3 & target, VECTOR3 *targetFwd, VECTOR3 *targetUp) const
{
  VECTOR3 tgtFwd = *targetFwd;
  VECTOR3 tgtUp = *targetUp;

  double ang_g = atan2(tgtUp.x, tgtUp.y);
  MATRIX3 rot_g = _M(cos(ang_g), -sin(ang_g), 0.0,
    sin(ang_g), cos(ang_g), 0.0,
    0.0, 0.0, 1.0);
  VECTOR3 tgt2 = mul(rot_g, tgtFwd);
  VECTOR3 tgtUp2 = mul(rot_g, tgtUp);

  double ang_a = atan2(-target.y, target.z);
  MATRIX3 rot_a = _M(1.0, 0.0, 0.0,
    0.0, cos(ang_a), -sin(ang_a),
    0.0, sin(ang_a), cos(ang_a));
  VECTOR3 tgt3 = mul(rot_a, tgt2);
  VECTOR3 tgtUp3 = mul(rot_a, tgtUp2);

  double ang_b = atan2(-target.x, target.z);
  MATRIX3 rot_b = _M(cos(ang_b), 0.0, -sin(ang_b),
    0.0, 1.0, 0.0,
    sin(ang_b), 0.0, cos(ang_b));

  *targetFwd = mul(rot_b, tgt3);
  *targetUp = mul(rot_b, tgtUp3);

  return _V(ang_a, ang_b, ang_g);
}

void Lagrange_AP::Update(double SimDT)
{
  if (length(m_targetVector) < 1e-6) {
    Disable();
    return;
  }

  if (!IsEnabled()) Enable();

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
  VECTOR3 angleToTarget =  GetRotationToTarget(m_targetVector, &m_targetVectorFwd, &m_targetVectorUp);

  SetRotThrust(angleToTarget, SimDT);

  sprintf(oapiDebugString(), "Pro:{%.2f,%.2f,%.2f} PLC:{%.2f,%.2f,%.2f} Tgt:{%.2f,%.2f,%.2f} Ang:{%.2f,%.2f,%.2f} VRot:{%.2f,%.2f,%.2f}",
    m_trxPro.x, m_trxPro.y, m_trxPro.z,
    m_trxPlc.x, m_trxPlc.y, m_trxPlc.z,
    m_targetVector.x, m_targetVector.y, m_targetVector.z,
    angleToTarget.x * 180.0 / PI, angleToTarget.y * 180.0 / PI, angleToTarget.z * 180.0 / PI,
    vs.vrot.x * 180.0 / PI, vs.vrot.y * 180.0 / PI, vs.vrot.z * 180.0 / PI);
}


void Lagrange_AP::SetRotThrust(const VECTOR3 angleToTarget, const double SimDT) {
  int minaxis = 0; // set to 0 after debugging
  VECTOR3 aVel;
  VECTOR3 aAcc;
  VECTOR3 ang = angleToTarget;
  ang.y = -ang.y;
  ang.x = -ang.x;
  VECTOR3 rot{ 0.0,0.0,0.0 };
  m_dumpIx++;

  // Determines the agular acceleration in the last SimDT, and then calculates the Acc:Thrust ratio
  // From this, the average Thrust Reference Value is calculated each of the first 10 cycles, then updating each 10 cycles
  // (this removes the bumps from any one-off burn anomalies). 
  m_vessel->GetAngularVel(aVel);
  aAcc = (aVel - m_aVelLast) / SimDT;
  m_aVelLast = aVel;
  for (int axis = minaxis; axis < 3; axis++) {
    if (m_thrustLastTime[axis]) {
      int ix = m_thrustRefIx[axis];
      m_thrustRefData[ix].aAcc.data[axis] = (aVel.data[axis] - m_thrustRefData[ix].aVel.data[axis]) / SimDT;
      m_thrustRefData[ix].aAccThrustRatio.data[axis] = m_thrustRefData[ix].aAcc.data[axis] / m_thrustRefData[ix].thrust.data[axis];
      if (!m_fullCycle[axis] || ix == 9) {
        double sumAccRatio = 0.0;
        for (int i = 0; i <= ix; i++) {
          sumAccRatio += m_thrustRefData[ix].aAccThrustRatio.data[axis];
        }
        m_thrustRefVal.data[axis] = sumAccRatio / (ix + 1);
      }
      m_thrustRefIx[axis]++;
      if (m_thrustRefIx[axis] == 10) {
        m_fullCycle[axis] = true;
        m_thrustRefIx[axis] = 0;
      }
      m_thrustLastTime[axis] = false;
    }
  }

  //If we are spinning too much ... damp it first
  if (length(aVel) > 100.0 * RAD || m_dampspin) {
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
  if (abs(ang.z) > 5.0 * RAD) {
    ang = _V(-aAcc.x/10.0, -aAcc.y/10.0, angleToTarget.z);
  } else if (abs(ang.y) > 5.0 * RAD) {
    ang = _V(-aAcc.x/10.0, angleToTarget.y, angleToTarget.z);
  }

  VECTOR3 cycParam;  // Ideal number of cycles to null out the error. At high error angle, set this to e.g. 8, and wind down to 1.0 at the end. 
  VECTOR3 cycToHit;  // Based on current angular velocity, how many SimDT cycles to hit the target
  VECTOR3 aVelRatio; // Ratio of optimal aVel for hit in cycParam cycles, to actual aVel (drives the ideal thrust)
  VECTOR3 idealThr;  // Ideal thrust to change the aVel to the optimal value (assuming infinite engines ... i.e. this value goes beyond -1...+1)

  for (int axis = minaxis; axis < 3; axis++) {
    /* if (abs(ang.data[axis]) < 0.01 * RAD) { // Go quiet below 0.01 degrees of error
      cycParam.data[axis] = 0.0;
      cycToHit.data[axis] = 0.0;
      aVelRatio.data[axis] = 0.0;
      idealThr.data[axis] = 0.0;
      rot.data[axis] = 0.0;
      continue;
    } */
    if (m_thrustRefIx[axis] == -1) { // First try out with these thrusters and weight combination ... use 10% thrust and see the result
      m_thrustRefIx[axis]++;
      cycParam.data[axis] = 0.0;
      cycToHit.data[axis] = 0.0;
      aVelRatio.data[axis] = 0.0;
      idealThr.data[axis] = 0.0;
      m_thrustRefVal.data[axis] = 1.0;
      rot.data[axis] = ang.data[axis] < 0.0 ? -0.1 : 0.1;
      m_dumpTotT += abs(rot.data[axis]);
    } else {
      double aa = abs(ang.data[axis]) * DEG;
      if (aa >= 90.0) {
        cycParam.data[axis] = 8.0;
      } else if (aa < 90.0 && aa >= 45.0) {
        cycParam.data[axis] = 8.0 - 4.0*(90.0 - aa) / 45.0;
      } else if (aa < 45.0 && aa >= 5.0) {
        cycParam.data[axis] = 4.0 - 3.0*(45.0 - aa) / 40.0;
      } else {
        cycParam.data[axis] = 1.0;
      }
      cycToHit.data[axis] = abs(aVel.data[axis]) < 1e-9? 10000.0 : abs(ang.data[axis] / aVel.data[axis]); // Don't want a divide by zero
      aVelRatio.data[axis] = ang.data[axis] / cycParam.data[axis] - aVel.data[axis];
      double RCSsaver = 3.0; // This parameter just reduces the amount of fuel used for the turn (by lowering thrust levels). Set to 1 if you want faster response and more fuel burn.
      rot.data[axis] = aVelRatio.data[axis] / (RCSsaver * m_thrustRefVal.data[axis] * SimDT);
      idealThr.data[axis] = rot.data[axis];
      if (rot.data[axis] > 1.0) { // Cap the actual rot input to -1...+1, and remove the noise below 0.01% thrust
        rot.data[axis] = 1.0;
      } else if (rot.data[axis] < -1.0) {
        rot.data[axis] = -1.0;
      } else if (abs(rot.data[axis]) < 0.0001) {
        rot.data[axis] = 0.0;
      }
      if (rot.data[axis] * aVel.data[axis] > 0.0) { // if we are accelerating
        if (abs(aVel.data[axis])*DEG >= abs(m_thrustRefVal.data[axis])*(2.0/SimDT)) { // and we are at or above 10 SimDt's to stop, then no more
          rot.data[axis] = 0.0;
        }
      }
      m_dumpTotT += abs(rot.data[axis])*SimDT;
    }
  }

  int i = m_thrustRefIx[2] == 0? 9 : m_thrustRefIx[2]-1;
  if (m_dumping) {
    fprintf(m_dumpFile, "%d, %.2f, %.3f, %.2f, ", m_dumpIx, oapiGetSimTime(), SimDT, m_dumpTotT);
    for (int axis = 0; axis < 3; axis++) {
      fprintf(m_dumpFile, "%d, %.4f, %.4f, %.4f,", m_thrustRefIx[axis], -ang.data[axis] * DEG, m_thrustRefData[i].aAcc.data[axis] * DEG, aVel.data[axis] * DEG);
      fprintf(m_dumpFile, "%.6f, %.6f, %.6f, %.4f, %.4f, %.9f", cycParam.data[axis], cycToHit.data[axis], aVelRatio.data[axis], idealThr.data[axis], rot.data[axis], m_thrustRefVal.data[axis]);
      if (axis!=2) fprintf(m_dumpFile, ",");
    }
    fprintf(m_dumpFile, "\n");
  }

  for (int axis = minaxis; axis < 3; axis++) {

    m_vessel->SetAttitudeRotLevel(axis, rot.data[axis]);

    m_thrustLastTime[axis] = (abs(rot.data[axis]) >= 0.0001);
    if (m_thrustLastTime[axis]) {
      int i = m_thrustRefIx[axis];
      assert(i >= 0 && i <= 9);
      m_thrustRefData[i].aVel.data[axis] = aVel.data[axis];
      m_thrustRefData[i].thrust.data[axis] = rot.data[axis];
    }
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
    fprintf(m_dumpFile, "Ix, SimT, SimDT, TotT,");
    for (char c = 'X'; c <= 'Z'; c++) {
      fprintf(m_dumpFile, "%c_RefIx, %c_AngToTgt, %c_AACC, %c_AVEL, %c_CycParam, %c_CycToHit, %c_aVelRatio, %c_IdealThr, %c_ActThr, %c_ThRefVal",
                            c,        c,           c,       c,       c,           c,           c,            c,           c,         c);
      if (c!= 'Z') fprintf(m_dumpFile, ",");
    }
    fprintf(m_dumpFile, "\n");
  }

}

void Lagrange_AP::Disable()
{
  m_vessel->ActivateNavmode(NAVMODE_KILLROT);
  SetTargetVector(_V(0.0, 0.0, 0.0));
  SetRot0();
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