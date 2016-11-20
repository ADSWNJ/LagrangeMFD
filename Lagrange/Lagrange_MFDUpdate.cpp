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
    return DisplayDiagMode(skp);
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
  skp->SetTextColor(0xFFFFFF);

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
  if (GC->LU->vdata[GC->LU->act][VC->vix].orb_plot.size() != ORB_PLOT_COUNT) return true;

  // Calculates the vessel orbit
  oapi::IVECTOR2 iv[ORB_PLOT_COUNT-1];
  for (int i = 1; i < ORB_PLOT_COUNT; i++) {
    iv[i-1].x = (long) ((double) W * GC->LU->vdata[GC->LU->act][VC->vix].orb_plot[i].x);
    iv[i-1].y = (long) ((double) H * GC->LU->vdata[GC->LU->act][VC->vix].orb_plot[i].y);
  }

  skp->SetPen(pen[ORB_PEN_BRIGHT_GREEN]);
  skp->MoveTo((long)((double)W * GC->LU->vdata[GC->LU->act][VC->vix].orb_plot[1].x), (long)((double)H * GC->LU->vdata[GC->LU->act][VC->vix].orb_plot[1].y));
  skp->Polyline(iv, ORB_PLOT_COUNT-1);
  enc_x = (int)((double)W * GC->LU->vdata[GC->LU->act][VC->vix].orb_plot_ves_enc.x);
  enc_y = (int)((double)H * GC->LU->vdata[GC->LU->act][VC->vix].orb_plot_ves_enc.y);
  skp->Ellipse(enc_x - circrad, enc_y - circrad, enc_x + circrad, enc_y + circrad);

  for (int s = 1; s < ORB_MAX_LINES; s++) {
    if (GC->LU->LP.plotix[s] == -1) break;
    for (int i = 1; i < ORB_PLOT_COUNT; i++) {
      iv[i-1].x = (long)((double)W * GC->LU->l_orb[GC->LU->act].orb_plot[s][i].x);
      iv[i-1].y = (long)((double)H * GC->LU->l_orb[GC->LU->act].orb_plot[s][i].y);
    }
    skp->SetPen(pen[GC->LU->LP.plotixpen[s]]);
    skp->MoveTo((long)((double)W * GC->LU->l_orb[GC->LU->act].orb_plot[s][1].x), (long)((double)H * GC->LU->l_orb[GC->LU->act].orb_plot[s][1].y));
    skp->Polyline(iv, ORB_PLOT_COUNT-1);
    enc_x = (int)((double)W * GC->LU->vdata[GC->LU->act][VC->vix].orb_plot_body_enc[s].x);
    enc_y = (int)((double)H * GC->LU->vdata[GC->LU->act][VC->vix].orb_plot_body_enc[s].y);
    skp->Ellipse(enc_x - circrad, enc_y - circrad, enc_x + circrad, enc_y + circrad);

  }

  return true;
};

bool Lagrange::DisplayPlanMode(oapi::Sketchpad *skp) {
  Title(skp, "Lagrange: PLAN");
  skp->SetTextAlign(oapi::Sketchpad::LEFT, oapi::Sketchpad::BOTTOM);
  skp->SetTextColor(0xFFFFFF);

  int l = 4;
  char buf[128];

  skp->SetTextColor(0x00FFFF);
  sprintf_s(buf, 128, "TODO: Plan Mode Info");
  skp->Text(Col(1), Line(l++), buf, strlen(buf));

  return true;
};

bool Lagrange::DisplayBurnMode(oapi::Sketchpad *skp) {
  Title(skp, "Lagrange: BURN");
  skp->SetTextAlign(oapi::Sketchpad::LEFT, oapi::Sketchpad::BOTTOM);
  skp->SetTextColor(0xFFFFFF);

  int l = 4;
  char buf[128];

  skp->SetTextColor(0x00FFFF);
  sprintf_s(buf, 128, "TODO: Burn Mode Info");
  skp->Text(Col(1), Line(l++), buf, strlen(buf));

  return true;
};

bool Lagrange::DisplayLPMode(oapi::Sketchpad *skp) {
  Title(skp, "Lagrange: LP");
  skp->SetTextAlign(oapi::Sketchpad::LEFT, oapi::Sketchpad::BOTTOM);
  skp->SetTextColor(0xFFFFFF);


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
    DisplayEngUnit7(buf, "%9.2f","s", s4i_e->sec - oapiGetSimTime(),0);
    skp->Text(Col(4), Line(l++), buf, strlen(buf));
  } else { l++; };
  l++;

  return true;
};

bool Lagrange::DisplayDiagMode(oapi::Sketchpad *skp) {
  Title(skp, "Lagrange: DIAG");
  skp->SetTextAlign(oapi::Sketchpad::LEFT, oapi::Sketchpad::BOTTOM);
  skp->SetTextColor(0xFFFFFF);
  
  int l = 4;
  char buf[128];

  for (int i = 0; i < 6; i++) {
    sprintf_s(buf, 128, "D%i %12.3f", i, GC->LU->dbg[i]);
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
      skp->SetTextColor(0x00FFFF);
      sprintf_s(buf, 128, "> %s", GC->LU->lptab[i].name);
    } else {
      skp->SetTextColor(0xFFFFFF);
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

