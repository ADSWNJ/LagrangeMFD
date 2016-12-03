// ==============================================================
//
//	Lagrange (Vessel Core Persistence)
//	==================================
//
//	Copyright (C) 2016	Andrew (ADSWNJ) Stokes and Keith (Keithth G) Gelling
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
  vix = s;
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

  LU->vdata[LU->act][vix].vs4i[0].mass = LU->vdata[LU->act][vix].cmass = v->GetMass();
  if (LU->s4i_valid) {
    LU->lp_ves(vix, 0, LU->act);
  }
  return;
}
