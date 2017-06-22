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
#include "ParseFunctions.h"
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
    GC = g_SC;
    ReadColors();
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

  return;
}

Lagrange::~Lagrange ()
{
  oapiReleaseFont(font);
  //for (int i = 0; i < 12; i++) oapiReleasePen(pen[i]);
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
        if (LC->m == p_ui1) {
          LC->mode = p_i2;
          LC->PrvNxtMode = 1;
          LC->B.SelectPage(this, this->LC->mode);
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



void Lagrange::ReadColors() {
  // Read the color map first

  Lagrange_Drawing& draw = GC->draw;
  draw.Reset();
  FILE *CM;
  if (fopen_s(&CM, ".\\Config\\MFD\\Lagrange\\Colors.cfg", "r") == 0) {
    char buf[256];
    char *bp, *p, *p1, *p2, *p3;
    int r, g, b;

    while (fgets(buf, 255, CM)) {
      bp = p = buf;
      if (!ParseWhiteSpace(&bp)) continue;
      if (!ParseString(&bp, &p)) break;
      if (!strcmp(p, "COLOR")) {
        if (!ParseQuotedString(&bp, &p1)) break;
        if (!ParseInt(&bp, &r)) break;
        if (!ParseInt(&bp, &g)) break;
        if (!ParseInt(&bp, &b)) break;
        if (!draw.DefColor(p1, r, g, b)) break;
      } else if (!strcmp(p, "PLOT")) {
        if (!ParseString(&bp, &p1)) break;
        if (!ParseQuotedString(&bp, &p2)) break;
        if (!ParseString(&bp, &p3)) break;
        if (strcmp(p3, "SOLID") && strcmp(p3, "DASHED")) break;
        bool is_solid = !strcmp(p3, "SOLID");
        if (!strcmp(p1, "SUN")) {
          if (!draw.DefPlot("Sun", p2, is_solid)) break;
        } else if (!strcmp(p1, "EARTH")) {
          if (!draw.DefPlot("Earth", p2, is_solid)) break;
        } else if (!strcmp(p1, "MOON")) {
          if (!draw.DefPlot("Moon", p2, is_solid)) break;
        } else if (!strcmp(p1, "EMB")) {
          if (!draw.DefPlot("E-M B", p2, is_solid)) break;
        } else if (!strcmp(p1, "LP")) {
          if (!draw.DefPlot("LP", p2, is_solid)) break;
        } else if (!strcmp(p1, "VESLIVE")) {
          if (!draw.DefPlot("VL", p2, is_solid)) break;
        } else if (!strcmp(p1, "VESPLAN")) {
          if (!draw.DefPlot("VP", p2, is_solid)) break;
        } else break;
      } else if (!strcmp(p, "MFD")) {
        if (!ParseString(&bp, &p1)) break;
        if (!ParseQuotedString(&bp, &p2)) break;
        if (strcmp(p1, "DEF") && strcmp(p1, "HI") && strcmp(p1, "WARN")) break;
        if (!draw.DefMFDCol(p1, p2)) break;
      } else break;
    }

    fclose(CM);
  }


  if (!draw.GoodInit()) {
    char buf[128];
    sprintf(buf, "   >>> Bad data or missing Lagrange Colors.cfg: reverting to defaults\n");
    oapiWriteLog(buf);

    draw.Reset();
    draw.DefColor("White", 255, 255, 255);
    draw.DefColor("Yellow", 252, 252, 116);
    draw.DefColor("Red", 238, 32, 77);
    draw.DefColor("Magenta", 255, 29, 206);
    draw.DefColor("Bright Green", 0, 255, 0);
    draw.DefColor("Sun Yellow", 255, 255, 0);
    draw.DefColor("Earth Blue-Green", 72, 246, 238);
    draw.DefColor("Moon Grey", 190, 190, 190);
#define _SOLID true
#define _DASHED false
    draw.DefPlot("Sun", "Sun Yellow", _SOLID);
    draw.DefPlot("Earth", "Earth Blue-Green", _SOLID);
    draw.DefPlot("Moon", "Moon Grey", _SOLID);
    draw.DefPlot("E-M B", "Earth Blue-Green", _DASHED);
    draw.DefPlot("LP", "Magenta", _DASHED);
    draw.DefPlot("VL", "Bright Green", _SOLID);
    draw.DefPlot("VP", "Magenta", _DASHED);
    draw.DefMFDCol("DEF", "White");
    draw.DefMFDCol("HI", "Yellow");
    draw.DefMFDCol("WARN", "Red");
  } else {
    char buf[128];
    sprintf(buf, "   >>> Lagrange colors successfully loaded from Colors.cfg!!\n");
    oapiWriteLog(buf);
  }

}
