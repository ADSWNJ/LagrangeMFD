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
  void Update(VESSEL* v, const double SimT, const double SimDT, const VECTOR3 & targetVector, const OBJHANDLE hRefBody);
  void SetTargetVector(VESSEL* v, const VECTOR3 & targetVector, const OBJHANDLE hRefBody);
  void Disable(VESSEL* v);
  void Enable(VESSEL* v);
  bool IsEnabled() const;
  VECTOR3 GetAVel() const;
  VECTOR3 GetATT() const;
protected:
private:
  void MECO(VESSEL* v);
  void MainEngineOn(VESSEL* v, double level);
  void SetRot0(VESSEL* v);
  void ExecuteRotAP(VESSEL* v, const VECTOR3 angleToTarget, const double SimT, const double SimDT);
  void GetTransXTarget(VESSEL* v, const VECTOR3 & trxVec, const OBJHANDLE hRefBody);
  VECTOR3 GetRotationToTarget(const VECTOR3 & target, VECTOR3 *tgtFwd, VECTOR3 *tgtUp) const;
  VECTOR3  m_targetVector;
  VECTOR3  m_targetVectorUp;
  VECTOR3  m_targetVectorFwd;
  VECTOR3  m_targetVectorUnit;
  double   m_targetLengthPrev;
  bool     m_isEnabled;
  VECTOR3  m_trxOut;
  VECTOR3  m_trxPlc;
  VECTOR3  m_trxPro;
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