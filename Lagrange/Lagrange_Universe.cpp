// =================================================================================
//
//	LagrangeUniverse (All calculations for the LP and related body state progation)
//	===============================================================================
//
//	Copyright (C) 2016-2017	Andrew (ADSWNJ) Stokes and Keith (Keithth G) Gelling
//                   All rights reserved
//
//	See Lagrange.cpp
//
// ==================================================================================

using namespace std;
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <sys/timeb.h>
#include <cassert>
#include <thread>
#include <limits>
#include "orbitersdk.h"
#include "Lagrange_GCore.hpp"
#include "Lagrange_Universe.hpp"


inline VECTOR3 operator* (const double f, const VECTOR3 &a)
{
  VECTOR3 c;
  c.x = a.x*f;
  c.y = a.y*f;
  c.z = a.z*f;
  return c;
}

// ====================================================================================================================
// Global variables

extern Lagrange_GCore *g_SC;    // points to the static persistence core

LagrangeUniverse::LagrangeUniverse() {

  draw = &(g_SC->draw);

  s4i_canstart = false;
  for (int i = 0; i < 2; i++) {
    s4int_count[i] = 20000;
    s4int_timestep[i] = 30.0;
  }
  s4int_hysteresis = 10.0;

  defBody(&body[0], 0, "Sun", 695700 + 10000, 695700 + 2000);
  defBody(&body[1], 1, "Earth", 6371 + 400, 6371 + 120);
  defBody(&body[2], 2, "Moon", 1737 + 250, 1737 + 10);
  defBary(&body[3], 3, "E-M B", LU_EARTH, LU_MOON);


  // LP Definitions: see http://www.orbiter-forum.com/showthread.php?t=36110 for commentary on these values
  // Meanings:
  //              Index    Name       LP#  Mean Radius   Alpha Val             Plot Center  Major     Minor    Other   
  defLP(&lptab[0], 0, "Earth Moon L1", 1,    384405000, 0.836915194872059889706, LU_EARTH, LU_EARTH, LU_MOON, LU_SUN);  // EML1
  defLP(&lptab[1], 1, "Earth Moon L2", 2,    384405000, 1.15568211143362165272,  LU_EARTH, LU_EARTH, LU_MOON, LU_SUN);  // EML2
  defLP(&lptab[2], 2, "Earth Moon L3", 3,    384405000, -1.00506263995930385239, LU_EARTH, LU_EARTH, LU_MOON, LU_SUN);  // EML3
  defLP(&lptab[3], 3, "Earth Moon L4", 4,    384405000, 0.0,                     LU_EARTH, LU_EARTH, LU_MOON, LU_SUN);  // EML4
  defLP(&lptab[4], 4, "Earth Moon L5", 5,    384405000, 0.0,                     LU_EARTH, LU_EARTH, LU_MOON, LU_SUN);  // EML5
  defLP(&lptab[5], 5, "Sun Earth L1",  1,    384405000, 0.989985982345709235260, LU_SUN,   LU_SUN,   LU_EARTHMOONBARY); // SEL1
  defLP(&lptab[6], 6, "Sun Earth L2",  2, 149600000000, 1.01007520001973933176,  LU_SUN,   LU_SUN,   LU_EARTHMOONBARY); // SEL2
  defLP(&lptab[7], 7, "Sun Earth L3",  3, 149600000000, -1.00000126684308386748, LU_SUN,   LU_SUN,   LU_EARTHMOONBARY); // SEL3
  defLP(&lptab[8], 8, "Sun Earth L4",  4, 149600000000, 0.0,                     LU_SUN,   LU_SUN,   LU_EARTHMOONBARY); // SEL4
  defLP(&lptab[9], 9, "Sun Earth L5",  5, 149600000000, 0.0,                     LU_SUN,   LU_SUN,   LU_EARTHMOONBARY); // SEL5

  // LP Orbit plot definitions
  // Meanings:
  //                     Plot 1     Plot 2    Plot 3          
  defOrbPlot(&lptab[0], LU_EARTH,  LU_MOON); 
  defOrbPlot(&lptab[1], LU_EARTH,  LU_MOON);
  defOrbPlot(&lptab[2], LU_EARTH,  LU_MOON);
  defOrbPlot(&lptab[3], LU_EARTH,  LU_MOON);
  defOrbPlot(&lptab[4], LU_EARTH,  LU_MOON);
  defOrbPlot(&lptab[5], LU_SUN,    LU_EARTH, LU_MOON);
  defOrbPlot(&lptab[6], LU_SUN,    LU_EARTH, LU_MOON);
  defOrbPlot(&lptab[7], LU_SUN,    LU_EARTH, LU_MOON);
  defOrbPlot(&lptab[8], LU_SUN,    LU_EARTH, LU_MOON);
  defOrbPlot(&lptab[9], LU_SUN,    LU_EARTH, LU_MOON);

  for (int i = 0; i < 2; i++) {
    s4i[i].resize(s4int_count[0]);
  }

  wkg = 0;
  act = 1;
  s4i_valid = false;
  s4i_waitrel = false;
  next_s4i_time = -100.0;
  LP = lptab[0];
  selectLP(0);
  s4int_refresh = 0.0;
  dmp_enc = false;
  dmp_log = false;
  dmp_orb = false;

  orbLegend = false;
  orbFocus = 0;
  orbProj = 0;
  orbZoom = 0;
  orbPrevZoom = 0;
  for (int i = 0; i < 3; i++) {
    orbPanHoriz[i] = 0.0;
    orbPanVert[i] = 0.0;
    orbScale[i] = 10000.0;
  }
  orbFocVix = 0;
  orbFocLock = false;
  orbFocLockX = 0.5;
  orbFocLockY = 0.5;

  // Initialize the thread control structures
  s4i_mstate = s4i_wstate = 'I';
  s4i_canstart = true;
  return;
}

LagrangeUniverse::~LagrangeUniverse() {
  return;
}

int getMilliCount(){       // Credit http://www.firstobject.com/getmillicount-milliseconds-portable-c++.htm
	timeb tb;
	ftime(&tb);
	int nCount = tb.millitm + (tb.time & 0xfffff) * 1000;
	return nCount;
}

int getMilliSpan(int nTimeStart){   // Credit http://www.firstobject.com/getmillicount-milliseconds-portable-c++.htm
	int nSpan = getMilliCount() - nTimeStart;
	if(nSpan < 0)
		nSpan += 0x100000 * 1000;
	return nSpan;
}

void LagrangeUniverse::defBody(LagrangeUniverse_Body *pbodyinst, int p0, char* p1, double proxDist, double impactDist) {
  // Initialize each celestial body
  pbodyinst->ix = p0;
  pbodyinst->pen = draw->GetPen(p1);
  strcpy(pbodyinst->name, p1);
  pbodyinst->hObj = oapiGetGbodyByName(pbodyinst->name);
  pbodyinst->mass = oapiGetMass(pbodyinst->hObj);
  pbodyinst->gm = pbodyinst->mass * GGRAV;
  pbodyinst->isBary = false;
  pbodyinst->b_e[0] = -1;
  pbodyinst->proxWarnDist = proxDist;
  pbodyinst->impactWarnDist = impactDist;
  return;
}

void LagrangeUniverse::defBary(LagrangeUniverse_Body *pbodyinst, int p0, char* p1, int maj, int min) {
  // Initialize each barycenter
  pbodyinst->ix = p0;
  pbodyinst->pen = draw->GetPen(p1);
  strcpy(pbodyinst->name, p1);
  pbodyinst->mass = body[maj].mass + body[min].mass;
  pbodyinst->gm = pbodyinst->mass * GGRAV;
  pbodyinst->isBary = true;
  pbodyinst->b_e[0] = maj;
  pbodyinst->b_e[1] = min;
  pbodyinst->b_e[2] = -1;
  pbodyinst->proxWarnDist = -1.0; // Can't crash into a barycenter!
  pbodyinst->impactWarnDist = -1.0;
  return;
}

void LagrangeUniverse::defLP(LagrangeUniverse_LP_Def *lpdef, int p0, char *p1, int Lnum, double mr, double a, int ref, int maj, int min, int oth1,int oth2) {
  // e.g. &lptab[0], 0, "Earth Moon L1", 1, 384405000,0.836915194872059889706, LU_EARTHMOONBARY, LU_EARTH, LU_MOON, LU_SUN
  lpdef->nxix = p0;
  lpdef->ix = p0;
  strcpy(lpdef->name, p1);
  lpdef->ref = ref;
  lpdef->maj = maj;
  lpdef->min = min;
  lpdef->Lnum = Lnum;
  lpdef->mradius = mr;
  lpdef->alpha = a;
  lpdef->gm1 = GGRAV * body[maj].mass;
  lpdef->gm2 = GGRAV * body[min].mass;
  lpdef->gm  = lpdef->gm1 + lpdef->gm2;
  lpdef->mu1 = lpdef->gm1 / lpdef->gm;
  lpdef->mu2 = lpdef->gm2 / lpdef->gm;

  int ent_load[6] = {ref, maj, min, oth1, oth2, -1};
  int countpBody = 0;
  int countpBary = 0;
  int e, b, k;
  lpdef->bodyIx[countpBody] = -1;
  lpdef->baryIx[countpBary] = -1;

  /*
   * Figure out each element to update for this LP. If barycenter, insert the underlying elements before our barycenter.
   * Make sure we do not duplicate any celestial elements. 
   */
  for (int i=0; ent_load[i]!=-1; i++) {
    e = ent_load[i]; // e is entity index, e.g. LU_EARTH
    if (body[e].isBary) {
      for (int j=0; body[e].b_e[j]!=-1; j++) {
        b = body[e].b_e[j];
        for (k=0; k<countpBody; k++) {
          if (lpdef->bodyIx[k] == b) break;
        }
        if (k==countpBody) {
          lpdef->bodyIx[countpBody++] = b;
          lpdef->bodyIx[countpBody] = -1;
        }
      }
      for (k=0; k<countpBary; k++) {
        if (lpdef->baryIx[k] == e) break;
      }
      if (k==countpBary) {
        lpdef->baryIx[countpBary++] = e;
        lpdef->baryIx[countpBary] = -1;
      }
    } else {
      for (k = 0; k<countpBody; k++) {
        if (lpdef->bodyIx[k] == e) break;
      }
      if (k==countpBody) {
        lpdef->bodyIx[countpBody++] = e;
        lpdef->bodyIx[countpBody] = -1;
      }
    }
  }
  return;
}

void LagrangeUniverse::defOrbPlot(LagrangeUniverse_LP_Def *lptab, int b1, int b2, int b3) {
  lptab->plotix[0] = b1;        // Body 1
  lptab->plotPen[0] = b1 < 0 ? NULL : draw->GetPen(body[b1].name);
  lptab->plotix[1] = -2;        // LP
  lptab->plotPen[1] = draw->GetPen("LP");
  lptab->plotix[2] = b2;        // Body 2
  lptab->plotPen[2] = b2 < 0 ? NULL : draw->GetPen(body[b2].name);
  lptab->plotix[3] = b3;        // Body 3
  lptab->plotPen[3] = b3 < 0 ? NULL : draw->GetPen(body[b3].name);
}

int LagrangeUniverse::selectNextLP() {
  selectLP(LP.nxix+1);
  return LP.nxix;
}
int LagrangeUniverse::selectPrevLP() {
  selectLP(LP.nxix-1);
  return LP.nxix;
}
int LagrangeUniverse::getLP() {
  return LP.nxix;
}

inline QP_struct& QP_struct::operator=(const QP_struct& x) {this->P = x.P; this->Q = x.Q; return *this; }

inline LagrangeUniverse::LagrangeUniverse_LP& LagrangeUniverse::LagrangeUniverse_LP::operator=(const LagrangeUniverse_LP_Def& x) {
  this->ix = x.ix;
  this->nxix = x.ix;
  strcpy(this->name,x.name);
  this->ref = x.ref;
  this->maj = x.maj;
  this->min = x.min;
  this->Lnum = x.Lnum;
  this->mradius = x.mradius;
  this->alpha = x.alpha;
  this->gm1 = x.gm1;
  this->gm2 = x.gm2;
  this->gm = x.gm;
  this->mu1 = x.mu1;
  this->mu2 = x.mu2;


  int i;
  for (i=0; x.bodyIx[i]!=-1; i++) this->bodyIx[i] = x.bodyIx[i];
  this->bodyIx[i] = -1;
  for (i=0; x.baryIx[i]!=-1; i++) this->baryIx[i] = x.baryIx[i];
  this->baryIx[i] = -1;
  for (i = 0; i < ORB_MAX_LINES; i++) {
    this->plotix[i] = x.plotix[i];
    this->plotPen[i] = x.plotPen[i];
  }

  return *this;
}

inline Lagrange_vdata& Lagrange_vdata::operator=(const Lagrange_vdata& x) {
  this->v = x.v;
  if (this->vs4i.size() != x.vs4i.size()) this->vs4i.resize(x.vs4i.size());
  this->vs4i[0] = x.vs4i[0];
  this->curMass = x.curMass;
  this->burnMJD = x.burnMJD;
  this->burndV = x.burndV;
  this->enc_ix = 0;
  return *this;
}

void LagrangeUniverse::selectLP(int i) {
  //
  // Driven by TGT select mode: modifies LagrangeUniverse current LP point of interest data
  //
  if (i>=COUNT_LP) { i = 0; }
  else if (i<0) {i = COUNT_LP - 1;}
  LP.nxix = i;
  s4i[act][0].sec = oapiGetSimTime();
  s4i[act][0].MJD = oapiGetSimMJD();
  return;
}


void LagrangeUniverse::updateUniverse() {
  // Update current state vectors (Q = pos, P = vel) for the interesting components in our LP system
  // E.g. for S-E, update Sun, Earth, Moon, and the E-M barycenter
  // E.g. for E-M, update Sun, Earth, Moon

  LagrangeUniverse_Body *b;
  int k, ix;

  for (int i=0; i<COUNT_BODY; i++) {
    b = &body[i];
    ix = b->ix;
    if (!b->isBary) {
      // Regular celestial body ... just get current settings
      b->hObj = oapiGetGbodyByName(b->name);
      oapiGetGlobalPos(b->hObj, &s4i[act][0].body[ix].Q);
      oapiGetGlobalVel(b->hObj, &s4i[act][0].body[ix].P);
    } else {
      // Barycenter ... calculate weighted average from each component
      // Note: the celestial bodies *must* be updated before the barycenter can be computed, obviously!
      s4i[act][0].body[ix].Q = _V(0.0,0.0,0.0);
      s4i[act][0].body[ix].P = _V(0.0,0.0,0.0);
      for (int j=0; j<COUNT_BODY;j++) {
        if (b->b_e[j]==-1) break;
        k = b->b_e[j];
        s4i[act][0].body[ix].Q += s4i[act][0].body[k].Q * (body[k].mass / body[i].mass);
        s4i[act][0].body[ix].P += s4i[act][0].body[k].P * (body[k].mass / body[i].mass);
      }
    }
  }
  if (LP.Lnum<4) {
    lp123(0,act);
  } else {
    lp45(0,act);
  }
  return;
}

void LagrangeUniverse::lp123(const int n, const int s) {
  //
  // Finds L1-L3 Lagrange Points for the currently selected LP
  //
  // Return LP Q and P in global frame on gblLP and vessel-relative in vesLP
  //
  // Algorithm credit: Keith "Keithth G" Gelling: see http://www.orbiter-forum.com/showthread.php?t=36110
  //


  // e1->Q = _V(110209675609.86, -104343678290.24, -21151067.147451);
  // e1->P = {20087.76469068, 21448.672842097, -1.6819678198699);

  // e2->Q = {109840514601.5,-104503902072.66, 2956232.3922426);
  // e2->P = {20459.660769246, 20554.432819659, 66.536098699205);

  struct QP_struct dex, m1, m2;
  VECTOR3 com, cov, r, v, e, xHat, yHat, zHat;
  double  vsq, rln, rv, gm, ecc, a, nu, k1, k2, k3, cnu, snu, alpha;

  m1 = s4i[s][n].body[LP.maj];
  m2 = s4i[s][n].body[LP.min];
  gm = LP.gm;

  //calculate the weighted average position and velocity of the two dominant bodies (M1 and M2)
  com = (m1.Q * LP.mu1) + (m2.Q * LP.mu2);
  cov = (m1.P * LP.mu1) + (m2.P * LP.mu2);

  //calculate the position of M2 relative to M1
  r = m2.Q - m1.Q;
  v = m2.P - m1.P;

  //pre-calc some repeating values
  vsq = dotp(v,v);
  rln = length(r);
  rv = dotp(r,v);

  //calculate:
  //  e = eccentricity vector of M2 relative to M1
  //  ecc = magnitude of eccecntricity (liner value)
  //  a = semi-major axis
  //  nu = mean anomaly
  e = (r *(vsq/gm)) - (v *(rv/gm)) - (r/rln);
  ecc = length(e);
  a = gm / (2.0*gm/rln - vsq);
  nu = acos( dotp(e, r) / (ecc * rln));
  if (dotp(r,v) < 0) nu = 2.0 * PI - nu;

  //calculate the unit vectors of a dextral reference frame aligned with M2's
	//orbital plane and orbital orientation:
	//		'xHat'	- a unit vector pointing in the direction to M2's orbital periapsis
	//		'zHat'	- a unit vector point normal to the M2's orbital plane
	//		'yHat'	- the third unit vector to complete the trio
  xHat = unit_s(e);
  zHat = unit(crossp_s(r, v));
  yHat = unit(crossp_s(zHat, xHat));

  //pre-calc more intermediate vlaues
  k1 = a * (1.0 - ecc * ecc);
  k2 = sqrt(gm/k1);
  cnu = cos(nu);
  snu = sin(nu);
  k3 = 1.0 + ecc * cnu;
  alpha = LP.alpha;

	//calculate the position of the Lagrange point in the dextral reference frame:
	//		'dexQ.x'	- the position of the Lagrange point in the 'xhat' direction
	//		'dexQ.y'	- the position of the Lagrange point in the 'yhat' direction
	//		'dexQ.z' = 0  by definition
	//		'dexP.x'	- the speed of the Lagrange point in the 'xhat' direction
	//		'dexP.y' 	- the speed of the Lagrange point in the 'yhat' direction
	//		'dexP.z' = 0  by definition
  dex.Q.x = alpha * k1 * cnu / k3;
  dex.Q.y = alpha * k1 * snu / k3;
  dex.P.x = -alpha * snu * k2; 
  dex.P.y = alpha * (ecc + cnu) * k2;

  
  //calculate the Lagrange Point
  s4i[s][n].LP.Q = (xHat * dex.Q.x) + (yHat * dex.Q.y)  + com;
  s4i[s][n].LP.P = (xHat * dex.P.x) + (yHat * dex.P.y)  + cov;

  return;
}

void LagrangeUniverse::lp45(const int n, const int s) {
  //
  // Finds L4-L5 Lagrange Points for the currently selected LP at timestep n (e.g. 0-20000), using working set s (e.g. 0 or 1)
  //
  // Return LP Q and P in global frame on gblLP and vessel-relative in vesLP
  //
  // Algorithm credit: Keith "Keithth G" Gelling: see http://www.orbiter-forum.com/showthread.php?t=36110
  // Updated algorithm Jan 2017, following discussions between Keith Gelling and Brian (BrianJ) Jones. 
  //
  //

  struct QP_struct dex, m1, m2;
  VECTOR3 com, cov, r, v, e, xHat, yHat, zHat;
  double  vsq, rln, rdotv, gm, ecc, a, nu, k1, k2, k3, k4, cnu, snu, mu1, mu2;

  double sign = (LP.Lnum == 4) ? 1.0 : -1.0;                      // Determines which terms to reverse sign for L5 calculation

  m1 = s4i[s][n].body[LP.maj];                                    // current S4 integrator Q & P values for the major reference (e.g. Earth)
  m2 = s4i[s][n].body[LP.min];                                    // current S4 integrator Q & P values for the minor reference (e.g. Moon)

  gm = LP.gm;                                                     // gm = GM1 + GM2
  mu1 = LP.mu1;                                                   // mu1 = GM1/gm
  mu2 = LP.mu2;                                                   // mu2 = GM2/gm

  com = (m1.Q * mu1) + (m2.Q * mu2);                              // COM = mu1.Q1 + mu2.Q2
  cov = (m1.P * mu1) + (m2.P * mu2);                              // COV = mu1.P1 + mu2.P2

  r = m2.Q - m1.Q;                                                // r = Q2 - Q1   ... the position of the minor w.r.t. major
  v = m2.P - m1.P;                                                // v = P2 - P1   ... the velocity of the minor w.r.t. major

  vsq = dotp(v, v);                                               // Helpers: v2 = v.v
  rln = length(r);                                                //          rln = |r|
  rdotv = dotp(r, v);                                             //          rdotv = r.v

  e = ((vsq / gm) * r) - ((rdotv / gm) * v) - (r / rln);          // e = (v2/gm).r - {(r.v}/gm}.v - r/|r|  ... the eccentricity vecotr
  ecc = length(e);                                                // |e| is the eccentricity
  a = gm / (2.0*gm / rln - vsq);                                  // a = gm / {2.gm /|r| - v2} ... the semi-major axis of the elliptical orbit
  nu = acos(dotp(e, r) / (ecc * rln));                            // nu = arccos(e.r / |e||r|) ... true anomaly of the minor
  if (rdotv < 0) nu = 2.0 * PI - nu;                              // such that if r.v < 0 then nu = 2.PI - nu

  k1 = a * (1.0 - ecc * ecc);                                     // Helpers: k1 = a(1-e2)
  k2 = sqrt(gm / k1);                                             //          k2 = root{gm /(a(1-e2))}
  cnu = cos(nu);
  snu = sin(nu);
  k3 = 1.0 + ecc * cnu;                                           //          k3 = 1 + e.cos(nu)
  k4 = sqrt(3) / 2;                                               //          k4 = root(3)/2

                                                                  // Dextral reference system: x/y coordinates of LP in rectilinear inertial ref frame centered on major
                                                                  // with x pointing to orbital periapsis of minor, y in the orbital plane pointing in its direction of motion
                                                                  // z is zero by definition (i.e. LP is in the plane of the minor orbit).
  dex.Q.x = (k1 / k3) * ((0.5 - mu2)*cnu - sign*k4*snu);          // Qx for L4 = {a(1-e2)/(1+e.cos(nu)} . {(0.5-mu2)cos(nu) - root(3)/2.sin(nu)}
  dex.Q.y = (k1 / k3) * ((0.5 - mu2)*snu + sign*k4*cnu);          // Qy for L4 = {a(1-e2)/(1+e.cos(nu)} . {(0.5-mu2)sin(nu) + root(3)/2.cos(nu)}
  dex.P.x = -sign * k2 * (k4*(ecc + cnu) + sign*(0.5 - mu2)*snu); // Px for L4 = -{root{gm /(a(1-e2))} . {root(3)/2.(sin(nu)(|e| + cos(nu)) + (0.5 - u2).sin(nu)}
  dex.P.y = -sign * k2 * (k4*snu - sign*(0.5-mu2)*(ecc+cnu));     // Py for L4 = -{root{gm /(a(1-e2))} . {root(3)/2.sin(nu) - (0.5 - u2)*(|e| + cos(nu))}

                                                                  // Conversion to Orbiter's global reference frame
  xHat = unit_s(e);                                               // ^x = e / |e|
  zHat = unit(crossp_s(r, v));                                    // ^z = (r x v) / |(r x v)|
  yHat = unit(crossp_s(zHat, xHat));                              // ^y = ^z x ^x

  s4i[s][n].LP.Q = (dex.Q.x * xHat) + (dex.Q.y * yHat) + com;     // Q in Orbiter global ref frame 
  s4i[s][n].LP.P = (dex.P.x * xHat) + (dex.P.y * yHat) + cov;     // P in Orbiter global ref frame

  if ((LP.maj == LU_EARTH) && (LP.min == LU_MOON)) {              // In the special case of determining EML4 or EML5, we need to factor in the Sun's influence
    struct QP_struct ms;
    VECTOR3 Q1s, Q2s;
    double r1s, r2s, GMs, az1, az2, az, Pz;

    ms = s4i[s][n].body[LU_SUN];                                  // current S4 integrator Q & P values for the Sun
    Q1s = m1.Q - ms.Q;                                            // Relative position of Earth to Sun
    Q2s = m2.Q - ms.Q;                                            // Relative position of Moon to Sun
    r1s = length(Q1s);                                            // r1s = | Q1 - Qs |
    r2s = length(Q2s);                                            // r1s = | Q1 - Qs |
    GMs = body[LU_SUN].gm;                                        // GMs = G . Ms
    az1 = (GMs * dotp(m1.Q - ms.Q, zHat) / (r1s * r1s * r1s));
    az2 = (GMs * dotp(m2.Q - ms.Q, zHat) / (r2s * r2s * r2s));
    az = az2 - az1;                                               // az = {GMs.(Q2z - Qsz)}/r2s^3 - {GMs.(Q1z - Qsz)}/r1s^3

    Pz = -k4 * sqrt(k1 / gm) * (k1 / (k3 * k3)) * az;
    s4i[s][n].LP.P = s4i[s][n].LP.P + sign * Pz * zHat;           // Apply off-plane correction for E-M system
  }

}

void LagrangeUniverse::lp_ves(const int s, const int i, const int w) {

  /*
   * Convert LP data into vessel relative coords and dV
   *
   * vessel index s
   * timestep i
   * working set w
   */

  QP_struct rel, mref;
  VECTOR3 Rt, Vt, XtHat, YtHat, ZtHat;
  Lagrange_ves_s4i *vs4i = &vdata[w][s].vs4i[i];
  QP_struct *ves = &vs4i->ves;
  QP_struct *vesLP = &vs4i->vesLP;

  // calculate the deltas to the LP position and velocity 
  rel.Q = ves->Q - s4i[w][i].LP.Q;
  rel.P = ves->P - s4i[w][i].LP.P;


  // Find two vectors in the plane of our reference body ... take the radius vector and the velocity vector
  mref = s4i[w][i].body[vdata[w][s].refEnt];
  Rt = ves->Q - mref.Q;
  Vt = ves->P - mref.P;

  //transform relative pos/vel into TransX-style prograde, plane change, inner coords
  //for TransX - +X is forward (just our velocity vector), +Y is up (crossp of velocity and radius), and +Z is outward (crossp of fwd and up)
  //note ... if you are on an eccentric orbit, same still applies... so outward is always normal to your velocity, away from the ref body
  //for global coords for Orbiter, +x is right, +y is up, +z is forward
  XtHat = unit_s(Vt);
  YtHat = unit(crossp_s(Vt, Rt));
  ZtHat = unit(crossp_s(XtHat, YtHat));

  vesLP->Q.x = dotp(rel.Q, XtHat);
  vesLP->Q.y = dotp(-rel.Q, YtHat);
  vesLP->Q.z = dotp(-rel.Q, ZtHat);
  vs4i->dQ = length(vesLP->Q);

  vesLP->P.x = dotp(rel.P, XtHat);
  vesLP->P.y = dotp(-rel.P, YtHat);
  vesLP->P.z = dotp(-rel.P, ZtHat);
  vs4i->dP = length(vesLP->P);
  if (i == 0 && w == act) {
    VECTOR3 dbg02 = vesLP->Q;
    VECTOR3 dbg35 = vesLP->P;
  }
//  if (vs4i->dQ < vdata[w][s].enc_Q) {
//    vdata[w][s].enc_Q = vs4i->dQ;
//    vdata[w][s].enc_P = vs4i->dP;
//    vdata[w][s].enc_ix = i;
//  }
  return;
}


void LagrangeUniverse::threadCtrlMain() {
  /*
  * s4i_mstate is the master thread's state with the data buffers
  *            'I' we are in Initialization phase (no worker thread yet)
  *            '0' we are active on buffer A
  *            '1' we are active on buffer B
  */
  if (s4i_mstate == 'I') {
    s4i_mstate = '0';
    s4i_wkill = false;
    s4i_finished = false;
    s4i_pause = false;
    // Lock & load worker thread
    s4i_trafficlight[1].lock(); // No writing on B
    thread s4i_worker = thread(&LagrangeUniverse::threadCtrlWorker, this);
    s4i_worker.detach();
  }
  double nowTime = oapiGetSysTime();

  if (s4i_waitrel) {
    if (nowTime >= next_s4i_time) {
      next_s4i_time = nowTime + s4int_refresh;
      s4i_trafficlight[wkg].unlock(); // Release worker to fill the new wkg buffer
      s4i_waitrel = false;
      return;
    }
  }

  if (!s4i_finished) return;
  int i = s4i_mstate - '0';

  s4i_trafficlight[i].lock(); // Both buffers locked at this point, worker paused, so we are clear to set act and wkg, and transfer over any data or config changes
  /*
   * Thread Data Buffer Swapover start
   */
  {
    LP = lptab[LP.nxix];

    act = i;          // transitioning the worker buffer to active (i.e. to displaying in the MFD) 
    wkg = 1 - act;    // ... and get ready to update the new wkg buffers

    if (vdata[wkg].size() != vdata[act].size()) {  // Vessel created or destroyed in the last swap cycle, so burn our last calc cycle, reset new act side to match the old act
      vdata[act].erase(vdata[act].begin(), vdata[act].end());
      for (auto e = vdata[wkg].begin(); e != vdata[wkg].end(); e++) {
        vdata[act].push_back(*e);
      }
      s4i_valid = false;
    }

    for (unsigned int k = 0; k < vdata[act].size(); k++) {
      if (vdata[act][k].v != vdata[wkg][k].v) {
        vdata[act].erase(vdata[act].begin(), vdata[act].end());
        for (auto e = vdata[wkg].begin(); e != vdata[wkg].end(); e++) {
          vdata[act].push_back(*e);
        }
        s4i_valid = false;
        break;
      }
    }

    for (unsigned int e = 0; e < vdata[act].size(); e++) {
        
      vdata[act][e].burnArmed = vdata[wkg][e].burnArmed;
      vdata[act][e].burnMJD = vdata[wkg][e].burnMJD;
      vdata[act][e].burndV = vdata[wkg][e].burndV;
      vdata[act][e].refEnt = vdata[wkg][e].refEnt;


      bool mmext; 
      int fromTransX;
      if (vdata[wkg][e].burnArmed) {
        mmext = ModMsgPut("Man. date", vdata[wkg][e].burnMJD, vdata[wkg][e].v);
        mmext = ModMsgPut("Prograde vel.", vdata[wkg][e].burndV.x, vdata[wkg][e].v);
        mmext = ModMsgPut("Outward vel.", vdata[wkg][e].burndV.y, vdata[wkg][e].v);
        mmext = ModMsgPut("Ch. plane vel.", vdata[wkg][e].burndV.z, vdata[wkg][e].v);
        mmext = ModMsgPut("InstantaneousBurnTime", (vdata[wkg][e].burnMJD - oapiGetSimMJD())*24.0*60.0*60.0, vdata[wkg][e].v);
        mmext = ModMsgPut("TargetVelocity", vdata[wkg][e].burndV, vdata[wkg][e].v);
      } else {
        mmext = ModMsgPut("Man. date", 0.0, vdata[wkg][e].v);
        mmext = ModMsgPut("Prograde vel.", 0.0, vdata[wkg][e].v);
        mmext = ModMsgPut("Outward vel.", 0.0, vdata[wkg][e].v);
        mmext = ModMsgPut("Ch. plane vel.", 0.0, vdata[wkg][e].v);
        mmext = ModMsgPut("InstantaneousBurnTime", 0.0, vdata[wkg][e].v);
        mmext = ModMsgPut("TargetVelocity", _V(0.0,0.0,0.0), vdata[wkg][e].v);
      }
      mmext = EnjoLib::ModuleMessagingExt().ModMsgGet("TransX", "CurrentBodyIndex", &fromTransX, vdata[wkg][e].v);
      if (mmext) {
        vdata[act][e].TransX_CurrentBodyIndex = (OBJHANDLE)fromTransX;
      } else {
        vdata[act][e].TransX_CurrentBodyIndex = false;
      }
      mmext = EnjoLib::ModuleMessagingExt().ModMsgGet("TransX", "PlanMajorIndex", &fromTransX, vdata[wkg][e].v);
      if (mmext) {
        vdata[act][e].TransX_PlanMajorIndex = (OBJHANDLE)fromTransX;
      } else {
        vdata[act][e].TransX_PlanMajorIndex = false;
      }
    }

    s4i[wkg][0].sec = oapiGetSimTime();            // Initialize the working buffers at current time and MJD
    s4i[wkg][0].MJD = oapiGetSimMJD();

    if (s4int_count[act] != s4int_count[wkg] || s4int_timestep[act] != s4int_timestep[wkg]) {  // If the iteration params changed, burn the last cycle and reset
      s4int_count[act] = s4int_count[wkg];
      s4int_timestep[act] = s4int_timestep[wkg];
      s4i_valid = false;
    }
    s4i_finished = false;
    s4i_mstate = '0' + wkg;
  }
  /*
  * Thread Data Buffer Swapover end
  */


  if (nowTime >= next_s4i_time) {
    next_s4i_time = nowTime + s4int_refresh;
    s4i_trafficlight[wkg].unlock(); // Release worker to fill the new wkg buffer
    s4i_waitrel = false;
  } else {
    s4i_waitrel = true;
  }

  return;
}

void LagrangeUniverse::threadCtrlPauseToggle() {
  if (s4i_pause) {
    s4i_pause = false;
    s4i_valid = false;
    s4i_pauselight.unlock();
  } else {
    if (s4i_wstate == 'P') return; //edge-case if you try to toggle too fast
    s4i_pauselight.lock();
    s4i_pause = true;
  }
  return;
}

void LagrangeUniverse::threadCtrlWorker() {
  /*
  * s4i_wstate is the worker thread's state with the data buffers
  *            'a' worker waiting to start data set a (index 0)
  *            'A' working on data set A
  *            'b' waiting to start data set b (index 1)
  *            'B' working on data set B
  *            'K' responded to kill request and exited
  */
  while (true) {
    s4i_wstate = 'a';
    s4i_trafficlight[0].lock();
    s4i_wstate = 'A';
    integrateUniverse();
    s4i_trafficlight[0].unlock();
    s4i_finished = true;
    if (s4i_wkill) break;
    if (s4i_pause) {
      s4i_wstate = 'P';
      s4i_pauselight.lock();
      s4i_pauselight.unlock();
    }
    s4i_wstate = 'b';
    s4i_trafficlight[1].lock();
    s4i_wstate = 'B';
    integrateUniverse();
    s4i_trafficlight[1].unlock();
    s4i_valid = true;
    s4i_finished = true;
    if (s4i_wkill) break;
  }
  s4i_wstate = 'K';
  return;
}



void LagrangeUniverse::integrateUniverse() {
  //
  // Compute future locations and velocities of celestial elements in our LP system (e.g. sun, moon, vessel, LP)
  //
  // This algorithm implements a 4th order symplectic integrator as described here: http://www.orbiter-forum.com/showthread.php?t=37268
  //
  
  int s_time, ms_elap;

  if (!s4i_canstart || (vdata[wkg].size() == 0)) return;

  s_time = getMilliCount();

  bool _dmp_log = dmp_log;
  bool _dmp_enc = dmp_enc;
  if (vdata[wkg].size() == 0) {
    _dmp_log = false;
    _dmp_enc = false;
  }

  FILE *dump_enc = NULL;

  // S4 Integrator
  double dt, w0, w1;
  int prev = 0;
  int ent;
  dt  = s4int_timestep[wkg];
  w0 = 1.35120719195966;
  w1 = -1.70241438391932;

  Lagrange_ves_s4i *vs4i;
  Lagrange_ves_s4i *vs4i_prev;

  try {
    s4i[wkg].resize(s4int_count[wkg]);
  }
  catch (...) {
    s4int_count[wkg] = 20000;
    s4i[wkg].resize(s4int_count[wkg]);
  }

  for (unsigned int s = 0; s < vdata[wkg].size(); s++) {
    try {
      vdata[wkg][s].vs4i.resize(s4int_count[wkg]);
    }
    catch (...) {
      s4int_count[wkg] = 20000;
      vdata[wkg][s].vs4i.resize(s4int_count[wkg]);
    }
    Lagrange_ves_s4i *vs4i0 = &vdata[wkg][s].vs4i[0];

    vdata[wkg][s].enc_Q = vdata[wkg][s].vs4i[0].dQ;;
    vdata[wkg][s].enc_P = vdata[wkg][s].vs4i[0].dP;;
    vdata[wkg][s].enc_count = 0;
    vdata[wkg][s].enc_ix = 0;
    vdata[wkg][s].block_scan = 2;
    vdata[wkg][s].burn_ix = -1;
    vdata[wkg][s].alarm_state = 0;
  }

  {
    if (_dmp_log) {

      FILE *dump_s4i;
      if (fopen_s(&dump_s4i, ".\\Config\\MFD\\Lagrange\\Diags\\SNAP.csv", "w") == 0) {

        fprintf(dump_s4i, "\"\"--MJD\"\n");
        fprintf(dump_s4i, "%.15lf\n", s4i[wkg][0].MJD);
        fprintf(dump_s4i, "\n");
        fprintf(dump_s4i, "\"\"-- State vectors of Earth\"\n");
        fprintf(dump_s4i, "%.15lf, %.15lf, %.15lf\n", s4i[wkg][0].body[LU_EARTH].Q.x, s4i[wkg][0].body[LU_EARTH].Q.z, s4i[wkg][0].body[LU_EARTH].Q.y);
        fprintf(dump_s4i, "%.15lf, %.15lf, %.15lf\n", s4i[wkg][0].body[LU_EARTH].P.x, s4i[wkg][0].body[LU_EARTH].P.z, s4i[wkg][0].body[LU_EARTH].P.y);
        fprintf(dump_s4i, "\n");
        fprintf(dump_s4i, "\"\"-- State vectors of Moon\"\n");
        fprintf(dump_s4i, "%.15lf, %.15lf, %.15lf\n", s4i[wkg][0].body[LU_MOON].Q.x, s4i[wkg][0].body[LU_MOON].Q.z, s4i[wkg][0].body[LU_MOON].Q.y);
        fprintf(dump_s4i, "%.15lf, %.15lf, %.15lf\n", s4i[wkg][0].body[LU_MOON].P.x, s4i[wkg][0].body[LU_MOON].P.z, s4i[wkg][0].body[LU_MOON].P.y);
        fprintf(dump_s4i, "\n");
        fprintf(dump_s4i, "\"\"-- State vectors of Sun\"\n");
        fprintf(dump_s4i, "%.15lf, %.15lf, %.15lf\n", s4i[wkg][0].body[LU_SUN].Q.x, s4i[wkg][0].body[LU_SUN].Q.z, s4i[wkg][0].body[LU_SUN].Q.y);
        fprintf(dump_s4i, "%.15lf, %.15lf, %.15lf\n", s4i[wkg][0].body[LU_SUN].P.x, s4i[wkg][0].body[LU_SUN].P.z, s4i[wkg][0].body[LU_SUN].P.y);
        fprintf(dump_s4i, "\n");
        fprintf(dump_s4i, "\"\"-- State vectors of Vessel\"\n");
        fprintf(dump_s4i, "%.15lf, %.15lf, %.15lf\n", vdata[wkg][0].vs4i[0].ves.Q.x, vdata[wkg][orbFocVix].vs4i[0].ves.Q.z, vdata[wkg][0].vs4i[0].ves.Q.y);
        fprintf(dump_s4i, "%.15lf, %.15lf, %.15lf\n", vdata[wkg][0].vs4i[0].ves.P.x, vdata[wkg][orbFocVix].vs4i[0].ves.P.z, vdata[wkg][0].vs4i[0].ves.P.y);
        fprintf(dump_s4i, "\n\n");

        fclose(dump_s4i);
      }
    }
  }
  {
    if (_dmp_enc) {

      if (fopen_s(&dump_enc, ".\\Config\\MFD\\Lagrange\\Diags\\ENC.csv", "w") != 0) {
        _dmp_enc = false;
      } else {
        fprintf(dump_enc, "Encounter Trace taken at Sim Time %f using hysteresis %f\n", oapiGetSimTime(), (double)s4int_hysteresis);
        fprintf(dump_enc, "Reason, Chop, Interval, VIx, S4Ix1, S4Ix2, S4Ix3, SimT1, SimT2, SimT3, D1, D2, D3, D2-D1, D3-D2, TREND\n");
        fflush(dump_enc);
      }
    }
  }

  int interpolation_chop = 1;
  unsigned int last_regix = 0;
  unsigned int last_encix = 0;
  
  for (unsigned int cur = 1; cur < s4int_count[wkg]; cur++) {

    //sprintf(oapiDebugString(), "WKG:%d CUR:%d", (int)wkg, cur);
    // S4 Integrator main loop
    int i, j, k;
    double nextdt = dt / (double)interpolation_chop;
    double nextMJD = s4i[wkg][prev].MJD + nextdt / (24.0*60.0*60.0);
    double nextsec = s4i[wkg][prev].sec + nextdt;

    for (unsigned int s = 0; s < vdata[wkg].size(); s++) {
      if (vdata[wkg][s].burnArmed && vdata[wkg][s].burnMJD > s4i[wkg][prev].MJD && vdata[wkg][s].burnMJD <= nextMJD) {
        nextMJD = vdata[wkg][s].burnMJD;
        nextdt = (nextMJD - s4i[wkg][prev].MJD) * (24.0 * 60.0 * 60.0);
        nextsec = s4i[wkg][prev].sec + nextdt;
      }
    }

    s4i[wkg][cur].sec = nextsec;
    s4i[wkg][cur].MJD = nextMJD;
    // The actual S4I algorithm
    {
      // Step 1a, rolling forward from prev
      for (i = 0; LP.bodyIx[i] != -1; i++) {
        ent = LP.bodyIx[i];
        s4i[wkg][cur].body[ent].P = s4i[wkg][prev].body[ent].P;
        s4i[wkg][cur].body[ent].Q = s4i[wkg][prev].body[ent].Q + s4i[wkg][cur].body[ent].P  * (0.5 * w0 * nextdt);
      }

      // Step 1a Vessel Calc, rolling forward from prev
      for (unsigned int s = 0; s < vdata[wkg].size(); s++) {
        vs4i = &vdata[wkg][s].vs4i[cur];
        vs4i_prev = &vdata[wkg][s].vs4i[prev];
        vs4i->ves.P = vs4i_prev->ves.P;
        vs4i->ves.Q = vs4i_prev->ves.Q + vs4i->ves.P  * (0.5 * w0 * nextdt);
      }

      //Step 1b
      for (i = 0; LP.bodyIx[i] != -1; i++) {
        ent = LP.bodyIx[i];
        s4i[wkg][cur].body[ent].P += s4iforce(ent, cur) * (w0 * nextdt);
      }

      //Step 1b Vessel Calc
      for (unsigned int s = 0; s < vdata[wkg].size(); s++) {
        vs4i = &vdata[wkg][s].vs4i[cur];
        vs4i->ves.P += s4iforce_ves(s, cur) * (w0 * nextdt);
      }

      //Step 1c, 2a
      for (i = 0; LP.bodyIx[i] != -1; i++) {
        ent = LP.bodyIx[i];
        s4i[wkg][cur].body[ent].Q += s4i[wkg][cur].body[ent].P * (0.5 * (w0 + w1) * nextdt);
      }

      //Step 1c, 2a Vessel Calc
      for (unsigned int s = 0; s < vdata[wkg].size(); s++) {
        vs4i = &vdata[wkg][s].vs4i[cur];
        vs4i->ves.Q += vs4i->ves.P * (0.5 * (w0 + w1) * nextdt);
      }

      //Step 2b
      for (i = 0; LP.bodyIx[i] != -1; i++) {
        ent = LP.bodyIx[i];
        s4i[wkg][cur].body[ent].P += s4iforce(ent, cur) * (w1 * nextdt);
      }

      //Step 2b Vessel Calc
      for (unsigned int s = 0; s < vdata[wkg].size(); s++) {
        vs4i = &vdata[wkg][s].vs4i[cur];
        vs4i->ves.P += s4iforce_ves(s, cur) * (w1 * nextdt);
      }

      //Step 2c, 3a
      for (i = 0; LP.bodyIx[i] != -1; i++) {
        ent = LP.bodyIx[i];
        s4i[wkg][cur].body[ent].Q += s4i[wkg][cur].body[ent].P * (0.5 * (w1 + w0) * nextdt);
      }

      //Step 2c, 3a Vessel Calc
      for (unsigned int s = 0; s < vdata[wkg].size(); s++) {
        vs4i = &vdata[wkg][s].vs4i[cur];
        vs4i->ves.Q += vs4i->ves.P * (0.5 * (w1 + w0) * nextdt);
      }

      //Step 3b
      for (i = 0; LP.bodyIx[i] != -1; i++) {
        ent = LP.bodyIx[i];
        s4i[wkg][cur].body[ent].P += s4iforce(ent, cur) * (w0 * nextdt);
      }

      //Step 3b Vessel Calc
      for (unsigned int s = 0; s < vdata[wkg].size(); s++) {
        vs4i = &vdata[wkg][s].vs4i[cur];
        vs4i->ves.P += s4iforce_ves(s, cur) * (w0 * nextdt);
      }

      //Step 3c
      for (i = 0; LP.bodyIx[i] != -1; i++) {
        ent = LP.bodyIx[i];
        s4i[wkg][cur].body[ent].Q += s4i[wkg][cur].body[ent].P * (0.5 * (w0)* nextdt);
      }

      //Step 3c Vessel Calc
      for (unsigned int s = 0; s < vdata[wkg].size(); s++) {
        vs4i = &vdata[wkg][s].vs4i[cur];
        vs4i->ves.Q += vs4i->ves.P * (0.5 * (w0)* nextdt);
      }

      // Step 3d Impulse Step: integrate plan mode burn
      for (unsigned int s = 0; s < vdata[wkg].size(); s++) {
        if (vdata[wkg][s].burnArmed && abs(nextMJD - vdata[wkg][s].burnMJD) < 1E-06) {

          // Convert Prograde/Plane/Outer into global coords
          vs4i = &vdata[wkg][s].vs4i[cur];
          VECTOR3 vesDQmaj = vs4i->ves.Q - s4i[wkg][cur].body[vdata[wkg][s].refEnt].Q;
          VECTOR3 vesDPmaj = vs4i->ves.P - s4i[wkg][cur].body[vdata[wkg][s].refEnt].P;
          VECTOR3 vesCross = crossp(vesDQmaj, vesDPmaj);
          VECTOR3 proHat = vesDPmaj / length(vesDPmaj);
          VECTOR3 plaHat = vesCross / length(vesCross);
          VECTOR3 outHat = crossp(proHat, plaHat);
          VECTOR3 impulse;
          impulse.x = vdata[wkg][s].burndV.x * proHat.x + vdata[wkg][s].burndV.y * outHat.x + vdata[wkg][s].burndV.z * plaHat.x;
          impulse.y = vdata[wkg][s].burndV.x * proHat.y + vdata[wkg][s].burndV.y * outHat.y + vdata[wkg][s].burndV.z * plaHat.y;
          impulse.z = vdata[wkg][s].burndV.x * proHat.z + vdata[wkg][s].burndV.y * outHat.z + vdata[wkg][s].burndV.z * plaHat.z;
          vdata[wkg][s].burn = vs4i->ves;
          vs4i->ves.P += impulse;
          vdata[wkg][s].burndVg = impulse;
          vdata[wkg][s].burn_ix = cur;
          vdata[wkg][s].deltaMass = vdata[wkg][s].curMass - (vdata[wkg][s].curMass / exp(length(impulse) / vdata[wkg][s].mainExVel0));
          vdata[wkg][s].burnDuration = vdata[wkg][s].deltaMass * vdata[wkg][s].mainExVel0 / vdata[wkg][s].mainThrust;

        }
      }

      //Barycenter update step
      for (i = 0; LP.baryIx[i] != -1; i++) {
        ent = LP.baryIx[i];
        s4i[wkg][cur].body[ent].Q = _V(0.0, 0.0, 0.0);
        s4i[wkg][cur].body[ent].P = _V(0.0, 0.0, 0.0);
        for (j = 0; body[ent].b_e[j] != -1; j++) {
          k = body[ent].b_e[j];
          s4i[wkg][cur].body[ent].Q += s4i[wkg][cur].body[k].Q * (body[k].mass / body[ent].mass);
          s4i[wkg][cur].body[ent].P += s4i[wkg][cur].body[k].P * (body[k].mass / body[ent].mass);
        }
      }

      // Find LP Q and P
      if (LP.Lnum < 4) {
        lp123(cur, wkg);
      } else {
        lp45(cur, wkg);
      }

      // Find vessel relative LP data
      for (unsigned int s = 0; s < vdata[wkg].size(); s++) {
        lp_ves(s, cur, wkg);
      }
    }

    // Look for encounter points (where we need to binary chop interpolate into the encounters)
    bool rewind = false;
    for (unsigned int s = 0; s < vdata[wkg].size(); s++) {
      if (!vdata[wkg][s].block_scan) { // Look at last 2 data points to see if we are at an encounter 
        double EDM2 = length(vdata[wkg][s].vs4i[cur - 2].vesLP.Q);
        double EDM1 = length(vdata[wkg][s].vs4i[cur - 1].vesLP.Q);
        double EDM0 = length(vdata[wkg][s].vs4i[cur - 0].vesLP.Q);
        double EDM2_1 = 0.0, EDM1_1 = 0.0, EDM0_1 = 0.0;
        if (s == 0 && vdata[wkg].size() > 1) {
          EDM2_1 = length(vdata[wkg][s+1].vs4i[cur - 2].vesLP.Q);
          EDM1_1 = length(vdata[wkg][s+1].vs4i[cur - 1].vesLP.Q);
          EDM0_1 = length(vdata[wkg][s+1].vs4i[cur - 0].vesLP.Q);
        }

        double SECM2 = s4i[wkg][cur - 2].sec;
        double SECM1 = s4i[wkg][cur - 1].sec;
        double SECM0 = s4i[wkg][cur - 0].sec;

        bool minDeltaRange = abs(EDM1 - EDM0) < 1e-5;
        if (_dmp_enc) {
          double EDM12 = EDM1 - EDM2;
          double EDM01 = EDM0 - EDM1;
          if (interpolation_chop > 1) {
            fprintf(dump_enc, "HUNT, ");
          } else {
            fprintf(dump_enc, "Normal, ");
          }
          fprintf(dump_enc, "%d, %.15lf, %u, ", interpolation_chop, dt / (double)interpolation_chop, s);
          fprintf(dump_enc, "%u, %u, %u, ", cur - 2, cur - 1, cur);
          fprintf(dump_enc, "%.15f, %.15f, %.15f, ", s4i[wkg][cur - 2].sec, s4i[wkg][cur - 1].sec, s4i[wkg][cur].sec);
          fprintf(dump_enc, "%.15f, %.15f, %.15f, %+.15f, %+.15f,", EDM2, EDM1, EDM0, EDM12, EDM01);
          if (EDM12 <= 0.0) {
            if (EDM01 >= 0.0) {
              fprintf(dump_enc, "INVERSION\n");
            } else {
              fprintf(dump_enc, "Reduce\n");
            }
          } else {
            if (EDM01 > 0.0) {
              fprintf(dump_enc, "Increase\n");
            } else {
              fprintf(dump_enc, "SADDLE\n");
            }
          }
        }
        if ((EDM2 >= EDM1 && EDM1 <= EDM0) ||               // Inversion found
            (interpolation_chop > 64 && minDeltaRange) ||   // Inside min range
            (EDM1 <= EDM0 && interpolation_chop > 1)) {     // Beyond inversion at higher search


          if (interpolation_chop < 16384 && !minDeltaRange) {
            if (interpolation_chop == 1) {
              last_regix = cur - 2;
            }

            interpolation_chop *= 2;  // Double the resolution (up to max of 1024)
            for (unsigned int ss = 0; ss < vdata[wkg].size(); ss++) vdata[wkg][ss].block_scan = 1;
            rewind = true;
            break;
          } else {
            vdata[wkg][s].enc_count++;
            if (_dmp_enc) {
              fprintf(dump_enc, "END\n");
            }
            interpolation_chop = 1; // Release the pressure ... we have enough accuracy now on the search
            for (unsigned int ss = 0; ss < vdata[wkg].size(); ss++) vdata[wkg][ss].block_scan = 2;
            last_encix = cur - 1;
            // Now we need to move the encounter to last_regix+1. However, if we have any burns inbetween then make sure we keep those lines
            last_regix++; // Now pointing to first insert point
            unsigned int kk = last_regix;
            bool intervening_burn = false;
            while (kk < last_encix) {
              for (unsigned int c = 0; c < vdata[wkg].size(); c++) {
                if (vdata[wkg][c].burnArmed && abs(s4i[wkg][kk].MJD - vdata[wkg][c].burnMJD) < 1E-06) {
                  intervening_burn = true;
                  if (_dmp_enc) {
                    fprintf(dump_enc, "BURN,,,%u,,,%u,,,%.6lf\n", s, kk, s4i[wkg][kk].sec);
                  }
                  break;
                }
              }
              if (intervening_burn) {
                break;
              }
              kk++;
            }
            // fix up map now, so either the intervening burn or the encounter comes after the last regular delta-time
            s4i[wkg][last_regix] = s4i[wkg][kk];
            for (unsigned int c = 0; c < vdata[wkg].size(); c++) {
              vdata[wkg][c].vs4i[last_regix] = vdata[wkg][c].vs4i[kk];

              if (vdata[wkg][c].vs4i[last_regix].dQ < vdata[wkg][c].enc_Q - (vdata[wkg][s].enc_count > 0)?(double) s4int_hysteresis:0.0) {
                vdata[wkg][c].enc_Q = vdata[wkg][c].vs4i[last_regix].dQ;
                vdata[wkg][c].enc_P = vdata[wkg][c].vs4i[last_regix].dP;
                vdata[wkg][c].enc_ix = last_regix;
                vdata[wkg][c].block_scan = 2;
                if (_dmp_enc) {
                  fprintf(dump_enc, "BESTENC,");
                }
              } else {
                if (_dmp_enc) {
                  fprintf(dump_enc, "NOTBEST,");
                }
              }
              if (_dmp_enc) {
                fprintf(dump_enc, " , , %u, ", c);
                fprintf(dump_enc, "%d, %u, , ", vdata[wkg][c].enc_ix, last_regix);
                fprintf(dump_enc, "%.15f, %.15f, , ", s4i[wkg][vdata[wkg][c].enc_ix].sec,  s4i[wkg][last_regix].sec);
                fprintf(dump_enc, "%.15f, %.15f, %.15f, ,%f, \n", vdata[wkg][c].enc_Q, vdata[wkg][c].vs4i[last_regix].dQ, vdata[wkg][c].vs4i[last_regix].dQ - vdata[wkg][c].enc_Q, (vdata[wkg][s].enc_count > 0) ? (double)s4int_hysteresis : 0.0);
              }

            }
            cur = last_regix;
            prev = cur - 1;
            if (cur < 2) break;

            if (_dmp_enc) {
              fprintf(dump_enc, "FIXUP, , , , ");
              fprintf(dump_enc, "%u, %u, %u, ", cur - 2, cur - 1, cur);
              fprintf(dump_enc, "%.15f, %.15f, %.15f, ", s4i[wkg][cur - 2].sec, s4i[wkg][cur - 1].sec, s4i[wkg][cur].sec);
              fprintf(dump_enc, ", , \n");
            }
            break;
          }
        }
      }
    }

    // If we have found an inversion point, and we are not a max resolution, then we need to back up 2 iterations and re-run them at a finer resolution
    if (rewind) {
      cur -= 2;
      prev--;
      continue;
    }

    for (unsigned int s = 0; s < vdata[wkg].size(); s++) {
      if (vdata[wkg][s].block_scan) {
        vdata[wkg][s].block_scan--;
      }
    }

    prev++;
  }


  // Generate the orb_plots
  VECTOR2 max_Q, min_Q;
  double scale; 
  bool _dmp_orb = dmp_orb;
  FILE *dump_orb = NULL;
  if (_dmp_orb) {
    if (fopen_s(&dump_orb, ".\\Config\\MFD\\Lagrange\\Diags\\OrbPlot.csv", "w") != 0) {
      _dmp_orb = false;
    } else {
      fprintf(dump_orb, "Orbit Plot Dump at Simulation Time:,,, %.2f\n", oapiGetSimTime());

      fprintf(dump_orb, "Plot Count:,,, %d\n", ORB_PLOT_COUNT);
      fprintf(dump_orb, "LP:,,, %s\n", LP.name);
      Lagrange_vdata *lvd = &vdata[wkg][orbFocVix];
      fprintf(dump_orb, "FRM:,,, %s\n", body[lvd->refEnt].name);

      char *PrjTxt[3] = { "Std", "X-Edge", "Z-Edge" };
      char FocTxt[5][32] = { "", "", "Ves Live", "Ves Enc", "Ves Burn" };
      strcpy(FocTxt[0], body[LP.maj].name);
      strcpy(FocTxt[1], body[LP.min].name);
      fprintf(dump_orb, "FOC:,,, %s\n", FocTxt[orbFocus]);
      fprintf(dump_orb, "PRJ:,,, %s\n", PrjTxt[orbProj]);
      fprintf(dump_orb, "Zoom:,,, %d\n", (int)orbZoom);
      fprintf(dump_orb, "PanRightLeft:,,,%f,%f,%f\n",
        (double) orbPanHoriz[0],
        (double)orbPanHoriz[1],
        (double)orbPanHoriz[2]);
      fprintf(dump_orb, "PanUpDown:,,,%f,%f,%f\n",
        (double)orbPanVert[0],
        (double)orbPanVert[1],
        (double)orbPanVert[2]);

      for (int i = 0; i < ORB_MAX_LINES; i++) {
        if (LP.plotix[i] >= 0) {
          fprintf(dump_orb, "Plot Line %i:,,, %-12s(%s)\n", i, body[LP.plotix[i]].name, draw->GetPlotColor(body[LP.plotix[i]].name));
        } else if (lptab->plotix[i] == -2) {
          fprintf(dump_orb, "Plot Line %i:,,, %-12s(%s)\n", i, "LP", draw->GetPlotColor("LP"));
        }
      }
      fprintf(dump_orb, "Plot Line %i:,,, %-12s(%s)\n", ORB_MAX_LINES, "Vessel Live", draw->GetPlotColor("VL"));
      fprintf(dump_orb, "Plot Line %i:,,, %-12s(%s)\n\n", ORB_MAX_LINES, "Vessel Plan", draw->GetPlotColor("VP"));
      fprintf(dump_orb, "OrbIx,S4IIx,FocX,FocY,VX,VY,LPX,LPY,MajX,MajY,MinX,MinY,OthX,OthY,LoX,HiX,LoY,HiY\n");
    }
  }
  
  {
    bool def_Q = false;
    int _orbProj = orbProj;
    int _orbFocus = orbFocus;
    int _orbFocVix = orbFocVix;
    int _orbPrevZoom = orbPrevZoom;
    int _orbZoom = orbZoom;

    for (unsigned int s = 0; s < ORB_MAX_LINES; s++) {
      l_orb[wkg].orb_km[s].resize(ORB_PLOT_COUNT);
      l_orb[wkg].orb_plot[s].resize(ORB_PLOT_COUNT);
    }
    for (unsigned int v = 0; v < vdata[wkg].size(); v++) {
      vdata[wkg][v].orb_km.resize(ORB_PLOT_COUNT);
      vdata[wkg][v].orb_plot.resize(ORB_PLOT_COUNT);
      vdata[wkg][v].orb_plot_body_enc.resize(ORB_MAX_LINES);
    }
    unsigned int zd = (s4int_count[wkg] - 1) / (ORB_PLOT_COUNT - 1);
    unsigned int z = 0;

    int ax, ay;
    switch (_orbProj) {
    case 0: ax = 0; ay = 2; break;
    case 1: ax = 0; ay = 1; break;
    case 2: ax = 2; ay = 1; break;
    }
    double hPan = orbPanHoriz[_orbProj];
    double vPan = -orbPanVert[_orbProj];

    // scan the Q values to generate relatives to the major body (in km)
    for (unsigned int s = 0; s < ORB_PLOT_COUNT; s++) {
      int pix;
      VECTOR2 Q_foc;
      switch (_orbFocus) {
      case 0:  // Focus on the MAJOR entity
        Q_foc.x = s4i[wkg][z].body[LP.maj].Q.data[ax];
        Q_foc.y = s4i[wkg][z].body[LP.maj].Q.data[ay];
        break;
      case 1:  // Focus on the MINOR entity
        Q_foc.x = s4i[wkg][z].body[LP.min].Q.data[ax];
        Q_foc.y = s4i[wkg][z].body[LP.min].Q.data[ay];
        break;
      case 2: // Focus on the Vessel Relative
        Q_foc.x = vdata[wkg][_orbFocVix].vs4i[z].ves.Q.data[ax];
        Q_foc.y = vdata[wkg][_orbFocVix].vs4i[z].ves.Q.data[ay];
        break;
      case 3: // Focus on the Vessel Encounter point
        pix = vdata[wkg][_orbFocVix].enc_ix;
        if (pix == -1) {
          if (vdata[wkg][_orbFocVix].vs4i[0].dQ < vdata[wkg][_orbFocVix].vs4i[s4int_count[wkg] - 1].dQ) {
            pix = 0;
          } else {
            pix = s4int_count[wkg] - 1;
          }
        }
        Q_foc.x = vdata[wkg][_orbFocVix].vs4i[pix].ves.Q.data[ax];
        Q_foc.y = vdata[wkg][_orbFocVix].vs4i[pix].ves.Q.data[ay];
        break;
      case 4: // Focus on the Vessel Burn point
        pix = vdata[wkg][_orbFocVix].burn_ix;
        if (pix == -1) {
          pix = 0;
        }
        Q_foc.x = vdata[wkg][_orbFocVix].vs4i[pix].ves.Q.data[ax];
        Q_foc.y = vdata[wkg][_orbFocVix].vs4i[pix].ves.Q.data[ay];
        break;
      }
      
      // LP orbit delta from focus point (in km)
      l_orb[wkg].orb_km[1][s].x = (s4i[wkg][z].LP.Q.data[ax] - Q_foc.x) / 1000.0;
      l_orb[wkg].orb_km[1][s].y = (s4i[wkg][z].LP.Q.data[ay] - Q_foc.y) / 1000.0;
      if (def_Q) {
        if (l_orb[wkg].orb_km[1][s].x < min_Q.x) min_Q.x = l_orb[wkg].orb_km[1][s].x;
        if (l_orb[wkg].orb_km[1][s].y < min_Q.y) min_Q.y = l_orb[wkg].orb_km[1][s].y;
        if (l_orb[wkg].orb_km[1][s].x > max_Q.x) max_Q.x = l_orb[wkg].orb_km[1][s].x;
        if (l_orb[wkg].orb_km[1][s].y > max_Q.y) max_Q.y = l_orb[wkg].orb_km[1][s].y;
      } else {
        min_Q = max_Q = l_orb[wkg].orb_km[1][s];
        def_Q = true;
      }

      // Vessel orbit delta from focus point (in km)
      for (unsigned int v = 0; v < vdata[wkg].size(); v++) {
        vdata[wkg][v].orb_km[s].x = (vdata[wkg][v].vs4i[z].ves.Q.data[ax] - Q_foc.x) / 1000.0;
        vdata[wkg][v].orb_km[s].y = (vdata[wkg][v].vs4i[z].ves.Q.data[ay] - Q_foc.y) / 1000.0;
        if (v == _orbFocVix) {
          if (vdata[wkg][v].orb_km[s].x < min_Q.x) min_Q.x = vdata[wkg][v].orb_km[s].x;
          if (vdata[wkg][v].orb_km[s].y < min_Q.y) min_Q.y = vdata[wkg][v].orb_km[s].y;
          if (vdata[wkg][v].orb_km[s].x > max_Q.x) max_Q.x = vdata[wkg][v].orb_km[s].x;
          if (vdata[wkg][v].orb_km[s].y > max_Q.y) max_Q.y = vdata[wkg][v].orb_km[s].y;
        }
      }

      // Entity orbit delta from focus point (in km)
      for (unsigned int i = 0; i < ORB_MAX_LINES; i++) {
        if (LP.plotix[i] == -1) break;
        if (LP.plotix[i] == -2) continue;
        l_orb[wkg].orb_km[i][s].x = (s4i[wkg][z].body[LP.plotix[i]].Q.data[ax] - Q_foc.x) / 1000.0;
        l_orb[wkg].orb_km[i][s].y = (s4i[wkg][z].body[LP.plotix[i]].Q.data[ay] - Q_foc.y) / 1000.0;
        if (l_orb[wkg].orb_km[i][s].x < min_Q.x) min_Q.x = l_orb[wkg].orb_km[i][s].x;
        if (l_orb[wkg].orb_km[i][s].y < min_Q.y) min_Q.y = l_orb[wkg].orb_km[i][s].y;
        if (l_orb[wkg].orb_km[i][s].x > max_Q.x) max_Q.x = l_orb[wkg].orb_km[i][s].x;
        if (l_orb[wkg].orb_km[i][s].y > max_Q.y) max_Q.y = l_orb[wkg].orb_km[i][s].y;
      }

      if (_dmp_orb) {
//      fprintf(dump_orb, "OrbIx,S4IIx,FocX,FocY,VX,VY,LPX,LPY,MajX,MajY,MinX,MinY,OthX,OthY,LoX,HiX,LoY,HiY\n");
        fprintf(dump_orb, "%ui,%ui,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f\n",
          s, z, 
          Q_foc.x / 1000.0, Q_foc.y / 1000.0,
          vdata[wkg][_orbFocVix].orb_km[s].x / 1000.0, vdata[wkg][_orbFocVix].orb_km[s].y / 1000.0,
          l_orb[wkg].orb_km[1][s].x / 1000.0, l_orb[wkg].orb_km[1][s].y / 1000.0,
          l_orb[wkg].orb_km[0][s].x / 1000.0, l_orb[wkg].orb_km[0][s].y / 1000.0,
          l_orb[wkg].orb_km[2][s].x / 1000.0, l_orb[wkg].orb_km[2][s].y / 1000.0,
          l_orb[wkg].orb_km[3][s].x / 1000.0, l_orb[wkg].orb_km[3][s].y / 1000.0,
          min_Q.x / 1000.0, max_Q.x / 1000.0, min_Q.y / 1000.0, max_Q.y / 1000.0
        );
      }

      z += zd;
    }

    scale = max_Q.x - min_Q.x;
    if ((max_Q.y - min_Q.y) > scale) {
      scale = max_Q.y - min_Q.y;
    }
    double halfway_x = min_Q.x + (max_Q.x - min_Q.x) / 2.0;
    double halfway_y = min_Q.y + (max_Q.y - min_Q.y) / 2.0;
    double prevScale = scale * 1.2 * pow(1.1, (double)_orbPrevZoom);
    scale *= 1.2 * pow(1.1, (double)_orbZoom);
    orbScale[_orbProj] = scale;

    if (_orbPrevZoom != _orbZoom) {
      hPan = - (scale / prevScale) * (-halfway_x + 0.5 * prevScale - hPan) - halfway_x + 0.5*scale;
      vPan = (scale / prevScale) * (halfway_y + 0.5 * prevScale + vPan) - halfway_y - 0.5 * scale;
      orbPanHoriz[_orbProj] = hPan;
      orbPanVert[_orbProj] = -vPan;
      orbPrevZoom = _orbZoom;
    }

    if (orbFocLock) {
      hPan = -scale*orbFocLockX - halfway_x + 0.5 * scale;
      vPan = scale*orbFocLockY - halfway_y - 0.5*scale;
      orbPanHoriz[_orbProj] = hPan;
      orbPanVert[_orbProj] = -vPan;
    }

    double xo = -(halfway_x - 0.5 * scale + hPan) / scale;
    double yo = (halfway_y + 0.5 * scale + vPan) / scale;

    orbFocLockX = xo;
    orbFocLockY = yo;

    min_Q.x = halfway_x - 0.5 * scale + hPan;
    max_Q.x = halfway_x + 0.5 * scale + hPan;
    min_Q.y = halfway_y - 0.5 * scale + vPan;
    max_Q.y = halfway_y + 0.5 * scale + vPan;



    l_orb[wkg].origPlot.x = (halfway_x - min_Q.x) / scale;
    l_orb[wkg].origPlot.y = (max_Q.y-halfway_y) / scale;

    


   // sprintf(oapiDebugString(), "Scale:%.3f, hPan:%.3f, vPan:%.3f, x_orig:%.3f, y_orig: %.3f", scale, hPan, vPan, -min_Q.x / scale, max_Q.y / scale);


    // Convert km distances into a 0.0-1.0 scale, ready for plotting on the MFD. Note the MFD origin is top left,
    // with the y-axis going positively DOWN the window, so the y calculation is reversed (i.e. (MAX - km)/scale)
    z = 0;
    for (unsigned int s = 0; s < ORB_PLOT_COUNT; s++) {

      // Vessels orbit plots
      for (unsigned int v = 0; v < vdata[wkg].size(); v++) {
        vdata[wkg][v].orb_plot[s].x = (vdata[wkg][v].orb_km[s].x - min_Q.x) / scale;
        vdata[wkg][v].orb_plot[s].y = (max_Q.y - vdata[wkg][v].orb_km[s].y) / scale;
      }

      // LP & Entities orbit plots
      for (unsigned int i = 0; i < ORB_MAX_LINES; i++) {
        if (LP.plotix[i] == -1) break;
        l_orb[wkg].orb_plot[i][s].x = (l_orb[wkg].orb_km[i][s].x - min_Q.x) / scale;
        l_orb[wkg].orb_plot[i][s].y = (max_Q.y - l_orb[wkg].orb_km[i][s].y) / scale;
      }
      z += zd;
    }

    for (unsigned int v = 0; v < vdata[wkg].size(); v++) {
      vdata[wkg][v].orb_plot_origin.x = (-min_Q.x) / scale;
      vdata[wkg][v].orb_plot_origin.y = (max_Q.y) / scale;
    }

    l_orb[wkg].scale = scale;

    // Find the focus point at the current vessel encounter ix
    int enc_ix = vdata[wkg][_orbFocVix].enc_ix;
    if (enc_ix == -1) {
      if (vdata[wkg][_orbFocVix].vs4i[0].dQ < vdata[wkg][_orbFocVix].vs4i[s4int_count[wkg] - 1].dQ) {
        enc_ix = 0;
      } else {
        enc_ix = s4int_count[wkg] - 1;
      }
    };
    VECTOR2 Q_foc;
    int pix;
    switch (_orbFocus) {
    case 0:
      Q_foc.x = s4i[wkg][enc_ix].body[LP.maj].Q.data[ax];
      Q_foc.y = s4i[wkg][enc_ix].body[LP.maj].Q.data[ay];
      break;
    case 1:
      Q_foc.x = s4i[wkg][enc_ix].body[LP.min].Q.data[ax];
      Q_foc.y = s4i[wkg][enc_ix].body[LP.min].Q.data[ay];
      break;
    case 2:
      Q_foc.x = vdata[wkg][_orbFocVix].vs4i[enc_ix].ves.Q.data[ax];
      Q_foc.y = vdata[wkg][_orbFocVix].vs4i[enc_ix].ves.Q.data[ay];
      break;
    case 3:
      Q_foc.x = vdata[wkg][_orbFocVix].vs4i[enc_ix].ves.Q.data[ax];
      Q_foc.y = vdata[wkg][_orbFocVix].vs4i[enc_ix].ves.Q.data[ay];
      break;
    case 4:
      pix = vdata[wkg][_orbFocVix].burn_ix;
      if (pix == -1) {
        pix = 0;
      }
      Q_foc.x = vdata[wkg][_orbFocVix].vs4i[pix].ves.Q.data[ax];
      Q_foc.y = vdata[wkg][_orbFocVix].vs4i[pix].ves.Q.data[ay];
      break;
    }

    // Finish up the encounter X Y plots
    for (unsigned int v = 0; v < vdata[wkg].size(); v++) {
      int venc_ix = vdata[wkg][v].enc_ix;
      int venc_count = vdata[wkg][v].enc_count;
      if (venc_count == 0) {
        if (vdata[wkg][v].vs4i[0].dQ < vdata[wkg][v].vs4i[s4int_count[wkg] - 1].dQ) {
          venc_ix = 0;
          vdata[wkg][v].enc_typ = -1;
        } else {
          venc_ix = s4int_count[wkg] - 1;
          vdata[wkg][v].enc_typ = 1;
        }
        vdata[wkg][v].enc_ix = venc_ix;
        vdata[wkg][v].enc_Q = vdata[wkg][v].vs4i[venc_ix].dQ;
        vdata[wkg][v].enc_P = vdata[wkg][v].vs4i[venc_ix].dP;
      } else {
        vdata[wkg][v].enc_typ = 0;
      };




      vdata[wkg][v].orb_plot_ves_enc.x = ((vdata[wkg][v].vs4i[enc_ix].ves.Q.data[ax] - Q_foc.x) / 1000.0 - min_Q.x) / scale;
      vdata[wkg][v].orb_plot_ves_enc.y = (max_Q.y - ((vdata[wkg][v].vs4i[enc_ix].ves.Q.data[ay] - Q_foc.y) / 1000.0)) / scale;

      vdata[wkg][v].orb_plot_body_enc[1].x = ((s4i[wkg][enc_ix].LP.Q.data[ax] - Q_foc.x) / 1000.0 - min_Q.x) / scale;
      vdata[wkg][v].orb_plot_body_enc[1].y = (max_Q.y - ((s4i[wkg][enc_ix].LP.Q.data[ay] - Q_foc.y) / 1000.0)) / scale;

      for (unsigned int i = 0; i < ORB_MAX_LINES; i++) {
        if (LP.plotix[i] == -1) break;
        if (LP.plotix[i] == -2) continue;
        vdata[wkg][v].orb_plot_body_enc[i].x = ((s4i[wkg][enc_ix].body[LP.plotix[i]].Q.data[ax] - Q_foc.x) / 1000.0 - min_Q.x) / scale;
        vdata[wkg][v].orb_plot_body_enc[i].y = (max_Q.y - ((s4i[wkg][enc_ix].body[LP.plotix[i]].Q.data[ay] - Q_foc.y) / 1000.0)) / scale;
      }
    }
  }
  if (_dmp_orb) {
    fclose(dump_orb);
    dmp_orb = false;
  }

  ms_elap = getMilliSpan(s_time);
  //sprintf(oapiDebugString(), "S4I Run for time: %8.3f MJD Range: %8.3f to %8.3f, runtime: %ims", s4i[wkg][0].sec, s4i[wkg][0].MJD, s4i[wkg][s4int_count[wkg]-1].MJD, ms_elap);

  dbg[wkg][0] = s4i[wkg][0].sec;
  dbg[wkg][1] = s4i[wkg][0].MJD;
  dbg[wkg][2] = s4i[wkg][s4int_count[wkg] - 1].MJD;
  dbg[wkg][3] = s4int_refresh;
  dbg[wkg][4] = s4int_count[wkg];
  dbg[wkg][5] = s4int_timestep[wkg];
  dbg[wkg][6] = ((double) ms_elap)/1000.0;
  dbg[wkg][7] = ORB_PLOT_COUNT;


  {
    if (_dmp_log) {

      FILE *dump_s4i;
      if (fopen_s(&dump_s4i, ".\\Config\\MFD\\Lagrange\\Diags\\S4I.csv", "w") == 0) {

        fprintf(dump_s4i, "INDEX, MJD, T, dT, "
                          "E - QX, E - QY, E - QZ, "
                          "M - QX, M - QY, M - QZ, "
                          "S - QX, S - QY, S - QZ, "
                          "V - QX, V - QY, V - QZ, "
                          "E - PX, E - PY, E - PZ, "
                          "M - PX, M - PY, M - PZ, "
                          "S - PX, S - PY, S - PZ, "
                          "V - PX, V - PY, V - PZ, "
                          "I - PX, I - PY, I - PZ, "
                          "LPg - QX, LPg - QY, LPg - QZ, "
                          "LPg - PX, LPg - PY, LPg - PZ, "
                          //"Vr - QX, Vr - QY, "
                          //"Mr - QX, Mr - QY, "
                          //"LPr - QX, LPr - QY, "
                          "Vrlp - QX, Vrlp - QY, Vrlp - QZ, "
                          "Vrlp - PX, Vrlp - PY, Vrpl - PZ, "
                          "Vrlp - dQ, Vrlp - dP"
                "\n");

        for (unsigned int i = 0; i < s4int_count[wkg]; i++) {
          fprintf(dump_s4i, "%u, %.15lf, %.6lf, %.6lf, ", i, s4i[wkg][i].MJD, s4i[wkg][i].sec, (i>0)? s4i[wkg][i].sec- s4i[wkg][i-1].sec: s4i[wkg][i].sec);
          fprintf(dump_s4i, "%.15lf, %.15lf, %.15lf,   ", s4i[wkg][i].body[LU_EARTH].Q.x, s4i[wkg][i].body[LU_EARTH].Q.z, s4i[wkg][i].body[LU_EARTH].Q.y);
          fprintf(dump_s4i, "%.15lf, %.15lf, %.15lf,   ", s4i[wkg][i].body[LU_MOON].Q.x, s4i[wkg][i].body[LU_MOON].Q.z, s4i[wkg][i].body[LU_MOON].Q.y);
          fprintf(dump_s4i, "%.15lf, %.15lf, %.15lf,   ", s4i[wkg][i].body[LU_SUN].Q.x, s4i[wkg][i].body[LU_SUN].Q.z, s4i[wkg][i].body[LU_SUN].Q.y);
          fprintf(dump_s4i, "%.15lf, %.15lf, %.15lf,   ", vdata[wkg][0].vs4i[i].ves.Q.x, vdata[wkg][0].vs4i[i].ves.Q.z, vdata[wkg][0].vs4i[i].ves.Q.y); // ves Q
          fprintf(dump_s4i, "%.15lf, %.15lf, %.15lf,   ", s4i[wkg][i].body[LU_EARTH].P.x, s4i[wkg][i].body[LU_EARTH].P.z, s4i[wkg][i].body[LU_EARTH].P.y);
          fprintf(dump_s4i, "%.15lf, %.15lf, %.15lf,   ", s4i[wkg][i].body[LU_MOON].P.x, s4i[wkg][i].body[LU_MOON].P.z, s4i[wkg][i].body[LU_MOON].P.y);
          fprintf(dump_s4i, "%.15lf, %.15lf, %.15lf,   ", s4i[wkg][i].body[LU_SUN].P.x, s4i[wkg][i].body[LU_SUN].P.z, s4i[wkg][i].body[LU_SUN].P.y);
          fprintf(dump_s4i, "%.15lf, %.15lf, %.15lf,   ", vdata[wkg][0].vs4i[i].ves.P.x, vdata[wkg][0].vs4i[i].ves.P.z, vdata[wkg][0].vs4i[i].ves.P.y); // ves P
          if (i != vdata[wkg][0].burn_ix) {
            fprintf(dump_s4i, "%.15lf, %.15lf, %.15lf,   ", 0.0, 0.0, 0.0);
          } else {
            fprintf(dump_s4i, "%.15lf, %.15lf, %.15lf,   ", vdata[wkg][0].burndVg.x, vdata[wkg][0].burndVg.y, vdata[wkg][0].burndVg.z); // impulse
          }
          fprintf(dump_s4i, "%.15lf, %.15lf, %.15lf,   ", s4i[wkg][i].LP.Q.x, s4i[wkg][i].LP.Q.z, s4i[wkg][i].LP.Q.y);
          fprintf(dump_s4i, "%.15lf, %.15lf, %.15lf,   ", s4i[wkg][i].LP.P.x, s4i[wkg][i].LP.P.z, s4i[wkg][i].LP.P.y);
//          fprintf(dump_s4i, "%.15lf, %.15lf,   ",         vdata[wkg][0].orb_km[s].x, vdata[wkg][0].orb_km[s].y);
//          fprintf(dump_s4i, "%.15lf, %.15lf,   ",         l_orb[wkg].orb_km[2][s].x, l_orb[wkg].orb_km[2][s].y);
//          fprintf(dump_s4i, "%.15lf, %.15lf,   ",         l_orb[wkg].orb_km[1][s].x, l_orb[wkg].orb_km[1][s].y);
          fprintf(dump_s4i, "%.15lf, %.15lf, %.15lf,   ", vdata[wkg][0].vs4i[i].vesLP.Q.x, vdata[wkg][0].vs4i[i].vesLP.Q.z, vdata[wkg][0].vs4i[i].vesLP.Q.y); // ves Q
          fprintf(dump_s4i, "%.15lf, %.15lf, %.15lf,   ", vdata[wkg][0].vs4i[i].vesLP.P.x, vdata[wkg][0].vs4i[i].vesLP.P.z, vdata[wkg][0].vs4i[i].vesLP.P.y); // ves P
          fprintf(dump_s4i, "%.15lf, %.15lf\n", vdata[wkg][0].vs4i[i].dQ, vdata[wkg][0].vs4i[i].dP);
        }
        fprintf(dump_s4i, "\n\nMaxX, MinX, MaxY, MinY, Scale\n");
        fprintf(dump_s4i, "%.1f, %.1f, %.1f, %.1f, %.1f\n", max_Q.x, min_Q.x, max_Q.y, min_Q.y, scale);
      }
      fclose(dump_s4i);
    }
  } 
  if (_dmp_log) {
    dmp_log = false;
  }
  if (_dmp_enc) {
    fclose(dump_enc);
    dmp_enc = false;
  }
  return;
}


inline VECTOR3 LagrangeUniverse::s4iforce(const int e, const int i) {
  // Gravity force summation function
  // ... determines the relative distance from entity (e) to each gravity body (g) at iteration step (i)
  // ... then sums up effective forces
  VECTOR3 Fadd;
  VECTOR3 F = _V(0.0,0.0,0.0);
  VECTOR3 Qe = s4i[wkg][i].body[e].Q;
  VECTOR3 Qg;
  VECTOR3 Qeg;
  double temp;
  double gm;

  for (int ix = 0; LP.bodyIx[ix] != -1; ix++) {
    int g = LP.bodyIx[ix];
    if (g==e) continue;
    Qg = s4i[wkg][i].body[g].Q;
    Qeg = Qe - Qg;
    temp = pow((Qeg.x*Qeg.x)+(Qeg.y*Qeg.y) + (Qeg.z*Qeg.z), 1.5);
    gm = body[g].gm;
    Fadd = Qeg * (-gm / temp);
    F += Fadd;
  }
  return F;
}

inline VECTOR3 LagrangeUniverse::s4iforce_ves(const int s, const int i) {
  // Gravity force summation function for vessels
  // ... determines the relative distance from vessel (s) to each gravity body (g) at iteration step (i)
  // ... then sums up effective forces

  VECTOR3 F = _V(0.0,0.0,0.0);
  VECTOR3 Qv = vdata[wkg][s].vs4i[i].ves.Q;
  VECTOR3 Qg;
  VECTOR3 Qvg;
  double temp;
  double gm;
  double lenQvgKM;

  for (int ix = 0; LP.bodyIx[ix] != -1; ix++) {
    int g = LP.bodyIx[ix];
    Qg = s4i[wkg][i].body[g].Q;
    Qvg = Qv - Qg;
    lenQvgKM = length(Qvg) / 1000.0;
    temp = pow((Qvg.x*Qvg.x) + (Qvg.y*Qvg.y) + (Qvg.z*Qvg.z), 1.5);
    gm = body[g].gm;
    F -= Qvg * (gm / temp);
    if (vdata[wkg][s].alarm_state < 2 && lenQvgKM < body[g].impactWarnDist) {
      vdata[wkg][s].alarm_state = 2;
      vdata[wkg][s].alarm_body = g;
      vdata[wkg][s].alarm_ix = i;
    }
    if (vdata[wkg][s].alarm_state < 1 && lenQvgKM < body[g].proxWarnDist) {
      vdata[wkg][s].alarm_state = 1;
      vdata[wkg][s].alarm_body = g;
      vdata[wkg][s].alarm_ix = i;
    }
  }
  return F;
}


inline VECTOR3 unit_s (const VECTOR3 &a)
{
  if (abs(a.x)>1e-9 || abs(a.y)>1e-9 || abs(a.z)>1e-9) return a / length(a);
  return a;
}
inline VECTOR3 crossp_s (const VECTOR3 &a, const VECTOR3 &b)
{
  if (abs(a.y*b.z - b.y*a.z)>1e-9 || abs(a.z*b.x - b.z*a.x)>1e-9 || abs(a.x*b.y - b.x*a.y)>1e-9) return _V(a.y*b.z - b.y*a.z, a.z*b.x - b.z*a.x, a.x*b.y - b.x*a.y);
  VECTOR3 c = b + _V(0.6, 0.8, 0.0); // Collinear vectors: add a 1.0m length bias in the X-Y plane to force a non-zero cross product
  return _V(a.y*c.z - c.y*a.z, a.z*c.x - c.z*a.x, a.x*c.y - c.x*a.y);
}