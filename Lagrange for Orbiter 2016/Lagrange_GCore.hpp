// ==============================================================
//
//	Lagrange (Global Core Header)
//	=============================
//
//	Copyright (C) 2016	Andrew (ADSWNJ) Stokes and Keith (Keithth G) Gelling
//                   All rights reserved
//
//	See Lagrange.cpp
//
// ==============================================================

#include <list>
#include <string>
#include "windows.h"
#include "orbitersdk.h"
#include "Lagrange_Buttons.hpp"
#include "Lagrange_Universe.hpp"
#include "MFDPersist.hpp"
#include <EnjoLib/ModuleMessagingExtBase.hpp>
#include <EnjoLib/ModuleMessagingExtPut.hpp>
#include <EnjoLib/ModuleMessagingExt.hpp>
using namespace std;

#ifndef _LAGRANGE_GCORE_H
#define _LAGRANGE_GCORE_H


//+++++
// Global Persistence core. One of these is instantiated for the whole orbiter session, on the first launch of this MFD type
//+++++

class Lagrange_GCore {
  public:
    void corePreStep(double SimT,double SimDT,double mjd);

    // Global references ... instantiation and a link to the persistence library (running the linked lists)
    Lagrange_GCore();
    ~Lagrange_GCore();
    MFDPersist P;

    LagrangeUniverse *LU; 

    double next_s4i_time;

  private:
    double coreSimT;
    double coreSimDT;
};


#endif // _LAGRANGE_GCORE_H