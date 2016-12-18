// ==============================================================
//
//	StationKeep (Core Persistence)
//	=======================
//
//	Copyright (C) 2013	Andrew (ADSWNJ) Stokes
//                   All rights reserved
//
//	See StationKeep.cpp
//
// ==============================================================

#include "StationKeep_GCore.hpp"
#include "StationKeep_VCore.hpp"

StationKeep_GCore::StationKeep_GCore(HINSTANCE h) {
  hDLL = h;
  coreSimT = 0.0;
  return;
}


void StationKeep_GCore::corePreStep(double SimT,double SimDT,double mjd) {
  if (coreSimT == 0) {
    coreSimT = SimT;
    return;
  }
  if (coreSimT == SimT) return;

  coreSimDT = SimT - coreSimT;
  coreSimT = SimT;
  //sprintf(oapiDebugString(),"GCORE PRESTEP: %15.15f", coreSimDT);

  // Once per update - call vessel corePreSteps
  for (StationKeep_VCore* VC = (StationKeep_VCore*) P.IterateVC(false); VC != NULL; VC = (StationKeep_VCore*) P.IterateVC(true)) {
    VC->corePreStep(coreSimT, coreSimDT, mjd);
  }
}
