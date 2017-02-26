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
  //ap_armed = false;
  //sk_armed = false;
  apMode = apState = 0;
  apHoldInRange = false;

  for (int i=0; i<5; i++) burnGranularity[i] = 1;
  burnVar = 0;
  burnTdV_lock = false;
  //autocenter = false;
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

  if (LU->s4i_valid) {

    VECTOR3 curQ, curP;
    double curMass;
    auto *vdata = &(LU->vdata[LU->act][vix]);
    auto *vs4i_0 = &(vdata->vs4i[0]);
    auto *es4i_0 = &(LU->s4i[LU->act][0].body[LU->LP.ref]);

    v->GetGlobalPos(curQ);
    v->GetGlobalVel(curP);
    curMass = v->GetMass();

    vs4i_0->ves.Q = curQ;
    vs4i_0->ves.P = curP;
    vs4i_0->mass = curMass;
    vdata->curMass = curMass;

    burnFrozen = ap.IsBurnFrozen();

    apHoldInRange = vs4i_0->dQ < 1.0e6; // LP within 1000km

    if (((vdata->burnArmed) && (vdata->burn_ix > 0)) ||(burnFrozen)) {
      if (apMode != 1) apState = 1; // if we are switching into plan, then put AP on standby
      apMode = 1;
      if (burnFrozen) vdata->burnArmed = false;
    } else if (apHoldInRange) {
      if (apMode != 2) {
        apState = 1; // if we are switching into hold, then put AP on standby
        ap_ahStartMassV = v->GetMass();
      }
      apMode = 2;
    } else {
      apMode = 0;
      apState = 0;
    }

    LU->lp_ves(vix, 0, LU->act);

    VECTOR3 burnQv, burnQe, burnPv, burnPe, burn_Q, burn_P;
    double burnSimT, burnMJD;

    if (apMode == 0) return; // AP inactive

    if (apMode == 1) {
      int i = vdata->burn_ix;
      if (i < 0) i = 0;
      //auto *vs4i_b = &(vdata->vs4i[i]);
      auto *es4i_b = &(LU->s4i[LU->act][i].body[LU->LP.ref]);
      burnQv = vdata->burn.Q;
      burnPv = vdata->burn.P;
      burnQe = es4i_b->Q;
      burnPe = es4i_b->P;
      burnSimT = LU->s4i[LU->act][i].sec;
      burnMJD = LU->s4i[LU->act][i].MJD;
      burn_Q = burnQv - burnQe;
      burn_P = burnPv - burnPe;
      double lenBurnP = length(burn_P);
      ap.Update_PlanMode(v, apState, SimT, SimDT, burnSimT, LU->vdata[LU->act][vix].burndV, burn_Q, burn_P);
      if (apState > 1) {
        burnStart = ap.GetBurnStart();
        burnEnd = ap.GetBurnEnd();
        burnDurn = burnEnd - burnStart;
        VESSELSTATUS vs;
        v->GetStatus(vs);
        burnCV = length(vs.rvel);
        burnDV = ap.GetBurnDV();
        burnTV = ap.GetBurnTgtV();
      }
    } else {
      burnQv = vs4i_0->ves.Q;
      burnPv = vs4i_0->ves.P;
      burnQe = es4i_0->Q;
      burnPe = es4i_0->P;
      burn_Q = burnQv - burnQe;
      burn_P = burnPv - burnPe;
      ap.Update_HoldMode(v, apState, SimT, SimDT, burn_Q, burn_P, vs4i_0->vesLP.Q, vs4i_0->vesLP.P);
    }
  }
  return;
}