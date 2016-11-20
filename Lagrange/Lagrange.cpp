// ====================================================================================================================//
//	Lagrange MFD
//	============
//
//	Copyright (C) 2016	Andrew (ADSWNJ) Stokes and Keith (Keithth G) Gelling
//                   All rights reserved
//
//	Description:
//
//	This is a trajectory control MFD tool for Orbiter to calculate Lagrangian Points and to execute burns
//  to reach and hold station at thes points. See http://www.orbiter-forum.com/showthread.php?t=37268
//  for the original discussion on this topic.  
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
//	Orbiter Simulator	(c) 2003-2016 Martin (Martins) Schweiger
// 	MFDButtonPage		(c) 2012-2016 Szymon (Enjo) Ender
// 	original ModuleMessaging (c) 2014-2016 Szymon (Enjo) Ender
//	
//
//	Release History:
//
//  V1.00	Initial Release
// ====================================================================================================================

#define STRICT
#define ORBITER_MODULE

#include "windows.h"
#include "orbitersdk.h"
#include "Lagrange.hpp"
#include "Lagrange_GCore.hpp"
#include "Lagrange_VCore.hpp"
#include "Lagrange_LCore.hpp"
#include "Lagrange_DialogFunc.hpp"
#include "MFDPersist.hpp"

// ====================================================================================================================
// Global variables

Lagrange_GCore *g_SC;    // points to the static persistence core

// ====================================================================================================================
// MFD class implementation

// Constructor executes on any F8, any resize of an ExtMFD, or any vessel switch
Lagrange::Lagrange (DWORD w, DWORD h, VESSEL *vessel, UINT mfd)
: MFD2 (w, h, vessel)
{
  if (g_SC == nullptr) {
    g_SC = new Lagrange_GCore();
  }
  GC = g_SC;
  if (GC->LU == nullptr) {
    GC->LU = new LagrangeUniverse;
  }

  VC = (Lagrange_VCore*) GC->P.findVC(vessel);		  // Locate our vessel core
  if (!VC) {
    VC = new Lagrange_VCore(vessel, GC);				    // ... if missing, initialize it.
    GC->P.addVC(vessel, VC);
  }

  LC = (Lagrange_LCore*) GC->P.findLC(vessel, mfd);	// Locate our local (vessl+MFD position) core
  if (!LC) {
    LC = new Lagrange_LCore(vessel, mfd, GC);			  // ... if missing, initialize it.
    GC->P.addLC(vessel, mfd, LC);
  }

  // Any construction for the display side of this MFD instance
  font = oapiCreateFont (h/25, true, "Fixed", FONT_NORMAL, 0);

  // Orbiter: BGR colors, not RGB
  pen[0] = oapiCreatePen(1, 1, 0xFFFFFF); // WHITE
  pen[1] = oapiCreatePen(1, 1, 0x74FCFD); // YELLOW
  pen[2] = oapiCreatePen(1, 1, 0x4382FF); // ORANGE
  pen[3] = oapiCreatePen(1, 1, 0x4D20EE); // RED
  pen[4] = oapiCreatePen(1, 1, 0xCE1DFF); // MAGENTA
  pen[5] = oapiCreatePen(2, 1, 0xE2DB78); // DASHED AQUA
  pen[6] = oapiCreatePen(2, 1, 0xFE751F); // DASHED BLUE
  pen[7] = oapiCreatePen(2, 1, 0xD97398); // DASHED VIOLET
  pen[8] = oapiCreatePen(1, 1, 0x8FB03B); // LIGHT GREEN
  pen[9] = oapiCreatePen(2, 1, 0x14F91D); // DASHED GREEN
  pen[10] = oapiCreatePen(1, 1, 0x00FF00); // BRIGHT GREEN
  pen[11] = oapiCreatePen(1, 1, 0x00FFFF); // BRIGHT YELLOW

  return;
}

Lagrange::~Lagrange ()
{
  oapiReleaseFont(font);
  for (int i = 0; i < 12; i++) oapiReleasePen(pen[i]);
  return;
}





// ====================================================================================================================
// Save/load from .scn functions
void Lagrange::ReadStatus(FILEHANDLE scn) {
  return;
}

void Lagrange::WriteStatus(FILEHANDLE scn) const {
  return;
}



