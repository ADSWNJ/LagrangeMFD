// ==============================================================
//
//	Lagrange (AutoPilot)
//	==================================
//
//	Copyright (C) 2016	Andrew (ADSWNJ) Stokes and Keith (Keithth G) Gelling
//                   All rights reserved
//
//	See Lagrange.cpp
//
// ==============================================================

#ifndef _LAGRANGE_AP_CLASS
#define _LAGRANGE_AP_CLASS

#include "orbitersdk.h"
class Lagrange_AP
{
public:
  Lagrange_AP();
  virtual ~Lagrange_AP();
  void Update(double SimDT);
  void SetTargetVector(const VECTOR3 & targetVector); // Setting _V(0,0,0) disables the AP
  void Disable();
  void Enable();
  bool IsEnabled() const;
  void SetVessel(VESSEL* vessel);
  void SetRefBody(const OBJHANDLE hRefBody);
protected:
private:
  void MECO(VESSEL* vessel);
  void MainEngineOn(VESSEL* vessel, double level);
  void SetRot0();
  void SetRotThrust(const VECTOR3 angleToTarget, const double SimDT);
  void GetTransXTarget(const VECTOR3 & trxVec);
  VECTOR3 GetRotationToTarget(const VECTOR3 & target, VECTOR3 *tgtFwd, VECTOR3 *tgtUp) const;
  VECTOR3  m_targetVector;
  VECTOR3  m_targetVectorUp;
  VECTOR3  m_targetVectorFwd;
  VECTOR3  m_targetVectorUnit;
  double   m_targetLengthPrev;
  bool     m_isEnabled;
  VESSEL   *m_vessel;
  VECTOR3  m_trxOut;
  VECTOR3  m_trxPlc;
  VECTOR3  m_trxPro;
  OBJHANDLE m_hRefBody;
  struct {
    VECTOR3  aVel;
    VECTOR3  aAcc;
    VECTOR3  thrust;
    VECTOR3  aAccThrustRatio;
  } m_thrustRefData[10];
  int m_thrustRefIx[3];
  VECTOR3 m_thrustRefVal;
  bool m_thrustLastTime[3];
  bool m_fullCycle[3];
  FILE *m_dumpFile;
  bool m_dumping;
  long m_dumpIx;
  double m_dumpTotT;
  bool m_dampspin;
  VECTOR3 m_aVelLast;
};

#endif     /// _LAGRANGE_AP_CLASS