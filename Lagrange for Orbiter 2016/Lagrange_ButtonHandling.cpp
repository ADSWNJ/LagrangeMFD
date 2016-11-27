// ==============================================================
//
//	Lagrange (Button Handling Code)
//	===============================
//
//	Copyright (C) 2016	Andrew (ADSWNJ) Stokes and Keith (Keithth G) Gelling
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
// ADM = Plan Mode: Adjust Granularity Down
void Lagrange::Button_ADM() {
  VC->burnGranularity--;
  if (VC->burnGranularity < 0) VC->burnGranularity = 8;
}
// ADJ = Plan Mode: Adjust Granularity Up
void Lagrange::Button_ADJ() {
  VC->burnGranularity++;
  if (VC->burnGranularity > 8) VC->burnGranularity = 0;
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
  if (VC->burnGranularity == 8) {
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
    }
  } else {
    adj *= 10.0 / pow(10.0, (double)VC->burnGranularity);
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
    }
  }
}

// ENT = Plan Mode: Enter Value
void Lagrange::Button_ENT() {
  Lagrange_vdata *vdata = &VC->LU->vdata[VC->LU->act][VC->vix];
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
  }
}
// TGT = Orbit Mode: Select Target - e.g. Sun Earth L2
void Lagrange::Button_TGT() {
  LC->mode = 6;
  LC->B.SwitchPage(this, LC->mode);
//  sprintf_s(VC->targetText, 128, "%s %s %s", VC->cel[0].name, VC->cel[1].name, VC->LPname);
//  oapiOpenInputBox("Enter Lagrangian point target: e.g. Sun Earth L2", Lagrange_DialogFunc::clbkTGT, VC->targetText, 40, LC);
  return;
}
// FRM = Orbit Mode: Select Frame - i.e. EQU / ECL
void Lagrange::Button_FRM() {
  //TODO: Complete Function
  return Button_NotImplementedYet();
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
// BAB = Burn Mode: Auto Burn
void Lagrange::Button_BAB() {
  //TODO: Complete Function
  return Button_NotImplementedYet();
}
// BAC = Burn Mode: Auto Center
void Lagrange::Button_BAC() {
  //TODO: Complete Function
  return Button_NotImplementedYet();
}
// BAT = Burn Mode: Auto Trim
void Lagrange::Button_BAT() {
  //TODO: Complete Function
  return Button_NotImplementedYet();
}
// PAH = On LP Mode: Auto Hold
void Lagrange::Button_PAH() {
  //TODO: Complete Function
  return Button_NotImplementedYet();
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
  GC->LU->selectNextLP();
  return;
};

// PRV = Lagrange Prev Target
void Lagrange::Button_PRV() {
  GC->LU->selectPrevLP();
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