// ==============================================================
//
//	Bare Bones Auto Pilot (StationKeep)
//	============================
//
//	Copyright (C) 2014	Andrew (ADSWNJ) Stokes
//                   All rights reserved
//
//	See StationKeep.cpp
//
// ==============================================================

#include "windows.h"
#include "orbitersdk.h"
#include "StationKeep_Buttons.hpp"
#include "MFDPersist.hpp"
#include "StationKeep_GCore.hpp"
#include "StationKeep_VCore.hpp"
#include <list>
#include <string>
#include <EnjoLib/ModuleMessagingExtBase.hpp>
#include <EnjoLib/ModuleMessagingExtPut.hpp>
#include <EnjoLib/ModuleMessagingExt.hpp>
using namespace std;

#ifndef _StationKeep_LCORE_H
#define _StationKeep_LCORE_H

//+++++
// Local Persistence core. One of these is instantiated per Vessel AND MFD panel location. Local defaults for that combination.
//+++++

class StationKeep_LCore {
  public:
    // Local references ... instantiation, references for vesseland mfd position, and links to the appropriate VC, MC and GC
    StationKeep_LCore(VESSEL *vin, UINT mfdin, StationKeep_GCore* gcin);
    VESSEL *v;
    UINT m;
    StationKeep_GCore* GC;
    StationKeep_VCore* VC;

    // Add local vessel+panel data here

    StationKeep_Buttons B;
    bool showMessage;
    bool okMessagePage;
    string Message;
};


#endif // _StationKeep_CORE_CLASSES
