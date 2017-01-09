// ==============================================================
//
//	Lagrange (Global Core Header)
//	=============================
//
//	Copyright (C) 2016-2017	Andrew (ADSWNJ) Stokes
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


  private:
    double coreSimT;
    double coreSimDT;
};


#endif // _LAGRANGE_GCORE_H