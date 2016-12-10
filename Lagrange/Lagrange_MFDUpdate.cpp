// ==============================================================
//
//	Lagrange (MFD Update)
//	=====================
//
//	Copyright (C) 2016	Andrew (ADSWNJ) Stokes and Keith (Keithth G) Gelling
//                   All rights reserved
//
//	See Lagrange.cpp
//
// ==============================================================

#include "Lagrange.hpp"
#include "Lagrange_Universe.hpp"
#include "DisplayEngUnitFunctions.h"
#include <math.h>
#include <stdarg.h>

#define CLR_YELLOW 0x00FFFF
#define CLR_WHITE 0xFFFFFF

bool Lagrange::Update(oapi::Sketchpad *skp)
{
  LC->skp = skp;
  if (GC->LU == nullptr) return true;
  if (LC->showMessage) return DisplayMessageMode();
  switch (LC->mode) {
  case 0:
    return DisplayOrbitMode(); break;
  case 1:
    return DisplayPlanMode(); break;
  case 2:
    return DisplayBurnMode(); break;
  case 3:
    return DisplayLPMode(); break;
  case 4:
    return DisplayS4IMode(); break;
  case 5:
    return DisplayFrmMode(); break;
  case 6:
    return DisplayTgtMode(); break;
  }
  return true;
};

bool Lagrange::DisplayOrbitMode() {
  skpTitle("Lagrange: ORBIT");

  int l = 3;
  int circrad = (int)(W / 120);
  int enc_x, enc_y;

  LagrangeUniverse* LU = GC->LU;
  LagrangeUniverse::LagrangeUniverse_LP* LP = &LU->LP;
  Lagrange_vdata *lvd = &LU->vdata[GC->LU->act][VC->vix];
  Lagrange_orb_disp *lod = &LU->l_orb[GC->LU->act];

  skpFormatText(0, l, "LP: %s", LP->name);
  skpFormatText(4, l++, "FRM: %s", LU->body[lvd->refEnt].name);

//  for (int i = 0; i < 10; i++) {
//    skp->MoveTo(W * (i+1) / 12, H * 1/4);
//    skp->SetPen(pen[i]);
//    skp->LineTo(W * (i+1) / 12, H * 3/4);
//  }

  if (!GC->LU->s4i_valid) return true;
  if (lvd->orb_plot.size() != ORB_PLOT_COUNT) return true;

  oapi::IVECTOR2 iv[ORB_PLOT_COUNT-1];
  for (int i = 1; i < ORB_PLOT_COUNT; i++) {
    iv[i-1].x = (long) ((double) W * lvd->orb_plot[i].x);
    iv[i-1].y = (long) ((double) H * lvd->orb_plot[i].y);
  }

  LC->skp->SetPen(pen[ORB_PEN_BRIGHT_GREEN]);
  LC->skp->MoveTo((long)((double)W * lvd->orb_plot[1].x), (long)((double)H * lvd->orb_plot[1].y));
  LC->skp->Polyline(iv, ORB_PLOT_COUNT-1);

  if (lvd->enc_ix >= 0) {
    enc_x = (int)((double)W * lvd->orb_plot_ves_enc.x);
    enc_y = (int)((double)H * lvd->orb_plot_ves_enc.y);
    LC->skp->Ellipse(enc_x - circrad, enc_y - circrad, enc_x + circrad, enc_y + circrad);
  }

  for (int s = 1; s < ORB_MAX_LINES; s++) {
    if (LP->plotix[s] == -1) break;
    for (int i = 1; i < ORB_PLOT_COUNT; i++) {
      iv[i-1].x = (long)((double)W * lod->orb_plot[s][i].x);
      iv[i-1].y = (long)((double)H * lod->orb_plot[s][i].y);
    }
    LC->skp->SetPen(pen[LP->plotixpen[s]]);
    LC->skp->MoveTo((long)((double)W * lod->orb_plot[s][1].x), (long)((double)H * lod->orb_plot[s][1].y));
    LC->skp->Polyline(iv, ORB_PLOT_COUNT-1);

    if (lvd->enc_ix >= 0) {
      if (abs(lvd->orb_plot_body_enc[s].x - lvd->orb_plot_origin.x) > 0.02 ||
        abs(lvd->orb_plot_body_enc[s].y - lvd->orb_plot_origin.y) > 0.02) {
        enc_x = (int)((double)W * lvd->orb_plot_body_enc[s].x);
        enc_y = (int)((double)H * lvd->orb_plot_body_enc[s].y);
        LC->skp->Ellipse(enc_x - circrad, enc_y - circrad, enc_x + circrad, enc_y + circrad);
      }
    }     

  }
  return true;
};

bool Lagrange::DisplayPlanMode() {
  skpTitle("Lagrange: PLAN");

  int l = 4;
  char *adjText[9] = { "Rough", "Coarse", "Medium", "Fine", "Super", "Ultra", "Hyper", "Micro", "Reset" };

  Lagrange_vdata *vdata = &VC->LU->vdata[VC->LU->act][VC->vix];

  if (vdata->burnArmed) skpFmtColText(0, l, true, CLR_YELLOW, CLR_WHITE, ">");
  skpFmtColText(0, l++, vdata->burnArmed, CLR_YELLOW, CLR_WHITE, "  Planning Mode:  %s", vdata->burnArmed ? "Active" : "Inactive");
  skpFormatText(0, l++, "  Adjustment:     %s", adjText[VC->burnGranularity]);
  l++;

  if (vdata->burnMJD == 0.0) vdata->burnMJD = oapiGetSimMJD() + (1 / (10 * 60 * 24));
  skpFmtColText(0, l + VC->burnVar, true,   CLR_YELLOW, CLR_WHITE, ">");
  skpFmtColText(0, l++, (VC->burnVar == 0), CLR_YELLOW, CLR_WHITE, "  Plan MJD:     %14.6f", vdata->burnMJD);
  skpFmtColText(0, l++, (VC->burnVar == 1), CLR_YELLOW, CLR_WHITE, "  Prograde dV:  %14.6f", vdata->burndV.x);
  skpFmtColText(0, l++, (VC->burnVar == 2), CLR_YELLOW, CLR_WHITE, "  Plane Chg dV: %14.6f", vdata->burndV.y);
  skpFmtColText(0, l++, (VC->burnVar == 3), CLR_YELLOW, CLR_WHITE, "  Outward dV:   %14.6f", vdata->burndV.z);
  skpFmtColText(0, l,   (VC->burnVar == 4), CLR_YELLOW, CLR_WHITE, "  Total dV:     %14.6f", length(vdata->burndV));
  if (VC->burnTdV_lock) skpFormatText(5,l, "(lock)");
  l+=2;
  if (!GC->LU->s4i_valid || vdata->enc_ix < 0) return true;

  Lagrange_s4i *s4i_e = &GC->LU->s4i[GC->LU->act][vdata->enc_ix];
  skpFormatText(0, l++, "  Enc. MJD:     %14.6f", s4i_e->MJD);
  skpFmtEngText(0, l++, "  Enc. Time:    %10.2f", "s", s4i_e->sec - oapiGetSimTime());
  return true;
};

bool Lagrange::DisplayBurnMode() {
  skpTitle("Lagrange: BURN");
  int l = 4;
  skpColor(CLR_YELLOW);
  skpFormatText(1, l++, "TODO: Burn Mode Info");
  return true;
};

bool Lagrange::DisplayLPMode() {
  skpTitle("Lagrange: LP");

  int l = 4;
  Lagrange_vdata *vdata = &GC->LU->vdata[GC->LU->act][VC->vix];
  Lagrange_ves_s4i *vs4i = &vdata->vs4i[0];
  QP_struct *ves = &vs4i->ves;
  QP_struct *vesLP = &vs4i->vesLP;
  Lagrange_ves_s4i *vs4i_e = (vdata->enc_ix >= 0) ? &vdata->vs4i[vdata->enc_ix] : nullptr;
  QP_struct *ves_e = &vs4i_e->ves;
  QP_struct *vesLP_e = &vs4i_e->vesLP;
  Lagrange_s4i *s4i_e = (vdata->enc_ix >= 0) ? &GC->LU->s4i[GC->LU->act][vdata->enc_ix] : nullptr;
  LC->skpLoB = 1;

  skpFormatText(0, l++, "LP: %s", GC->LU->LP.name);
  l++;
  int rl = l;

  skpFormatText(2, l++, "Rel. Pos.");
  skpFmtEngText(0, l++, "Prograde:  %8.3f", "m", vesLP->Q.x);
  skpFmtEngText(0, l++, "Plane:     %8.3f", "m", vesLP->Q.y);
  skpFmtEngText(0, l++, "Outward:   %8.3f", "m", vesLP->Q.z);
  skpFmtEngText(0, l++, "TOTAL:     %8.3f", "m", vs4i->dQ);

  l++;
  LC->skpLoB = 0;
  skpFormatText(2, l++, "Rel. Vel.");
  skpFmtEngText(0, l++, "Prograde:  %8.3f", "m/s", vesLP->P.x);
  skpFmtEngText(0, l++, "Plane:     %8.3f", "m/s", vesLP->P.y);
  skpFmtEngText(0, l++, "Outward:   %8.3f", "m/s", vesLP->P.z);
  skpFmtEngText(0, l++, "TOTAL:     %8.3f", "m/s", vs4i->dP);

  l++;
  if (GC->LU->s4i_valid && vdata->enc_ix >= 0) {
    LC->skpLoB = 1;
    skpFormatText(4, rl++, "Enc. Pos.");
    skpFmtEngText(4, rl++, "%8.3f", "m", vesLP_e->Q.x);
    skpFmtEngText(4, rl++, "%8.3f", "m", vesLP_e->Q.y);
    skpFmtEngText(4, rl++, "%8.3f", "m", vesLP_e->Q.z);
    skpFmtEngText(4, rl++, "%8.3f", "m", vs4i_e->dQ);

    rl++;
    LC->skpLoB = 0;
    skpFormatText(4, rl++, "Enc. Vel.");
    skpFmtEngText(4, rl++, "%8.3f", "m", vesLP_e->P.x);
    skpFmtEngText(4, rl++, "%8.3f", "m", vesLP_e->P.y);
    skpFmtEngText(4, rl++, "%8.3f", "m", vesLP_e->P.z);
    skpFmtEngText(4, rl++, "%8.3f", "m", vs4i_e->dP);

    rl++;
    skpFormatText(0, rl++, "Enc. MJD:");
    skpFormatText(0, rl++, "Enc. Time:");
    skpFormatText(0, rl++, "Enc. dTime:");
    rl -= 3;
    skpFormatText(4, rl++, "%10.4f", s4i_e->MJD);
    skpFmtEngText(4, rl++, "%10.4f", "s", s4i_e->sec);
    skpFmtEngText(4, rl++, "%10.4f", "s", s4i_e->sec - oapiGetSimTime());
  }
  return true;
};

bool Lagrange::DisplayS4IMode() {
  skpTitle("Lagrange: S4I");
  int l = 4;

  switch (GC->LU->threadWorkerState()) {
  case 'A':
  case 'a':
  case 'B':
  case 'b':
    skpFormatText(0,l++, "S4I State            Active");
    break;
  case 'P':
    skpFormatText(0, l++, "S4I State            Paused");
    break;
  case 'K':
    skpFormatText(0, l++, "S4I State            Killed");
    break;
  default:
    skpFormatText(0, l++, "S4I State");
  }

  l++;
  char *DiagText[7] = { "S4I Run    ",
                        "MJD From   ",
                        "MJD To     ",
                        "S4I deltaT ",
                        "Calc time  ",
                        "Orb Plot # ",
                        "S4I Iter # "};

  for (int i = 0; i < 5; i++) {
    skpFormatText(0, l++, "%s %15.3f", DiagText[i], GC->LU->dbg[GC->LU->act][i]);
  }
  for (int i = 5; i < 7; i++) {
    skpFormatText(0, l++, "%s %15.0f", DiagText[i], GC->LU->dbg[GC->LU->act][i]);
  }
  return true;
};

bool Lagrange::DisplayFrmMode() {
  skpTitle("Lagrange: FRM");
  int l = 4;
  int curRef = GC->LU->vdata[GC->LU->act][VC->vix].refEnt;
  skpColor(CLR_YELLOW);
  skpFormatText(0, l + curRef, ">");
  for (int i = 0; i<COUNT_BODY; i++) {
    skpFmtColText(0, l++, (i == curRef), CLR_YELLOW, CLR_WHITE, "  %s", GC->LU->body[i].name);
  }
  return true;
};

bool Lagrange::DisplayTgtMode() {
  skpTitle("Lagrange: TGT");
  int l = 4;
  int curLP = GC->LU->getLP();
  skpColor(CLR_YELLOW);
  skpFormatText(0, l + curLP, ">");
  for (int i = 0; i<COUNT_LP; i++) {
    skpFmtColText(0, l++, (i == curLP), CLR_YELLOW, CLR_WHITE, "  %s", GC->LU->lptab[i].name);
  }
  return true;
};


bool Lagrange::DisplayMessageMode() {
  skpTitle("Lagrange MFD");
  showMessage();
  return true;
};


// MFD Line formatting helper
void Lagrange::showMessage() {

  char localMsg[750];
  strcpy_s(localMsg,750, LC->Message.c_str());
  char *bp = localMsg;
  char *bp2 = localMsg;
  char *bp3 = localMsg;
  char c1, c2;
  int i = 0;
  int j;
  int l = 4;
  bool eol = false;

  do {
    if ((*bp2 == '\n') || (*bp2 == '\0')) {     // Look for user newline or end of buffer
      eol = true;
      c1 = *bp2;
      *bp2 = '\0';
    } else {
      if (i==34) {                              // 34 chars no newline ... need to break the line
        eol=true;
        bp3 = bp2;
        for (j=34; j>20; j--) {                 // look for a space from 21 to 34
          if (*bp3==' ') break;
          bp3--;
        }
        if (j>20) {                             // space found
          bp2 = bp3;
          c1 = *bp2;
          *bp2 = '\0';
        } else {                                // no space ... insert hyphen
          bp3 = bp2 + 1;
          c1 = *bp2;
          c2 = *bp3;
          *bp2 = '-';
          *bp3 = '\0';
        }
      } else {                                  // Scan forward      
        i++;
        bp2++;
      }
    }

    if (eol) {                                  // EOL flag ... write out buffer from bp to bp2.
  	  LC->skp->Text (_Col(0), _Line(l++), bp, strlen(bp));
      eol = false;
      if (c1 == '\0') {
        bp = bp2;     // End of buffer
      } else if ((c1 == '\n') || (c1 == ' ')) {
        bp = bp2+1;   // Reset for next line of the buffer
        bp2++;
        i=0;
      } else {
        bp = bp2;     // Put back the chars we stomped
        *bp2 = c1;
        *bp3 = c2;
        i=0;
      }
    }
  } while (*bp);

  return;
}

