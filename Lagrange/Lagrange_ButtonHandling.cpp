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
    vdata->burnMJD = oapiGetSimMJD() + (15.0 / (60.0 * 24.0)); // bump initial MJD 15 mins in the future
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
  VC->burnVar = 2;
}
// OUT = Plan Mode: Outward Var
void Lagrange::Button_OUT() {
  VC->burnVar = 3;
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
      ratio = (TdV + adj) / TdV;
      vdata->burndV *= ratio;
    }
  }
  if (VC->burnTdV_lock) {
    switch (VC->burnVar) {
    case 0:
      break;
    case 1:
      if (vdata->burndV.x < TdV) {
        ratio = sqrt(TdV * TdV - vdata->burndV.x * vdata->burndV.x) / sqrt(vdata->burndV.y * vdata->burndV.y + vdata->burndV.z * vdata->burndV.z);
        vdata->burndV.y *= ratio;
        vdata->burndV.z *= ratio;
      } else {
        vdata->burndV.y = 0.0;
        vdata->burndV.z = 0.0;
      }
      break;
    case 2:
      if (vdata->burndV.y < TdV) {
        ratio = sqrt(TdV * TdV - vdata->burndV.y * vdata->burndV.y) / sqrt(vdata->burndV.x * vdata->burndV.x + vdata->burndV.z * vdata->burndV.z);
        vdata->burndV.x *= ratio;
        vdata->burndV.z *= ratio;
      } else {
        vdata->burndV.x = 0.0;
        vdata->burndV.z = 0.0;
      }
      break;
    case 3:
      if (vdata->burndV.z < TdV) {
        ratio = sqrt(TdV * TdV - vdata->burndV.z * vdata->burndV.z) / sqrt(vdata->burndV.x * vdata->burndV.x + vdata->burndV.y * vdata->burndV.y);
        vdata->burndV.x *= ratio;
        vdata->burndV.y *= ratio;
      } else {
        vdata->burndV.x = 0.0;
        vdata->burndV.y = 0.0;
      }
      break;
    case 4:
      break;
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
    oapiOpenInputBox("Enter Plan Burn MJD", Lagrange_DialogFunc::clbkENT, GC->LU->buf, 20, LC);
    break;
  case 1:
    sprintf(GC->LU->buf, "%.6f", vdata->burndV.x);
    oapiOpenInputBox("Enter Plan Prograde Delta-V", Lagrange_DialogFunc::clbkENT, GC->LU->buf, 20, LC);
    break;
  case 2:
    sprintf(GC->LU->buf, "%.6f", vdata->burndV.y);
    oapiOpenInputBox("Enter Plan Plane Change Delta-V", Lagrange_DialogFunc::clbkENT, GC->LU->buf, 20, LC);
    break;
  case 3:
    sprintf(GC->LU->buf, "%.6f", vdata->burndV.z);
    oapiOpenInputBox("Enter Plan Outward Delta-V", Lagrange_DialogFunc::clbkENT, GC->LU->buf, 20, LC);
    break;
  case 4:
    sprintf(GC->LU->buf, "%.6f", TdV);
    oapiOpenInputBox("Enter Plan Total Delta-V", Lagrange_DialogFunc::clbkENT, GC->LU->buf, 20, LC);
    break;
  }
}
// TGT = Orbit Mode: Select Target - e.g. Sun Earth L2
void Lagrange::Button_TGT() {
  LC->mode = 6;
  LC->B.SwitchPage(this, LC->mode);
  return;
}
// FRM = Orbit Mode: Select Frame - i.e. EQU / ECL
void Lagrange::Button_FRM() {
  LC->mode = 7;
  LC->B.SwitchPage(this, LC->mode);
  return;
}
// PRJ = Orbit Mode: Select Projection
void Lagrange::Button_PRJ() {
  //TODO: Complete Function
  return Button_NotImplementedYet();
}
// ZMU = Orbit Mode: Zoom Up
void Lagrange::Button_ZMU() {
  //TODO: Complete Function
  return Button_NotImplementedYet();
}
// ZMD = Orbit Mode: Zoom Down
void Lagrange::Button_ZMD() {
  //TODO: Complete Function
  return Button_NotImplementedYet();
}
// MUP = Orbit Mode: Move Up
void Lagrange::Button_MUP() {
  //TODO: Complete Function
  return Button_NotImplementedYet();
}
// MDN = Orbit Mode: Move Down
void Lagrange::Button_MDN() {
  //TODO: Complete Function
  return Button_NotImplementedYet();
}
// MLF = Orbit Mode: Move Left
void Lagrange::Button_MLF() {
  //TODO: Complete Function
  return Button_NotImplementedYet();
}
// MRG = Orbit Mode: Move Right
void Lagrange::Button_MRG() {
  //TODO: Complete Function
  return Button_NotImplementedYet();
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

// NXT = Lagrange Next Target
void Lagrange::Button_NXT() {
  if (LC->mode == 6) {
    GC->LU->selectNextLP();
  } else {
    int *prefEnt = &GC->LU->vdata[GC->LU->act][VC->vix].refEnt;
    (*prefEnt)++;
    if (*prefEnt == COUNT_BODY) *prefEnt = 0;
  }
  return;
};

// PRV = Lagrange Prev Target
void Lagrange::Button_PRV() {
  if (LC->mode == 6) {
    GC->LU->selectPrevLP();
  } else {
    int *prefEnt = &GC->LU->vdata[GC->LU->act][VC->vix].refEnt;
    (*prefEnt)--;
    if (*prefEnt < 0) *prefEnt = COUNT_BODY - 1;
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

// TSP = Adjust Iteration TimeStep
void Lagrange::Button_TSP() {
  sprintf(GC->LU->buf, "%.1f", GC->LU->s4int_timestep[GC->LU->act]);
  oapiOpenInputBox("Enter Iteration Timestep (e.g. 30.0)", Lagrange_DialogFunc::clbkTSP, GC->LU->buf, 20, LC);
  return;
}


// WT = Adjust S4I Wait Time
void Lagrange::Button_WT() {
  sprintf(GC->LU->buf, "%.1f", GC->LU->s4int_refresh);
  oapiOpenInputBox("Enter S4I wait time (e.g. 0.0 or 10.0)", Lagrange_DialogFunc::clbkWT, GC->LU->buf, 20, LC);
  return;
}

// DML = Trigger Dump Log
void Lagrange::Button_DML() {
  GC->LU->dmp_log = true;
}

// DML = Trigger Dump Enc
void Lagrange::Button_DME() {
  GC->LU->dmp_enc = true;
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

