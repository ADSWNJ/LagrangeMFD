// ==============================================================
//
//	Lagrange (Vessel Core Header)
//	=============================
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
#include "Lagrange_Universe.hpp"
#include "Lagrange_AP.hpp"
#include "ExportStruct.hpp"
#include <list>
#include <string>
#include <vector>
#include <EnjoLib/ModuleMessagingExtBase.hpp>
#include <EnjoLib/ModuleMessagingExtPut.hpp>
#include <EnjoLib/ModuleMessagingExt.hpp>
#include <Orbiter/AutopilotRotation.hpp>
using namespace std;

#ifndef _LAGRANGE_VCORE_H
#define _LAGRANGE_VCORE_H

//+++++
// Vessel Persistence core. One of these is instantiated per Vessel flown with RV Orientation up.
//+++++

class Lagrange_VCore : public EnjoLib::ModuleMessagingExtPut{
  public:
    void corePreStep(double SimT,double SimDT,double mjd);

    // Core references ... instantiation, vessel reference and GC.
    Lagrange_VCore(VESSEL *vin, Lagrange_GCore* gcin);
    ~Lagrange_VCore();
    Lagrange_GCore* GC;
    LagrangeUniverse *LU;

    const char* ModuleMessagingGetModuleName() const { return "Lagrange"; }

		// Add Vessel data here
    VESSEL *v;
    int vix; // Vessel index in LU

    int burnGranularity[5];
    int burnVar;
    bool burnTdV_lock;
    class Lagrange_AP ap;

    int apMode;     //  0 = inactive, 1 = Plan mode, 2 = station keep mode
    int apState;    //  0 = inactive, 1 = Ready (armed), 2 = Point, 3 = Burn
    bool apHoldInRange; 
    bool burnFrozen;
    double burnStart, burnEnd, burnDurn;
    double burnCV, burnTV, burnDV; 
    double ap_ahStartMassV;


  private:
};


#endif // _LAGRANGE_VCORE_H




