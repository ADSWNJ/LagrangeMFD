// ==============================================================
//
//	Lagrange (Dialog Function Handlers)
//	===============================
//
//	Copyright (C) 2016-2017	Andrew (ADSWNJ) Stokes
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

bool Lagrange_DialogFunc::clbkOPC(void *id, char *str, void *usrdata) {
  unsigned int i;

  if (strlen(str) == 0) return true;
  if (sscanf_s(str, "%u", &i) != 1) return true;
  if (i < 10 || i > 10000) return false;
  Lagrange_LCore* LC = (Lagrange_LCore *)usrdata;
  LC->GC->LU->orbPlotCountReq = i;
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

bool Lagrange_DialogFunc::clbkRNG(void *id, char *str, void *usrdata) {
  float f;
  float mult = 1.0;
  if (strlen(str) == 0) return true;
  char lst = *(str + strlen(str) - 1);
  if (lst == 'D' || lst == 'd') {
    mult = 60.0 * 60.0 * 24.0;
    *(str + strlen(str) - 1) = '\0';
  } else if (lst == 'H' || lst == 'h') {
    mult = 60.0 * 60.0;
    *(str + strlen(str) - 1) = '\0';
  } else if (lst == 'M' || lst == 'm') {
    mult = 60.0;
    *(str + strlen(str) - 1) = '\0';
  } else if (lst == 'S' || lst == 's') {
    *(str + strlen(str) - 1) = '\0';
  }
  if (sscanf_s(str, "%f", &f) != 1) return true;
  if (f <= 0.0) return false;
  f *= mult;
  Lagrange_LCore* LC = (Lagrange_LCore *)usrdata;
  double itr = LC->GC->LU->s4int_count[LC->GC->LU->act];
  double tsp = LC->GC->LU->s4int_timestep[LC->GC->LU->act];
  double rng = itr * tsp;
  double ratio = f / rng;

  LC->GC->LU->s4int_timestep[LC->GC->LU->act] *= ratio;
  return true;
}

bool Lagrange_DialogFunc::clbkENC(void *id, char *str, void *usrdata) {
  float f;
  float mult = 1.0;
  if (strlen(str) == 0) return true;
  char lst = *(str + strlen(str) - 1);
  if (lst == 'D' || lst == 'd') {
    mult = 60.0 * 60.0 * 24.0;
    *(str + strlen(str) - 1) = '\0';
  } else if (lst == 'H' || lst == 'h') {
    mult = 60.0 * 60.0;
    *(str + strlen(str) - 1) = '\0';
  } else if (lst == 'M' || lst == 'm') {
    mult = 60.0;
    *(str + strlen(str) - 1) = '\0';
  } else if (lst == 'S' || lst == 's') {
    *(str + strlen(str) - 1) = '\0';
  }
  if (sscanf_s(str, "%f", &f) != 1) return true;
  if (f <= 0.0) return false;
  f *= mult;
  Lagrange_LCore* LC = (Lagrange_LCore *)usrdata;
  double itr = LC->GC->LU->s4int_count[LC->GC->LU->act];
  double tsp = LC->GC->LU->s4int_timestep[LC->GC->LU->act];
  double rng = itr * tsp;
  double ratio = f / rng;

  LC->GC->LU->s4int_timestep[LC->GC->LU->act] *= ratio;
  return true;
}


bool Lagrange_DialogFunc::clbkRCT(void *id, char *str, void *usrdata) {
  float f;

  if (strlen(str) == 0) return true;
  if (sscanf_s(str, "%f", &f) != 1) return true;
  if (f <= 0.0) return false;
  Lagrange_LCore* LC = (Lagrange_LCore *)usrdata;

  unsigned int itr_ui = LC->GC->LU->s4int_count[LC->GC->LU->act];
  double itr = (double)itr_ui;
  double tsp = LC->GC->LU->s4int_timestep[LC->GC->LU->act];
  double cur_ct_ms = LC->GC->LU->dbg[LC->GC->LU->act][6] * 1000.0;
  double f_ms = f * 1000.0;
  double ratio = f_ms / cur_ct_ms;
  double new_itr = itr * ratio;
  unsigned int new_itr_ui = (unsigned int)new_itr;
  LC->GC->LU->s4int_count[LC->GC->LU->act] = (unsigned int) new_itr_ui;
  ratio = (double) new_itr_ui / (double) itr_ui;
  tsp /= ratio;
  LC->GC->LU->s4int_timestep[LC->GC->LU->act] = tsp;
  return true;
}

bool Lagrange_DialogFunc::clbkHYS(void *id, char *str, void *usrdata) {
  float f;

  if (strlen(str) == 0) return true;
  if (sscanf_s(str, "%f", &f) != 1) return true;
  if (f <= 0.0) return false;
  Lagrange_LCore* LC = (Lagrange_LCore *)usrdata;
  LC->GC->LU->s4int_hysteresis = f;
  return true;
}


bool Lagrange_DialogFunc::clbkWT(void *id, char *str, void *usrdata) {
  float f;

  if (strlen(str) == 0) return true;
  if (sscanf_s(str, "%f", &f) != 1) return true;
  if (f < 0.0) return false;
  Lagrange_LCore* LC = (Lagrange_LCore *)usrdata;
  LC->GC->LU->s4int_refresh = f;
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
  double TdV = length(vdata->burndV);
  double ratio;

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
  case 4:
    if (TdV > 0.0) {
      vdata->burndV *= f / TdV;
    } else {
      vdata->burndV = _V(f,0.0,0.0);
    }
    break;
  }
  if (VC->burnTdV_lock && VC->burnVar > 0 && VC->burnVar < 4) {
    double sq_other;
    int i = VC->burnVar-1;
    int j = (i + 1) % 3;
    int k = (i + 2) % 3;
    sq_other = vdata->burndV.data[j] * vdata->burndV.data[j] + vdata->burndV.data[k] * vdata->burndV.data[k];
    if (sq_other > 1.0e-6) {
      ratio = sqrt(TdV * TdV - vdata->burndV.data[i] * vdata->burndV.data[i]) / sqrt(sq_other);
      vdata->burndV.data[j] *= ratio;
      vdata->burndV.data[k] *= ratio;
    }
  }
  return true;
}
