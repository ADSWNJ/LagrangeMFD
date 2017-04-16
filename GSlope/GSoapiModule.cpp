// ==========================================================================================================
//  Glideslope 2 Orbiter Interface
//  Copyright (c) 2012-2017 Andrew (ADSWNJ) Stokes, Licensed under GNU LGPL
//
//  Designed to support flying of an accurate glide slope for the Orbiter simulator (tested for Orbiter 2010)
//
//
//	Derivitive work based on original Glideslope MFD Copyright (c) 2006, 2009 by Chris "KWAN3217" Jeppesen
//    licensed under GNU GPL
// 
//  The Orbiter 2010 simulator (c) Martin Schweiger
//  The Orbiter 2016 simulator (c) Martin Schweiger
//
//                   All rights reserved
//
// Glideslope2.cpp
// ===========================================================================================================

#define STRICT
#define ORBITER_MODULE

#include "orbitersdk.h"
#include "GlideslopeMFD.hpp"
#include "GSoapiModule.hpp"


//
// ==============================================================
// Pointers to Global variables in Glideslope
// ==============================================================
//

extern GSoapiModule *g_coreMod;
extern GlideslopeCore *GCoreData[32];
extern OBJHANDLE GCoreVessel[32];
extern int nGutsUsed;
extern int g_MFDmode;

//
// Static globals for this compilation file
//

static char *name = "Glideslope";
static char *verdetail = "v2.6";
static char *verdate = __DATE__;


//
// ==============================================================
// Orbiter DLL API interface
// ==============================================================
//

DLLCLBK void InitModule(HINSTANCE hDLL) {          // Called by Orbiter when module selected in the Launchpad
  g_coreMod = new GSoapiModule(hDLL);           // Declare a single root class instance for Glideslope for this simulation run


  MFDMODESPECEX spec;
  spec.name = name;
  spec.key = OAPI_KEY_W;                            // MFD mode selection key (change as needed)
  spec.msgproc = GSoapiModule::MsgProc;         // MFD mode callback function
  spec.context = NULL;
  g_MFDmode = oapiRegisterMFDMode (spec);           // Tell Orbiter we are an MFD
  oapiRegisterModule(g_coreMod);                    // Register this whole module with Orbiter
  char buf[128];
  sprintf(buf, "%s module init: version %s, date %s", name, verdetail, verdate);
  oapiWriteLog(buf);
  nGutsUsed=0;
}

DLLCLBK void ExitModule(HINSTANCE hDLL) {          // Called by Orbiter when module deselected in the Launchpad
  oapiUnregisterMFDMode (g_MFDmode);                // Unregister use as an MFD. Note - don't kill the g_coreMod module (Orbiter does this)
  g_coreMod = NULL;
  nGutsUsed = 0;
  char buf[128];
  sprintf(buf, "%s module exit", name);
  oapiWriteLog(buf);
}

int GSoapiModule::MsgProc(UINT msg, UINT mfd, WPARAM wparam, LPARAM lparam) {  // Message parser, handling MFD open requests
  switch (msg) {
  case OAPI_MSG_MFD_OPENED:
    return (int)(new GlideslopeMFD (LOWORD(wparam), HIWORD(wparam), (VESSEL*)lparam, mfd));    // Open an ephemeral Glideslope instance each time we make a new Glideslope MFD, plus F8, etc/ 
  }
  return 0;
}

GSoapiModule::GSoapiModule(HINSTANCE hDLL) : oapi::Module( hDLL ) {}
GSoapiModule::~GSoapiModule() {}

void GSoapiModule::clbkSimulationStart(RenderMode mode) {
  char buf[128];
  sprintf(buf, "%s module sim start", name);
  oapiWriteLog(buf);
}

void GSoapiModule::clbkSimulationEnd() {                                      // When we exit sim back to Launchpad, make sure we tidy things up properly
  for(int i=0;i<nGutsUsed;i++) {
    delete GCoreData[i];
    GCoreVessel[i] = NULL;
  }
  nGutsUsed = 0;
  char buf[128];
  sprintf(buf, "%s module sim end", name);
  oapiWriteLog(buf);
  return;
}
void GSoapiModule::clbkPreStep(double simt, double simdt, double mjd) {      // Called on each iteration of the calc engine (more often than the MFD Update
  for(int i=0;i<nGutsUsed;i++) {
    if(GCoreData[i]->isInit)GCoreData[i]->MinorCycle(simt, simdt);
  }
  return;
}

void GSoapiModule::clbkPostStep(double simt, double simdt, double mjd) {}

void GSoapiModule::clbkDeleteVessel(OBJHANDLE hVessel) {                     // Tidy up when a vessel is deleted (stops clbkPreStep calling a dead vessel)
  for(int i=0;i<nGutsUsed;i++) {
    if (GCoreVessel[i]==hVessel) {
      delete GCoreData[i];
      for (int j=i; j<nGutsUsed-1; j++) {
        GCoreVessel[j] = GCoreVessel[j+1];
        GCoreData[j] = GCoreData[j+1];
      }
      nGutsUsed--;
      break;
    }
  }
}