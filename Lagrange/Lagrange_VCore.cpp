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
  ap.SetVessel(v);
  ap_armed = false;

  burnGranularity = 1;
  burnVar = 0;
  burnTdV_lock = false;
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
  // Run every step of the simulation
  v->GetGlobalPos(LU->vdata[LU->act][vix].vs4i[0].ves.Q);
  v->GetGlobalVel(LU->vdata[LU->act][vix].vs4i[0].ves.P);

  LU->vdata[LU->act][vix].vs4i[0].mass = LU->vdata[LU->act][vix].curMass = v->GetMass();
  if (LU->s4i_valid) {
    LU->lp_ves(vix, 0, LU->act);
  }

  if (LU->vdata[LU->act][vix].burnArmed) {
    if (!ap_armed) {
      ap.SetVessel(v);
      ap.SetRefBody(LU->body[LU->LP.ref].hObj);
      ap.Enable();
      ap_armed = true;
    }
    double burnTimer = (LU->vdata[LU->act][vix].burnMJD - mjd) * 24.0 * 60.0 * 60.0;
    if (burnTimer >= 0.0 && burnTimer < 1000.0) {
      ap.SetTargetVector(LU->vdata[LU->act][vix].burndV);
      //ap.Update(SimDT);
    }
  } else {
    if (ap_armed) {
      ap.Disable();
      ap.Update(SimDT);
      ap_armed = false;
    }
  }
  return;
}
