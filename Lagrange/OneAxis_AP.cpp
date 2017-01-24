/* =================================================================================================
*
*	One Axis Auto Pilot
*	===================
*
*	Copyright (C) 2017	Andrew (ADSWNJ) Stokes
*                   All rights reserved
*
*  See OneAxis_AP.hpp for brief description
* =================================================================================================
*/

#include "OneAxis_AP.hpp"
#include <assert.h>

OneAxis_AP::OneAxis_AP() :
  m_axis(-1),
  m_A(1.0),
  m_loA(0.0),
  m_goA(0.0),
  m_zBand(0.0),
  m_fullCali(false),
  m_ix(-2),
  m_dT(0.0),
  m_enabled(false),
  m_lastWarp(0.0)
{}

OneAxis_AP::~OneAxis_AP() {}

void OneAxis_AP::Enable(VESSEL *v, int axis, bool dump) {
  m_enabled = true;
  m_axis = axis+1;
  m_ThMode = ' ';
  
  switch (m_axis) {
  case  1: {
    m_thG_Plus = THGROUP_ATT_PITCHUP;
    m_thG_Minus = THGROUP_ATT_PITCHDOWN;
    break;
  }
  case  2: {
    m_thG_Plus = THGROUP_ATT_YAWLEFT;
    m_thG_Minus = THGROUP_ATT_YAWRIGHT;
    break;
  }
  case  3: {
    m_thG_Plus = THGROUP_ATT_BANKRIGHT;
    m_thG_Minus = THGROUP_ATT_BANKLEFT;
    break;
  }
  case  4: {
    m_thG_Plus = THGROUP_ATT_FORWARD;
    m_thG_Minus = THGROUP_ATT_BACK;
    break;
  }
  case  5: {
    m_thG_Plus = THGROUP_ATT_DOWN;
    m_thG_Minus = THGROUP_ATT_UP;
    break;
  }
  case  6: {
    m_thG_Plus = THGROUP_ATT_RIGHT;
    m_thG_Minus = THGROUP_ATT_LEFT;
    break;
  }
  default: { throw("ERROR IN ExecuteThrust: bad axis"); }
  }
  m_dumping = false;
  if (!dump) return;
  
  char buf[256];
  int err;
  sprintf_s(buf, 256, ".\\Config\\MFD\\Lagrange\\Diags\\OneAxisAP_%d.csv", m_axis);
  if (err = fopen_s(&m_dbgF, buf, "w") != 0) {
    char errbuf[256];
    sprintf(errbuf, strerror(err));
    return;
  }
  m_dumping = true;
  if (m_dbgF) fprintf(m_dbgF, "Type, SimT, SimDT, AvgDT, A, P, V, Thr, Mode, OptT, OptA, LoV, GoV, TrimAd, TrimAv, FinA, loA, goA \n");
}

void OneAxis_AP::SetAccParams(double A, double loApct, double goApct) {
  m_A = A;
  m_loApct = loApct;
  m_goApct = goApct;
  m_loA = m_loApct * A;
  m_goA = m_goApct * A;
}

/*
 * Sets the deadband for the position error (and velocity/dT error)
 */
void OneAxis_AP::SetDeadband(double zBand) {
  m_zBand = zBand;
}

/*
 * Calculate the desired thrust level on our axis.
 * Steps:
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
 * Work it all assuming P is posivive, and reverse signs at the end if needed
 */
double OneAxis_AP::CalcThrust(double P, double V, double simT, double dT) {
  if (!m_enabled) return 0.0;
  double sign;
  double warp = oapiGetTimeAcceleration();
  double warpBias = warp <= 1.0 ? 1.0 : warp;

  if (warp != m_lastWarp) {
    m_ix = -2;
    m_fullCali = false;
    m_lastWarp = warp;
  }

  m_P = P;
  m_V = V;
  m_T = simT;
  m_this_dT = dT;
  if (m_dT == 0.0) m_dT = m_this_dT;

  //if (P != 0.0) sign = (P < 0.0) ? 1.0 : -1.0; else sign = 0.0;
  sign = (P < 0.0) ? 1.0 : -1.0;

  if (m_ix == -2) {                                     // First burn with no calibration ... see what happens
    m_thrustPct = 1.0 / warpBias;
    if (sign < 0.0) m_thrustPct = -1.0 / warpBias;
    m_A = 1.0;
    if (m_dumping) {
      //              "SimT, SimDT, AvgDT, A, P, V, TH%, Mode, OptT, OptA, LoV, GoV, TrimAd, TrimAv, FinA ");
      fprintf(m_dbgF, "THR, %.3f, %.4f, %.4f, %.4f, %.4f,%.4f,%.4f,%c, %.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f\n",
        m_T, m_this_dT, m_dT, m_A, m_P, m_V, m_thrustPct, 'I', 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    }
    return m_thrustPct;
  }



  double l_A = m_A / warpBias;
  double l_loA = m_loA / warpBias;
  double l_goA = m_goA / warpBias;

  double aP = abs(P); 
  double finA = 0.0;
  double aV = V * sign;
  double optA = 0.0;
  double optT = 0.0;
  double loV = 0.0;
  double goV = 0.0;
  double trimAd = 0.0;
  double trimAv = 0.0;
  double trimAvg = 0.0;

  if (aP < m_zBand && (abs(V) * warpBias / m_dT / 10.0 < m_zBand)) {
      m_thrustPct = 0.0;
      if (m_dumping) {
        //              "SimT, SimDT, AvgDT, A, P, V, TH%, Mode, OptT, OptA, LoV, GoV, TrimAd, TrimAv, FinA ");
        fprintf(m_dbgF, "THR, %.3f, %.4f, %.4f, %.4f, %.4f, %.4f,%.4f,%c, %.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f\n",
          m_T, m_this_dT, m_dT, m_A, m_P, m_V, m_thrustPct, 'F', 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
      }
      return m_thrustPct;                               // Both the Pos Error and the Vel Error are inside the deadband, so exit
  } else {
    optA = (aV * aV) / (2.0 * aP);                 // Solves 0 = P + V.t + 0.5.A.t^2 and 0 = V + A.t for A, given P and V
    optT = (optA < 1e-6)? 1e10 : aV / optA;      // Solves 0 = V + A.t for t, given V and A
    if (aP == 0.0) {
      optA = aV / m_dT / 10.0;
      optT = 0.0;
    }
    trimAd = 2.0 * (-aP + aV*m_dT) / (m_dT * m_dT);  // Solve 0 = P + V.t + 0.5.A.t^2 for A, given P, V, t
    trimAv = aV / m_dT;                        // Solve 0 = V + A.t for A, given V, t
    trimAvg = (trimAd/4.0 + 4.0*trimAv) / 4.25;
    if (abs(trimAvg) < l_A && abs(trimAd/4.0) < l_A && abs(trimAv) < l_A) {                                  // Trim phase trgger
      m_ThMode = 'T';
      finA = -trimAvg;                   // Average these accelerations to hit the target in 2 iterations
    } else {
      loV = sqrt(2.0 * aP * l_loA);                // Solves 0 = P + V.t + 0.5.A.t^2 and 0 = V + a.t for V given P and A
      goV = sqrt(2.0 * aP * l_goA);                // Solves 0 = P + V.t + 0.5.A.t^2 and 0 = V + a.t for V given P and A
      if (aV < loV) {
        m_ThMode = 'B';
        finA = (loV - aV) / m_dT;                 // Boost phase (to increase closure rate)
      } else if (aV < goV  && m_ThMode != 'D') {
        m_ThMode = 'C';
        finA = 0.0;                                     // Cruise phase waiting for optimal deceleration
      } else {
        m_ThMode = 'D';
        finA = -optA;                 // Deceleration phase, tracking to P=0 and V=0
      }
    }
  }
  m_thrustPct = sign * finA / m_A;


  if (m_thrustPct >  1.00 / warpBias) m_thrustPct =  1.00 / warpBias;
  if (m_thrustPct < -1.00 / warpBias) m_thrustPct = -1.00 / warpBias;

  if (m_dumping) {
    //              "     SimT, SimDT, AvgDT, A,   P,   V,    aV, TH%, Mode, OptT, OptA, LoV, GoV, TrimAd, TrimAv, FinA ");
    fprintf(m_dbgF, "THR, %.3f, %.4f, %.4f, %.4f, %.4f, %.4f,%.7f,%c, %.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f\n",
      m_T, m_this_dT, m_dT, m_A, m_P, m_V, m_thrustPct, m_ThMode, optT, optA, loV, goV, trimAd, trimAv, finA, m_loA, m_goA);
    fflush(m_dbgF);
  }


  return m_thrustPct;
}

/*
 * Execute the desired thrust level on our axis. If the thrust is non-zero, prime the calibration
 * for calcuation, else zero it
 */
void OneAxis_AP::ExecuteThrust(VESSEL *v) {
  double warp = oapiGetTimeAcceleration();
  if (!m_enabled) return;
  int sign = m_thrustPct >= 0.0 ? 1 : -1;

  v->SetThrusterGroupLevel(sign == 1? m_thG_Minus : m_thG_Plus, 0.0);
  v->SetThrusterGroupLevel(sign == 1? m_thG_Plus : m_thG_Minus, abs(m_thrustPct));

  if (m_ix == -2) m_ix = -1;
  int ix = m_ix + 1;
  if (ix > 9) ix = 0;
  if (abs(m_thrustPct) * warp > 1e-7) {
    m_cali[ix] = { m_T, m_this_dT, m_V, m_thrustPct, 0.0 };
  } else {
    m_cali[ix] = { 0.0, 0.0, 0.0, 0.0, 0.0 };
  }
}

/*
 * Calculate Calibration Parameters. Based on the dV since our last thrust, calculate the acceleration.
 * For each of the first 10 burns, and each 10 thereafter, recalibrate the average dT and thrust Ref
 */
void OneAxis_AP::CalcCali(double V, double simT, double dT) {
  if (!m_enabled) return;
  if (m_ix == -2) return;
  int ix = m_ix+1;
  if (ix > 9) ix = 0;
  double warp = oapiGetTimeAcceleration();
  double warpBias = warp <= 1.0 ? 1.0 : warp;

  if (abs(m_cali[ix].T - (simT - dT)) < 1e-3 && abs(m_cali[ix].thPct) > 0.25 / warpBias) {
    m_cali[ix].A = (V - m_cali[ix].V) / dT / m_cali[ix].thPct;
    
    // if (m_dumping) fprintf(m_dbgF, "CalcCali, %f, %f,, %f,,%f,%f, %d\n", simT, dT, m_cali[ix].A,V - m_cali[ix].V, m_cali[ix].thPct, ix);
    m_ix++;
    if (m_ix > 9) {
      m_ix = 0;
      m_fullCali = true;
    }
    if (m_ix == 9 || !m_fullCali) {
      m_A = 0.0;
      m_dT = 0.0;
      for (int i = 0; i <= m_ix; i++) {
        m_A += m_cali[i].A;
        m_dT += m_cali[i].dT;
      }
      m_A /= 1.0 * (m_ix + 1);
      m_dT /= 1.0 * (m_ix + 1);
      m_loA = m_A * m_loApct;
      m_goA = m_A * m_goApct;
      //  if (m_dumping) fprintf(m_dbgF, "CalcUPDT, %f, %f,, %f,,,, %d\n", simT, dT, m_cali[ix].A, ix);
    }
  }
}

/*
 * Clean shutoff of AP
 */
void OneAxis_AP::Disable() {
  m_enabled = false;
  if (m_dumping) {
    fflush(m_dbgF);
    fclose(m_dbgF);
  }
}