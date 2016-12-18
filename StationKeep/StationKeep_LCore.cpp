// ==========================================================================
//
//	StationKeep (Local (Vessel+MFD Panel) Core Persistence)
//	================================================
//
//	Copyright (C) 2013	Andrew (ADSWNJ) Stokes
//                   All rights reserved
//
//	See StationKeep.cpp
//
// ==========================================================================

#include "StationKeep_GCore.hpp"
#include "StationKeep_VCore.hpp"
#include "StationKeep_LCore.hpp"

StationKeep_LCore::StationKeep_LCore(VESSEL *vin, UINT mfdin, StationKeep_GCore* gcin) {
  GC = gcin;
  v = vin;
  m = mfdin;

  VC = (StationKeep_VCore*) GC->P.FindVC(v);

  showMessage = false;
  okMessagePage = true;
  return;
}
