// ==============================================================
//
//	Lagrange (Orbiter Interface)
//	============================
//
//	Copyright (C) 2016-2017	Andrew (ADSWNJ) Stokes
//                   All rights reserved
//
//	See Lagrange.cpp
//
//

#include "Lagrange_Interface.hpp"

// ====================================================================================================================
// Global variables

Lagrange_Interface *g_OrbIF = nullptr;
extern Lagrange_GCore *g_SC;    // points to the static persistence core

static int g_MFDmode;			// holds the mode identifier for our MFD
static char *name = "Lagrange";
static char *version = "1.0";
static char *compdate = __DATE__;

// ====================================================================================================================
// Orbiter API interface 

/*
 * InitModule called when this module is selected in the launchpad
 */
DLLCLBK void InitModule (HINSTANCE hDLL)
{
  g_OrbIF = new Lagrange_Interface(hDLL);
  g_SC = nullptr;
  MFDMODESPECEX spec;
  spec.name = name;
  spec.key = OAPI_KEY_L;
  spec.context = NULL;
  spec.msgproc = Lagrange_Interface::MsgProc;
  g_MFDmode = oapiRegisterMFDMode (spec);
  oapiRegisterModule(g_OrbIF);
  char buf[128];
  sprintf(buf, "   >>> %s module initialized: version %s, compile date %s", name, version, compdate);
  oapiWriteLog(buf);
}

/*
 * ExitModule called when this module is deselected in the launchpad
 */
DLLCLBK void ExitModule (HINSTANCE hDLL)
{
  char buf[128];
  sprintf(buf, "   >>> %s module exited", name);
  oapiWriteLog(buf);
  oapiUnregisterMFDMode (g_MFDmode);
}

/*
 * MsgProc called when Orbiter tells us something has happened (e.g. opening an MFD)
 */
int Lagrange_Interface::MsgProc (UINT msg, UINT mfd, WPARAM wparam, LPARAM lparam)
{
  switch (msg) {
  case OAPI_MSG_MFD_OPENED:
    return (int)(new Lagrange (LOWORD(wparam), HIWORD(wparam), (VESSEL*)lparam, mfd));
  }
  return 0;
}

Lagrange_Interface::Lagrange_Interface(HINSTANCE hDLL) : oapi::Module( hDLL ) {}
Lagrange_Interface::~Lagrange_Interface() {}

/*
 * clbkSimulationStart called when Orbiter launches
 */
void Lagrange_Interface::clbkSimulationStart(RenderMode mode) {
  char buf[128];
  sprintf(buf, "   >>> %s module sim start", name);
  oapiWriteLog(buf);
}

/*
 * clbkSimulationStart called when Orbiter returns to the launchpad or exits to desktop
 */
void Lagrange_Interface::clbkSimulationEnd() {                                      // When we exit sim back to Launchpad, make sure we tidy things up properly
  if (g_SC != nullptr) {
    g_SC->P.delLC(nullptr);
    g_SC->P.delVC(nullptr);
    g_SC->LU->vdata[g_SC->LU->act].resize(0);
    g_SC->LU->next_s4i_time = -100.0;
  }
  char buf[128];
  sprintf(buf, "   >>> %s module sim end", name);
  oapiWriteLog(buf);
  //g_SC->next_s4i_time = -100.0; // Reset the main trigger in case we restart
  return;
}

/*
 * clbkPreStep called before each calculation step
 */
void Lagrange_Interface::clbkPreStep(double simt, double simdt, double mjd) {      // Called on each iteration of the calc engine (more often than the MFD Update
  if (g_SC) g_SC->corePreStep(simt, simdt, mjd);
  return;
}

/*
 * clbkPostStep called after each calculation step
 */
//void Lagrange_Interface::clbkPostStep(double simt, double simdt, double mjd) {}

/*
 * clbkDeleteVessel called when a vessel is deleted
 */
void Lagrange_Interface::clbkDeleteVessel(OBJHANDLE hVessel) {                     // Tidy up when a vessel is deleted (stops clbkPreStep calling a dead vessel)
  VESSEL *vin = oapiGetVesselInterface(hVessel);
  if (g_SC) {
    g_SC->P.delLC(vin);
    g_SC->P.delVC(vin);
    vector<Lagrange_vdata> *lvdv = &g_SC->LU->vdata[g_SC->LU->act];
    for (auto it = lvdv->begin(); it < lvdv->end(); it++) {
      if (it->v == vin) {
        lvdv->erase(it);
        break;
      }
    }
  }
}