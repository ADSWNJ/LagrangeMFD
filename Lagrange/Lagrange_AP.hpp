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
#include <vector>
#include "OneAxis_AP.hpp"
class Lagrange_AP
{
public:
  Lagrange_AP();
  virtual ~Lagrange_AP();
  void Update_PlanMode(VESSEL* v, const int apState, const double SimT, const double SimDT, const double burnSimT, const VECTOR3 &targetVector, const VECTOR3 &Qr, const VECTOR3 &Pr);
  void Update_HoldMode(VESSEL* v, const int apState, const double SimT, const double SimDT, const VECTOR3 &Qr, const VECTOR3 &Pr, const VECTOR3 &QLPr, const VECTOR3 &PLPr);
  void Disable(VESSEL* v);
  void Enable(VESSEL* v);
  bool IsEnabled() const;
  VECTOR3 GetAVel() const;
  VECTOR3 GetATT() const;
  bool IsBurnFrozen() const;
  double GetBurnStart() const;
  double GetBurnEnd() const;
  double GetBurnDV() const;
  double GetBurnTgtV() const;
protected:
private:
  void SetRot0(VESSEL* v);
  double ExecuteRotAP(VESSEL* v, const VECTOR3 angleToTarget, const double SimT, const double SimDT);
  void ExecuteLinAP(VESSEL* v, const VECTOR3 &QLPr, const VECTOR3 &PLPr, const double SimT, const double SimDT);
  void ExecuteBurn(VESSEL *v, double SimT, double SimDT);
  void GetVesselBurnData(VESSEL *v, double burnSimT);
  void AddVesselToStack(VESSEL *v, std::vector<VESSEL*> &stack);
  void WarpKillRot(VESSEL* v, char axis);

  VECTOR3 ConvertTransXTarget(VESSEL* v, const VECTOR3 &trxVec, const VECTOR3 &Qr, const VECTOR3 &Pr, VECTOR3 &trxGbl, const bool burnFrozen);
  VECTOR3 GetRotationToTarget(const VECTOR3 &target) const;
  VECTOR3 m_targetVector;
  VECTOR3 m_targetVectorLocal;
  VECTOR3 m_targetVectorGlobal;
  //VECTOR3 m_targetProAxis;
  //VECTOR3 m_targetPlcAxis;
  //VECTOR3 m_targetOutAxis;
  VECTOR3 m_aVelLast;
  VECTOR3 m_angleToTarget;
  VECTOR3 m_aVel;

  double  m_targetLengthPrev;
  bool    m_isEnabled;
  FILE   *m_dumpFile;
  bool    m_dumping;
  long    m_dumpIx;
  double  m_dumpTotT;
  bool    m_dampspin;
  bool    m_burnFrozen;
  double  m_deltaV;
  double  m_tgtV;
  double  m_startBurnTime;
  double  m_endBurnTime;
  double  m_stackMass;
  double  m_thrust;
  double  m_ISP;

  OneAxis_AP one_ap[6];
};

#endif     /// _LAGRANGE_AP_CLASS