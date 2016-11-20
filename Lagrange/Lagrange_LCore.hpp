// ==============================================================
//
//	Lagrange (Local Core Header)
//	============================
//
//	Copyright (C) 2016	Andrew (ADSWNJ) Stokes and Keith (Keithth G) Gelling
//                   All rights reserved
//
//	See Lagrange.cpp
//
// ==============================================================

#include "windows.h"
#include "orbitersdk.h"
#include "Lagrange_Buttons.hpp"
#include "MFDPersist.hpp"
#include "Lagrange_GCore.hpp"
#include "Lagrange_VCore.hpp"
#include <list>
#include <string>
#include <EnjoLib/ModuleMessagingExtBase.hpp>
#include <EnjoLib/ModuleMessagingExtPut.hpp>
#include <EnjoLib/ModuleMessagingExt.hpp>
using namespace std;

#ifndef _LAGRANGE_LCORE_H
#define _LAGRANGE_LCORE_H

//+++++
// Local Persistence core. One of these is instantiated per Vessel AND MFD panel location. Local defaults for that combination.
//+++++

class Lagrange_LCore {
  public:
    // Local references ... instantiation, references for vesseland mfd position, and links to the appropriate VC, MC and GC
    Lagrange_LCore(VESSEL *vin, UINT mfdin, Lagrange_GCore* gcin);
    VESSEL *v;
    UINT m;
    Lagrange_GCore* GC;
    Lagrange_VCore* VC;

    // Add local vessel+panel data here

    Lagrange_Buttons B;
    bool showMessage;
    bool okMessagePage;
    string Message;
    int mode;
};


#endif // _LAGRANGE_CORE_CLASSES