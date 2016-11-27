// ==============================================================
//
//	Lagrange (Dialog Function Handlers)
//	===============================
//
//	Copyright (C) 2016	Andrew (ADSWNJ) Stokes and Keith (Keithth G) Gelling
//                   All rights reserved
//
//	See Lagrange.cpp
//
// ==============================================================

#include "Lagrange_DialogFunc.hpp"
#include "Lagrange_GCore.hpp"
#include "Lagrange_VCore.hpp"
#include "Lagrange_LCore.hpp"
#include <EnjoLib/ModuleMessagingExt.hpp>


bool Lagrange_DialogFunc::skipWS(char **p) {

  while (**p == ' ' || **p == '\t') (*p)++;
  return (**p == '\0');
}

bool Lagrange_DialogFunc::spanStr(char **p) {
  while (**p != ' ' && **p != '\t' && **p != '\0') (*p)++;
  return (**p == '\0');
}

bool Lagrange_DialogFunc::clbkITR(void *id, char *str, void *usrdata) {
  unsigned int i;

  if (strlen(str) == 0) return true;
  if (sscanf_s(str, "%u", &i) != 1) return true;
  if (i <= 0) return false;
  Lagrange_LCore* LC = (Lagrange_LCore *)usrdata;
  LC->GC->LU->s4int_count[LC->GC->LU->act] = i;
  return true;
}

bool Lagrange_DialogFunc::clbkTSP(void *id, char *str, void *usrdata) {
  float f;

  if (strlen(str) == 0) return true;
  if (sscanf_s(str, "%f", &f) != 1) return true;
  if (f <= 0.0) return false;
  Lagrange_LCore* LC = (Lagrange_LCore *)usrdata;
  LC->GC->LU->s4int_timestep[LC->GC->LU->act] = f;
  return true;
}

bool Lagrange_DialogFunc::clbkENT(void *id, char *str, void *usrdata) {
  double f;

  if (strlen(str) == 0) return true;
  if (sscanf_s(str, "%lf", &f) != 1) return true;
  Lagrange_LCore* LC = (Lagrange_LCore *)usrdata;
  Lagrange_VCore* VC = LC->VC;
  LagrangeUniverse* LU = LC->GC->LU;
  Lagrange_vdata *vdata = &VC->LU->vdata[VC->LU->act][VC->vix];
  switch (VC->burnVar) {
  case 0:
    vdata->burnMJD = f;
    break;
  case 1:
    vdata->burndV.x = f;
    break;
  case 2:
    vdata->burndV.y = f;
    break;
  case 3:
    vdata->burndV.z = f;
    break;
  }
  return true;
}
