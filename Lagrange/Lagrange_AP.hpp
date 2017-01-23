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
  void Update(VESSEL* v, const double SimT, const double SimDT, const double burnSimDT, const VECTOR3 &targetVector, const VECTOR3 &Qr, const VECTOR3 &Pr);
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
  void WarpKillRot(VESSEL* v, char axis);
  VECTOR3 ConvertTransXTarget(VESSEL* v, const VECTOR3 &trxVec, const VECTOR3 &Qr, const VECTOR3 &Pr, VECTOR3 &proHat, VECTOR3 &plcHat, VECTOR3 &outHat);
  VECTOR3 GetRotationToTarget(const VECTOR3 &target) const;
  VECTOR3  m_targetVector;
  VECTOR3  m_targetVectorLocal;
  VECTOR3 m_targetProAxis;
  VECTOR3 m_targetPlcAxis;
  VECTOR3 m_targetOutAxis;

  double   m_targetLengthPrev;
  bool     m_isEnabled;

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