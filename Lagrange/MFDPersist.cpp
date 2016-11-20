// ==============================================================
//
//	MFDPersist
//	==========
//
//	Copyright (C) 2016	Andrew (ADSWNJ) Stokes
//                   All rights reserved
//
//	Description:
//
//  MFDPersist establishes a hierarchy of linked class instances that hold state across
//  the destruction events that happen all the time to your MFD in Orbiter due to 
//  switching cockpit views with F8, or resizing external MFD's, or switching vessels.
//
//  The principle of this support library is that you want to have a Global Core location
//  for all data you want to persist across any of your MFD's in any vessel. You want to
//  have a Vessel Core location for any data specific to this vessel (e.g. so you can switch
//  between vessels quite happily and come back to data that makes sense for this vessel, or
//  you want to bring up the same MFD on the other MFD position or on an extern and have it
//  remember what you selected.) You want a position-specific location for when you want defaults
//  for say left MFD instead of right MFD. And you want an MFD-Vessel specific location for
//  anything you are doing locally in one MFD that you don't want stomping on the other MFD.
//  (E.g. controlling the menu page selection or an error message, or tracking data.)
//
//	Usage:
//	Include this file and the .hpp header into your code. You should not need to change anything in here. 
//
//	Copyright Notice:
//
//	This program is free software: you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	For full licencing terms, pleaserefer to the GNU General Public License
//	(gpl-3_0.txt) distributed with this release, or see
//	http://www.gnu.org/licenses/.
//
//
//	Credits:
//
//	Orbiter Simulator	(c) 2003-2013 Martin (Martins) Schweiger - Thanks for providing us this sim
//
//	Release History:
//
//	V1.00	Initial Release
//
// ==============================================================

#include "MFDPersist.hpp"

MFDPersist::MFDPersist() {
  return;
}

void* MFDPersist::findVC(VESSEL* vin) {
  return find(VCL, vin, 0);
}

void* MFDPersist::findLC(VESSEL* vin, UINT mfd) {
  return find(LCL, vin, mfd);
}

void* MFDPersist::find(list<MFDPersistLookup*> &L, VESSEL* vin, UINT mfd) {
  list<MFDPersistLookup*>::iterator iv;
  for (iv = L.begin(); iv != L.end(); iv++) {
    if (((*iv)->v == vin) && ((*iv)->mfd == mfd)) break;
  };
  if (iv == L.end()) {
    return 0;
  };
  return  (*iv)->core;
}

void MFDPersist::addVC(VESSEL* vin, void *VC) {
  add(VCL, vin, 0, VC);
}

void MFDPersist::addLC(VESSEL* vin, UINT mfd, void *LC) {
  add(LCL, vin, mfd, LC);
}

void MFDPersist::add(list<MFDPersistLookup*> &L, VESSEL* vin, UINT mfd, void *C) {
  MFDPersistLookup *e;
  e = new MFDPersistLookup;
  e->v = vin;
  e->mfd = mfd;
  e->core = C;
  L.push_front(e);
  return;
}

void* MFDPersist::firstVC() {
  iterVC = VCL.begin(); 
  return (iterVC == VCL.end()) ? NULL : (*iterVC)->core;
}

void* MFDPersist::nextVC() {
  iterVC++;
  return (iterVC == VCL.end()) ? NULL : (*iterVC)->core;
}

void MFDPersist::delVC(VESSEL* vin) {
  return del(VCL, vin);
}

void MFDPersist::delLC(VESSEL* vin) {
  return del(LCL, vin);
}

void MFDPersist::del(list<MFDPersistLookup*> &L, VESSEL* vin) {
  list<MFDPersistLookup*>::iterator iv = L.begin();
  while (iv != L.end()) {
    if ((*iv)->v == vin || vin == nullptr) {
       delete (*iv)->core;
       L.erase(iv++);
    } else {
      ++iv;
    }
  };
}