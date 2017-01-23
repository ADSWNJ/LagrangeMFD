// ==============================================================
//
//	Lagrange (Vessel Core Persistence)
//	==================================
//
//	Copyright (C) 2016-2017	Andrew (ADSWNJ) Stokes
//                   All rights reserved
//
//	See Lagrange.cpp
//
// ==============================================================

#include "Lagrange_GCore.hpp"
#include "Lagrange_VCore.hpp"
#include "ParseFunctions.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <assert.h>

Lagrange_VCore::Lagrange_VCore(VESSEL *vin, Lagrange_GCore* gcin) {
	// Vessel core constructor
  GC = gcin;
	v = vin;
  ap_armed = false;
  sk_armed = false;

  for (int i=0; i<5; i++) burnGranularity[i] = 1;
  burnVar = 0;
  burnTdV_lock = false;
  autocenter = false;
  LU = GC->LU;
  assert(LU != nullptr);
  int s = LU->vdata[LU->act].size();
  LU->vdata[LU->act].resize(s + 1);
  LU->vdata[LU->act][s].v = v;
  LU->vdata[LU->act][s].vs4i.resize(LU->s4int_count[LU->act]);
  LU->vdata[LU->act][s].burnArmed = false;
  LU->vdata[LU->act][s].burnMJD = 0.0;
  LU->vdata[LU->act][s].burndV = _V(0.0,0.0,0.0);
  LU->vdata[LU->act][s].refEnt = LU->LP.ref;
  LU->vdata[LU->act][s].TransX_CurrentBodyIndex = nullptr;
  LU->vdata[LU->act][s].TransX_PlanMajorIndex = nullptr;
  vix = s;

  // Figure out the delta mass per +1m/s of dV
  THGROUP_HANDLE thg = v->GetThrusterGroupHandle(THGROUP_MAIN);
  DWORD thgc = v->GetGroupThrusterCount(thg);
  double mainThrust = 0.0;
  double mainISP0 = 0.0;
  for (unsigned int i = 0; i < thgc; i++) {
    THRUSTER_HANDLE th = v->GetGroupThruster(thg, i);
    mainThrust += v->GetThrusterMax0(th);
    mainISP0 += v->GetThrusterIsp0(th);
  }
  mainISP0 /= thgc;
  LU->vdata[LU->act][s].mainThrust = mainThrust;
  LU->vdata[LU->act][s].mainExVel0 = mainISP0;
  LU->vdata[LU->act][s].curMass = v->GetMass();
  return;
};

Lagrange_VCore::~Lagrange_VCore() {
  // Vessel core destructor
  // Need to tell LU that we no longer exist, else he'll access violate on v.getMass(), etc. 
  for (auto e = LU->vdata[LU->act].begin(); e != LU->vdata[LU->act].end(); e++) {
    if (e->v == v) {
      LU->vdata[LU->act].erase(e);
      break;
    }
  }
}


void Lagrange_VCore::corePreStep(double SimT,double SimDT,double mjd) {
  VECTOR3 curQ, curP;
  double curMass;
  auto *vdata = &(LU->vdata[LU->act][vix]); 
  auto *vs4i_0 = &(vdata->vs4i[0]);

  v->GetGlobalPos(curQ);
  v->GetGlobalVel(curP);
  curMass = v->GetMass();

  vs4i_0->ves.Q = curQ;
  vs4i_0->ves.P = curP;
  vs4i_0->mass = curMass;
  vdata->curMass = curMass;

  if (LU->s4i_valid) {
    LU->lp_ves(vix, 0, LU->act);

    VECTOR3 burnQv, burnQe, burnPv, burnPe;
    double burnSimT, burnMJD;

    if ((vdata->burnArmed) && (vdata->burn_ix > 0)) {
      auto *vs4i_b = &(vdata->vs4i[vdata->burn_ix]);
      auto *es4i_b = &(LU->s4i[LU->act][vdata->burn_ix].body[LU->LP.ref]);
      burnQv = vs4i_b->ves.Q;
      burnPv = vs4i_b->ves.P;
      burnQe = es4i_b->Q;
      burnPe = es4i_b->P;
      burnSimT = LU->s4i[LU->act][vdata->burn_ix].sec;
      burnMJD = LU->s4i[LU->act][vdata->burn_ix].MJD;
    } else {
      OBJHANDLE hRef = (LU->body[LU->LP.ref].hObj);
      burnQv = curQ;
      burnPv = curP;
      oapiGetGlobalPos(hRef, &burnQe);
      oapiGetGlobalVel(hRef, &burnPe);
      burnSimT = 0.0;
      burnMJD = 0.0;
    }

    ap.Update(v, SimT, SimDT, burnSimT, LU->vdata[LU->act][vix].burndV, (burnQv - burnQe), (burnPv - burnPe));

//  if (ap_armed) {
//    double burnTimer = (LU->vdata[LU->act][vix].burnMJD - mjd) * 24.0 * 60.0 * 60.0;
//    if (burnTimer >= 0.0 && burnTimer < 1000.0) {
//      ap.Update(SimT, SimDT);
//    }
//  }
  }
  return;
}


void Lagrange_VCore::ap_arm(bool arm) {
  if (arm) {
    ap.Enable(v);
    ap_armed = true;
  } else {
    ap.Disable(v);
    autocenter = false;
    ap_armed = false;
  }
}