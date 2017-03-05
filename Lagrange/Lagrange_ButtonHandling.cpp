// ==============================================================
//
//	Lagrange (Button Handling Code)
//	===============================
//
//	Copyright (C) 2016-2017	Andrew (ADSWNJ) Stokes
//                   All rights reserved
//
//	See Lagrange.cpp
//
// ==============================================================

#include "Lagrange.hpp"
#include "Lagrange_DialogFunc.hpp"
#include "Lagrange_Universe.hpp"
#include <EnjoLib/ModuleMessagingExt.hpp>
#include <math.h>

// ==============================================================
// MFD button hooks to Button Page library
//
char* Lagrange::ButtonLabel (int bt)
{
	return LC->B.ButtonLabel(bt);
}

// Return button menus
int Lagrange::ButtonMenu (const MFDBUTTONMENU **menu) const
{
	return LC->B.ButtonMenu(menu);
}

// Return clicked button
bool Lagrange::ConsumeButton (int bt, int event) {
  return LC->B.ConsumeButton(this, bt, event);
}

// Return pressed keystroke
bool Lagrange::ConsumeKeyBuffered (DWORD key) {
  return LC->B.ConsumeKeyBuffered(this, key);
}



// ==============================================================
// MFD Button Handler Callbacks
//

// MOD = Mode Select
void Lagrange::Button_MOD() {
  LC->mode++;
  if (LC->mode > 4) LC->mode = 0;
  LC->B.SwitchPage(this, LC->mode);
  return;
};
// ARM = Plan Mode: Arm/Disarm the burn planning
void Lagrange::Button_BURNARM() {
  LagrangeUniverse *LU = VC->LU;
  Lagrange_vdata *vdata = &VC->LU->vdata[VC->LU->act][VC->vix];
  vdata->burnArmed = !vdata->burnArmed;
  if (vdata->burnArmed) {
    if (vdata->burnMJD < oapiGetSimMJD()) {
      vdata->burnMJD = oapiGetSimMJD() + (15.0 / (60.0 * 24.0)); // bump initial MJD 15 mins in the future
    }
  }
}
// MJD = Plan Mode: Date Select
void Lagrange::Button_MJD() {
  VC->burnVar = 0;
}
// PRO = Plan Mode: Prograde Var
void Lagrange::Button_PRO() {
  VC->burnVar = 1;
}
// PLC = Plan Mode: Plane Change Var
void Lagrange::Button_PLC() {
  VC->burnVar = 3;
}
// OUT = Plan Mode: Outward Var
void Lagrange::Button_OUT() {
  VC->burnVar = 2;
}
// TDV = Plan Mode: Total DV
void Lagrange::Button_TDV() {
  if (VC->burnVar == 4) VC->burnTdV_lock = !VC->burnTdV_lock;
  VC->burnVar = 4;
}


// ADM = Plan Mode: Adjust Granularity Down
void Lagrange::Button_ADM() {
  VC->burnGranularity[VC->burnVar]--;
  if (VC->burnGranularity[VC->burnVar] < 0) VC->burnGranularity[VC->burnVar] = 8;
}
// ADJ = Plan Mode: Adjust Granularity Up
void Lagrange::Button_ADJ() {
  VC->burnGranularity[VC->burnVar]++;
  if (VC->burnGranularity[VC->burnVar] > 8) VC->burnGranularity[VC->burnVar] = 0;
}
// AUP = Plan Mode: Increment Value
void Lagrange::Button_AUP() {
  return ButtonHelper_AdjVar(+1.0);
}
// ADN = Plan Mode: Decrement Value
void Lagrange::Button_ADN() {
  return ButtonHelper_AdjVar(-1.0);
}
void Lagrange::ButtonHelper_AdjVar(double adj) {
  Lagrange_vdata *vdata = &VC->LU->vdata[VC->LU->act][VC->vix];
  double TdV = length(vdata->burndV);
  double ratio;
  if (VC->burnGranularity[VC->burnVar] == 8) {
    switch (VC->burnVar) {
    case 0:
      vdata->burnMJD = oapiGetSimMJD();
      break;
    case 1:
      vdata->burndV.x = 0.0;
      break;
    case 2:
      vdata->burndV.y = 0.0;
      break;
    case 3:
      vdata->burndV.z = 0.0;
      break;
    case 4:
      vdata->burndV = _V(0.0, 0.0, 0.0);
      break;
    }
  } else {
    adj *= 10.0 / pow(10.0, (double)VC->burnGranularity[VC->burnVar]);
    switch (VC->burnVar) {
    case 0:
      vdata->burnMJD += adj;
      break;
    case 1:
      vdata->burndV.x += adj;
      break;
    case 2:
      vdata->burndV.y += adj;
      break;
    case 3:
      vdata->burndV.z += adj;
      break;
    case 4:
      if (TdV == 0.0) {
        vdata->burndV = _V(TdV + adj, 0.0, 0.0);
      } else {
        ratio = (TdV + adj) / TdV;
        vdata->burndV *= ratio;
      }
    }
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
}

// ENT = Plan Mode: Enter Value
void Lagrange::Button_ENT() {
  Lagrange_vdata *vdata = &VC->LU->vdata[VC->LU->act][VC->vix];
  double TdV = length(vdata->burndV);

  switch (VC->burnVar) {
  case 0:
    sprintf(GC->LU->buf, "%.6f", vdata->burnMJD);
    ButtonHelper_TrimEntBox(GC->LU->buf);
    oapiOpenInputBox("Enter Plan Burn MJD", Lagrange_DialogFunc::clbkENT, GC->LU->buf, 20, LC);
    break;
  case 1:
    sprintf(GC->LU->buf, "%.6f", vdata->burndV.x);
    ButtonHelper_TrimEntBox(GC->LU->buf);
    oapiOpenInputBox("Enter Plan Prograde Delta-V", Lagrange_DialogFunc::clbkENT, GC->LU->buf, 20, LC);
    break;
  case 2:
    sprintf(GC->LU->buf, "%.6f", vdata->burndV.y);
    ButtonHelper_TrimEntBox(GC->LU->buf);
    oapiOpenInputBox("Enter Plan Outward Delta-V", Lagrange_DialogFunc::clbkENT, GC->LU->buf, 20, LC);
    break;
  case 3:
    sprintf(GC->LU->buf, "%.6f", vdata->burndV.z);
    ButtonHelper_TrimEntBox(GC->LU->buf);
    oapiOpenInputBox("Enter Plan Plane Change Delta-V", Lagrange_DialogFunc::clbkENT, GC->LU->buf, 20, LC);
    break;
  case 4:
    sprintf(GC->LU->buf, "%.6f", TdV);
    ButtonHelper_TrimEntBox(GC->LU->buf);
    oapiOpenInputBox("Enter Plan Total Delta-V", Lagrange_DialogFunc::clbkENT, GC->LU->buf, 20, LC);
    break;
  }
}
void Lagrange::ButtonHelper_TrimEntBox(char *buf) {
  char *b = buf + strlen(buf) -1;
  while (*b == '0') {
    *b-- = '\0';
  }
  if (*b == '.') {
    *b = '\0';
  }
  if (strlen(buf) == 1 && buf[0] == '0') buf[0] = '\0';
}

// TGT = Orbit Mode: Select Target - e.g. Sun Earth L2
void Lagrange::Button_TGT() {
  GC->LU->PrvNxtMode = 0;
  LC->mode = 6;
  LC->B.SwitchPage(this, LC->mode);
  return;
}
// FRM = Orbit Mode: Select Frame of Reference for PROGRADE calcs
void Lagrange::Button_FRM() {
  GC->LU->PrvNxtMode = 1;
  LC->mode = 7;
  LC->B.SwitchPage(this, LC->mode);
  return;
}
// FOC = Orbit Mode: Select Focus - 0 = Major, 1 = Minor, 2 = Vessel, 3 = Enc, 4 = Burn
void Lagrange::Button_FOC() {
  GC->LU->PrvNxtMode = 2;
  LC->mode = 7;
  LC->B.SwitchPage(this, LC->mode);
  return;
}
// LEG = Orbit Mode: Legend off/on
//void Lagrange::Button_LEG() {
//  GC->LU->orbLegend = !GC->LU->orbLegend;
//  return;
//}

// RST = Orbit Mode: Reset Pan/Zoom
void Lagrange::Button_RST() {
  GC->LU->orbZoom = 0;
  for (int i = 0; i < 3; i++) {
    GC->LU->orbPanHoriz[i] = 0.0;
    GC->LU->orbPanVert[i] = 0.0;
  }
  return;
}


// PRJ = Orbit Mode: Select Projection
void Lagrange::Button_PRJ() {
  if (GC->LU->orbProj == 2) {
    GC->LU->orbProj = 0;
  } else {
    GC->LU->orbProj++;
  }
  return;
}
// ZMU = Orbit Mode: Zoom Up
void Lagrange::Button_ZMU() {
  GC->LU->orbZoom--;
  return ;
}
// ZMD = Orbit Mode: Zoom Down
void Lagrange::Button_ZMD() {
  GC->LU->orbZoom++;
  return;
}
// MUP = Orbit Mode: Move Up
void Lagrange::Button_MUP() {
  double offset = 10000000.0; // 10000 km for Y-plane
  if (GC->LU->orbProj == 0) offset = (GC->LU->LP.mradius / 10.0);
  double new_pan = GC->LU->orbPanVert[GC->LU->orbProj] + offset;
  GC->LU->orbPanVert[GC->LU->orbProj] = new_pan;
  return;
}
// MDN = Orbit Mode: Move Down
void Lagrange::Button_MDN() {
  double offset = 10000000.0; // 10000 km for Y-plane
  if (GC->LU->orbProj == 0) offset = (GC->LU->LP.mradius / 10.0);
  double new_pan = GC->LU->orbPanVert[GC->LU->orbProj] - offset;
  GC->LU->orbPanVert[GC->LU->orbProj] = new_pan;
  return;
}
// MLF = Orbit Mode: Move Left
void Lagrange::Button_MLF() {
  double new_pan = GC->LU->orbPanHoriz[GC->LU->orbProj] - (GC->LU->LP.mradius / 10.0);
  GC->LU->orbPanHoriz[GC->LU->orbProj] = new_pan;
}
// MRG = Orbit Mode: Move Right
void Lagrange::Button_MRG() {
  double new_pan = GC->LU->orbPanHoriz[GC->LU->orbProj] + (GC->LU->LP.mradius / 10.0);
  GC->LU->orbPanHoriz[GC->LU->orbProj] = new_pan;
}
// AAB = AP Mode: Auto Burn
void Lagrange::Button_AAB() {
  Lagrange_vdata *vdata = &VC->LU->vdata[VC->LU->act][VC->vix];
  Lagrange_ves_s4i *vs4i = &vdata->vs4i[0];
  if (VC->apMode != 1) return; // AB button off if not in plan mode
  if (VC->apState == 3) {
    VC->apState = 2;
  } else {
    VC->apState = 3;
  }
}
// AAC = AP Mode: Auto Align
void Lagrange::Button_AAC() {
  Lagrange_vdata *vdata = &VC->LU->vdata[VC->LU->act][VC->vix];
  Lagrange_ves_s4i *vs4i = &vdata->vs4i[0];
  if (VC->apMode != 1) return; // AA button off if not in plan mode
  if (VC->apState >= 2) {
    VC->apState = 1;
    VC->ap.Disable(VC->v);
  } else {
    VC->apState = 2;
  }
}
// AAH = AP Mode: Auto Hold
void Lagrange::Button_AAH() {
  Lagrange_vdata *vdata = &VC->LU->vdata[VC->LU->act][VC->vix];
  Lagrange_ves_s4i *vs4i = &vdata->vs4i[0];
  if (VC->apMode != 2) return; // AH button off if not in hold mode
  if (VC->apState >= 2) {
    VC->apState = 1;
  } else {
    VC->apState = 3;
  }
}
// OK = Message Acknowledge
void Lagrange::Button_OK() {
  LC->showMessage = false;
  LC->B.SwitchPage(this, LC->mode);
  return;
}
void Lagrange::Button_NotImplementedYet() {
  LC->Message = "Function not yet implemented in this version of Lagrange MFD!";
  LC->showMessage = true;
  LC->B.SwitchPage(this, 5);
  return;
}

// TOG = Lagrange S4I Toggle
void Lagrange::Button_S4IARM() {
  GC->LU->threadCtrlPauseToggle();
  return;
};

// NXT = Lagrange Next LP, FrameRef, Focus Mode
void Lagrange::Button_NXT() {
  int *prefEnt;
  switch (GC->LU->PrvNxtMode) {
  case 0:
    GC->LU->selectNextLP();
    break;
  case 1:
    prefEnt = &GC->LU->vdata[GC->LU->act][VC->vix].refEnt;
    (*prefEnt)++;
    if (*prefEnt == COUNT_BODY) *prefEnt = 0;
    break;
  case 2:
    if (GC->LU->orbFocus == 4) {
      GC->LU->orbFocus = 0;
    } else {
      GC->LU->orbFocus++;
    }
    break;
  }
  return;
};

// PRV = Lagrange Prev LP, FrameRef, Focus Mode
void Lagrange::Button_PRV() {
  int *prefEnt;
  switch (GC->LU->PrvNxtMode) {
  case 0:
    GC->LU->selectPrevLP();
    break;
  case 1:
    prefEnt = &GC->LU->vdata[GC->LU->act][VC->vix].refEnt;
    (*prefEnt)--;
    if (*prefEnt < 0) *prefEnt = COUNT_BODY - 1;
    break;
  case 2:
    if (GC->LU->orbFocus == 0) {
      GC->LU->orbFocus = 4;
    } else {
      GC->LU->orbFocus--;
    }
    break;
  }
  return;
};

// TOK = Target Acknowledge
void Lagrange::Button_TOK() {
  LC->mode = 0;
  LC->B.SwitchPage(this, LC->mode);
  return;
}

// ITR = Adjust Iteration Count
void Lagrange::Button_ITR() {
  sprintf(GC->LU->buf, "%u", GC->LU->s4int_count[GC->LU->act]);
  oapiOpenInputBox("Enter Iteration Count (e.g. 20000)", Lagrange_DialogFunc::clbkITR, GC->LU->buf, 20, LC);
  return;
}

// HYS = Adjust Hysteresis Value (need to be better than this deadband to update an encounter point)
void Lagrange::Button_HYS() {
  double tmp = GC->LU->s4int_hysteresis;
  sprintf(GC->LU->buf, "%.1f", tmp);
  ButtonHelper_TrimEntBox(GC->LU->buf);
  oapiOpenInputBox("Enter Enc. Hysteresis Dist in m (e.g. 1.0)", Lagrange_DialogFunc::clbkHYS, GC->LU->buf, 20, LC);
  return;
}

// TSP = Adjust Iteration TimeStep
void Lagrange::Button_TSP() {
  sprintf(GC->LU->buf, "%.1f", GC->LU->s4int_timestep[GC->LU->act]);
  ButtonHelper_TrimEntBox(GC->LU->buf);
  oapiOpenInputBox("Enter Iteration Timestep (e.g. 30.0)", Lagrange_DialogFunc::clbkTSP, GC->LU->buf, 20, LC);
  return;
}

// RNG = Adjust S4I Calc Range
void Lagrange::Button_RNG() {
  sprintf(GC->LU->buf, "");
  oapiOpenInputBox("Enter S4I Calc Range [d, h, m, s] (e.g. 365d, 24.0h, 23.123m)", Lagrange_DialogFunc::clbkRNG, GC->LU->buf, 20, LC);
  return;
}

// RCT = Adjust S4I Calc Time
void Lagrange::Button_RCT() {
  sprintf(GC->LU->buf, "");
  oapiOpenInputBox("Enter Requested Calc Time (e.g. 0.750)", Lagrange_DialogFunc::clbkRCT, GC->LU->buf, 20, LC);
  return;
}

// WT = Adjust S4I Wait Time
void Lagrange::Button_WT() {
  sprintf(GC->LU->buf, "%.1f", GC->LU->s4int_refresh);
  ButtonHelper_TrimEntBox(GC->LU->buf);
  oapiOpenInputBox("Enter S4I wait time (e.g. 0.0 or 10.0)", Lagrange_DialogFunc::clbkWT, GC->LU->buf, 20, LC);
  return;
}

// DML = Trigger Dump Log
void Lagrange::Button_DML() {
  GC->LU->dmp_log = true;
}

// DME = Trigger Dump Enc
void Lagrange::Button_DME() {
  GC->LU->dmp_enc = true;
}

// DME = Trigger Dump Orb Plot
void Lagrange::Button_DMO() {
  GC->LU->dmp_orb = true;
}

// PMT = Put me There
void Lagrange::Button_PMT() {
  VESSELSTATUS vs;
  VESSEL* v = VC->v;
  LagrangeUniverse* LU = GC->LU;
  v->GetStatus(vs);
  vs.rbody = LU->body[LU->LP.ref].hObj;
  vs.rpos = LU->s4i[LU->act][0].LP.Q - LU->s4i[LU->act][0].body[LU->LP.ref].Q;
  vs.rvel = LU->s4i[LU->act][0].LP.P - LU->s4i[LU->act][0].body[LU->LP.ref].P;
  vs.vrot = _V(0.0, 0.0, 0.0);
  vs.arot = _V(0.0, 0.0, 0.0);
  v->DefSetState(&vs);
}

