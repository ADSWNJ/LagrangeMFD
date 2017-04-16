//
// Glideslope Core Functions
//
// Purpose ... persistent class supporting the core glideslope calculations
// (The Glideslope class is ephemeral ... it gets destroyed and reinstantiated each time you change view, etc, so persist the core into this class)
//
//
// (c) Andrew Stokes (ADSWNJ) 2012-2017
//
// All rights reserved
//

#include "windows.h"
#include "orbitersdk.h"
#include "DisplayEngUnitFunctions.h"
#include "ParseFunctions.h"
#include "GSMiscFunctions.hpp"
#include "MFDButtonPage.hpp"
#include "GSmfdButtons.hpp"

#ifndef GS_OAPI_MODULE_INTERFACE
#define GS_OAPI_MODULE_INTERFACE

class GSoapiModule : public oapi::Module {
public:
  GSoapiModule(HINSTANCE hDLL);
  ~GSoapiModule();
  void clbkSimulationStart(RenderMode mode);
  void clbkSimulationEnd();
  void clbkPreStep (double simt, double simdt, double mjd);
  void clbkPostStep (double simt, double simdt, double mjd);
  void clbkDeleteVessel (OBJHANDLE hVessel);
  static int MsgProc (UINT msg, UINT mfd, WPARAM wparam, LPARAM lparam);

};
#endif // GS_OAPI_MODULE_INTERFACE