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

#ifndef LAGRANGE_INTERFACE
#define LAGRANGE_INTERFACE

#include "windows.h"
#include "orbitersdk.h"
#include "Lagrange.hpp"


class Lagrange_Interface : public oapi::Module {
public:
  Lagrange_Interface(HINSTANCE hDLL);
  ~Lagrange_Interface();
  void clbkSimulationStart(RenderMode mode);
  void clbkSimulationEnd();
  void clbkPreStep (double simt, double simdt, double mjd);
  //void clbkPostStep (double simt, double simdt, double mjd);
  void clbkDeleteVessel (OBJHANDLE hVessel);
  static int MsgProc (UINT msg, UINT mfd, WPARAM wparam, LPARAM lparam);

};
#endif // LAGRANGE_INTERFACE