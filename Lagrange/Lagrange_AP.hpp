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

#ifndef _LAGRANGE_AP_CLASS
#define _LAGRANGE_AP_CLASS

#include "orbitersdk.h"
#include "OneAxis_AP.hpp"
class Lagrange_AP
{
public:
  Lagrange_AP();
  virtual ~Lagrange_AP();
  void Update(double SimT, double SimDT);
  void SetTargetVector(const VECTOR3 & targetVector); // Setting _V(0,0,0) disables the AP
  void Disable();
  void Enable();
  bool IsEnabled() const;
  void SetVessel(VESSEL* vessel);
  void SetRefBody(const OBJHANDLE hRefBody);
  OBJHANDLE GetRefBody();
  VECTOR3 GetAVel();
  VECTOR3 GetATT();
protected:
private:
  void MECO(VESSEL* vessel);
  void MainEngineOn(VESSEL* vessel, double level);
  void SetRot0();
  void ExecuteRotAP(const VECTOR3 angleToTarget, const double SimT, const double SimDT);
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
  FILE *m_dumpFile;
  bool m_dumping;
  long m_dumpIx;
  double m_dumpTotT;
  bool m_dampspin;
  VECTOR3 m_aVelLast;
  VECTOR3 m_angleToTarget;
  VECTOR3 m_aVel;

  OneAxis_AP one_ap[6];
};

#endif     /// _LAGRANGE_AP_CLASS