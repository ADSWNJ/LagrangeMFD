// ==============================================================
//
//	Lagrange (MFD Update)
//	=====================
//
//	Copyright (C) 2016-2017	Andrew (ADSWNJ) Stokes
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

#define CLR_DEF   GC->LU->draw->GetMFDColor("DEF")
#define CLR_HI GC->LU->draw->GetMFDColor("HI")
#define CLR_WARN GC->LU->draw->GetMFDColor("WARN")

bool Lagrange::Update(oapi::Sketchpad *skp)
{
  LC->skp = skp;
  if (GC->LU == nullptr) return true;

  LC->skp->SetTextColor(CLR_DEF);

  GC->LU->orbFocVix = VC->vix;
  if (LC->showMessage) return DisplayMessageMode();
  switch (LC->mode) {
  case 0:
    return DisplayOrbitMode(); break;
  case 1:
    return DisplayLPMode(); break;
  case 2:
    return DisplayPlanMode(); break;
  case 3:
    return DisplayAPMode(); break;
  case 4:
    return DisplayS4IMode(); break;
  case 6:
    return DisplayTgtMode(); break;
  case 7:
    return DisplayFrmFocMode(); break;
  }
  return true;
};

/*
 *   O R B I T   M O D E 
 */
bool Lagrange::DisplayOrbitMode() {
  int l = 3;
  Lagrange_vdata *vdata = &GC->LU->vdata[GC->LU->act][VC->vix];

  if (vdata->burnArmed) {
    skpTitle("Lagrange: ORBIT (PLAN)");
  } else {
    skpTitle("Lagrange: ORBIT (LIVE)");
  }

  int circrad = (int)(W / 120);
  int enc_x, enc_y;

  LagrangeUniverse* LU = GC->LU;
  LagrangeUniverse::LagrangeUniverse_LP* LP = &LU->LP;
  Lagrange_vdata *lvd = &LU->vdata[GC->LU->act][VC->vix];
  Lagrange_orb_disp *lod = &LU->l_orb[GC->LU->act];

  skpFormatText(0, l, "LP: %s", LP->name);
  skpFormatText(4, l++, "FRM: %s", LU->body[lvd->refEnt].name);

  char *PrjTxt[3] = { "Std", "X-Edge", "Z-Edge" };
  char FocTxt[5][32] = { "", "", "Ves Orbit", "Ves Enc", "Ves Burn"};
  strcpy(FocTxt[0], LU->body[LP->maj].name);
  strcpy(FocTxt[1], LU->body[LP->min].name);
  char locked[4] = "(L)";
  if (!LU->orbFocLock) locked[0] = '\0';
  skpFormatText(0, 24, "FOC: %s %s", FocTxt[LU->orbFocus], locked);
  skpFormatText(4, 24, "PRJ: %s", PrjTxt[LU->orbProj]);
  if (lvd->alarm_state == 0) {
    skpFormatText(0, 25, "ZM: %d", -LU->orbZoom);
    skpFmtEngText(2, 25, "H: %.0f", "m", LU->orbPanHoriz[LU->orbProj] * pow(1.1, (double)LU->orbZoom), 1);
    skpFmtEngText(4, 25, "V: %.0f", "m", LU->orbPanVert[LU->orbProj] * pow(1.1, (double)LU->orbZoom), 1);
  } else   if (lvd->alarm_state == 2) {
    char buf2[256];
    skpColor(CLR_WARN);
    if (lvd->alarm_body == 1) {
      sprintf(buf2, "ALARM: %s reentry in %%.1f", LU->body[lvd->alarm_body].name);
    } else {
      sprintf(buf2, "ALARM: %s impact in %%.1f", LU->body[lvd->alarm_body].name);
    }
    skpFmtEngText(0, 25, buf2, "s", LU->s4i[GC->LU->act][lvd->alarm_ix].sec - oapiGetSimTime());
    skpColor(CLR_DEF);
  } else if (lvd->alarm_state == 1) {
    char buf2[256];
    skpColor(CLR_HI);
    sprintf(buf2, "WARN: %s proximity in %%.1f", LU->body[lvd->alarm_body].name);
    skpFmtEngText(0, 25, buf2, "s", LU->s4i[GC->LU->act][lvd->alarm_ix].sec - oapiGetSimTime());
    skpColor(CLR_DEF);
  }

  if (!GC->LU->s4i_valid) return true;
  if (lvd->orb_plot.size() != ORB_PLOT_COUNT) return true;

  oapi::IVECTOR2 iv[ORB_PLOT_COUNT];


  for (int s = 0; s < ORB_MAX_LINES; s++) {
    if (LP->plotix[s] == -1) break;
    for (int i = 0; i < ORB_PLOT_COUNT; i++) {
      iv[i].x = (long)((double)W * lod->orb_plot[s][i].x);
      iv[i].y = (long)((double)H * lod->orb_plot[s][i].y);
    }
    if (LP->plotix[s] != -2) {
      LC->skp->SetPen(GC->LU->draw->GetPen(LU->body[LP->plotix[s]].name));

    } else {
      LC->skp->SetPen(GC->LU->draw->GetPen("LP"));
    }

    LC->skp->MoveTo((int)((double)W * lod->orb_plot[s][1].x), (int)((double)H * lod->orb_plot[s][1].y));
    LC->skp->Polyline(iv, ORB_PLOT_COUNT);

    if (LP->plotix[s] != -2) {
      LC->skp->SetPen(GC->LU->draw->GetPen(LU->body[LP->plotix[s]].name, true));

    } else {
      LC->skp->SetPen(GC->LU->draw->GetPen("LP", true));
    }

    if (lvd->enc_ix >= 0) {
      if (abs(lvd->orb_plot_body_enc[s].x - lvd->orb_plot_origin.x) >= 0.00 ||
        abs(lvd->orb_plot_body_enc[s].y - lvd->orb_plot_origin.y) >= 0.00) {

        enc_x = (int)((double)W * lvd->orb_plot_body_enc[s].x);
        enc_y = (int)((double)H * lvd->orb_plot_body_enc[s].y);
        LC->skp->Ellipse(enc_x - circrad, enc_y - circrad, enc_x + circrad, enc_y + circrad);

        if (LP->plotix[s] != -2) {
          // Determine Body PROX limit and IMPACT LIMIT
          double proxLim = GC->LU->body[LP->plotix[s]].proxWarnDist;
          double impactLim = GC->LU->body[LP->plotix[s]].impactWarnDist;
          double circradKM = (double)circrad / (double)W * GC->LU->orbScale[GC->LU->orbProj];
          if (circradKM < impactLim) {
            int bodyRadPx = (int)((double)W * impactLim / GC->LU->orbScale[GC->LU->orbProj] + 0.5);
            double rf = bodyRadPx;
            for (int xscan = 0; xscan <= bodyRadPx; xscan++) {
              double xf = (double) xscan;
              double ang = asin(xf / rf);
              double yf = rf * cos(ang) + 0.5;
              int yof = (int) yf;
              LC->skp->Line(enc_x + xscan, enc_y + yof, enc_x + xscan, enc_y - yof);
              LC->skp->Line(enc_x - xscan, enc_y + yof, enc_x - xscan, enc_y - yof);
            }
          }
        }
      }
    }     
  }

  for (int i = 0; i < ORB_PLOT_COUNT; i++) {
    iv[i].x = (long)((double)W * lvd->orb_plot[i].x);
    iv[i].y = (long)((double)H * lvd->orb_plot[i].y);
  }

  if (vdata->burnArmed) {
    LC->skp->SetPen(GC->LU->draw->GetPen("VP"));
  } else {
    LC->skp->SetPen(GC->LU->draw->GetPen("VL"));
  }

  LC->skp->MoveTo((long)((double)W * lvd->orb_plot[0].x), (long)((double)H * lvd->orb_plot[0].y));
  LC->skp->Polyline(iv, ORB_PLOT_COUNT);
  //LC->skp->Line((int)((double)W * lvd->orb_plot_origin.x), (int)((double)H *lvd->orb_plot_origin.y), (int)((double)W * lvd->orb_plot[0].x), (long)((double)H * lvd->orb_plot[0].y));

  if (vdata->burnArmed) {
    LC->skp->SetPen(GC->LU->draw->GetPen("VP", true));
  } else {
    LC->skp->SetPen(GC->LU->draw->GetPen("VL", true));
  }
  if (lvd->enc_ix >= 0) {
    enc_x = (int)((double)W * lvd->orb_plot_ves_enc.x);
    enc_y = (int)((double)H * lvd->orb_plot_ves_enc.y);
    LC->skp->Ellipse(enc_x - circrad, enc_y - circrad, enc_x + circrad, enc_y + circrad);
  }

  return true;
};

/*
 *   E N C O U N T E R    M O D E
 */
bool Lagrange::DisplayLPMode() {

  int l = 4;
  Lagrange_vdata *vdata = &GC->LU->vdata[GC->LU->act][VC->vix];
  
  if (vdata->burnArmed) {
    skpTitle("Lagrange: ENCOUNTER (PLAN)");
  } else {
    skpTitle("Lagrange: ENCOUNTER (LIVE)");
  }
  
  Lagrange_ves_s4i *vs4i = &vdata->vs4i[0];
  QP_struct *ves = &vs4i->ves;
  QP_struct *vesLP = &vs4i->vesLP;
  Lagrange_ves_s4i *vs4i_e = (vdata->enc_ix >= 0) ? &vdata->vs4i[vdata->enc_ix] : nullptr;
  QP_struct *ves_e = (vs4i_e) ? &vs4i_e->ves : nullptr;
  QP_struct *vesLP_e = (vs4i_e) ? &vs4i_e->vesLP : nullptr;
  Lagrange_s4i *s4i_e = (vdata->enc_ix >= 0) ? &GC->LU->s4i[GC->LU->act][vdata->enc_ix] : nullptr;
  LC->skpLoB = 1;

  skpFormatText(0, l++, "LP: %s", GC->LU->LP.name);
  l++;
  int rl = l;

  skpFormatText(2, l++, "Rel. Pos.");
  skpFmtEngText(0, l++, "Prograde:  %8.3f", "m", vesLP->Q.x);
  skpFmtEngText(0, l++, "Outward:   %8.3f", "m", vesLP->Q.z);
  skpFmtEngText(0, l++, "Plane Chg: %8.3f", "m", vesLP->Q.y);
  skpFmtEngText(0, l++, "TOTAL:     %8.3f", "m", vs4i->dQ);

  l++;
  LC->skpLoB = 0;
  skpFormatText(2, l++, "Rel. Vel.");
  skpFmtEngText(0, l++, "Prograde:  %8.3f", "m/s", vesLP->P.x);
  skpFmtEngText(0, l++, "Outward:   %8.3f", "m/s", vesLP->P.z);
  skpFmtEngText(0, l++, "Plane Chg: %8.3f", "m/s", vesLP->P.y);
  skpFmtEngText(0, l++, "TOTAL:     %8.3f", "m/s", vs4i->dP);

  l++;
  if (GC->LU->s4i_valid && vdata->enc_ix >= 0 && vesLP_e) {
    LC->skpLoB = 1;
    skpFormatText(4, rl++, "Enc. Pos.");
    skpFmtEngText(4, rl++, "%8.3f", "m", vesLP_e->Q.x);
    skpFmtEngText(4, rl++, "%8.3f", "m", vesLP_e->Q.z);
    skpFmtEngText(4, rl++, "%8.3f", "m", vesLP_e->Q.y);
    skpFmtEngText(4, rl++, "%8.3f", "m", vs4i_e->dQ);

    rl++;
    LC->skpLoB = 0;
    skpFormatText(4, rl++, "Enc. Vel.");
    skpFmtEngText(4, rl++, "%8.3f", "m/s", vesLP_e->P.x);
    skpFmtEngText(4, rl++, "%8.3f", "m/s", vesLP_e->P.z);
    skpFmtEngText(4, rl++, "%8.3f", "m/s", vesLP_e->P.y);
    skpFmtEngText(4, rl++, "%8.3f", "m/s", vs4i_e->dP);

    rl++;
    skpFormatText(0, rl++, "Enc. MJD:");
    skpFormatText(0, rl++, "Enc. Time:");
    skpFormatText(0, rl++, "Enc. dTime:");
    if (vdata->enc_typ == -1) {
      skpFormatText(0, rl++, "Enc. Type:               Before Scan");
    } else if (vdata->enc_typ == 1) {
      skpFormatText(0, rl++, "Enc. Type:               After Scan");
    } else {
      skpFormatText(0, rl++, "Enc. Type:               During Scan");
    }
    rl -= 4;
    skpFormatText(4, rl++, "%10.4f", s4i_e->MJD);
    skpFmtEngText(4, rl++, "%10.4f", "s", s4i_e->sec);
    skpFmtEngText(4, rl++, "%10.4f", "s", s4i_e->sec - oapiGetSimTime());
  }
  if (rl > l) l = rl+1;
  l++; 
  //double vm = VC->v->GetMass();
  //skpFormatText(0, l++, "Mass: %10.3fkg", vm);

  LagrangeUniverse* LU = GC->LU;
  Lagrange_vdata *lvd = &LU->vdata[GC->LU->act][VC->vix];

  if (lvd->alarm_state == 2) {
    char buf2[256];
    skpColor(CLR_WARN);
    if (lvd->alarm_body == 1) {
      sprintf(buf2, "ALARM: %s reentry in %%.1f", LU->body[lvd->alarm_body].name);
    } else {
      sprintf(buf2, "ALARM: %s impact in %%.1f", LU->body[lvd->alarm_body].name);
    }
    skpFmtEngText(0, 25, buf2, "s", LU->s4i[GC->LU->act][lvd->alarm_ix].sec - oapiGetSimTime());
    skpColor(CLR_DEF);
  } else if (lvd->alarm_state == 1) {
    char buf2[256];
    skpColor(CLR_HI);
    sprintf(buf2, "WARN: %s proximity in %%.1f", LU->body[lvd->alarm_body].name);
    skpFmtEngText(0, 25, buf2, "s", LU->s4i[GC->LU->act][lvd->alarm_ix].sec - oapiGetSimTime());
    skpColor(CLR_DEF);
  }

  return true;
};


/*
 *   P L A N    M O D E
 */
bool Lagrange::DisplayPlanMode() {
  skpTitle("Lagrange: PLAN");

  int l = 4;
  char *adjText[9] = { "Rough", "Coarse", "Medium", "Fine", "Super", "Ultra", "Hyper", "Micro", "Reset" };

  Lagrange_vdata *vdata = &VC->LU->vdata[VC->LU->act][VC->vix];

  if (vdata->burnArmed) skpFmtColText(0, l, true, CLR_HI, CLR_DEF, ">");

  if (!VC->ap.IsBurnFrozen()) {
    skpFmtColText(0, l++, vdata->burnArmed, CLR_HI, CLR_DEF, "  Planning Mode:  %s", vdata->burnArmed ? "Active" : "Inactive");
  } else {
    skpColor(CLR_WARN);
    skpFormatText(0, l++, "  Planning Mode:  %s", "Frozen");
    skpColor(CLR_DEF);
  }

  skpFormatText(0, l++, "  Adjustment:     %s", adjText[VC->burnGranularity[VC->burnVar]]);
  l++;

  if (vdata->burnMJD == 0.0) vdata->burnMJD = oapiGetSimMJD() + (1 / (10 * 60 * 24));
  skpFmtColText(0, VC->burnVar==0? l : l + 1 + VC->burnVar, true,   CLR_HI, CLR_DEF, ">");
  skpFmtColText(0, l++, (VC->burnVar == 0), CLR_HI, CLR_DEF, "  Burn MJD:     %14.6f", vdata->burnMJD);
  skpFmtEngText(0, l++, "  Burn Point:   %11.3f", "s", (vdata->burnMJD-oapiGetSimMJD())*24.0*60.0*60.0);
  skpFmtColText(0, l++, (VC->burnVar == 1), CLR_HI, CLR_DEF, "  Prograde dV:  %14.6fm/s", vdata->burndV.x);
  skpFmtColText(0, l++, (VC->burnVar == 2), CLR_HI, CLR_DEF, "  Outward dV:   %14.6fm/s", vdata->burndV.y);
  skpFmtColText(0, l++, (VC->burnVar == 3), CLR_HI, CLR_DEF, "  Plane Chg dV: %14.6fm/s", vdata->burndV.z);
  if (VC->burnTdV_lock) {
    skpFmtColText(0, l, (VC->burnVar == 4), CLR_HI, CLR_DEF, "  Total dV Lock:%14.6fm/s", length(vdata->burndV));
  } else {
    skpFmtColText(0, l, (VC->burnVar == 4), CLR_HI, CLR_DEF, "  Total dV:     %14.6fm/s", length(vdata->burndV));
  }
  l+=2;
  if (!GC->LU->s4i_valid || vdata->enc_ix < 0) return true;

  Lagrange_ves_s4i *vs4i_e = (vdata->enc_ix >= 0) ? &vdata->vs4i[vdata->enc_ix] : nullptr;
  if (!vs4i_e) return true;

  Lagrange_s4i *s4i_e = &GC->LU->s4i[GC->LU->act][vdata->enc_ix];
  QP_struct *vesLP_e = (vs4i_e) ? &vs4i_e->vesLP : nullptr;
  
  if (vdata->burnArmed) {
    skpFormatText(0, l++, "  Encounter: PLAN");
  } else {
    skpFormatText(0, l++, "  Encounter: LIVE");
  }
  skpFormatText(0, l++, "  Enc. MJD:     %14.6f", s4i_e->MJD);
  skpFmtEngText(0, l++, "  Enc. Time:  %15.5f", "s", s4i_e->sec - oapiGetSimTime());
  l++;
  int rl = l;
  skpFormatText(0, l++, "       Enc. Pos.");

  skpFmtEngText(0, l++, "  Pro:%8.3f", "m", vesLP_e->Q.x);
  skpFmtEngText(0, l++, "  Out:%8.3f", "m", vesLP_e->Q.z);
  skpFmtEngText(0, l++, "  PlC:%8.3f", "m", vesLP_e->Q.y);
  skpFmtEngText(0, l++, "  TOT:%8.3f", "m", vs4i_e->dQ);
  skpFormatText(3, rl++, " Enc. dVel.");
  skpFmtEngText(3, rl++, "%9.4f", "m/s", vesLP_e->P.x);
  skpFmtEngText(3, rl++, "%9.4f", "m/s", vesLP_e->P.z);
  skpFmtEngText(3, rl++, "%9.4f", "m/s", vesLP_e->P.y);
  skpFmtEngText(3, rl++, "%9.4f", "m/s", vs4i_e->dP);

  l++; l++;



  if (vdata->burnArmed && (vdata->burnMJD <  GC->LU->dbg[GC->LU->act][1] || vdata->burnMJD >  GC->LU->dbg[GC->LU->act][2])) {
    skpColor(CLR_HI);
    skpFormatText(0, l++, "NOTE: Burn Time not in S4I range");
  }

  LagrangeUniverse* LU = GC->LU;
  Lagrange_vdata *lvd = &LU->vdata[GC->LU->act][VC->vix];

  if (lvd->alarm_state == 2) {
    char buf2[256];
    skpColor(CLR_WARN);
    if (lvd->alarm_body == 1) {
      sprintf(buf2, "ALARM: %s reentry in %%.1f", LU->body[lvd->alarm_body].name);
    } else {
      sprintf(buf2, "ALARM: %s impact in %%.1f", LU->body[lvd->alarm_body].name);
    }
    skpFmtEngText(0, 25, buf2, "s", LU->s4i[GC->LU->act][lvd->alarm_ix].sec - oapiGetSimTime());
    skpColor(CLR_DEF);
  } else if (lvd->alarm_state == 1) {
    char buf2[256];
    skpColor(CLR_HI);
    sprintf(buf2, "WARN: %s proximity in %%.1f", LU->body[lvd->alarm_body].name);
    skpFmtEngText(0, 25, buf2, "s", LU->s4i[GC->LU->act][lvd->alarm_ix].sec - oapiGetSimTime());
    skpColor(CLR_DEF);
  }

  return true;
};


/*
 *   A U T O P I L O T    M O D E
 */
bool Lagrange::DisplayAPMode() {
  skpTitle("Lagrange: AUTOPILOT");
  int l = 4; 
  int l2;

  Lagrange_vdata *vdata = &VC->LU->vdata[VC->LU->act][VC->vix];
  Lagrange_ves_s4i *vs4i = &vdata->vs4i[0];

  bool inac = VC->apMode == 0;
  bool apPlan = VC->apMode == 1;
  bool apHold = VC->apMode == 2;
  bool apPoint = VC->apState >= 2;
  bool apBurn = VC->apState == 3;

  if (inac) {
    skpColor(CLR_DEF);
    skpFormatText(0, l++, "   AP Mode:      DISARMED");
    l += 12;
    skpFormatText(0, l++, "AP arming conditions:");
    skpFormatText(0, l++, "-Plan armed and burn in S4I range");
    skpFormatText(0, l++, "-Within 1,000km of Lagrange Point");
    return true;
  }

  skpFmtColText(0, l++, apPoint, CLR_WARN, CLR_HI,   "   AP Mode:      %s", apPlan ? "Plan" : "Hold LP");
  if (VC->apMode == 1) {
    skpFmtColText(0, l++, apPoint, CLR_WARN, CLR_HI, "   AP AutoAlign: %s", apPoint ? "Active" : "Armed");
    skpFmtColText(0, l++, apBurn, CLR_WARN, CLR_HI,  "   AP AutoBurn:  %s", apBurn ? "Active" : "Armed");
  } else {
    skpFmtColText(0, l++, apBurn, CLR_WARN, CLR_HI,  "   AP AutoHold:  %s", apBurn ? "Active" : "Armed");
  }


  l++; l++;

  VECTOR3 att = VC->ap.GetATT()*DEG;
  VECTOR3 avl = VC->ap.GetAVel()*DEG;
  for (int i = 0; i < 3; i++) {
    if (abs(att.data[i]) < 0.01) att.data[i] = 0.0;
    if (abs(avl.data[i]) < 0.001) avl.data[i] = 0.0;
  }

  unsigned char deg[2] = { 0xb0, '\0' };
  unsigned char degs[4] = { 0xb0, '/', 's', '\0' };
  skpColor(CLR_DEF);
  l2 = l;
  if (VC->apMode == 1) {
    skpFormatText(0, l++, "   Alignment Error");
  } else {
    skpFormatText(0, l++, "   Prograde Error");
  }


  skpFmtEngText(0, l++, "   Pitch:    %7.2f", deg, att.x);
  skpFmtEngText(0, l++, "   Yaw:      %7.2f", deg, att.y);
  skpFmtEngText(0, l++, "   Roll:     %7.2f", deg, att.z);

  l = l2;

  skpFormatText(4, l++, "   Rate");
  skpFmtEngText(4, l++, "%8.3f", degs, avl.x);
  skpFmtEngText(4, l++, "%8.3f", degs, avl.y);
  skpFmtEngText(4, l++, "%8.3f", degs, avl.z);

  l++; l++;
  if (VC->apMode == 1) {
    double simT = oapiGetSimTime();
    double burnStart = VC->burnStart - simT;
    double burnEnd = VC->burnEnd - simT;
    if (VC->apState > 1) {
      skpFmtEngText(0, l++, "   Burn Duration: %8.3f", "s", VC->burnDurn);
      l++;
      if (burnStart >= 0.0 && burnStart < 30.0 && burnStart != burnEnd) skpColor(CLR_HI);
      if (burnStart >= 0.0 && burnStart < 10.0 && burnStart != burnEnd) skpColor(CLR_WARN);
      skpFmtEngText(0, l++, "   Burn Start:    %8.3f", "s", VC->burnStart - simT);
      skpColor(CLR_DEF);
      if (burnStart < 0.0 && burnEnd >= 0.0 && burnEnd < 30.0 && burnStart != burnEnd) skpColor(CLR_HI);
      if (burnStart < 0.0 && burnEnd >= 0.0 && burnEnd < 10.0 && burnStart != burnEnd) skpColor(CLR_WARN);
      skpFmtEngText(0, l++, "   Burn End:      %8.3f", "s", VC->burnEnd - simT);
      l++;
    }
    if (VC->burnStart<simT && VC->burnEnd > simT) {
      skpColor(CLR_WARN);
      if (VC->apState == 2) {
        skpFormatText(0, l++, "   >>> BURN NOW <<<");
      } else if (VC->apState == 3) {
        skpFormatText(0, l++, "   Auto-Burning");
      }
      skpColor(CLR_DEF);
    }
    return true;
  }

  l2 = l;

  skpFormatText(0, l++, "   LP Offset");
  skpFmtEngText(0, l++, "   X:        %7.2f", "m", vs4i->vesLP.Q.x);
  skpFmtEngText(0, l++, "   Y:        %7.2f", "m", vs4i->vesLP.Q.y);
  skpFmtEngText(0, l++, "   Z:        %7.2f", "m", vs4i->vesLP.Q.z);

  l = l2;

  skpFormatText(4, l++, "   Rate");
  skpFmtEngText(4, l++, "%8.3f", "m/s", vs4i->vesLP.P.x);
  skpFmtEngText(4, l++, "%8.3f", "m/s", vs4i->vesLP.P.y);
  skpFmtEngText(4, l++, "%8.3f", "m/s", vs4i->vesLP.P.z);

  l++; l++;
  skpFmtEngText(0, l++, "   AH Fuel Burn:    %8.3f", "kg", VC->ap_ahStartMassV - VC->v->GetMass());
  return true;
};


/*
 *   S 4 I     M O D E
 */
bool Lagrange::DisplayS4IMode() {

  skpTitle("Lagrange: S4I");
  int l = 4;

  switch (GC->LU->threadWorkerState()) {
  case 'A':
  case 'a':
  case 'B':
  case 'b':
    skpFormatText(0,l++,  "S4I State                  Active");
    break;
  case 'P':
    skpFormatText(0, l++, "S4I State                  Paused");
    break;
  case 'K':
    skpFormatText(0, l++, "S4I State                  Killed");
    break;
  default:
    skpFormatText(0, l++, "S4I State");
  }
  if (!GC->LU->s4i_valid) return true;
  l++;
  char *DiagText[12] = { "Last S4I Run     ",
                        "MJD From         ",
                        "MJD To           ",
                        "Wait Time        ",
                        "S4I Calc Rng (d) ",
                        "S4I Calc Rng (h) ",
                        "S4I Calc Rng (m) ",
                        "S4I Calc Rng (s) ",
                        "S4I Iter #       ",
                        "S4I DeltaT       ",
                        "S4I Calc time    ",
                        "Orb Plot #       "
                      };

  for (int i = 0; i < 4; i++) {
    skpFormatText(0, l++, "%s %15.3f", DiagText[i], GC->LU->dbg[GC->LU->act][i]);
  }
  l++;
  double s4i_cp = (GC->LU->dbg[GC->LU->act][4] * GC->LU->dbg[GC->LU->act][5]) / 60.0 / 60.0 / 24.0;
  if (s4i_cp > 1.0e64) return true;
  skpFormatText(0, l++, "%s %15.8f", DiagText[4], s4i_cp);
  s4i_cp *= 24.0;
  skpFormatText(0, l++, "%s %15.6f", DiagText[5], s4i_cp);
  s4i_cp *= 60.0;
  skpFormatText(0, l++, "%s %15.4f", DiagText[6], s4i_cp);
  s4i_cp *= 60.0;
  skpFormatText(0, l++, "%s %15.2f", DiagText[7], s4i_cp);
  l++;
  skpFormatText(0, l++, "%s %15.0f", DiagText[8], GC->LU->dbg[GC->LU->act][4]);
  skpFormatText(0, l++, "%s %15.3f", DiagText[9], GC->LU->dbg[GC->LU->act][5]);
  skpFormatText(0, l++, "%s %15.3f", DiagText[10], GC->LU->dbg[GC->LU->act][6]);
  l++; l++;
  skpFormatText(0, l++, "%s %15.0f", DiagText[11], GC->LU->dbg[GC->LU->act][7]);
  return true;
};

bool Lagrange::DisplayFrmFocMode() {
  if (GC->LU->PrvNxtMode == 1) {
    skpTitle("Lagrange: FRAME");
    int l = 4;
    int curRef = GC->LU->vdata[GC->LU->act][VC->vix].refEnt;
    skpColor(CLR_HI);
    skpFormatText(0, l + curRef, ">");
    for (int i = 0; i<COUNT_BODY; i++) {
      skpFmtColText(0, l++, (i == curRef), CLR_HI, CLR_DEF, "  %s", GC->LU->body[i].name);
    }
  } else if (GC->LU->PrvNxtMode == 2) {
    skpTitle("Lagrange: FOCUS");
    int l = 4;
    char focusNames[5][32];
    strcpy(focusNames[0], GC->LU->body[GC->LU->LP.maj].name);
    strcpy(focusNames[1], GC->LU->body[GC->LU->LP.min].name);
    strcpy(focusNames[2], "Vessel Orbit");
    strcpy(focusNames[3], "Vessel Enc");
    strcpy(focusNames[4], "Vessel Burn");

    int curFoc = GC->LU->orbFocus;
    skpColor(CLR_HI);
    skpFormatText(0, l + curFoc, ">");
    for (int i = 0; i<5; i++) {
      skpFmtColText(0, l++, (i == curFoc), CLR_HI, CLR_DEF, "  %s", focusNames[i]);
    }

    l = 24;
    if (GC->LU->orbFocLock) {
      skpFormatText(0, l++, "Focus Point: Locked");
    } else {
      skpFormatText(0, l++, "Focus Point: Unlocked");
    }

  }
  return true;
};

/*
 *   T A R G E T    M O D E
 */
bool Lagrange::DisplayTgtMode() {
  skpTitle("Lagrange: TGT");
  int l = 4;
  int curLP = GC->LU->getLP();
  skpColor(CLR_HI);
  skpFormatText(0, l + curLP, ">");
  for (int i = 0; i<COUNT_LP; i++) {
    skpFmtColText(0, l++, (i == curLP), CLR_HI, CLR_DEF, "  %s", GC->LU->lptab[i].name);
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

