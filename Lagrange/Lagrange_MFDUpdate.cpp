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
  if (GC->LU == nullptr) return true;

  if (LC->showMessage) {
    return DisplayMessageMode(skp);
  }
  switch (LC->mode) {
  case 0:
    return DisplayOrbitMode(skp);
    break;
  case 1:
    return DisplayPlanMode(skp);
    break;
  case 2:
    return DisplayBurnMode(skp);
    break;
  case 3:
    return DisplayLPMode(skp);
    break;
  case 4:
    return DisplayS4IMode(skp);
    break;
  case 6:
    return DisplayTgtMode(skp);
    break;
  }
  return true;
};

bool Lagrange::DisplayOrbitMode(oapi::Sketchpad *skp) {
  Title(skp, "Lagrange: ORBIT");
  skp->SetTextAlign(oapi::Sketchpad::LEFT, oapi::Sketchpad::BOTTOM);
  skp->SetTextColor(CLR_WHITE);

  int l = 4;
  char buf[128];
  int circrad = (int)(W / 120);
  int enc_x, enc_y;

  sprintf_s(buf, 128, "LP: %s", GC->LU->LP.name);
  skp->Text(Col(0), Line(l++), buf, strlen(buf));

//  for (int i = 0; i < 10; i++) {
//    skp->MoveTo(W * (i+1) / 12, H * 1/4);
//    skp->SetPen(pen[i]);
//    skp->LineTo(W * (i+1) / 12, H * 3/4);
//  }

  if (!GC->LU->s4i_valid) return true;

  Lagrange_vdata *lvd = &GC->LU->vdata[GC->LU->act][VC->vix];
  Lagrange_orb_disp *lod = &GC->LU->l_orb[GC->LU->act];

  if (lvd->orb_plot.size() != ORB_PLOT_COUNT) return true;

  // Calculates the vessel orbit
  oapi::IVECTOR2 iv[ORB_PLOT_COUNT-1];
  for (int i = 1; i < ORB_PLOT_COUNT; i++) {
    iv[i-1].x = (long) ((double) W * lvd->orb_plot[i].x);
    iv[i-1].y = (long) ((double) H * lvd->orb_plot[i].y);
  }

  skp->SetPen(pen[ORB_PEN_BRIGHT_GREEN]);
  skp->MoveTo((long)((double)W * lvd->orb_plot[1].x), (long)((double)H * lvd->orb_plot[1].y));
  skp->Polyline(iv, ORB_PLOT_COUNT-1);
  enc_x = (int)((double)W * lvd->orb_plot_ves_enc.x);
  enc_y = (int)((double)H * lvd->orb_plot_ves_enc.y);
  skp->Ellipse(enc_x - circrad, enc_y - circrad, enc_x + circrad, enc_y + circrad);

  for (int s = 1; s < ORB_MAX_LINES; s++) {
    if (GC->LU->LP.plotix[s] == -1) break;
    for (int i = 1; i < ORB_PLOT_COUNT; i++) {
      iv[i-1].x = (long)((double)W * lod->orb_plot[s][i].x);
      iv[i-1].y = (long)((double)H * lod->orb_plot[s][i].y);
    }
    skp->SetPen(pen[GC->LU->LP.plotixpen[s]]);
    skp->MoveTo((long)((double)W * lod->orb_plot[s][1].x), (long)((double)H * lod->orb_plot[s][1].y));
    skp->Polyline(iv, ORB_PLOT_COUNT-1);

    if (abs(lvd->orb_plot_body_enc[s].x - lvd->orb_plot_origin.x) > 0.02 || 
        abs(lvd->orb_plot_body_enc[s].y - lvd->orb_plot_origin.y) > 0.02) {
      enc_x = (int)((double)W * lvd->orb_plot_body_enc[s].x);
      enc_y = (int)((double)H * lvd->orb_plot_body_enc[s].y);
      skp->Ellipse(enc_x - circrad, enc_y - circrad, enc_x + circrad, enc_y + circrad);
    }

  }

  return true;
};

bool Lagrange::DisplayPlanMode(oapi::Sketchpad *skp) {
  Title(skp, "Lagrange: PLAN");
  skp->SetTextAlign(oapi::Sketchpad::LEFT, oapi::Sketchpad::BOTTOM);
  skp->SetTextColor(CLR_WHITE);

  int l = 4;
  char *adjText[9] = { "Rough", "Coarse", "Medium", "Fine", "Super", "Ultra", "Hyper", "Micro", "Reset" };

  Lagrange_vdata *vdata = &VC->LU->vdata[VC->LU->act][VC->vix];

  if (vdata->burnArmed) SkpFmtColText(skp, Col(0), Line(l), true, CLR_YELLOW, CLR_WHITE, ">");
  SkpFmtColText(skp, Col(0), Line(l++), vdata->burnArmed, CLR_YELLOW, CLR_WHITE, "  Planning Mode:  %s", vdata->burnArmed ? "Active" : "Inactive");
  SkpFormatText(skp, Col(0), Line(l++), "  Adjustment:     %s", adjText[VC->burnGranularity]);
  l++;

  if (vdata->burnMJD == 0.0) vdata->burnMJD = oapiGetSimMJD();

  SkpFmtColText(skp, Col(0), Line(l + VC->burnVar), true,   CLR_YELLOW, CLR_WHITE, ">");
  SkpFmtColText(skp, Col(0), Line(l++), (VC->burnVar == 0), CLR_YELLOW, CLR_WHITE, "  Plan MJD:     %14.6f", vdata->burnMJD);
  SkpFmtColText(skp, Col(0), Line(l++), (VC->burnVar == 1), CLR_YELLOW, CLR_WHITE, "  Prograde dV:  %14.6f", vdata->burndV.x);
  SkpFmtColText(skp, Col(0), Line(l++), (VC->burnVar == 2), CLR_YELLOW, CLR_WHITE, "  Plane Chg dV: %14.6f", vdata->burndV.y);
  SkpFmtColText(skp, Col(0), Line(l++), (VC->burnVar == 3), CLR_YELLOW, CLR_WHITE, "  Outward dV:   %14.6f", vdata->burndV.z);
  l++;

  if (!GC->LU->s4i_valid) return true;

  Lagrange_s4i *s4i_e = &GC->LU->s4i[GC->LU->act][vdata->enc_ix];
  SkpFormatText(skp, Col(0), Line(l++), "  Enc. MJD:     %14.6f", s4i_e->MJD);
  SkpFmtEngText(skp, Col(0), Line(l++), "  Enc. Time:    %10.2f", "s", s4i_e->sec - oapiGetSimTime(), 0);

  return true;
};

bool Lagrange::DisplayBurnMode(oapi::Sketchpad *skp) {
  Title(skp, "Lagrange: BURN");
  skp->SetTextAlign(oapi::Sketchpad::LEFT, oapi::Sketchpad::BOTTOM);
  skp->SetTextColor(CLR_WHITE);

  int l = 4;
  char buf[128];

  skp->SetTextColor(CLR_YELLOW);
  sprintf_s(buf, 128, "TODO: Burn Mode Info");
  skp->Text(Col(1), Line(l++), buf, strlen(buf));

  return true;
};

bool Lagrange::DisplayLPMode(oapi::Sketchpad *skp) {
  Title(skp, "Lagrange: LP");
  skp->SetTextAlign(oapi::Sketchpad::LEFT, oapi::Sketchpad::BOTTOM);
  skp->SetTextColor(CLR_WHITE);


  int l = 4;
  char buf[256];
  Lagrange_vdata *vdata = &GC->LU->vdata[GC->LU->act][VC->vix];
  Lagrange_ves_s4i *vs4i = &vdata->vs4i[0];
  QP_struct *ves = &vs4i->ves;
  QP_struct *vesLP = &vs4i->vesLP;
  Lagrange_ves_s4i *vs4i_e = &vdata->vs4i[vdata->enc_ix];
  QP_struct *ves_e = &vs4i_e->ves;
  QP_struct *vesLP_e = &vs4i_e->vesLP;
  Lagrange_s4i *s4i_e = &GC->LU->s4i[GC->LU->act][vdata->enc_ix];

  sprintf_s(buf, 128, "LP: %s", GC->LU->LP.name);
  skp->Text(Col(0), Line(l++), buf, strlen(buf));
  l++;

  sprintf_s(buf, 128, "Rel. Pos.");
  skp->Text(Col(2), Line(l), buf, strlen(buf));
  sprintf_s(buf, 128, " Enc. Pos");
  skp->Text(Col(4), Line(l++), buf, strlen(buf));

  DisplayEngUnit7(buf, "Prograde:  %8.3f","m", vesLP->Q.x, 1);
  skp->Text(Col(0), Line(l), buf, strlen(buf));
  if (GC->LU->s4i_valid) {
    DisplayEngUnit7(buf, "%8.3f", "m", vesLP_e->Q.x, 1);
    skp->Text(Col(4), Line(l++), buf, strlen(buf));
  } else { l++; };
  DisplayEngUnit7(buf, "Plane:     %8.3f","m", vesLP->Q.y,1);
  skp->Text(Col(0), Line(l), buf, strlen(buf));
  if (GC->LU->s4i_valid) {
    DisplayEngUnit7(buf, "%8.3f","m", vesLP_e->Q.y, 1);
    skp->Text(Col(4), Line(l++), buf, strlen(buf));
  } else { l++; };
  DisplayEngUnit7(buf, "Outward:   %8.3f","m", vesLP->Q.z,1);
  skp->Text(Col(0), Line(l), buf, strlen(buf));
  if (GC->LU->s4i_valid) {
    DisplayEngUnit7(buf, "%8.3f","m", vesLP_e->Q.z, 1);
    skp->Text(Col(4), Line(l++), buf, strlen(buf));
  DisplayEngUnit7(buf, "TOTAL:     %8.3f","m", vs4i->dQ,1);
  skp->Text(Col(0), Line(l), buf, strlen(buf));
  } else { l++; };
  if (GC->LU->s4i_valid) {
    DisplayEngUnit7(buf, "%8.3f","m", vs4i_e->dQ, 1);
    skp->Text(Col(4), Line(l++), buf, strlen(buf));
  } else { l++; };
  l++;
  
  sprintf_s(buf, 128, "Rel. Vel.");
  skp->Text(Col(2), Line(l), buf, strlen(buf));
  sprintf_s(buf, 128, " Enc. Vel");
  skp->Text(Col(4), Line(l++), buf, strlen(buf));

  DisplayEngUnit7(buf, "Prograde:  %8.3f","m/s", vesLP->P.x,0);
  skp->Text(Col(0), Line(l), buf, strlen(buf));
  if (GC->LU->s4i_valid) {
    DisplayEngUnit7(buf, "%8.3f","m/s", vesLP_e->P.x,0);
    skp->Text(Col(4), Line(l++), buf, strlen(buf));
  } else { l++; };
  DisplayEngUnit7(buf, "Plane:     %8.3f","m/s", vesLP->P.y,0);
  skp->Text(Col(0), Line(l), buf, strlen(buf));
  if (GC->LU->s4i_valid) {
    DisplayEngUnit7(buf, "%8.3f","m/s", vesLP_e->P.y,0);
    skp->Text(Col(4), Line(l++), buf, strlen(buf));
  } else { l++; };
  DisplayEngUnit7(buf, "Outward:   %8.3f","m/s", vesLP->P.z,0);
  skp->Text(Col(0), Line(l), buf, strlen(buf));
  if (GC->LU->s4i_valid) {
    DisplayEngUnit7(buf, "%8.3f","m/s", vesLP_e->P.z,0);
    skp->Text(Col(4), Line(l++), buf, strlen(buf));
  } else { l++; };
  DisplayEngUnit7(buf, "TOTAL:     %8.3f","m/s", vs4i->dP,0);
  skp->Text(Col(0), Line(l), buf, strlen(buf));
  if (GC->LU->s4i_valid) {
    DisplayEngUnit7(buf, "%8.3f","m/s", vs4i_e->dP,0);
   skp->Text(Col(4), Line(l++), buf, strlen(buf));
  } else { l++; };
  l++;
  sprintf(buf,"Enc. MJD:");
  skp->Text(Col(0), Line(l), buf, strlen(buf));
  if (GC->LU->s4i_valid) {
    sprintf(buf,"%10.4f", s4i_e->MJD);
    skp->Text(Col(4), Line(l++), buf, strlen(buf));
  } else { l++; };
  sprintf(buf,"Enc. Time:");
  skp->Text(Col(0), Line(l), buf, strlen(buf));
  if (GC->LU->s4i_valid) {
    DisplayEngUnit7(buf, "%8.2f","s", s4i_e->sec - oapiGetSimTime(),0);
    skp->Text(Col(4), Line(l++), buf, strlen(buf));
  } else { l++; };
  l++;

  return true;
};

bool Lagrange::DisplayS4IMode(oapi::Sketchpad *skp) {
  Title(skp, "Lagrange: S4I");
  skp->SetTextAlign(oapi::Sketchpad::LEFT, oapi::Sketchpad::BOTTOM);
  skp->SetTextColor(CLR_WHITE);

  int l = 4;
  char buf[128];

  switch (GC->LU->threadWorkerState()) {
  case 'A':
  case 'a':
  case 'B':
  case 'b':
    sprintf(buf, "S4I State            Active");
    break;
  case 'P':
    sprintf(buf, "S4I State            Paused");
    break;
  case 'K':
    sprintf(buf, "S4I State            Killed");
    break;
  default:
    sprintf(buf, "S4I State");
  }

  skp->Text(Col(0), Line(l++), buf, strlen(buf));
  l++;

  char *DiagText[7] = { "S4I Run    ",
                        "MJD From   ",
                        "MJD To     ",
                        "S4I deltaT ",
                        "Calc time  ",
                        "Orb Plot # ",
                        "S4I Iter # "};


  for (int i = 0; i < 5; i++) {
    sprintf_s(buf, 128, "%s %15.3f", DiagText[i], GC->LU->dbg[GC->LU->act][i]);
    skp->Text(Col(0), Line(l++), buf, strlen(buf));
  }
  for (int i = 5; i < 7; i++) {
    sprintf_s(buf, 128, "%s %15.0f", DiagText[i], GC->LU->dbg[GC->LU->act][i]);
    skp->Text(Col(0), Line(l++), buf, strlen(buf));
  }

//  GC->LU->s4integrator();

  return true;
};

bool Lagrange::DisplayTgtMode(oapi::Sketchpad *skp) {
  Title(skp, "Lagrange: TGT");
  skp->SetTextAlign(oapi::Sketchpad::LEFT, oapi::Sketchpad::BOTTOM);

  int l = 4;
  char buf[128];
  int curLP = GC->LU->getLP();

  for (int i = 0; i<COUNT_LP; i++) {
    if (i == curLP) {
      skp->SetTextColor(CLR_YELLOW);
      sprintf_s(buf, 128, "> %s", GC->LU->lptab[i].name);
    } else {
      skp->SetTextColor(CLR_WHITE);
      sprintf_s(buf, 128, "  %s", GC->LU->lptab[i].name);
    }
    skp->Text(Col(0), Line(l++), buf, strlen(buf));
  }

  return true;
};

bool Lagrange::DisplayMessageMode(oapi::Sketchpad *skp) {
  Title(skp, "Lagrange MFD");
  ShowMessage(skp);
  return true;
};


// MFD Line formatting helper
void Lagrange::ShowMessage(oapi::Sketchpad *skp) {

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
  	  skp->Text (Col(0), Line(l++), bp, strlen(bp));
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


// MFD Positioning Helper Functions
int Lagrange::Line( int row ) {  // row is 0-24, for 24 rows. e.g. Line(12)
  int ret;
  ret = (int) ((H-(int)(ch/4)) * row / 25) + (int) (ch/4);
  return ret;
};

int Lagrange::Col( int pos ) {  // pos is 0-5, for 6 columns. Eg Col(3) for middle
  int ret = (int) ((W-(int)(cw/2)) * pos / 6) + int (cw/2);
  return ret;
};

int Lagrange::Col2( int pos ) {  // pos is 0-11, for 12 columns. Eg Col(6) for middle
  int ret = (int) ((W-(int)(cw/2)) * pos / 12) + int (cw/2);
  return ret;
};


// MFD Format and print helper
void Lagrange::SkpFormatText(oapi::Sketchpad *skp, int col, int line, const char* fmt, ...) {
  char buf[128];
  va_list args;
  va_start(args, fmt);
  vsprintf_s(buf, 128, fmt, args);
  va_end(args);
  skp->Text(col, line, buf, strlen(buf));
}

void Lagrange::SkpFmtColText(oapi::Sketchpad *skp, int col, int line, bool test, DWORD truecol, DWORD falsecol, const char* fmt, ...) {
  skp->SetTextColor(test ? truecol : falsecol);
  char buf[128];
  va_list args;
  va_start(args, fmt);
  vsprintf_s(buf, 128, fmt, args);
  va_end(args);
  skp->Text(col, line, buf, strlen(buf));
  skp->SetTextColor(falsecol);
}

void Lagrange::SkpFmtEngText(oapi::Sketchpad *skp, int col, int line, const char* fmt, char* sfx, double val, int loB) {
  char buf[128], tmpbuf[128];

  char engUnit[12] = "pnum kMGTPE";
  double cnvVal = val;
  int i = 4;

  if (loB<-4) loB = -4;
  if (loB>6) loB = 6;
  loB += 4;

  if (fabs(cnvVal) < 1) {
    while ((fabs(cnvVal) < 1) && (i>loB)) {
      i--;
      cnvVal *= 1000;
    }
  } else if (fabs(cnvVal) >= 1000) {
    while ((fabs(cnvVal) >= 1000) && (i< 10)) {
      i++;
      cnvVal /= 1000;
    }
  }
  while (i<loB) {
    i++;
    cnvVal /= 1000;
  }
  if (engUnit[i] == ' ') {
    sprintf_s(tmpbuf, 128, "%s%s", fmt, sfx);
    sprintf_s(buf, 128, tmpbuf, cnvVal);
  } else {
    sprintf_s(tmpbuf, 128, "%s%%c%s", fmt, sfx);
    sprintf_s(buf, 128, tmpbuf, cnvVal, engUnit[i]);
  }
  skp->Text(col, line, buf, strlen(buf));
}