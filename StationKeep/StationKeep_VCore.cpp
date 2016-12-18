// ==============================================================
//
//	StationKeep (Vessel Core Persistence)
//	==============================
//
//	Copyright (C) 2013	Andrew (ADSWNJ) Stokes
//                   All rights reserved
//
//	See StationKeep.cpp
//
// ==============================================================

#include "StationKeep_GCore.hpp"
#include "StationKeep_VCore.hpp"
#include "ParseFunctions.h"
#include <stdio.h>
#include <math.h>

StationKeep_VCore::StationKeep_VCore(VESSEL *vin, StationKeep_GCore* gcin) {
	GC = gcin;
	v = vin;
	return;
};

StationKeep_VCore::~StationKeep_VCore() {
}



void StationKeep_VCore::corePreStep(double SimT,double SimDT,double mjd) {


}
