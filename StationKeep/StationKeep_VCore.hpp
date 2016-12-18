// ==============================================================
//
//	Station Keep MFD
//	================
//
//	Copyright (C) 2016	Andrew (ADSWNJ) Stokes
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
#include <list>
#include <string>
#include <EnjoLib/ModuleMessagingExtBase.hpp>
#include <EnjoLib/ModuleMessagingExtPut.hpp>
#include <EnjoLib/ModuleMessagingExt.hpp>
using namespace std;

#ifndef _StationKeep_VCORE_H
#define _StationKeep_VCORE_H

//+++++
// Vessel Persistence core. One of these is instantiated per Vessel flown with RV Orientation up.
//+++++

class StationKeep_VCore : public EnjoLib::ModuleMessagingExtPut{
  public:
    void corePreStep(double SimT,double SimDT,double mjd);

    // Core references ... instantiation, vessel reference and GC.
    StationKeep_VCore(VESSEL *vin, StationKeep_GCore* gcin);
    ~StationKeep_VCore();
    StationKeep_GCore* GC;

    const char* ModuleMessagingGetModuleName() const { return "StationKeep"; }

		// Add Vessel data here
    VESSEL *v;

    VECTOR3 tpos;
    VECTOR3 tvel;
  private:
};


#endif // _StationKeep_VCORE_H




