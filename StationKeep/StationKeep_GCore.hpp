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
#include <list>
#include <string>
#include <EnjoLib/ModuleMessagingExtBase.hpp>
#include <EnjoLib/ModuleMessagingExtPut.hpp>
#include <EnjoLib/ModuleMessagingExt.hpp>
using namespace std;

#ifndef _StationKeep_GCORE_H
#define _StationKeep_GCORE_H


//+++++
// Global Persistence core. One of these is instantiated for the whole orbiter session, on the first launch of this MFD type
//+++++

class StationKeep_GCore {
  public:
    void corePreStep(double SimT,double SimDT,double mjd);

    // Global references ... instantiation and a link to the persistence library (running the linked lists)
    StationKeep_GCore(HINSTANCE h);
    MFDPersist P;
    HINSTANCE hDLL;

  private:
    double coreSimT;
    double coreSimDT;
};


#endif // _StationKeep_GCORE_H
