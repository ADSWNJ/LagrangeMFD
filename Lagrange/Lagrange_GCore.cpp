// ==============================================================
//
//	Lagrange (Core Persistence)
//	===========================
//
//	Copyright (C) 2016	Andrew (ADSWNJ) Stokes and Keith (Keithth G) Gelling
//                   All rights reserved
//
//	See Lagrange.cpp
//
// ==============================================================

#include "Lagrange_GCore.hpp"
#include "Lagrange_VCore.hpp"

Lagrange_GCore::Lagrange_GCore() {
  coreSimT = 0.0;
  LU = nullptr;
  return;
}

Lagrange_GCore::~Lagrange_GCore() {
  if (LU != nullptr) delete LU;
  return;
}


void Lagrange_GCore::corePreStep(double simT,double simDT,double mjd) {
  if (coreSimT == 0) {
    coreSimT = simT;
    return;
  }
  if (coreSimT == simT) return;

  if (P.firstVC() == NULL) return; // No vessels interested in Lagrange yet

  if (LU == nullptr) { 
    LU = new LagrangeUniverse;
  }

  LU->updateUniverse();

  coreSimDT = simT - coreSimT;
  coreSimT = simT;
  //sprintf(oapiDebugString(),"GCORE PRESTEP: %15.15f", coreSimDT);

  // Once per update - call vessel corePreSteps
  for (Lagrange_VCore* VC = (Lagrange_VCore*) P.firstVC(); VC != NULL; VC = (Lagrange_VCore*) P.nextVC()) {
    VC->corePreStep(coreSimT, coreSimDT, mjd);
  }

  LU->threadCtrlMain();
}
