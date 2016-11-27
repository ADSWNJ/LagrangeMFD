// ==========================================================================
//
//	Lagrange (Local (Vessel+MFD Panel) Core Persistence)
//	================================================
//
//	Copyright (C) 2016	Andrew (ADSWNJ) Stokes and Keith (Keithth G) Gelling
//                   All rights reserved
//
//	See Lagrange.cpp
//
// ==========================================================================

#include "Lagrange_GCore.hpp"
#include "Lagrange_VCore.hpp"
#include "Lagrange_LCore.hpp"

Lagrange_LCore::Lagrange_LCore(VESSEL *vin, UINT mfdin, Lagrange_GCore* gcin) {
  GC = gcin;
  v = vin;
  m = mfdin;

  VC = (Lagrange_VCore*) GC->P.findVC(v);

  showMessage = false;
  okMessagePage = true;
  mode = 0;
  return;
}
