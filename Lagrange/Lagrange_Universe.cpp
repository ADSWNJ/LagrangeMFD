// =================================================================================
//
//	LagrangeUniverse (All calculations for the LP and related body state progation)
//	===============================================================================
//
//	Copyright (C) 2016	Andrew (ADSWNJ) Stokes and Keith (Keithth G) Gelling
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
#include "orbitersdk.h"
#include "Lagrange_Universe.hpp"


LagrangeUniverse::LagrangeUniverse() {

  s4i_canstart = false;
  for (int i = 0; i < 2; i++) {
    s4int_count[i] = 20000;
    s4int_timestep[i] = 30.0;
  }

  defBody(&body[0], 0, "Sun");
  defBody(&body[1], 1, "Earth");
  defBody(&body[2], 2, "Moon");
  defBary(&body[3], 3, "E-M Bary", LU_EARTH, LU_MOON);

  // LP Definitions: see http://www.orbiter-forum.com/showthread.php?t=36110 for commentary on these values
  // Meanings:
  //              Index    Name       LP#         Alpha Val      Plot Center  Major     Minor              Other   
  defLP(&lptab[0], 0, "Earth Moon L1", 1, 0.836915194872059889706, LU_EARTH, LU_EARTH, LU_MOON,           LU_SUN);  // EML1
  defLP(&lptab[1], 1, "Earth Moon L2", 2, 1.15568211143362165272,  LU_EARTH, LU_EARTH, LU_MOON,           LU_SUN);  // EML2
  defLP(&lptab[2], 2, "Earth Moon L3", 3, -1.00506263995930385239, LU_EARTH, LU_EARTH, LU_MOON,           LU_SUN);  // EML3
  defLP(&lptab[3], 3, "Sun Earth L1",  1, 0.989985982345709235260, LU_SUN,   LU_SUN,   LU_EARTHMOONBARY);           // SEL1
  defLP(&lptab[4], 4, "Sun Earth L2",  2, 1.01007520001973933176,  LU_SUN,   LU_SUN,   LU_EARTHMOONBARY);           // SEL2
  defLP(&lptab[5], 5, "Sun Earth L3",  3, -1.00000126684308386748, LU_SUN,   LU_SUN,   LU_EARTHMOONBARY);           // SEL3

  // LP Orbit plot definitions
  // Meanings:
  //                      LP Pen                 Plot Center       Ent 1     Ent 1 Pen       Ent 2       Ent 2 Pen            
  defOrbPlot(&lptab[0], ORB_PEN_DASHED_VIOLET, LU_EARTH,         LU_MOON, ORB_PEN_BRIGHT_YELLOW); 
  defOrbPlot(&lptab[1], ORB_PEN_DASHED_VIOLET, LU_EARTH,         LU_MOON, ORB_PEN_BRIGHT_YELLOW);
  defOrbPlot(&lptab[2], ORB_PEN_DASHED_VIOLET, LU_EARTH,         LU_MOON, ORB_PEN_BRIGHT_YELLOW);
  defOrbPlot(&lptab[3], ORB_PEN_DASHED_VIOLET, LU_SUN,           LU_EARTH, ORB_PEN_WHITE,   LU_MOON, ORB_PEN_BRIGHT_YELLOW);
  defOrbPlot(&lptab[4], ORB_PEN_DASHED_VIOLET, LU_SUN,           LU_EARTH, ORB_PEN_WHITE,   LU_MOON, ORB_PEN_BRIGHT_YELLOW);
  defOrbPlot(&lptab[5], ORB_PEN_DASHED_VIOLET, LU_SUN,           LU_EARTH, ORB_PEN_WHITE,   LU_MOON, ORB_PEN_BRIGHT_YELLOW);

  for (int i = 0; i < 2; i++) {
    s4i[i].resize(s4int_count[0]);
  }

  wkg = 0;
  act = 1;
  s4i_valid = false;
  LP = lptab[0];
  selectLP(0);

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

void LagrangeUniverse::defBody(LagrangeUniverse_Body *pbodyinst, int p0, char* p1) {
  // Initialize each celestial body
  pbodyinst->ix = p0;
  strcpy(pbodyinst->name, p1);
  pbodyinst->hObj = oapiGetGbodyByName(pbodyinst->name);
  pbodyinst->mass = oapiGetMass(pbodyinst->hObj);
  pbodyinst->mu = pbodyinst->mass * GGRAV;
  pbodyinst->isBary = false;
  pbodyinst->b_e[0] = -1;
  return;
}

void LagrangeUniverse::defBary(LagrangeUniverse_Body *pbodyinst, int p0, char* p1, int maj, int min) {
  // Initialize each barycenter
  pbodyinst->ix = p0;
  strcpy(pbodyinst->name, p1);
  pbodyinst->mass = body[maj].mass + body[min].mass;
  pbodyinst->mu = pbodyinst->mass * GGRAV;
  pbodyinst->isBary = true;
  pbodyinst->b_e[0] = maj;
  pbodyinst->b_e[1] = min;
  pbodyinst->b_e[2] = -1;
  return;
}

void LagrangeUniverse::defLP(LagrangeUniverse_LP_Def *lpdef, int p0, char *p1, int Lnum, double a, int ref, int maj, int min, int oth1,int oth2) {
  // e.g. &lptab[0], 0, "Earth Moon L1", 1, 0.836915194872059889706, LU_EARTHMOONBARY, LU_EARTH, LU_MOON, LU_SUN
  lpdef->nxix = p0;
  lpdef->ix = p0;
  strcpy(lpdef->name, p1);
  lpdef->ref = ref;
  lpdef->maj = maj;
  lpdef->min = min;
  lpdef->Lnum = Lnum;
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

void LagrangeUniverse::defOrbPlot(LagrangeUniverse_LP_Def *lptab, int lppen, int b1, int b2, int b2pen, int b3, int b3pen) {
  lptab->plotix[0] = b1;        // Body 1
  lptab->plotixpen[0] = 0;
  lptab->plotix[1] = -2;        // LP
  lptab->plotixpen[1] = lppen;
  lptab->plotix[2] = b2;        // Body 2
  lptab->plotixpen[2] = b2pen;
  lptab->plotix[3] = b3;        // Body 3
  lptab->plotixpen[3] = b3pen;
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
    this->plotixpen[i] = x.plotixpen[i];
  }

  return *this;
}

inline Lagrange_vdata& Lagrange_vdata::operator=(const Lagrange_vdata& x) {
  this->v = x.v;
  if (this->vs4i.size() != x.vs4i.size()) this->vs4i.resize(x.vs4i.size());
  this->vs4i[0] = x.vs4i[0];
  this->cmass = x.cmass;
  this->dmass = x.dmass;
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
  //findLP( QP_struct *vesLP, QP_struct *gblLP, QP_struct *e1, QP_struct *e2, double *dQ, double *dP) {
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
  xHat = safe_unit(e);
  zHat = unit(safe_crossp(r, v));
  yHat = unit(safe_crossp(zHat, xHat));

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
/// TODO TODO code for LP4 and LP5
  return;
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


  // Find two vectors in the plane of our minor body ... take the radius vector and the velocity vector
  mref = s4i[w][i].body[vdata[w][s].refEnt];
  Rt = ves->Q - mref.Q;
  Vt = ves->P - mref.P;


  //transform relative pos/vel into TransX-style prograde, plane change, inner coords
  XtHat = safe_unit(Vt);
  YtHat = unit(safe_crossp(Vt, Rt));
  ZtHat = unit(safe_crossp(XtHat, YtHat));

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
  if (vs4i->dQ < vdata[w][s].enc_Q) {
    vdata[w][s].enc_Q = vs4i->dQ;
    vdata[w][s].enc_P = vs4i->dP;
    vdata[w][s].enc_ix = i;
  }
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

    for (unsigned int e = 0; e < vdata[act].size(); e++) {
      vdata[act][e].burnArmed = vdata[wkg][e].burnArmed;
      vdata[act][e].burnMJD = vdata[wkg][e].burnMJD;
      vdata[act][e].burndV = vdata[wkg][e].burndV;
      vdata[act][e].refEnt = vdata[wkg][e].refEnt;
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
  s4i_trafficlight[wkg].unlock(); // Release worker to fill the new wkg buffer
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
    // Best encounter so far is now, and lp_ves will update if we get closer
    try {
      vdata[wkg][s].vs4i.resize(s4int_count[wkg]);
    }
    catch (...) {
      s4int_count[wkg] = 20000;
      vdata[wkg][s].vs4i.resize(s4int_count[wkg]);
    }
    Lagrange_ves_s4i *vs4i0 = &vdata[wkg][s].vs4i[0];
    vdata[wkg][s].enc_Q = vs4i0->dQ;
    vdata[wkg][s].enc_P = vs4i0->dP;
    vdata[wkg][s].enc_ix = 0;
    vdata[wkg][s].block_scan = 1;
  }

  //s4i[wkg][0].sec = oapiGetSimTime();
  //s4i[wkg][0].MJD = oapiGetSimMJD();

  {
    if (!s4i_valid && vdata[wkg].size()>0) {

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
        fprintf(dump_s4i, "%.15lf, %.15lf, %.15lf\n", vdata[wkg][0].vs4i[0].ves.Q.x, vdata[wkg][0].vs4i[0].ves.Q.z, vdata[wkg][0].vs4i[0].ves.Q.y);
        fprintf(dump_s4i, "%.15lf, %.15lf, %.15lf\n", vdata[wkg][0].vs4i[0].ves.P.x, vdata[wkg][0].vs4i[0].ves.P.z, vdata[wkg][0].vs4i[0].ves.P.y);
        fprintf(dump_s4i, "\n\n");

        fclose(dump_s4i);
      }
    }
  }

  int interpolation_chop = 1;
  unsigned int last_regix = 0;
  unsigned int last_encix = 0;
  
  for (unsigned int cur=1; cur<s4int_count[wkg]; cur++) {
    // S4 Integrator main loop
    int i, j, k;
    double nextdt = dt / (double) interpolation_chop;
    double nextMJD = s4i[wkg][prev].MJD + nextdt / (24.0*60.0*60.0);

    for (unsigned int s = 0; s < vdata[wkg].size(); s++) {
      if (vdata[wkg][s].burnArmed && vdata[wkg][s].burnMJD > s4i[wkg][prev].MJD && vdata[wkg][s].burnMJD <= nextMJD) {
        nextMJD = vdata[wkg][s].burnMJD;
        nextdt = (nextMJD - s4i[wkg][prev].MJD) * (24.0 * 60.0 * 60.0);
      }
    }

    s4i[wkg][cur].sec = s4i[wkg][prev].sec + nextdt;
    s4i[wkg][cur].MJD = nextMJD;
//double __dbgM = s4i[wkg][cur].MJD;
//VECTOR3 __dbgF;
//VECTOR3 __dbgP;
//VECTOR3 __dbgQ;

    // Step 1a, rolling forward from prev
    for (i = 0; LP.bodyIx[i] != -1; i++) {
      ent = LP.bodyIx[i];
//__dbgP = s4i[wkg][prev].body[ent].P;
      s4i[wkg][cur].body[ent].P = s4i[wkg][prev].body[ent].P;
//__dbgQ = s4i[wkg][prev].body[ent].Q + s4i[wkg][cur].body[ent].P  * (0.5 * w0 * nextdt);
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
//__dbgF = s4iforce(ent, cur);
      s4i[wkg][cur].body[ent].P += s4iforce(ent, cur) * (w0 * nextdt);
//__dbgP = s4i[wkg][cur].body[ent].P;
    }

    //Step 1b Vessel Calc
    for (unsigned int s = 0; s < vdata[wkg].size(); s++) {
      vs4i = &vdata[wkg][s].vs4i[cur];
//__dbgF = s4iforce_ves(s, cur);
      vs4i->ves.P += s4iforce_ves(s, cur) * (w0 * nextdt);
    }

    //Step 1c, 2a
    for (i = 0; LP.bodyIx[i] != -1; i++) {
      ent = LP.bodyIx[i];
//__dbgQ = s4i[wkg][cur].body[ent].P * (0.5 * (w0 + w1) * nextdt);
      s4i[wkg][cur].body[ent].Q += s4i[wkg][cur].body[ent].P * (0.5 * (w0 + w1) * nextdt);
//__dbgQ = s4i[wkg][cur].body[ent].Q;
    }

    //Step 1c, 2a Vessel Calc
    for (unsigned int s = 0; s < vdata[wkg].size(); s++) {
      vs4i = &vdata[wkg][s].vs4i[cur];
      vs4i->ves.Q += vs4i->ves.P * (0.5 * (w0 + w1) * nextdt);
    }

    //Step 2b
    for (i = 0; LP.bodyIx[i] != -1; i++) {
      ent = LP.bodyIx[i];
//__dbgF = s4iforce(ent, cur);
      s4i[wkg][cur].body[ent].P += s4iforce(ent, cur) * (w1 * nextdt);
//__dbgP = s4i[wkg][cur].body[ent].P;
    }

    //Step 2b Vessel Calc
    for (unsigned int s = 0; s < vdata[wkg].size(); s++) {
      vs4i = &vdata[wkg][s].vs4i[cur];
//__dbgF = s4iforce_ves(s, cur);
      vs4i->ves.P += s4iforce_ves(s, cur) * (w1 * nextdt);
    }

    //Step 2c, 3a
    for (i = 0; LP.bodyIx[i] != -1; i++) {
      ent = LP.bodyIx[i];
//__dbgQ = s4i[wkg][cur].body[ent].P * (0.5 * (w1 + w0) * nextdt);
      s4i[wkg][cur].body[ent].Q += s4i[wkg][cur].body[ent].P * (0.5 * (w1 + w0) * nextdt);
//__dbgQ = s4i[wkg][cur].body[ent].Q;
    }

    //Step 2c, 3a Vessel Calc
    for (unsigned int s = 0; s < vdata[wkg].size(); s++) {
      vs4i = &vdata[wkg][s].vs4i[cur];
      vs4i->ves.Q += vs4i->ves.P * (0.5 * (w1 + w0) * nextdt);
    }

    //Step 3b
    for (i = 0; LP.bodyIx[i] != -1; i++) {
      ent = LP.bodyIx[i];
//__dbgF = s4iforce(ent, cur) * (w0 * nextdt);
      s4i[wkg][cur].body[ent].P += s4iforce(ent, cur) * (w0 * nextdt);
//__dbgP = s4i[wkg][cur].body[ent].P;
    }

    //Step 3b Vessel Calc
    for (unsigned int s = 0; s < vdata[wkg].size(); s++) {
      vs4i = &vdata[wkg][s].vs4i[cur];
//__dbgF = s4iforce_ves(s, cur);
      vs4i->ves.P += s4iforce_ves(s, cur) * (w0 * nextdt);
    }

    //Step 3c
    for (i = 0; LP.bodyIx[i] != -1; i++) {
      ent = LP.bodyIx[i];
      s4i[wkg][cur].body[ent].Q += s4i[wkg][cur].body[ent].P * (0.5 * (w0) * nextdt);
    }

    //Step 3c Vessel Calc
    for (unsigned int s = 0; s < vdata[wkg].size(); s++) {
      vs4i = &vdata[wkg][s].vs4i[cur];
      vs4i->ves.Q += vs4i->ves.P * (0.5 * (w0) * nextdt);
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
        vs4i->ves.P += impulse;
      }
    }

    //Barycenter update step
    for (i = 0; LP.baryIx[i] != -1; i++) {
      ent = LP.baryIx[i];
      s4i[wkg][cur].body[ent].Q = _V(0.0,0.0,0.0);
      s4i[wkg][cur].body[ent].P = _V(0.0,0.0,0.0);
      for (j=0; body[ent].b_e[j]!=-1; j++) {
        k = body[ent].b_e[j];
        s4i[wkg][cur].body[ent].Q += s4i[wkg][cur].body[k].Q * (body[k].mass / body[ent].mass);
        s4i[wkg][cur].body[ent].P += s4i[wkg][cur].body[k].P * (body[k].mass / body[ent].mass);
      }
    }
    
    // Find LP Q and P
    if (LP.Lnum<4) {
      lp123(cur,wkg);
    } else {
      lp45(cur,wkg);
    }

    // Find vessel relative LP data
    for (unsigned int s = 0; s < vdata[wkg].size(); s++) {
      lp_ves(s, cur, wkg);
    }

    bool rewind = false; 

    // Look for encounter points (where we need to binary chop interpolate into the encounters)
    for (unsigned int s = 0; s < vdata[wkg].size(); s++) {
      if (!vdata[wkg][s].block_scan) { // Look at last 2 data points to see if we are at an encounter 
        double EDM2 = length(vdata[wkg][s].vs4i[cur - 2].vesLP.Q);
        double EDM1 = length(vdata[wkg][s].vs4i[cur - 1].vesLP.Q);
        double EDM0 = length(vdata[wkg][s].vs4i[cur - 0].vesLP.Q);

        double SECM2 = s4i[wkg][cur - 2].sec;
        double SECM1 = s4i[wkg][cur - 1].sec;
        double SECM0 = s4i[wkg][cur - 0].sec;

        if (EDM2 >= EDM1 && EDM1 <= EDM0) {  // We have found an inversion of the curve
          if (interpolation_chop < 1024) {
            if (interpolation_chop == 1) {
              last_regix = cur - 2;
            }
            interpolation_chop *= 2;  // Double the resolution (up to max of 256)
            vdata[wkg][s].block_scan = 1;
            rewind = true;
            break;
          } else {
            interpolation_chop = 1; // Release the pressure ... we have enough accuracy now on the search
            last_encix = cur - 1;
            // Now we need to move the encounter to last_regix+1. However, if we have any burns inbetween then make sure we keep those lines
            last_regix++; // Now points to first insert point
            unsigned int k = last_regix;
            bool intervening_burn = false;
            while (k < last_encix) {
              for (unsigned int c = 0; c < vdata[wkg].size(); c++) {
                if (vdata[wkg][c].burnArmed && abs(s4i[wkg][k].MJD - vdata[wkg][c].burnMJD) < 1E-06) {
                  intervening_burn = true;
                  break;
                }
              }
              if (intervening_burn) {
                break;
              }
              k++;
            }
            // fix up map now, so either the intervening burn or the encounter comes after the last regular delta-time
            s4i[wkg][last_regix] = s4i[wkg][k];
            for (unsigned int c = 0; c < vdata[wkg].size(); c++) {
              vdata[wkg][c].vs4i[last_regix] = vdata[wkg][c].vs4i[k];
              vdata[wkg][c].block_scan = 2;
            }
            cur = last_regix;
            prev = cur -1;
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

    // This entry is committed. Decrement the blocks
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
  
  {
    bool def_Q = false;

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

    // scan the Q values to generate relatives to the major body (in km)
    for (unsigned int s = 0; s < ORB_PLOT_COUNT; s++) {
      VECTOR2 Q_maj;
      Q_maj.x = s4i[wkg][z].body[vdata[wkg][0].refEnt].Q.x;
      Q_maj.y = s4i[wkg][z].body[vdata[wkg][0].refEnt].Q.z;
      
      // LP orbit delta from major entity (in km)
      l_orb[wkg].orb_km[1][s].x = (s4i[wkg][z].LP.Q.x - Q_maj.x) / 1000.0;
      l_orb[wkg].orb_km[1][s].y = (s4i[wkg][z].LP.Q.z - Q_maj.y) / 1000.0;
      if (def_Q) {
        if (l_orb[wkg].orb_km[1][s].x < min_Q.x) min_Q.x = l_orb[wkg].orb_km[1][s].x;

        if (l_orb[wkg].orb_km[1][s].y < min_Q.y) min_Q.y = l_orb[wkg].orb_km[1][s].y;
        if (l_orb[wkg].orb_km[1][s].x > max_Q.x) max_Q.x = l_orb[wkg].orb_km[1][s].x;
        if (l_orb[wkg].orb_km[1][s].y > max_Q.y) max_Q.y = l_orb[wkg].orb_km[1][s].y;
      } else {
        min_Q = max_Q = l_orb[wkg].orb_km[1][s];
        def_Q = true;
      }



      // Vessels orbit delta from major entity (in km)
      for (unsigned int v = 0; v < vdata[wkg].size(); v++) {
        vdata[wkg][v].orb_km[s].x = (vdata[wkg][v].vs4i[z].ves.Q.x - Q_maj.x) / 1000.0;
        vdata[wkg][v].orb_km[s].y = (vdata[wkg][v].vs4i[z].ves.Q.z - Q_maj.y) / 1000.0;

        if (vdata[wkg][v].orb_km[s].x < min_Q.x) min_Q.x = vdata[wkg][v].orb_km[s].x;

        if (vdata[wkg][v].orb_km[s].y < min_Q.y) min_Q.y = vdata[wkg][v].orb_km[s].y;
        if (vdata[wkg][v].orb_km[s].x > max_Q.x) max_Q.x = vdata[wkg][v].orb_km[s].x;
        if (vdata[wkg][v].orb_km[s].y > max_Q.y) max_Q.y = vdata[wkg][v].orb_km[s].y;
      }

      // Entities orbit delta from major entity (in km)
      for (unsigned int i = 2; i < ORB_MAX_LINES; i++) {
        if (LP.plotix[i] == -1) break;
        l_orb[wkg].orb_km[i][s].x = (s4i[wkg][z].body[LP.plotix[i]].Q.x - Q_maj.x) / 1000.0;
        l_orb[wkg].orb_km[i][s].y = (s4i[wkg][z].body[LP.plotix[i]].Q.z - Q_maj.y) / 1000.0;
        if (l_orb[wkg].orb_km[i][s].x < min_Q.x) min_Q.x = l_orb[wkg].orb_km[i][s].x;

        if (l_orb[wkg].orb_km[i][s].y < min_Q.y) min_Q.y = l_orb[wkg].orb_km[i][s].y;
        if (l_orb[wkg].orb_km[i][s].x > max_Q.x) max_Q.x = l_orb[wkg].orb_km[i][s].x;
        if (l_orb[wkg].orb_km[i][s].y > max_Q.y) max_Q.y = l_orb[wkg].orb_km[i][s].y;
      }
      z += zd;
    }

    scale = max_Q.x - min_Q.x;
    if ((max_Q.y - min_Q.y) > scale) {
      scale = max_Q.y - min_Q.y;
    }
    scale *= 1.2;
    double halfway = min_Q.x + (max_Q.x - min_Q.x) / 2.0;
    min_Q.x = halfway - 0.5 * scale;
    max_Q.x = halfway + 0.5 * scale;
    halfway = min_Q.y + (max_Q.y - min_Q.y) / 2.0;
    min_Q.y = halfway - 0.5 * scale;
    max_Q.y = halfway + 0.5 * scale;


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
      for (unsigned int i = 1; i < ORB_MAX_LINES; i++) {
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


    // Finish up the encounter X Y plots
    for (unsigned int v = 0; v < vdata[wkg].size(); v++) {
      int enc_ix = vdata[wkg][v].enc_ix;
      VECTOR2 Q_maj;
      Q_maj.x = s4i[wkg][enc_ix].body[LP.plotix[0]].Q.x;
      Q_maj.y = s4i[wkg][enc_ix].body[LP.plotix[0]].Q.z;

      vdata[wkg][v].orb_plot_ves_enc.x = ((vdata[wkg][v].vs4i[enc_ix].ves.Q.x - Q_maj.x) / 1000.0 - min_Q.x) / scale;
      vdata[wkg][v].orb_plot_ves_enc.y = (max_Q.y - (vdata[wkg][v].vs4i[enc_ix].ves.Q.z - Q_maj.y) / 1000.0) / scale;

      vdata[wkg][v].orb_plot_body_enc[1].x = ((s4i[wkg][enc_ix].LP.Q.x - Q_maj.x) / 1000.0 - min_Q.x) / scale;
      vdata[wkg][v].orb_plot_body_enc[1].y = (max_Q.y - (s4i[wkg][enc_ix].LP.Q.z - Q_maj.y) / 1000.0) / scale;

      for (unsigned int i = 2; i < ORB_MAX_LINES; i++) {
        if (LP.plotix[i] == -1) break;
        vdata[wkg][v].orb_plot_body_enc[i].x = ((s4i[wkg][enc_ix].body[LP.plotix[i]].Q.x - Q_maj.x) / 1000.0 - min_Q.x) / scale;
        vdata[wkg][v].orb_plot_body_enc[i].y = (max_Q.y - (s4i[wkg][enc_ix].body[LP.plotix[i]].Q.z - Q_maj.y) / 1000.0) / scale;
      }
    }
  }


  ms_elap = getMilliSpan(s_time);
  //sprintf(oapiDebugString(), "S4I Run for time: %8.3f MJD Range: %8.3f to %8.3f, runtime: %ims", s4i[wkg][0].sec, s4i[wkg][0].MJD, s4i[wkg][s4int_count[wkg]-1].MJD, ms_elap);

  dbg[wkg][0] = s4i[wkg][0].sec;
  dbg[wkg][1] = s4i[wkg][0].MJD;
  dbg[wkg][2] = s4i[wkg][s4int_count[wkg] - 1].MJD;
  dbg[wkg][3] = s4int_timestep[wkg];
  dbg[wkg][4] = ((double) ms_elap)/1000.0;
  dbg[wkg][5] = ORB_PLOT_COUNT;
  dbg[wkg][6] = s4int_count[wkg];


  {{{
      if (!s4i_valid && vdata[wkg].size()>0) {

        FILE *dump_s4i;
        if (fopen_s(&dump_s4i, ".\\Config\\MFD\\Lagrange\\Diags\\S4I.csv", "w") == 0) {

          fprintf(dump_s4i, "INDEX, MJD, "
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
                            "Vr - QX, Vr - QY, "
                            "Mr - QX, Mr - QY, "
                            "LPr - QX, LPr - QY, "
                            "Vrlp - QX, Vrlp - QY, Vrlp - QZ, "
                            "Vrlp - PX, Vrlp - PY, Vrpl - PZ, "
                            "Vrlp - dQ, Vrlp - dP"
                  "\n");

          unsigned int i = 0; 
          unsigned int id = (s4int_count[wkg] - 1) / (ORB_PLOT_COUNT - 1);
          for (unsigned int s = 0; s < ORB_PLOT_COUNT; s++) {
            fprintf(dump_s4i, "%u, %.15lf,   ", i, s4i[wkg][i].MJD);
            fprintf(dump_s4i, "%.15lf, %.15lf, %.15lf,   ", s4i[wkg][i].body[LU_EARTH].Q.x, s4i[wkg][i].body[LU_EARTH].Q.z, s4i[wkg][i].body[LU_EARTH].Q.y);
            fprintf(dump_s4i, "%.15lf, %.15lf, %.15lf,   ", s4i[wkg][i].body[LU_MOON].Q.x, s4i[wkg][i].body[LU_MOON].Q.z, s4i[wkg][i].body[LU_MOON].Q.y);
            fprintf(dump_s4i, "%.15lf, %.15lf, %.15lf,   ", s4i[wkg][i].body[LU_SUN].Q.x, s4i[wkg][i].body[LU_SUN].Q.z, s4i[wkg][i].body[LU_SUN].Q.y);
            fprintf(dump_s4i, "%.15lf, %.15lf, %.15lf,   ", vdata[wkg][0].vs4i[i].ves.Q.x, vdata[wkg][0].vs4i[i].ves.Q.z, vdata[wkg][0].vs4i[i].ves.Q.y); // ves Q
            fprintf(dump_s4i, "%.15lf, %.15lf, %.15lf,   ", s4i[wkg][i].body[LU_EARTH].P.x, s4i[wkg][i].body[LU_EARTH].P.z, s4i[wkg][i].body[LU_EARTH].P.y);
            fprintf(dump_s4i, "%.15lf, %.15lf, %.15lf,   ", s4i[wkg][i].body[LU_MOON].P.x, s4i[wkg][i].body[LU_MOON].P.z, s4i[wkg][i].body[LU_MOON].P.y);
            fprintf(dump_s4i, "%.15lf, %.15lf, %.15lf,   ", s4i[wkg][i].body[LU_SUN].P.x, s4i[wkg][i].body[LU_SUN].P.z, s4i[wkg][i].body[LU_SUN].P.y);
            fprintf(dump_s4i, "%.15lf, %.15lf, %.15lf,   ", vdata[wkg][0].vs4i[i].ves.P.x, vdata[wkg][0].vs4i[i].ves.P.z, vdata[wkg][0].vs4i[i].ves.P.y); // ves P
            fprintf(dump_s4i, "%.15lf, %.15lf, %.15lf,   ", 0.0, 0.0, 0.0); // impulse
            fprintf(dump_s4i, "%.15lf, %.15lf, %.15lf,   ", s4i[wkg][i].LP.Q.x, s4i[wkg][i].LP.Q.z, s4i[wkg][i].LP.Q.y);
            fprintf(dump_s4i, "%.15lf, %.15lf, %.15lf,   ", s4i[wkg][i].LP.P.x, s4i[wkg][i].LP.P.z, s4i[wkg][i].LP.P.y);
            fprintf(dump_s4i, "%.15lf, %.15lf,   ",         vdata[wkg][0].orb_km[s].x, vdata[wkg][0].orb_km[s].y);
            fprintf(dump_s4i, "%.15lf, %.15lf,   ",         l_orb[wkg].orb_km[2][s].x, l_orb[wkg].orb_km[2][s].y);
            fprintf(dump_s4i, "%.15lf, %.15lf,   ",         l_orb[wkg].orb_km[1][s].x, l_orb[wkg].orb_km[1][s].y);
            fprintf(dump_s4i, "%.15lf, %.15lf, %.15lf,   ", vdata[wkg][0].vs4i[i].vesLP.Q.x, vdata[wkg][0].vs4i[i].vesLP.Q.z, vdata[wkg][0].vs4i[i].vesLP.Q.y); // ves Q
            fprintf(dump_s4i, "%.15lf, %.15lf, %.15lf,   ", vdata[wkg][0].vs4i[i].vesLP.P.x, vdata[wkg][0].vs4i[i].vesLP.P.z, vdata[wkg][0].vs4i[i].vesLP.P.y); // ves P
            fprintf(dump_s4i, "%.15lf, %.15lf\n", vdata[wkg][0].vs4i[i].dQ, vdata[wkg][0].vs4i[i].dP);
            i += id;
          }
          fprintf(dump_s4i, "\nMaxX, MinX, MaxY, MinY, Scale\n");
          fprintf(dump_s4i, "%.1f, %.1f, %.1f, %.1f, %.1f\n", max_Q.x, min_Q.x, max_Q.y, min_Q.y, scale);
          fclose(dump_s4i);
        }
      }
    }}} 
  s4i_valid = true;
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
  double mu;

  for (int ix = 0; LP.bodyIx[ix] != -1; ix++) {
    int g = LP.bodyIx[ix];
    if (g==e) continue;
    Qg = s4i[wkg][i].body[g].Q;
    Qeg = Qe - Qg;
    temp = pow((Qeg.x*Qeg.x)+(Qeg.y*Qeg.y) + (Qeg.z*Qeg.z), 1.5);
    mu = body[g].mu;
    Fadd = Qeg * (-mu / temp);
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
  double mu;

  for (int ix = 0; LP.bodyIx[ix] != -1; ix++) {
    int g = LP.bodyIx[ix];
    Qg = s4i[wkg][i].body[g].Q;
    Qvg = Qv - Qg;
    temp = pow((Qvg.x*Qvg.x) + (Qvg.y*Qvg.y) + (Qvg.z*Qvg.z), 1.5);
    mu = body[g].mu;
    F -= Qvg * (mu / temp);
  }
  return F;
}


inline VECTOR3 safe_unit (const VECTOR3 &a)
{
  if (abs(a.x)>1e-9 || abs(a.y)>1e-9 || abs(a.z)>1e-9) return a / length(a);
  return a;
}
inline VECTOR3 safe_crossp (const VECTOR3 &a, const VECTOR3 &b)
{
  if (abs(a.y*b.z - b.y*a.z)>1e-9 || abs(a.z*b.x - b.z*a.x)>1e-9 || abs(a.x*b.y - b.x*a.y)>1e-9) return _V(a.y*b.z - b.y*a.z, a.z*b.x - b.z*a.x, a.x*b.y - b.x*a.y);
  VECTOR3 c = b + _V(0.6, 0.8, 0.0); // Collinear vectors: add a 1.0m length bias in the X-Y plane to force a non-zero cross product
  return _V(a.y*c.z - c.y*a.z, a.z*c.x - c.z*a.x, a.x*c.y - c.x*a.y);
}