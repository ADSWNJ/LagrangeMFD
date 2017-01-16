/* =================================================================================================
 *
 *	One Axis Auto Pilot
 *	===================
 *
 *	Copyright (C) 2017	Andrew (ADSWNJ) Stokes
 *                   All rights reserved
 *
 *
 *  Controls thrust on a single axis (rotation or translation)
 *  to achieve zero DP and zero DV
 *
 *	Operation:
 *    1. Calculate the deceleration needed to hit P=0 and V=0 simultaneously (optA)
 *    2. Find loV, and goV, representing the hypotetical starting V to hit D=0 with a
 *         deceleration of loA, and goA respectively
 *    3. Accelerate until the closing velocity is above the loV
 *    4. Coast until the closing velocity is above goV, then 
 *         decelerate at optA (limited by 100% of A)
 *    5. When we are within a dT of the solution, trim the acceleration to solve for
 *        P=0 and V=0 at dT time from now (limited by 100% of A)
 *    6. When P and V*dT are within the deadband, then do nothing
 *
 * =================================================================================================
 */

#ifndef __OneAxis_AP
#define __OneAxis_AP

#include "orbitersdk.h"
class OneAxis_AP
{
public:
  OneAxis_AP();
  virtual ~OneAxis_AP();
  void Enable(const int axis, VESSEL *v, const bool dump);
  void Disable();
  void SetAccParams(const double A, const double loApct, const double goApct);
  void SetDeadband(const double zBand);
  double CalcThrust(const double P, const double V, const double simT, const double dT);
  void ExecuteThrust();
  void CalcCali(const double V, const double simT, const double dT);

protected:
private:
  VESSEL* m_v;
  int m_axis;
  double m_A, m_loA, m_goA;
  double m_loApct, m_goApct;
  double m_zBand;
  double m_thrustPct;
  double m_lastV;
  bool m_fullCali;
  int m_ix;
  struct {
    double T;
    double dT;
    double V;
    double thPct;
    double A;
  } m_cali[10];
  double m_dTref;
  double m_P;
  double m_V;
  double m_T;
  double m_dT;
  double m_this_dT;
  THGROUP_TYPE m_thG_Plus;
  THGROUP_TYPE m_thG_Minus;
  FILE* m_dbgF;
  bool m_dumping;
  char m_ThMode;
  bool m_enabled;
};

#endif // OneAxis_AP