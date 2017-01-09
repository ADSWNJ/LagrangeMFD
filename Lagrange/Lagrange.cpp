// ====================================================================================================================//
//	Lagrange MFD
//	============
//
//	Copyright (C) 2016-2017	Andrew (ADSWNJ) Stokes
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

Lagrange_GCore *g_SC = nullptr;    // points to the static persistence core

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
  char *line;
  char *val;
  int p_i1, p_i2;
  unsigned int p_ui1, p_ui2;
  double p_d1, p_d2, p_d3, p_d4;
  int act = GC->LU->act;
  int mfd = this->LC->m;
  OBJHANDLE ohv = nullptr;
  VESSEL* vv = nullptr;
  Lagrange_VCore *VC_v = nullptr;
  Lagrange_LCore *LC_v = nullptr;

  while (oapiReadScenario_nextline(scn, line)) {
    val = line;
    while (*val != '\0' && *val != ' ' && *val != '\t') val++; // strip any leading whitespace
    if (*val != '\0') *val++ = '\0';                             // line now points to the tag
    while (*val == ' ' || *val == '\t') val++;                  // strip leading whitespace on the value
    if (!_stricmp(line, "END_MFD")) {
      break;
    } else if (!_stricmp(line, "LA_MFD")) {
      if (sscanf(val, "%u %d", &p_ui1, &p_i2) == 2) {
        if (this->LC->m == p_ui1) {
          this->LC->mode = p_i2;
          LC->B.SwitchPage(this, this->LC->mode);
        }
      }
    } else if (!_stricmp(line, "LP")) {
      for (unsigned int i = 0; i < COUNT_LP; i++) {
        if (!_stricmp(val, GC->LU->lptab[i].name)) {
          GC->LU->selectLP(i);
          break;
        }
      }   
    } else if (!_stricmp(line, "S4I")) {
      if (sscanf(val, "%d %u %lf %lf", &p_i1, &p_ui2, &p_d1, &p_d2) == 4) {
        GC->LU->s4i_pause = (p_i1 == 0);
        GC->LU->s4int_count[act] = p_ui2;
        GC->LU->s4int_timestep[act] = p_d1;
        GC->LU->s4int_refresh = p_d2;
      }
    } else if (!_stricmp(line, "VESSEL")) {
      ohv = oapiGetVesselByName(val);
      if (ohv) {
        vv = oapiGetVesselInterface(ohv);
        VC_v = (Lagrange_VCore*)GC->P.findVC(vv);		      // Locate our vessel core
        if (!VC_v) {
          VC_v = new Lagrange_VCore(vv, GC);		          // ... if missing, initialize it.
          GC->P.addVC(vv, VC_v);
        }

        LC_v = (Lagrange_LCore*)GC->P.findLC(vv, mfd);	  // Locate our local (vessl+MFD position) core
        if (!LC_v) {
          LC_v = new Lagrange_LCore(vv, mfd, GC);		      // ... if missing, initialize it.
          GC->P.addLC(vv, mfd, LC_v);
        }
      }
    } else if (!_stricmp(line, "PLAN")) {
      if (vv && sscanf(val, "%d %lf %lf %lf %lf", &p_i1, &p_d1, &p_d2, &p_d3, &p_d4) == 5) {
        for (unsigned int i = 0; i < GC->LU->vdata[act].size(); i++) {
          Lagrange_vdata *lvd = &(GC->LU->vdata[act][i]);
          if (lvd->v == vv) {
            lvd->burnArmed = (p_i1 == 1);
            lvd->burnMJD = p_d1;
            lvd->burndV.x = p_d2;
            lvd->burndV.y = p_d3;
            lvd->burndV.z = p_d4;
          }
        }
      }
    }
  }
  return;
}

void Lagrange::WriteStatus(FILEHANDLE scn) const {
  char buf[128];
  int act = GC->LU->act;
  sprintf(buf, "%u %d", this->LC->m, this->LC->mode);
  oapiWriteScenario_string(scn, "LA_MFD", buf);
  oapiWriteScenario_string(scn,"LP",GC->LU->LP.name);
  sprintf(buf, "%d %u %lf %lf ", GC->LU->s4i_pause ? 0 : 1, GC->LU->s4int_count[act], GC->LU->s4int_timestep[act], GC->LU->s4int_refresh);
  oapiWriteScenario_string(scn, "S4I", buf);
  for (unsigned int i = 0; i < GC->LU->vdata[act].size(); i++) {
    Lagrange_vdata *lvd = &(GC->LU->vdata[act][i]);
    oapiWriteScenario_string(scn, "VESSEL", lvd->v->GetName());
    sprintf(buf, "%d %lf %lf %lf %lf", lvd->burnArmed ? 1 : 0, lvd->burnMJD, lvd->burndV.x, lvd->burndV.y, lvd->burndV.z);
    oapiWriteScenario_string(scn, "PLAN", buf);
  }
  return;
}



