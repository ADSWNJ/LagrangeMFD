// ==============================================================
//
//	Lagrange (Local Core Header)
//	============================
//
//	Copyright (C) 2016-2017	Andrew (ADSWNJ) Stokes
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
    int PrvNxtMode;                                         // = 0 for LP, 1 for frame, 2 = focus

    oapi::Sketchpad *skp; // points to local sketchpad for this MFD and vessel
    int skpLoB;           // Lowest precision for skp eng numnber formatting
    char skpBuf[128];     // Formatting buffer for MFD updates
    char skpFmtBuf[128];  // Formatting buffer for MFD updates
    int skpColPix;        // X-offset pixel (top left origin)
    int skpLinePix;       // Y offsel pixel (top left origin)

};


#endif // _LAGRANGE_CORE_CLASSES