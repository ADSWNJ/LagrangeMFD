
// ==========================================================================================================
//  Glideslope 2 Main code
//  Copyright (c) 2012-2017 Andrew (ADSWNJ) Stokes, Licensed under GNU GPL
//
//  Designed to support flying of an accurate glide slope for the Orbiter simulator (tested for Orbiter 2010)
//
//
//	Derivitive work based on original Glideslope MFD Copyright (c) 2006, 2009 by Chris "KWAN3217" Jeppesen
//    licensed under GNU GPL
// 
//  The Orbiter 2010 simulator (c) Martin Schweiger
//  The Orbiter 2016 simulator (c) Martin Schweiger
//
//                   All rights reserved
//
// Glideslope2.cpp
// ===========================================================================================================

#define STRICT

#include "windows.h"
#include "orbitersdk.h"
#include "DisplayEngUnitFunctions.h"
#include "ParseFunctions.h"
#include "GlideslopeCore.hpp"
#include "GSmiscFunctions.hpp"
#include "GlideslopeMFD.hpp"
#include "GSoapiModule.hpp"

// ==============================================================
// Global variables

GSoapiModule *g_coreMod;
GlideslopeCore *GCoreData[32];
OBJHANDLE GCoreVessel[32];
int nGutsUsed;
int g_MFDmode;


// ==============================================================
// MFD class implementation

// Constructor
GlideslopeMFD::GlideslopeMFD (DWORD w, DWORD h, VESSEL *v, UINT im): MFD (w, h, v) {
  vessel=v;
  width=w;
  height=h;
  mid=w/2;
  col1 = w * 3 / 10;
  col2 = w * 5 / 10;
  col3 = w * 7 / 10;

  ZoomX=10000.0*1e3;
  ZoomY=30;
  ZoomMap=60000;
  MinZoomMap=40000;
  ManZoomToggle = false;
  MinZoomX=ZoomX/(2<<11);
  MinZoomY=ZoomY/(2<<5);
  units=false;//False==metric, true==US
  imfd=im;
  FlashToggle = 0; 


// This code is looking for a pre-initialized common guts area. The issue is that the Glideslope class gets destroyed and recreated each time
// you change view (e.g. F8), or resize an external MFD. We need to persist common things (e.g. target runway) into the guts core.
  bool found=false;
  bool mfound=false;


  for(int i=0;i<nGutsUsed;i++) {
    if (i==32) {  // If we get to the 33rd ship in the scenario needed Glideslope ... wrap it onto the first ship again to stop mem overrun
      i=0;
      GCoreVessel[i]=vessel;
      GCoreData[i]->vessel=vessel;
    }
    if(GCoreVessel[i]==vessel) {
      found=true;
	    G=GCoreData[i];

// This code looks for a specific example of this vessel and this MFD. If not found, we load it and set a base mode for it. 
	    for(int j=0;((j<G->coreMFDcount)&&(!mfound));j++) {
		    if (G->coreMFDint[j] == imfd) {
		      mfound = true;
			    coreMFDref = j;
			    mode = G->coreMode[j];	// get my old mode & units back!
			    jumpmode = G->coreJumpMode[j];	// get my old mode & units back!
			    units = G->coreUnit[j];
			    ManZoomToggle = G->coreManZoomToggle[j];
   	      ZoomMap = G->coreZoom[j];
		    }
	    }
	    if (!mfound) {
		    coreMFDref = G->coreMFDcount;
        G->coreMFDint[coreMFDref] = imfd;		// Create this new vessel/MFD pair and init the settings
		    G->coreMode[coreMFDref] = (imfd == MFD_LEFT) ? G->ParamsModeL : (imfd == MFD_RIGHT) ? G->ParamsModeR : G->ParamsModeX;
        G->coreJumpMode[coreMFDref] = G->coreMode[coreMFDref];
        G->coreSwitchPage[coreMFDref] = 0;
        G->coreZoom[coreMFDref] = ZoomMap; 
        mode = G->coreMode[coreMFDref];
		    jumpmode = G->coreJumpMode[coreMFDref];
		    G->coreMFDcount++;
	    }
    }
  }

  if(!found) {
    GCoreData[nGutsUsed] = new GlideslopeCore;
    G=GCoreData[nGutsUsed];
    GCoreVessel[nGutsUsed]=vessel;
    G->vessel = vessel;
    G->exTrSave = false;
    G->GSqarOn = false;
    G->GSqarInit = false;
    nGutsUsed++;
    LoadConfig(); 
    G->reset(vessel);
	
    for (int i=1; i<G->CORE_MFD_LIMIT; i++) {
	    G->coreMFDint[i] = 0;
	    G->coreMode[i] = 0;
	    G->coreJumpMode[i] = 0;
      G->coreSwitchPage[i] = 0;
    }
    G->coreMFDcount=1;

	  coreMFDref=0;
    G->coreMFDint[coreMFDref] = imfd;		// Create this first vessel/MFD pair and init the settings
	  if (imfd == MFD_LEFT) {
  	  G->coreMode[coreMFDref] = G->ParamsModeL;
	  } else if (imfd == MFD_RIGHT) {
  	  G->coreMode[coreMFDref] = G->ParamsModeR;
	  } else {
  	  G->coreMode[coreMFDref] = G->ParamsModeX;
	  }
    mode = G->coreMode[coreMFDref];
    jumpmode = G->coreMode[coreMFDref];
  }


//  int oldMode=mode;
  if ((mode<1)||(mode>G->maxMod)) mode=1;
//sprintf(oapiDebugString(),"Glideslope constructor called...  iMFD(%i) gutsinit(%c) mfdinit(%c)",imfd, (found?'Y':'N'), (mfound?'Y':'N'));

  blueBrush=CreateSolidBrush(BLUE);
  dkblueBrush=CreateSolidBrush(DARK_BLUE);
  blackBrush=CreateSolidBrush(BLACK);
  grayBrush=CreateSolidBrush(GRAY);
  greenBrush=CreateSolidBrush(GREEN);
  whitePen=CreatePen(PS_SOLID,1,WHITE);
  yellowPen=CreatePen(PS_SOLID,1,YELLOW);
//  ltBluePen=CreatePen(PS_SOLID,1,RGB(0,176,240));
//  dkBluePen=CreatePen(PS_SOLID,1,RGB(0,127,172));

}


// Destructor
GlideslopeMFD::~GlideslopeMFD () {
  DeleteObject(blueBrush);
  DeleteObject(dkblueBrush);
  DeleteObject(blackBrush);
  DeleteObject(whitePen);
  DeleteObject(yellowPen);
//  DeleteObject(ltBluePen);
//  DeleteObject(dkBluePen);
}

// Write Scenario save data
// ... gets called once per MFD, on scenario save
void GlideslopeMFD::WriteStatus (FILEHANDLE scn) const {
	oapiWriteScenario_string(scn, "Base", G->BaseName[G->RunwayBase[G->runway]]);
  oapiWriteScenario_string(scn, "Rway", G->RunwayName[G->runway]);
	if (mode>=100) {
	  oapiWriteScenario_string(scn, "Page", jumpmode == 2? "TAPE" : jumpmode == 3? "DATA" :jumpmode == 4? "HSIT" :jumpmode == 5? "DIAG" :jumpmode == 6? "DEOR" : "VSIT");
	} else {
	  oapiWriteScenario_string(scn, "Page", mode == 2? "TAPE" : mode == 3? "DATA" :mode == 4? "HSIT" :mode == 5? "DIAG" :mode == 6? "DEO" : "VSIT");
	}
	oapiWriteScenario_string(scn, "Unit", units? "US" : "METRIC");
	oapiWriteScenario_string(scn, "ManZ", ManZoomToggle? "TRUE" : "FALSE");
	if (ManZoomToggle) oapiWriteScenario_float(scn,  "Zoom", ZoomMap); // Only write zoom if we have manually toggled it
	oapiWriteScenario_int(scn,  "HRad", G->hacRadius);
	oapiWriteScenario_int(scn,  "FDst", G->finalDist);
	oapiWriteScenario_string(scn, "Geom",
		(G->hac_sign==1 && G->hac_open==1)? "L-O" :
		(G->hac_sign==1 && G->hac_open==0)? "L-C" :
		(G->hac_sign==-1 && G->hac_open==1)? "R-O" : "R-C");
	oapiWriteScenario_string(scn, "APlt", G->steerActive? "TRUE" : "FALSE");
	oapiWriteScenario_string(scn, "XTSv", G->exTrSave? "TRUE" : "FALSE");
  oapiWriteScenario_string(scn, "QARc", G->GSqarInit? "TRUE" : "FALSE");
	oapiWriteScenario_string(scn, "Diag", G->diagOn? "TRUE" : "FALSE");
	oapiWriteScenario_string(scn, "Deor", G->deorOn? "TRUE" : "FALSE");
	oapiWriteScenario_string(scn, "GSlp", G->GSfileName[G->GSfile]);
	return;
}

// Read Scanerio data
// ... make sure each parameter is sanity-checked
void GlideslopeMFD::ReadStatus (FILEHANDLE scn) {
  char *line;
  char *elem; 
  char base[128];
	while (oapiReadScenario_nextline (scn, line)) {
    elem = line;
    while (*elem != '\0' && *elem != ' ' && *elem != '\t') elem++;
    if (*elem != '\0') *elem++ = '\0';
    while (*elem == ' ' || *elem == '\t') elem++;
    if (!_stricmp (line, "END_MFD"))
			break;
		
    else if (!_stricmp (line, "Page")) {
			if      (_stricmp(elem,"VSIT")==0)
			  {mode = 1;}
			else if (_stricmp(elem,"TAPE")==0)
			  {mode = 2;}
			else if (_stricmp(elem,"DATA")==0)
			  {mode = 3;}
			else if (_stricmp(elem,"HSIT")==0)
			  {mode = 4;}
			else if (_stricmp(elem,"DIAG")==0)
			  {mode = 5;}
			else if (_stricmp(elem,"DEOR")==0)
			  {mode = 6;}
	    G->coreMode[coreMFDref] = mode;
     	G->coreJumpMode[coreMFDref] = mode;

		} else if (!_stricmp (line, "Unit")) {
			if (!_stricmp(elem,"US")) {
			  units = true;
	     	G->coreUnit[coreMFDref] = units;
			} else if (!_stricmp(elem,"METRIC")) {
			  units = false;
	     	G->coreUnit[coreMFDref] = units;
			}

    } else if (!_stricmp (line, "Base")) {
      strcpy_s(base,elem);

		} else if (!_stricmp (line, "Rway")) {
			for (int i=0;i<G->RunwayCount;i++) {
				if (!_stricmp(base,G->BaseName[G->RunwayBase[i]]) && !_stricmp(elem,G->RunwayName[i])) {
					G->runway = i;
					break;
				}
			}

    } else if (!_stricmp (line, "ManZ")) {
			if (!_stricmp(elem,"TRUE")) {
			  ManZoomToggle = true;
	     	G->coreManZoomToggle[coreMFDref] = ManZoomToggle;
			} else if (!_stricmp(elem,"FALSE")) {
			  ManZoomToggle = false;
	     	G->coreManZoomToggle[coreMFDref] = ManZoomToggle;
			}

		} else if (!_stricmp (line, "Diag")) {
			if (!_stricmp(elem,"TRUE")) {
        G->diagOn = true;
			} else if (!_stricmp(elem,"FALSE")) {
        G->diagOn = false;
			}
      G->maxMod = (G->deorOn)? 6 : (G->diagOn)? 5 : 4;

    } else if (!_stricmp (line, "Deor")) {
			if (!_stricmp(elem,"TRUE")) {
        G->deorOn = true;
			} else if (!_stricmp(elem,"FALSE")) {
        G->deorOn = false;
			}
      G->maxMod = (G->deorOn)? 6 : (G->diagOn)? 5 : 4;

		} else if (!_stricmp (line, "Zoom")) {
			double tmpF;
			if (sscanf_s(elem,"%lf",&tmpF)) {
				ZoomMap = tmpF;
        if(ZoomMap<MinZoomMap)ZoomMap=MinZoomMap;
				G->coreZoom[coreMFDref] = ZoomMap;
			}

		} else if (!_stricmp (line, "HRad")) {
			int tmpI;
			if (sscanf_s(elem,"%i",&tmpI)) {
				G->hacRadius = (tmpI>80000? 80000 : tmpI<1000? 1000 : tmpI);
			}

		} else if (!_stricmp (line, "FDst")) {
			int tmpI;
			if (sscanf_s(elem,"%i",&tmpI)) {
				G->finalDist = (tmpI>50000? 50000 : tmpI<8000? 8000 : tmpI);
			}

		} else if (!_stricmp (line, "Geom")) {
			if (!_stricmp(elem,"L-O")) {
			  G->hac_sign = 1;
			  G->hac_open = 1;
			} else if (!_stricmp(elem,"L-C")) {
			  G->hac_sign = 1;
			  G->hac_open = 0;
			} else if (!_stricmp(elem,"R-O")) {
			  G->hac_sign = -1;
			  G->hac_open = 1;
			} else if (!_stricmp(elem,"R-C")) {
			  G->hac_sign = -1;
			  G->hac_open = 0;
			}

		} else if (!_stricmp (line, "APlt")) {
			if (!_stricmp(elem,"TRUE")) {
			  G->steerActive = true;
        G->VacLandRMode = 0;
        G->VacLandHMode = 0;
			} else if (!_stricmp(elem,"FALSE")) {
			  G->steerActive = false;
			}
      if (G->steerActive && !G->hasAtmo) G->VacLandHMode = G->VacLandRMode = 1; 

    } else if (!_stricmp (line, "XTSv")) {
			if (!_stricmp(elem,"TRUE")) {
			  G->exTrSave = true;
			} else if (!_stricmp(elem,"FALSE")) {
			  G->exTrSave = false;
			}

		} else if (!_stricmp (line, "QARc")) {
			if (!_stricmp(elem,"TRUE")) {
			  G->GSqarInit = true;
			} else if (!_stricmp(elem,"FALSE")) {
			  G->GSqarInit = false;
			}

    } else if (!_stricmp (line, "GSlp")) {
      for (int i=0; i<G->GSfileCount; i++) {
        if (!_stricmp(elem,G->GSfileName[i])) {
          G->GSfile =i;
          LoadPreferredGlideslopeConfig(G->GSfileName[G->GSfile]);
          break;
        }
		  }
    }
  }

  G->calcRunway(); // We may have updated the runway or geometry ... so recalc
  return;
}


int GlideslopeMFD::line( int i ) {
  return (int)floor(((float)i*((float)height/25.0)));
}

int GlideslopeMFD::MapX(double x) {
  return (int)(x*width/ZoomX);
}

int GlideslopeMFD::MapY(double y) {
  return height-(int)(y*height/ZoomY);
}

int GlideslopeMFD::MapX(AZ_RANGE a) {
  double x=a.range*sin(a.az);
  return (int)(x*width/ZoomMap+width/2);
}

int GlideslopeMFD::MapY(AZ_RANGE a) {
  double y=a.range*cos(a.az);
  return (int) (height*5./6.-(int)(y*height/ZoomMap));
}

// Repaint the MFD
void GlideslopeMFD::Update (HDC hDC) {
  char buf[256];
  bool DispRefs;
  if (!G->steerActive) {
    Title (hDC, "Glideslope v2.6");
  } else {
    Title (hDC, "Glideslope v2.6                AUT");
  }
  double SimT=oapiGetSimTime();

  int l=1;
  int maxx,maxy;
  VECTOR3 nre_tc_sc;
  VECTOR3 xyz_tc;
  MATRIX3 M_tc;
  VECTOR3 nre_sc_tc;
  AZ_RANGE ar_tc_sc;
  AZ_RANGE ar_sc_tc;
  AZ_RANGE ar_feature_sc;
  AZ_RANGE ar_feature_wp1;
  AZ_RANGE ar_feature_wp2;
  double turnRadius;
  int pwr = 0;
  char sym = ' ';
  DispRefs = (G->okVac) || ((G->RangeSeg>-1) && (G->Altitude < 130000));
  G->DeorbitActive = (mode==6);

  switch(mode) {
    case 0:
      //Should never get here
      SetTextColor( hDC, RED );
      sprintf(buf,"Mode 0 selected in error");
      TextOut(hDC,5,line(4),buf,strlen(buf));
	  break;
//
// MFD Mode 1 ... Reference glideslope altitude and speed
//
    case 1:
      //Draw reference glideslope (altitude)
      if (!G->steerActive) {
        Title (hDC, "Glideslope v2.6 VSIT");
      } else {
        Title (hDC, "Glideslope v2.6 VSIT           AUT");
      }
      SetTextColor( hDC, YELLOW );
      sprintf(buf,"Alt");
      TextOut(hDC,5,line(4),buf,strlen(buf));
      SetTextColor( hDC, GREEN ); 
      if (!G->okVac) {
        sprintf(buf,"TAS");
      } else {
        sprintf(buf,"GSpd");    // remove verical component of Airspeed for non-atmo land
      }
      TextOut(hDC,5,line(3),buf,strlen(buf));

	    //Draw Ref TAS
	    SelectDefaultPen(hDC,2);
      MoveToEx(hDC,MapX(G->RefSlope[0][2]+G->rwy_length-1000),MapY(G->RefSlope[0][0]/300.0),NULL);
      for(int i=1;i<G->RefSlopePts;i++) {
        LineTo(hDC,MapX(G->RefSlope[i][2]+G->rwy_length-1000),MapY(G->RefSlope[i][0]/300.0));
      }


      //Draw Ref ALT
	    SelectDefaultPen(hDC,3);
      MoveToEx(hDC,MapX(G->RefSlope[0][2]+G->rwy_length-1000),MapY(G->RefSlope[0][3]/5000.0),NULL);
      for(int i=1;i<G->RefSlopePts;i++) {
        LineTo(hDC,MapX(G->RefSlope[i][2]+G->rwy_length-1000),MapY(G->RefSlope[i][3]/5000.0));
      }
      //Draw tracks
      if(G->TrackPtr>0) {
        SelectDefaultPen(hDC,1);
        MoveToEx(hDC,MapX(G->TrackD[0]+G->rwy_length-1000),MapY(G->TrackV[0]/300),NULL);
        for(int i=1;i<G->TrackPtr;i++) {
          LineTo(hDC,MapX(G->TrackD[i]+G->rwy_length-1000),MapY(G->TrackV[i]/300));
        }
        HPEN oldPen=(HPEN)SelectObject(hDC,yellowPen);
        MoveToEx(hDC,MapX(G->TrackD[0]+G->rwy_length-1000),MapY(G->TrackA[0]/5000.0),NULL);
        for(int i=1;i<G->TrackPtr;i++) {
          LineTo(hDC,MapX(G->TrackD[i]+G->rwy_length-1000),MapY(G->TrackA[i]/5000.0));
        }

		    SelectObject(hDC,oldPen);
      }

 //Autozoom
      maxx=MapX(G->TrackD[G->TrackPtr-1]);
  	  if(maxx>width) ZoomX*=2;
      if(maxx<width*9/20) ZoomX/=2;
      if(ZoomX<MinZoomX)ZoomX=MinZoomX;
      maxy=MapY(G->TrackA[G->TrackPtr-1]/5000);
      if(MapY(G->TrackV[G->TrackPtr-1]/300)<maxy)maxy=MapY(G->TrackV[G->TrackPtr-1]/300);
      if(maxy<0)ZoomY*=2;
      if(maxy>height*12/20)ZoomY/=2;
      if(ZoomY<MinZoomY)ZoomY=MinZoomY;
      SetTextColor( hDC, WHITE );
      sprintf(buf,"%s",G->RunwayName[G->runway]);
      TextOut(hDC,5,line(24),buf,strlen(buf));
      sprintf(buf,"%s",G->BaseName[G->RunwayBase[G->runway]]);
      TextOut(hDC,5,line(1),buf,strlen(buf));

// Update bottom r-h corner ... HAC range DelRefTE, DelAz
      if (G->okVac) {
        l = 24; 
		    SetTextColor( hDC, (fabs(G->DelAz*RAD2DEG)< 5.0) ? GREEN : (G->DelAz*RAD2DEG <= -5)? RED : YELLOW );
		    DisplayEngUnit4(buf,"DelAz:%5.1f%c",G->DelAz*RAD2DEG,'°',0);
        TextOut(hDC,width-(14 * cw),line(l),buf,strlen(buf));
        SetTextColor(hDC, WHITE);
		    l--;
			  DisplayEngUnit2(buf,"Rng:%5.1f%c", "Rng:%5.2f%c","m","NMi",1.0/1852.0,units,G->range);
        SetTextColor( hDC, GREEN);
			  TextOut(hDC,width-(12 * cw),line(l),buf,strlen(buf));

      } else if (DispRefs) {
        l = 24; 
		    if (G->leg==1) {
		      SetTextColor( hDC, (fabs(G->DelAz*RAD2DEG)< 5.0) ? GREEN : (G->DelAz*RAD2DEG <= -5)? RED : YELLOW );
		      DisplayEngUnit4(buf,"DelAz:%5.1f%c",G->DelAz*RAD2DEG,'°',0);
          TextOut(hDC,width-(14 * cw),line(l),buf,strlen(buf));
          SetTextColor(hDC, WHITE);
		      l--;
		    }

		    SetTextColor( hDC, ((fabs(G->TE - G->RefTE)< (0.05 * G->RefTE))|| !DispRefs)? GREEN :  (G->TE < G->RefTE)? RED : YELLOW );
        DisplayEngUnit4(buf,"DelRefTE:%5.1f%c",100*(G->TE-G->RefTE)/G->RefTE,'%',0);
        TextOut(hDC,width-(17 * cw),line(l),buf,strlen(buf));
		    l--;

		    if (G->leg==1) {
		      if (G->range_wp1_sc < 200000) {
			    FlashToggle++;
			    if (FlashToggle>5) FlashToggle=-5;		
			    strcpy_s(buf,256,FlashToggle>=0?" *  ":"  * ");
	        SetTextColor( hDC, WHITE);
			    TextOut(hDC,width-(16 * cw),line(l),buf,strlen(buf));
			    DisplayEngUnit2(buf,"HAC:%5.1f%c", "HAC:%5.2f%c","m","NMi",1.0/1852.0,units,G->range_wp1_sc);
                SetTextColor( hDC, GREEN);
			    TextOut(hDC,width-(12 * cw),line(l),buf,strlen(buf));
		      } else {
	            DisplayEngUnit2(buf,"HAC:%5.1f%c", "HAC:%5.2f%c","m","NMi",1.0/1852.0,units,G->range_wp1_sc);
  	            SetTextColor( hDC, GREEN);
                TextOut(hDC,width-(12 * cw),line(l),buf,strlen(buf));
		      }
		    }
		    SetTextColor(hDC, WHITE);

	    } else {
        SetTextColor(hDC, GREEN);
	      DisplayEngUnit2(buf,"TopGS:%5.1f%c", "TopGSC:%5.2f%c","m","NMi",1.0/1852.0,units,G->range-G->RefSlope[0][2]);
        TextOut(hDC,width-(14 * cw),line(24),buf,strlen(buf));
		    SetTextColor(hDC, WHITE);
	    }
      break;
//
// MFD Mode 2 ... Tapes output mode
//
	case 2: // TAPES OUPUT
      if (!G->steerActive) {
        Title (hDC, "Glideslope v2.6 TAPE");
      } else {
        Title (hDC, "Glideslope v2.6 TAPE           AUT");
      }
      SelectDefaultFont(hDC,1);
      sprintf(buf,"%s",G->RunwayName[G->runway]);
      TextOut(hDC,5,line(1),buf,strlen(buf));
      DisplayEngUnit2(buf,"Range:     %6.1f%c","Range:  %9.3f%c","m","NMi",1.0/1852.0,units,G->range);
      TextOut(hDC,5,line(2),buf,strlen(buf));
      sprintf(buf,"DelAz");
      TextOut(hDC,5,line(24),buf,strlen(buf));
      //AOA scale
      VScale(hDC,0,"%4.1f","%4.1f",1.0,G->Refalpha*RAD2DEG,G->alpha*RAD2DEG,0,50,1*width/8,G->Refalpha*RAD2DEG-3,G->Refalpha*RAD2DEG+3 );
      //Airspeed scale
      VTape(hDC,"TAS","m/s","ft/s",500,100,1.0/0.3048,G->RefAirspeed,G->Airspeed,3*width/8,4*width/8-2);
      //Altitude scale
      VTape(hDC,"dTAS","m/s²","ft/s²",10,2,1.0/0.3048,G->RefAirspeedRate,G->AirspeedRate,4*width/8,5*width/8-2);
      //Altitude scale
      VTape(hDC,"ALT","m","ft",5000,1000,1.0/0.3048,G->RefAltitude,G->Altitude,5*width/8,6*width/8-2);
      //VSpd scale
      VTape(hDC,"VSPD","m/s","ft/s",50,10,1.0/0.3048,G->RefVspd,G->vspd,6*width/8,7*width/8-2);
      //VSpd Ratescale
      VTape(hDC,"VACC","m/s²","ft/s²",10,2,1.0/0.3048,G->RefVspdRate,G->VspdRate,7*width/8,8*width/8-2);
      //Energy scale
//      VTape(hDC,"ENE","J","ftlbf",5e5,1e5,0.7375621,G->RefTE,G->TE,6*width/8,7*width/8-2);
      //Energy loss
//      VTape(hDC,"E/W loss","J/s","ftlbf/s",5e3,1e3,0.7375621,fabs(G->RefTERate),fabs(G->TERate),7*width/8,8*width/8-2);
      //Delaz scale
      SelectDefaultFont(hDC,1);
      HScale(hDC,3,"%6.1f°","%6.1f°",1.0,G->DelAz*RAD2DEG,-20,20,line(24),true, -5, 5);
      SelectDefaultFont(hDC,1);
      break;

//
// MFD Mode 3 ... Digital Descent Display mode
//
    case 3: // Digital Descent Display
      if (!G->steerActive) {
        Title (hDC, "Glideslope v2.6 DATA");
      } else {
        Title (hDC, "Glideslope v2.6 DATA           AUT");
      }

      SelectDefaultFont(hDC,0);
      SetTextColor( hDC, WHITE );

      sprintf(buf,"%s",G->BaseName[G->RunwayBase[G->runway]]);
      TextOut(hDC,5,line(1),buf,strlen(buf));
      sprintf(buf,"%s",G->RunwayName[G->runway]);
      TextOut(hDC,5,line(2),buf,strlen(buf));
	    l=4; 
	    sprintf(buf, "          Actual");
      TextOut(hDC,5,line(l),buf,strlen(buf));
      SetTextColor( hDC, WHITE );
	    sprintf(buf, "  Reference");
      TextOut(hDC,mid,line(l),buf,strlen(buf));

	  // Delta Azimuth display
	    l++;
      SetTextColor( hDC, (fabs(G->DelAz*RAD2DEG)<= 5.0) ? GREEN : (G->DelAz*RAD2DEG <= -5)? RED : YELLOW );
      if (G->okVac && G->Altitude<500.0) SetTextColor(hDC, GREEN);
	    DisplayEngUnit3(buf,"DelAz:  %7.2f",G->DelAz*RAD2DEG,&sym,&pwr,0,6);
      TextOut(hDC,5,line(l),buf,strlen(buf));
      SetTextColor( hDC, BLUE );

	    DisplayEngUnit(buf,"  %7.2f    °",0.0);
      TextOut(hDC,mid,line(l),buf,strlen(buf));

	  // Angle of Attack (Alpha) display for atmo landings
      if (!G->okVac) {
	      l++;
        SetTextColor( hDC, (fabs(G->alpha*RAD2DEG - G->Refalpha*RAD2DEG)<= 2.1)? GREEN : (G->alpha*RAD2DEG < G->Refalpha*RAD2DEG)? RED : YELLOW );
	      DisplayEngUnit4(buf,"AoA:    %7.2f",G->alpha*RAD2DEG,' ',0);
        TextOut(hDC,5,line(l),buf,strlen(buf));
        SetTextColor( hDC, BLUE );

	      DisplayEngUnit4(buf,"  %7.2f    °",G->Refalpha*RAD2DEG, ' ', 0);
        TextOut(hDC,mid,line(l),buf,strlen(buf));
        l++;
      }

      // Range display
	    l++;
	    SetTextColor( hDC, GREEN );
	    DisplayEngUnit5(buf,"Range: %8.2f%c","Range:  %8.3f%c","","",1.0/1852.0,units,G->range,0);
      TextOut(hDC,5,line(l),buf,strlen(buf));
      SetTextColor( hDC, BLUE );
	    DisplayEngUnit5(buf," %8.2f%c","  %8.3f%c","   m","  NMi",1.0/1852.0,units,G->range,0);
      TextOut(hDC,mid,line(l),buf,strlen(buf));

      if (!G->okVac) {
	      // Range TO HAC WAYPOINT display only for atmo landings
	      if (G->leg == 1) {
		      l++;
		      SetTextColor( hDC, GREEN );
	        DisplayEngUnit5(buf," ToHAC:%7.1f%c"," ToHAC: %8.3f%c","","",1.0/1852.0,units,G->range_wp1_sc,0);
          TextOut(hDC,5,line(l),buf,strlen(buf));
          SetTextColor( hDC, BLUE );
	        DisplayEngUnit6(buf,"%8.1f%c","  %8.3f%c","    m","  NMi",1.0/1852.0,units,G->range_wp1_sc, 0, DispRefs);
          TextOut(hDC,mid,line(l),buf,strlen(buf));
	      }
	
	      // Energy display only for atmo landings
	      l++; l++;
        SetTextColor( hDC, ((fabs(G->TE - G->RefTE)<= (0.30 * G->RefTE))|| !DispRefs)? GREEN : (G->TE < G->RefTE)? RED : YELLOW );
        DisplayEngUnit2(buf,"Energy:%9.3f%c","Energy:%9.3f%c","","",0.7375621,units,G->TE);
        TextOut(hDC,5,line(l),buf,strlen(buf));
        SetTextColor( hDC, BLUE );

	      DisplayEngUnit6(buf," %9.3f%c","%9.3f%c","  J","   ftlbf",0.7375621,units,G->RefTE, -3, DispRefs);
	      TextOut(hDC,mid,line(l),buf,strlen(buf));

  //sprintf(oapiDebugString(),"TE(%9.6f) PE(%9.6f) KE(%9.6f) GM1(%9.6f) M2(%9.6f) rrwy(%9.6f) Alt(%9.6f) TAS(%9.6f)", 
  //	G->TE, G->PE, G->KE, G->GtimesM1, G->M2, G->rrwy, G->Altitude, G->Airspeed);  

  	    // Delta Energy display only for atmo landings
	      l++;
        SetTextColor( hDC, ((fabs(G->TERate - G->RefTERate)<= fabs(0.5 * G->RefTERate))|| !DispRefs)? GREEN : (G->TERate < G->RefTERate)? RED : YELLOW  );
	      DisplayEngUnit2(buf," dEngy: %6.1f%c"," dEngy: %6.1f%c","","",0.7375621,units,G->TERate);
        TextOut(hDC,5,line(l),buf,strlen(buf));
        SetTextColor( hDC, BLUE );

	      DisplayEngUnit6(buf,"  %6.1f%c","  %6.1f%c","    J/s","  ftlbf/s",0.7375621,units,G->RefTERate, -3, DispRefs);
        TextOut(hDC,mid,line(l),buf,strlen(buf));
	    }
	  
	  // Altitude display
	    l++; l++;
      SetTextColor( hDC, ((fabs(G->Altitude - G->RefAltitude)<= (0.1 * G->RefAltitude))|| !DispRefs)? GREEN : (G->Altitude < G->RefAltitude)? RED : YELLOW);
      if (fabs(G->Altitude - G->RefAltitude) < 10.0) SetTextColor(hDC, GREEN);
	    DisplayEngUnit5(buf,"Alt:    %7.2f%c","Alt:    %7.2f%c","","",1.0/0.3048,units,(G->vacLanded? 0.0 : G->Altitude),0);
      TextOut(hDC,5,line(l),buf,strlen(buf));
      SetTextColor( hDC, BLUE );

	    DisplayEngUnit6(buf,"  %7.2f%c","  %7.2f%c","   m","   ft",1.0/0.3048,units,G->RefAltitude, 0, DispRefs);
      TextOut(hDC,mid,line(l),buf,strlen(buf));
	  
      if (!G->okVac) {
	      // Vertical Speed (Delta Altitude) display
	      l++;
        SetTextColor( hDC, ((fabs(G->vspd - G->RefVspd)<= (0.25 * fabs(G->RefVspd)) || fabs(G->vspd - G->RefVspd)< 40.0)|| !DispRefs)? GREEN : (G->vspd < G->RefVspd)? RED : YELLOW);
	      DisplayEngUnit5(buf," VSpd:  %7.2f%c"," VSpd:  %7.2f%c","","",1.0/0.3048,units,G->vspd,0);
        TextOut(hDC,5,line(l),buf,strlen(buf));
        SetTextColor( hDC, BLUE );

	      DisplayEngUnit6(buf,"  %7.2f%c","  %7.2f%c","   m/s","   ft/s",1.0/0.3048,units,G->RefVspd, 0, DispRefs);
        TextOut(hDC,mid,line(l),buf,strlen(buf));

        // VAcc (Delta VSPD) display
	      l++;
        SetTextColor( hDC, ((fabs(G->VspdRate)<=2.0 )? GREEN : (fabs(G->VspdRate) > 5.0)? RED : YELLOW));
	      DisplayEngUnit5(buf," VAcc:  %7.2f%c"," VAcc:  %7.2f%c","","",1.0/0.3048,units,G->VspdRate,0);
        TextOut(hDC,5,line(l),buf,strlen(buf));
        SetTextColor( hDC, BLUE );

	      DisplayEngUnit6(buf,"  %7.2f%c","  %7.2f%c","   m/s","   ft/s",1.0/0.3048,units,0.0, 0, DispRefs);
        TextOut(hDC,mid,line(l),buf,strlen(buf));

        // True Air Speed display
	      l++; l++;
        SetTextColor( hDC, ((fabs(G->Airspeed - G->RefAirspeed)<= (0.1 * G->RefAirspeed))|| !DispRefs)? GREEN : (G->Airspeed < G->RefAirspeed)? RED : YELLOW);
	      DisplayEngUnit5(buf,"TAS:    %7.2f%c","TAS:    %7.2f%c","","",1.0/0.3048,units,G->Airspeed,0);
        TextOut(hDC,5,line(l),buf,strlen(buf));
        SetTextColor( hDC, BLUE );

	      DisplayEngUnit6(buf,"  %7.2f%c","  %7.2f%c","   m/s","   ft/s",1.0/0.3048,units,G->RefAirspeed, 0, DispRefs);
        TextOut(hDC,mid,line(l),buf,strlen(buf));

  	    // Delta True Air Speed display
	      l++;
        SetTextColor( hDC, ((fabs(G->AirspeedRate - G->RefAirspeedRate)<= (0.25 * fabs(G->RefAirspeedRate)) || fabs(G->AirspeedRate - G->RefAirspeedRate) < 20.0)|| !DispRefs)?
          GREEN : (G->AirspeedRate < G->RefAirspeedRate)? RED : YELLOW);
	      DisplayEngUnit5(buf," Acc:   %7.2f%c"," Acc:   %7.2f%c","","",1.0/0.3048,units,G->AirspeedRate,0);
        TextOut(hDC,5,line(l),buf,strlen(buf));
        SetTextColor( hDC, BLUE );
	      DisplayEngUnit6(buf,"  %7.2f%c","  %7.2f%c","   m/s²","   ft/s²",1.0/0.3048,units,G->RefAirspeedRate, 0, DispRefs);
        TextOut(hDC,mid,line(l),buf,strlen(buf));
        break; // End of digital display for atmo
      }

      // Remainder of digital display for vacuum land only
      // -------------------------------------------------

      if (G->vacLanded) {
        l++; l++;
        SetTextColor(hDC, GREEN);
        sprintf(buf, ">> Landed <<");
        TextOut(hDC, 5, line(l), buf, strlen(buf));
        break;
      }

	    // Vertical Speed (Delta Altitude) display
	    l++;
      SetTextColor( hDC, ((fabs(G->vspd - G->RefVspd)<= (0.25 * fabs(G->RefVspd)) || fabs(G->vspd - G->RefVspd)< 40.0)|| !DispRefs)? GREEN : (G->vspd < G->RefVspd)? RED : YELLOW);
	    DisplayEngUnit5(buf,"VSpd:   %7.2f%c","VSpd:   %7.2f%c","","",1.0/0.3048,units,G->vspd,0);
      TextOut(hDC,5,line(l),buf,strlen(buf));
      SetTextColor( hDC, BLUE );

      if (fabs(G->RefVspd)<=0.01) {
  	    DisplayEngUnit6(buf,"  %7.2f%c","  %7.2f%c","   m/s","   ft/s",1.0/0.3048,units,0, 0, DispRefs);
      } else {
	      DisplayEngUnit6(buf,"  %7.2f%c","  %7.2f%c","   m/s","   ft/s",1.0/0.3048,units,G->RefVspd, 0, DispRefs);
      }
      TextOut(hDC,mid,line(l),buf,strlen(buf));

      // Horizontal Speed display
	    l++;
      SetTextColor( hDC, ((fabs(G->Groundspeed - G->RefAirspeed)<= (0.25 * G->RefAirspeed))|| !DispRefs)? GREEN : (G->Groundspeed > G->RefAirspeed)? RED : YELLOW);
      if (G->Groundspeed < 0.05) SetTextColor(hDC, GREEN);
      DisplayEngUnit5(buf,"HSpd:    %7.3f%c","HSpd:    %7.3f%c","","",1.0/0.3048,units,G->Groundspeed,0);
      TextOut(hDC,5,line(l),buf,strlen(buf));
      SetTextColor( hDC, BLUE );

	    DisplayEngUnit6(buf,"   %7.3f%c","   %7.3f%c","  m/s","  ft/s",1.0/0.3048,units,G->RefAirspeed, 0, DispRefs);
      TextOut(hDC,mid,line(l),buf,strlen(buf));

      // Thrust indicators
	    l++;
      if (G->ThrustOrientation<0.0) {
        // Retro on main
        G->curRetThrP = vessel->GetThrusterGroupLevel(THGROUP_MAIN);
        G->maxRetThr = G->ThMAX[THGROUP_MAIN];
      } else {
        // Retro on retro
        G->curRetThrP = vessel->GetThrusterGroupLevel(THGROUP_RETRO);
        G->maxRetThr = G->ThMAX[THGROUP_RETRO];
      }
      G->curHvrThrP = vessel->GetThrusterGroupLevel(THGROUP_HOVER);
      G->maxHvrThr = G->ThMAX[THGROUP_HOVER];

      G->curRetThr = G->curRetThrP * G->maxRetThr;
      G->curHvrThr = G->curHvrThrP * G->maxHvrThr;

      SetTextColor( hDC, ((fabs(G->curRetThrP - G->retThrP)<= (0.05 * G->retThrP))|| !DispRefs)? GREEN : (G->curRetThrP < G->retThrP)? RED : YELLOW );
      if (G->ThrustOrientation<0.0) {
        DisplayEngUnit5(buf,"MThr:  %9.3f%c","MThr:  %9.3f%c","","",1.0/4.448222,units,G->curRetThrP*G->maxRetThr,0);
      } else {  
        DisplayEngUnit5(buf,"RThr:  %9.3f%c","RThr:  %9.3f%c","","",1.0/4.448222,units,G->curRetThrP*G->maxRetThr,0);
      }
      TextOut(hDC,5,line(l),buf,strlen(buf));
      SetTextColor( hDC, BLUE );

	    DisplayEngUnit6(buf," %9.3f%c"," %9.3f%c","  N","  lbf",1.0/4.448222,units,G->retThrP * G->maxRetThr, 0, DispRefs);
	    TextOut(hDC,mid,line(l),buf,strlen(buf));

      l++;
      SetTextColor( hDC, ((fabs(G->curHvrThrP - G->hvrThrP)<= (0.05 * G->hvrThrP))|| !DispRefs)? GREEN : (G->curHvrThrP < G->hvrThrP)? RED : YELLOW );
      DisplayEngUnit5(buf,"HThr:  %9.3f%c","HThr:  %9.3f%c","","",1.0/4.448222,units,G->curHvrThrP*G->maxHvrThr, 0);
      TextOut(hDC,5,line(l),buf,strlen(buf));
      SetTextColor( hDC, BLUE );
       
	    DisplayEngUnit6(buf," %9.3f%c"," %9.3f%c","  N","  lbf",1.0/4.448222,units,G->hvrThrP * G->maxHvrThr, 0, DispRefs);
	    TextOut(hDC,mid,line(l),buf,strlen(buf));

      l++; l++;
      {
        double maxPYRerr = fabs(G->alphaErr*RAD2DEG);
        if (fabs(G->betaErr*RAD2DEG) > maxPYRerr) maxPYRerr = fabs(G->betaErr*RAD2DEG);
        if (fabs(G->gammaErr*RAD2DEG) > maxPYRerr) maxPYRerr = fabs(G->gammaErr*RAD2DEG);
        SetTextColor( hDC, (maxPYRerr<= 1.1)? GREEN : (maxPYRerr >= 10.1)? RED : YELLOW );
      }
	    strcpy(buf,"DelPYR:");
      TextOut(hDC,5,line(l),buf,strlen(buf));
      SetTextColor( hDC, (fabs(G->alphaErr*RAD2DEG)<= 1.1)? GREEN : (fabs(G->alphaErr*RAD2DEG) > 10.1)? RED : YELLOW );
	    DisplayEngUnit4(buf,"%4.2f%c",G->alphaErr*RAD2DEG,'°',0);
      TextOut(hDC,col1,line(l),buf,strlen(buf));
      SetTextColor( hDC, (fabs(G->betaErr*RAD2DEG)<= 1.1)? GREEN : (fabs(G->betaErr*RAD2DEG) > 10.1)? RED : YELLOW );
	    DisplayEngUnit4(buf,"%4.2f%c",G->betaErr*RAD2DEG,'°',0);
      TextOut(hDC,col2,line(l),buf,strlen(buf));
      SetTextColor( hDC, (fabs(G->gammaErr*RAD2DEG)<= 1.1)? GREEN : (fabs(G->gammaErr*RAD2DEG) > 10.1)? RED : YELLOW );
	    DisplayEngUnit4(buf,"%4.2f%c",G->gammaErr*RAD2DEG,'°',0);
      TextOut(hDC,col3,line(l),buf,strlen(buf));
      l++; l++;

      // Command area: gear, retro, hover warnings, countdown timers
      {
        // Check thrusters are ok
        bool goodThrust = true;
        char eng[6];
        THGROUP_TYPE thGp;
        double simt = oapiGetSimTime();

        if (G->ThrustOrientation < 0.0) {
          // Using MAIN
          strcpy(eng,"MAIN");
          thGp = THGROUP_MAIN;
        } else {
          // Using RETRO
          strcpy(eng,"RETRO");
          thGp = THGROUP_RETRO;
        }
        if (!checkThGroup(thGp)) {
          SetTextColor( hDC, RED);
          sprintf(buf, ">> %s engine unavailable <<", eng);
          TextOut(hDC,5,line(l),buf,strlen(buf));
          l++;
          goodThrust = false;
        }
        if (!checkThGroup(THGROUP_HOVER)) {
          SetTextColor( hDC, RED);
          sprintf(buf, ">> HOVER engine Unavailable <<");
          TextOut(hDC,5,line(l),buf,strlen(buf));
          l++;
          goodThrust = false;
        }
        if (!goodThrust) {
          G->steerActive = false;
          SetTextColor( hDC, RED);
          sprintf(buf, ">> Check engine doors and fuel <<");
          TextOut(hDC,5,line(l),buf,strlen(buf));
          l++;
        }

        if (simt < G->TTod) {
          SetTextColor( hDC, simt + 30.0 < G->TTod ? GREEN : YELLOW);
          sprintf(buf, "%s burn in %.0f secs", eng, G->TTod-simt);
          TextOut(hDC,5,line(l),buf,strlen(buf));
          l++;
        } else if (simt < G->TOvh) {
          SetTextColor( hDC, simt + 30.0 < G->TOvh ? GREEN : YELLOW);
          sprintf(buf, "End %s burn in %.0f secs", eng, G->TOvh-simt);
          TextOut(hDC,5,line(l),buf,strlen(buf));
          l++;
        }
        if (simt < G->THvr) {
          SetTextColor( hDC, simt + 30.0 < G->THvr ? GREEN : YELLOW);
          sprintf(buf, "HOVER burn in %.0f secs", G->THvr-simt);
          TextOut(hDC,5,line(l),buf,strlen(buf));
          l++;
        }

        if (G->steerActive) {
          char rmodestr[4][5] = {"INAC", "ARMD", "RTRO", "PLND"};
          char hmodestr[6][5] = {"INAC","POPA","XBRN","ARMD","DSND","AHLD"};
          SetTextColor( hDC, GREEN);
          l++;
          sprintf(buf, "AP Mode: R=%s H=%s", rmodestr[G->VacLandRMode],hmodestr[G->VacLandHMode]);
          TextOut(hDC,5,line(l),buf,strlen(buf));
          l++;
        } else {
          l++;
          SetTextColor(hDC, RED);
          sprintf(buf, ">> AUT not enabled <<");
          TextOut(hDC, 5, line(l), buf, strlen(buf));
          l++;
        }
      }
      break;

//
// MFD Mode 4 ... Horizontal Status / HAC turn mode
//
    case 4: // Horizontal Status
      if (!G->steerActive) {
        Title (hDC, "Glideslope v2.6 HSIT");
      } else {
        Title (hDC, "Glideslope v2.6 HSIT           AUT");
      }

      SelectDefaultFont(hDC,1);
	    if (G->leg==1) {    // Put up DelAz just on descent phase. On HAC and Final ... no need for this
        HScale(hDC,3,"%6.1f%c°","%6.1f%c°",1.0,G->DelAz*RAD2DEG,-20,20,line(23),true, -5, 5);
      }

	    l=1;
	    if (G->leg==3) {
            sprintf(buf,"%s - %s",G->RunwayName[G->runway], ((int(G->leg)==1)? "Descent" : (int(G->leg)==2)? "On HAC" : "On Final" ));
	    } else {
           sprintf(buf,"%s - %s (HAC:%ik %c-%c)",G->RunwayName[G->runway], ((int(G->leg)==1)? "Descent" : (int(G->leg)==2)? "On HAC" : "On Final" ),
			   G->hacRadius/1000, G->hac_sign==1? 'L' : 'R', G->hac_open==1? 'O' : 'C');
	    }
      TextOut(hDC,5,line(l),buf,strlen(buf));
      l++;



  	  switch (G->leg) {
	    case 1:
//KSC33-Descent (R-O 8K)
//DelAz:  -28.40°    TurnRt: -0.023°/s
//Range:  222.5km    To HAC:    100.5km
//HACAlt:  19.5km

		    SetTextColor( hDC, (fabs(G->DelAz*RAD2DEG)< 5.0)? GREEN : (G->DelAz*RAD2DEG <= -5)? RED : YELLOW);
	      DisplayEngUnit4(buf,"DelAz:%5.2f%c",G->DelAz*RAD2DEG,'°',0);
        TextOut(hDC,5,line(l),buf,strlen(buf));

	      SetTextColor( hDC, GREEN );
	      DisplayEngUnit4(buf,"DelAzRt:%6.3f%c/s",(G->DelAzRate)*RAD2DEG,'°',0);
        TextOut(hDC,mid,line(l),buf,strlen(buf));
        SetTextColor(hDC, GREEN);
//sprintf(oapiDebugString(),"Bearing %6.3f°", G->Bearing*RAD2DEG);  

	      l++;
        DisplayEngUnit2(buf,"Range:%5.1f%c", "Range:%5.3f%c","m","NMi",1.0/1852.0,units,G->range);
        TextOut(hDC,5,line(l),buf,strlen(buf));
        DisplayEngUnit2(buf,"To HAC:%5.1f%c", "To HAC:%5.3f%c","m","NMi",1.0/1852.0,units,G->range_wp1_sc);
        TextOut(hDC,mid,line(l),buf,strlen(buf));
        l++;
        DisplayEngUnit(buf,"HAC Arc:%4.0f%c°",G->hac_arc*RAD2DEG);
        TextOut(hDC,mid,line(l),buf,strlen(buf));
        DisplayEngUnit2(buf,"HACAlt:%4.1f%c", "HACalt:%4.1f%c","m","ft",1.0/0.3048,units,G->opt_hac_alt);
        TextOut(hDC,5,line(l),buf,strlen(buf));
        break;
		
		  case 2:
//HAC Arc: 111.4°    TurnRt: -0.023°/s
//Range:  222.5km    To Final:  100.5km
//Opt Alt: 14.5km    TurnRad: 60.2km

        SetTextColor(hDC, GREEN);
        DisplayEngUnit(buf,"HAC Arc:%4.0f%c°",G->hac_arc*RAD2DEG);
	      TextOut(hDC,5,line(l),buf,strlen(buf));
	      SetTextColor( hDC, GREEN );
	      DisplayEngUnit4(buf,"TurnRt:%6.3f%c/s",(G->BearingRate)*RAD2DEG,'°',0);;
	  		TextOut(hDC,mid,line(l),buf,strlen(buf));
        SetTextColor(hDC, GREEN);			

  			l++;
        DisplayEngUnit2(buf,"Range:%5.1f%c", "Range:%5.3f%c","m","NMi",1.0/1852.0,units,G->range);
        TextOut(hDC,5,line(l),buf,strlen(buf));
        DisplayEngUnit2(buf,"To Final:%5.1f%c", "To Final:%5.3f%c","m","NMi",1.0/1852.0,units,G->arc_range);
        TextOut(hDC,mid,line(l),buf,strlen(buf));
        l++;
        DisplayEngUnit2(buf,"Opt Alt:%4.1f%c", "Opt Alt:%4.1f%c","m","ft",1.0/0.3048,units,G->opt_hac_alt);
        TextOut(hDC,5,line(l),buf,strlen(buf));
        turnRadius=G->Groundspeed/G->BearingRate;
        if (turnRadius<200000) {
          DisplayEngUnit2(buf,"TurnRad:%5.1f%c", "TurnRad:%5.3f%c","m","NMi",1.0/1852.0,units,turnRadius);
        } else {
          strcpy_s(buf,255, "TurnRad:");
        }
        TextOut(hDC,mid,line(l),buf,strlen(buf));
        break;

		  case 3:
//Range:   22.5km    XRange: -735.4m
        DisplayEngUnit2(buf,"Range:%5.1f%c", "Range:%5.3f%c","m","NMi",1.0/1852.0,units,G->range);
        TextOut(hDC,5,line(l),buf,strlen(buf));
	        
  			SetTextColor( hDC, (G->crossrange< 1000)? GREEN : (G->crossrange >= 1000)? YELLOW : RED );
        DisplayEngUnit2(buf,"XRange:%5.1f%c", "XRange:%5.3f%c","m","NMi",1.0/1852.0,units,G->crossrange);
        TextOut(hDC,mid,line(l),buf,strlen(buf));
			  break;
	    }


      if (!ManZoomToggle) {
        if(G->range<ZoomMap*1.5) ZoomMap/=1.1;
        if(G->range>ZoomMap*2) ZoomMap*=1.1;
  	    if(ZoomMap<MinZoomMap)ZoomMap=MinZoomMap;
	    }
	    turnRadius=G->Groundspeed/G->BearingRate;
      ar_tc_sc.range=turnRadius;
      ar_tc_sc.az=G->Bearing+PI/2;
      nre_tc_sc=ar_to_nre(ar_tc_sc,G->rrwy);
      xyz_tc=tmul(G->M_sc,nre_tc_sc);
      M_tc=xyz_to_matrix(xyz_tc);
      nre_sc_tc=mul(M_tc,G->xyz_sc);
      ar_sc_tc=nre_to_ar(nre_sc_tc,G->rrwy);
      SelectDefaultPen(hDC,0);
      for(int i=10;i<=60;i+=10) {
        AZ_RANGE ar_tp_tc=ar_sc_tc;
        ar_tp_tc.az+=G->BearingRate*i;
        VECTOR3 nre_tp_tc=ar_to_nre(ar_tp_tc,G->rrwy);
        VECTOR3 xyz_tp=tmul(M_tc,nre_tp_tc);
        VECTOR3 nre_tp_sc=mul(G->M_sc,xyz_tp);
        AZ_RANGE ar_tp_sc=nre_to_ar(nre_tp_sc,G->rrwy,G->Bearing);
        MoveToEx(hDC,MapX(ar_tp_sc)-4,MapY(ar_tp_sc),NULL);
        LineTo(hDC,MapX(ar_tp_sc)+5,MapY(ar_tp_sc));
        MoveToEx(hDC,MapX(ar_tp_sc),MapY(ar_tp_sc)-4,NULL);
        LineTo(hDC,MapX(ar_tp_sc),MapY(ar_tp_sc)+5);
      }

	    SelectDefaultPen(hDC,2);												// Draw the runway in bright green
      ar_feature_sc=nre_to_ar(G->nre_far_sc,G->rrwy,G->Bearing);
      MoveToEx(hDC,MapX(ar_feature_sc),MapY(ar_feature_sc),NULL);
      ar_feature_sc=nre_to_ar(G->nre_near_sc,G->rrwy,G->Bearing);
      LineTo(hDC,MapX(ar_feature_sc),MapY(ar_feature_sc));
      SelectDefaultPen(hDC,3);												// Draw final from runway to WP2 (exit of HAC)
      ar_feature_sc=nre_to_ar(G->nre_wp2_sc,G->rrwy,G->Bearing);
      LineTo(hDC,MapX(ar_feature_sc),MapY(ar_feature_sc));

	    if (G->hac_open==0) {													// If HAC is open, we fly WP1 to WP2 then down finals, else we do a whole circle. 
        ar_feature_sc=nre_to_ar(G->nre_hac_sc,G->rrwy,G->Bearing);		// Regular code to draw the HAC circle
        Ellipse(hDC,(int) (MapX(ar_feature_sc)-G->radius_hac*width/ZoomMap),
                  (int) (MapY(ar_feature_sc)-G->radius_hac*height/ZoomMap),
                  (int) (MapX(ar_feature_sc)+G->radius_hac*width/ZoomMap),
                  (int) (MapY(ar_feature_sc)+G->radius_hac*height/ZoomMap));
	    } else {
        ar_feature_wp1=nre_to_ar(G->nre_wp1_sc,G->rrwy,G->Bearing);		// Pick up the WP1 point
        ar_feature_wp2=nre_to_ar(G->nre_wp2_sc,G->rrwy,G->Bearing);		// Pick up the WP2 point
	    if (G->hac_sign==1) {
		    SetArcDirection(hDC, AD_COUNTERCLOCKWISE);						// Arc direction anticlockwise per HAC Sign
	    } else {
		    SetArcDirection(hDC, AD_CLOCKWISE);								// Arc direction clockwise per HAC sign
	    }
	    ar_feature_sc=nre_to_ar(G->nre_hac_sc,G->rrwy,G->Bearing);		// Draw a HAC ARC from WP1 to WP2
      Arc(hDC,(int) (MapX(ar_feature_sc)-G->radius_hac*width/ZoomMap),
              (int) (MapY(ar_feature_sc)-G->radius_hac*height/ZoomMap),
              (int) (MapX(ar_feature_sc)+G->radius_hac*width/ZoomMap),
              (int) (MapY(ar_feature_sc)+G->radius_hac*height/ZoomMap),
			    	  MapX(ar_feature_wp1),MapY(ar_feature_wp1),
				      MapX(ar_feature_wp2),MapY(ar_feature_wp2));
	    }

      if(G->leg==1) {
        MoveToEx(hDC,MapX(_AR(0,0)),MapY(_AR(0,0)),NULL);					// Draw bottom of screen to WP1 (entry to HAC)
        ar_feature_sc=nre_to_ar(G->nre_wp1_sc,G->rrwy,G->Bearing);
        LineTo(hDC,MapX(ar_feature_sc),MapY(ar_feature_sc));
      } else {
        //Glideslope scale only shown on HAC and Finals
        VScale(hDC,3,"%4.1f°","%4.1f°",1.0,20.0,atan(G->Altitude/G->range_papi_sc)*RAD2DEG,10,30,int(8.0*width/10), 18.0, 22.0);
	    }
      //Draw track
      if(G->TrackPtr>0) {
        SelectDefaultPen(hDC,1);
        VECTOR3 nre_trackpt_sc=mul(G->M_sc,G->xyz_track[0]);
        ar_feature_sc=nre_to_ar(nre_trackpt_sc,G->rrwy,G->Bearing);

        MoveToEx(hDC,MapX(ar_feature_sc),MapY(ar_feature_sc),NULL);
        for(int i=1;i<G->TrackPtr;i++) {
          nre_trackpt_sc=mul(G->M_sc,G->xyz_track[i]);
          ar_feature_sc=nre_to_ar(nre_trackpt_sc,G->rrwy,G->Bearing);
          LineTo(hDC,MapX(ar_feature_sc),MapY(ar_feature_sc));
        }
      }
      break;

//
// MFD Mode 5 ... Internal diagnostics mode ... Enable by putting INTERNAL DIAG on a new line in your Glideslope.CFG file. 
//
    case 5: // Diagnostics Page
      if (!G->steerActive) {
        Title (hDC, "Glideslope v2.6 DIAG");
      } else {
        Title (hDC, "Glideslope v2.6 DIAG            AUT");
      }
      SetTextColor( hDC, WHITE );
	    l=1;
	    sprintf(buf,"%s %+.3f %+.3f",G->BaseName[G->RunwayBase[G->runway]], G->BaseLL[G->RunwayBase[G->runway]][1],G->BaseLL[G->RunwayBase[G->runway]][0]);
      TextOut(hDC,5,line(l),"v2.6",4);
	    l++;
      TextOut(hDC,5,line(l),buf,strlen(buf));
	    l++;
      sprintf(buf,"%s %i %i %i %i",G->RunwayName[G->runway],
		  int(G->RunwayData[G->RunwayBase[G->runway]][2]),
		  int(G->RunwayData[G->RunwayBase[G->runway]][3]),
		  int(G->RunwayData[G->RunwayBase[G->runway]][4]),
		  int(G->RunwayData[G->RunwayBase[G->runway]][5]));
      TextOut(hDC,5,line(l),buf,strlen(buf));

	    l++;
	    DisplayEngUnit(buf,"Energy:%9.3f%cJ",G->TE);
      TextOut(hDC,5,line(l),buf,strlen(buf));
      SetTextColor( hDC, BLUE );
      DisplayEngUnit(buf,"    %9.3f%cJ",G->RefTE);
      TextOut(hDC,mid,line(l),buf,strlen(buf));
      l++;
      SetTextColor( hDC, WHITE );
      DisplayEngUnit(buf,"Mach:  %9.3f%c",G->Mach);
      TextOut(hDC,5,line(l),buf,strlen(buf));
      l++;
      DisplayEngUnit(buf,"Airspd:%9.3f%cm/s",G->Airspeed);
      TextOut(hDC,5,line(l),buf,strlen(buf));
      DisplayEngUnit(buf,"    %9.3f%ckt",G->Airspeed*3600.0/1852.0);
      TextOut(hDC,mid,line(l),buf,strlen(buf));
      l++;
      DisplayEngUnit(buf,"    %9.3f%cft/s",G->Airspeed/0.3048);
      TextOut(hDC,mid,line(l),buf,strlen(buf));
      l++;
      DisplayEngUnit(buf,"Ispeed:%9.3f%cm/s",G->InertialSpd);
      TextOut(hDC,5,line(l),buf,strlen(buf));
      DisplayEngUnit(buf,"    %9.3f%ckt",G->InertialSpd*3600.0/1852.0);
      TextOut(hDC,mid,line(l),buf,strlen(buf));
      l++;
      DisplayEngUnit(buf,"    %9.3f%cft/s",G->InertialSpd/0.3048);
      TextOut(hDC,mid,line(l),buf,strlen(buf));
      l++;
      DisplayEngUnit(buf,"Range: %9.3f%cm",G->range);
      TextOut(hDC,5,line(l),buf,strlen(buf));
      DisplayEngUnit(buf,"    %9.3f%cNmi",G->range/1852.0);
      TextOut(hDC,mid,line(l),buf,strlen(buf));
      l++;
      DisplayEngUnit(buf,"XRange:%9.3f%cm",G->crossrange);
      TextOut(hDC,5,line(l),buf,strlen(buf));
      DisplayEngUnit(buf,"    %9.3f%cNmi",G->crossrange/1852.0);
      TextOut(hDC,mid,line(l),buf,strlen(buf));
      l++;
      DisplayEngUnit(buf,"Alt:   %9.5f%cm",G->Altitude);
      TextOut(hDC,5,line(l),buf,strlen(buf));
      DisplayEngUnit(buf,"    %9.3f%cft",G->Altitude/0.3048);
      TextOut(hDC,mid,line(l),buf,strlen(buf));
      l++;
      DisplayEngUnit(buf,"    %9.3f%cNmi",G->Altitude/1852.0);
      TextOut(hDC,mid,line(l),buf,strlen(buf));
      l++;
      DisplayEngUnit(buf,"Vspd:  %9.3f%cm/s",G->vspd);
      TextOut(hDC,5,line(l),buf,strlen(buf));
      DisplayEngUnit(buf,"    %9.3f%cft/s",G->vspd/0.3048);
      TextOut(hDC,mid,line(l),buf,strlen(buf));
      l++;
      if(G->hasRefSlope) {
        SetTextColor( hDC, BLUE );
        DisplayEngUnit(buf,"RVspd: %9.3f%cm/s",G->RefVspd);
        TextOut(hDC,5,line(l),buf,strlen(buf));
        DisplayEngUnit(buf,"    %9.3f%cft/s",G->RefVspd/0.3048);
        TextOut(hDC,mid,line(l),buf,strlen(buf));
        SetTextColor( hDC, WHITE );
      }
      l++;
      DisplayEngUnit(buf,"HFlux: %9.3f%cW/cm²",G->HeatFlux/10000.0);
      TextOut(hDC,5,line(l),buf,strlen(buf));
      l++;
      DisplayEngUnit(buf,"DelAz: %9.3f%c°",G->DelAz*180.0/PI);
      TextOut(hDC,5,line(l),buf,strlen(buf));

      l++;
      turnRadius=G->Groundspeed/G->BearingRate;
      DisplayEngUnit(buf,"TurnRadius: %9.3f%cm",turnRadius);
      TextOut(hDC,5,line(l),buf,strlen(buf));
//sprintf(oapiDebugString(),"");
	  break;

//
// MFD Mode 6 ... Deorbit mode
//
    case 6: // Deorbit parameters page
      {
        if (!G->steerActive) {
          Title (hDC, "Glideslope v2.6 DEORBIT");
        }

        // SelectDefaultFont(hDC,1);
        SetTextColor( hDC, WHITE );

        ELEMENTS E;
        ORBITPARAM OP;
        vessel->GetElements(NULL,E,&OP);
        double PeAlt=OP.PeD-G->re;

        l++;
        if(G->AtmDensity>0.002 || PeAlt<=0.0 || G->DeorbitMode == 5) {
          strcpy(buf,"Deorbit burn completed");
          TextOut(hDC,5,line(l),buf,strlen(buf));
          l++;
          strcpy(buf,"Deorbit screen no longer needed");
          TextOut(hDC,5,line(l),buf,strlen(buf));
          l++;
          strcpy(buf,"Press MOD to select new mode");
          TextOut(hDC,5,line(l),buf,strlen(buf));
          l++;
          l++;
          strcpy(buf,"(Disable Prograde AP when aligned)");
          TextOut(hDC,5,line(l),buf,strlen(buf));
          l++;
          G->deorOn = false;
          G->maxMod =(G->diagOn)? 5: 4;
          break;
        } else {
          if (!G->BaseSyncConnected) {
            strcpy(buf,"BaseSync: Disconnected");
            TextOut(hDC,5,line(l),buf,strlen(buf));
            l++; l++;
            SetTextColor( hDC, RED );
            strcpy(buf,"Action: Open BaseSync MFD 3.1+");
            TextOut(hDC,5,line(l),buf,strlen(buf));
            l++;
          } else if (G->BS_trgt->lat != G->base.lat || G->BS_trgt->lon != G->base.lon) {
            strcpy(buf,"BaseSync: Target Mismatch");
            TextOut(hDC,5,line(l),buf,strlen(buf));
            l++; l++;
            SetTextColor( hDC, RED );
            strcpy(buf,"BaseSync Action: TGT GS");
            TextOut(hDC,5,line(l),buf,strlen(buf));
            l++;        
          } else {
            strcpy(buf,"BaseSync: Connected");
            TextOut(hDC,5,line(l),buf,strlen(buf));
            l++; l++;

            if ((G->DeorbitMode==0 || G->DeorbitMode==5) && (G->BS_sol->num > 1 || G->DeorbitDT < -100.0)) {
              l++; l++;
              strcpy(buf,"Waiting for best orbit");
              TextOut(hDC,5,line(l),buf,strlen(buf));
              l++;        
            } else {
              if (!G->BS_mode->deo) {
                strcpy(buf,"Deorbiting this orbit");
                TextOut(hDC,5,line(l),buf,strlen(buf));
                l++; l++;
                SetTextColor( hDC, RED );
                strcpy(buf,"BaseSync Action: Select DEO");
                TextOut(hDC,5,line(l),buf,strlen(buf));
                l++;
              } else {

                sprintf(buf, "Retro Burn In:  %9.3fs", G->DeorbitDT);
                TextOut(hDC,5,line(l),buf,strlen(buf));
                l++;
                sprintf(buf, "Retro Burn dV:  %9.3fm/s", G->DeorbitDV);
                TextOut(hDC,5,line(l),buf,strlen(buf));
                l++; l++;

                if (G->DeorbitExecute) {
                  SetTextColor( hDC, YELLOW );
                  if (G->DeorbitMode==0) {
                    strcpy(buf,"Autoburn: Waiting for T-120s");
                    TextOut(hDC,5,line(l),buf,strlen(buf));
                    l++; 
                  } else if (G->DeorbitMode==1) {
                    strcpy(buf,"Autoburn: Aligning for Burn");
                    TextOut(hDC,5,line(l),buf,strlen(buf));
                    l++; 
                  } else if (G->DeorbitMode==2) {
                    strcpy(buf,"Autoburn: Imminent");
                    TextOut(hDC,5,line(l),buf,strlen(buf));
                    l++; 
                  } else if (G->DeorbitMode==3) {
                    SetTextColor(hDC, RED);
                    strcpy(buf,"Autoburn: Burn Active");
                    TextOut(hDC,5,line(l),buf,strlen(buf));
                    l++; 
                  } else if (G->DeorbitMode==4) {
                    SetTextColor(hDC, RED);
                    strcpy(buf,"Autoburn: Trim Active");
                    TextOut(hDC,5,line(l),buf,strlen(buf));
                    l++; 
                  };
                  l++;
                  SetTextColor( hDC, WHITE );
                }
                l++; 
              }
            }

            if (!G->DeorbitExecute) {
              l++;
              strcpy(buf,"AutoBurn: Off");
              TextOut(hDC,5,line(l),buf,strlen(buf));
              l++; l++;
              SetTextColor( hDC, YELLOW );
              strcpy(buf,"Select AB for AutoBurn");
              TextOut(hDC,5,line(l),buf,strlen(buf));
              l++;
            }
/*          DisplayEngUnit2(buf,"VSpeed @ EI: %9.3f%c","VS@EI: %9.3f%c","m/s","ft/s",1.0/0.308,units,vspdEI);
          TextOut(hDC,5,line(l),buf,strlen(buf));
          l++;
	        DisplayEngUnit4(buf,"Slope  @ EI: %7.2f%c",gsEI*RAD2DEG,'°',0);
          TextOut(hDC,5,line(l),buf,strlen(buf));
          l++; l++;
*/
          }
        }
      }
    break;

//
// MFD Mode 100 ... Status after user slope save
//
    case 100: // Deorbit parameters page
      SetTextColor( hDC, WHITE );
      strcpy(buf,"Glideslope_UserSave.cfg created.");
      TextOut(hDC,5,line(5),buf,strlen(buf));
      strcpy(buf,"Hit OK to return.");
      TextOut(hDC,5,line(8),buf,strlen(buf));
	  break;

//
// MFD Mode 101 ... Config Screen
//
    case 101: // Config Settings Screen
      SetTextColor( hDC, WHITE );

  	  strcpy(buf,"Configuration Settings");
      TextOut(hDC,5,line(l),buf,strlen(buf));
	    l+=2;

	    sprintf(buf,"Base: %s",G->BaseName[G->RunwayBase[G->runway]]);
      TextOut(hDC,5,line(l),buf,strlen(buf));
      l++;
      sprintf(buf, "Rwy:  %s (%.0f%s x %.0f%s)", G->RunwayName[G->runway], (units ? G->RunwayData[G->runway][10] / 0.3048 : G->RunwayData[G->runway][10]), (units ? "ft" : "m"),
        (units ? G->RunwayData[G->runway][9] / 0.3048 : G->RunwayData[G->runway][9]), (units ? "ft" : "m"));
      TextOut(hDC, 5, line(l), buf, strlen(buf));
      l++;
      sprintf(buf,"GS:   %s",G->GSname[G->GSfile]);
      TextOut(hDC,5,line(l),buf,strlen(buf));
	    l+=2;
      sprintf(buf,"Enable Ext Track Save: %s",G->exTrSave?"Yes":"No");
      TextOut(hDC,5,line(l),buf,strlen(buf));
      l++;
      sprintf(buf,"Enable Diags Screen:   %s",G->diagOn?"Yes":"No");
      TextOut(hDC,5,line(l),buf,strlen(buf));
      l++;
      sprintf(buf,"Enable Deorbit Screen: %s",G->deorOn?"Yes":"No");
      TextOut(hDC,5,line(l),buf,strlen(buf));
      l++;
      sprintf(buf,"Enable Quick Access Recorder: %s",G->GSqarInit?"Yes":"No");
      TextOut(hDC,5,line(l),buf,strlen(buf));
      l+=2;
      strcpy(buf,"Hit OK to return.");
      TextOut(hDC,5,line(l),buf,strlen(buf));
	  break;

//
// MFD Mode 102 ... GS Reset
//
    case 102: // Deorbit parameters page
      SetTextColor( hDC, WHITE );
      strcpy(buf,"Glideslope Track Data reset.");
      TextOut(hDC,5,line(5),buf,strlen(buf));
      strcpy(buf,"Hit OK to return.");
      TextOut(hDC,5,line(8),buf,strlen(buf));
	  break;

//
// MFD Mode 103 ... GS Reset
//
    case 103: // Not hooked yet
      SetTextColor( hDC, WHITE );
      strcpy(buf,"Function not hooked up yet!");
      TextOut(hDC,5,line(5),buf,strlen(buf));
      strcpy(buf,"Hit OK to return.");
      TextOut(hDC,5,line(8),buf,strlen(buf));
	  break;

//
// MFD Mode 104 ... Visual Rwy Select
//
    case 104: // Displays runway layouts
      SetTextColor( hDC, WHITE );

	    sprintf(buf,"%s %.1f%c %.1f%c",G->BaseName[G->RunwayBase[G->runway]],
        abs(G->RunwayData[G->runway][0]),
        (G->RunwayData[G->runway][0]>=0.0? 'N' : 'S'),
        abs(G->RunwayData[G->runway][1]),
        (G->RunwayData[G->runway][1]>=0.0? 'E' : 'W'));
      TextOut(hDC,5,line(l),buf,strlen(buf));
      l++;
      sprintf(buf,"%s %.0f%s x %.0f%s",G->RunwayName[G->runway],(units? G->RunwayData[G->runway][10]/0.3048 : G->RunwayData[G->runway][10]),(units? "ft" : "m"),
                                              (units? G->RunwayData[G->runway][9]/0.3048 : G->RunwayData[G->runway][9]),(units? "ft" : "m"));
      TextOut(hDC,5,line(l),buf,strlen(buf));
	    l++;

      COLORREF oldColor = GetDCBrushColor(hDC);

      {
        int minRwy = G->BaseMinRwy[G->RunwayBase[G->runway]];
        int maxRwy = G->BaseMaxRwy[G->RunwayBase[G->runway]];

        double s0 = G->RunwayData[G->runway][2];
        double e0 = G->RunwayData[G->runway][3];
        double s1 = G->RunwayData[G->runway][4];
        double e1 = G->RunwayData[G->runway][5];
        double os = G->RunwayGeometry[G->runway].os;
        double oe = G->RunwayGeometry[G->runway].oe;
        double rngs = abs(1.4* (G->RunwayGeometry[G->runway].ds));
        double rnge = abs(1.4* (G->RunwayGeometry[G->runway].de));

        POINT dp[14];

        if (G->RunwayGeometry[G->runway].type == 'P') { // Pad logic
          rngs = rnge = 1000.0; 
        }
        double scale = width/rnge;
        scale = (scale > height / rngs? height/rngs : scale);



        for (int i = minRwy; i <= maxRwy; i++) {

          if (abs(G->RunwayData[i][5] - G->RunwayData[G->runway][3]) < 1.0 && 
              abs(G->RunwayData[i][3] - G->RunwayData[G->runway][5]) < 1.0 &&
              abs(G->RunwayData[i][2] - G->RunwayData[G->runway][4]) < 1.0 &&
              abs(G->RunwayData[i][4] - G->RunwayData[G->runway][2]) < 1.0) continue; // Don't draw our reverse runway over ours

          if (i==G->runway) {
            SelectDefaultPen(hDC,2);
          } else {
      	    SelectDefaultPen(hDC,5);
          }
          if (G->RunwayGeometry[i].type == 'P') {
            for (int j=0; j<2;j++) {
              dp[j].x = long((G->RunwayGeometry[i].p[j].x - oe + G->RunwayBias.x) * scale * G->RunwayBias.s + width/2 + 0.5);
              dp[j].y = long((G->RunwayGeometry[i].p[j].y - os + G->RunwayBias.y) * scale * G->RunwayBias.s + height/2 + 0.5);
            }
            Ellipse(hDC,dp[0].x,dp[0].y,dp[1].x,dp[1].y);
          } else {
            for (int j=0; j<14;j++) {
              dp[j].x = long((G->RunwayGeometry[i].p[j].x - oe + G->RunwayBias.x) * scale * G->RunwayBias.s + width/2 + 0.5);
              dp[j].y = long((G->RunwayGeometry[i].p[j].y - os + G->RunwayBias.y) * scale * G->RunwayBias.s + height/2 + 0.5);
            }
            Polygon(hDC,dp,5);
            if (i==G->runway) {
              Polyline(hDC,&(dp[5]),3);
              Polyline(hDC,&(dp[8]),3);
              Polyline(hDC,&(dp[11]),3);
            }
          }
        }
      }
      SetDCBrushColor(hDC,oldColor);
	  break;

  }
}

//
// Check Thruster Group is ready
//
bool GlideslopeMFD::checkThGroup(THGROUP_TYPE thGp) {
  int thc = vessel->GetGroupThrusterCount(thGp);
  THRUSTER_HANDLE th;
  PROPELLANT_HANDLE ph;
  double thr = 0.0;
  if (thc<1) return false;
  for (int i=0; i<thc; i++) {
    th = vessel->GetGroupThruster(thGp,i);
    ph = vessel->GetThrusterResource(th);
    if (ph == NULL) return false;
    if (vessel->GetPropellantMass(ph) == 0.0) return false;
    thr += vessel->GetThrusterMax0(th);
  }
  if (thr == 0.0) return false;
  return true;
}

//
// VTape ... Vertical Tape Display helper Function
//
void GlideslopeMFD::VTape(HDC hDC,char *title,char *metUnit,char* usUnit,int radius, int minor,double usFactor,double ref,double act,int x1, int x2) {
  char actbuf[256];
  char refbuf[256];
  char buf[256];
  int y;
  int title_pos=line(4);
  int top=line(5);
  int bot=line(21);
  char sym = '?';
  int pwr = -99;


  // sprintf(oapiDebugString(),"cw %i ch %i",cw, ch);


  HGDIOBJ oldBrush=SelectObject(hDC,dkblueBrush);
  HGDIOBJ oldPen=SelectObject(hDC,whitePen);
  Rectangle(hDC,x1,top,x2,bot);
  double ybot,ytop;
  if(!units) {
    ybot=act-radius;
    ytop=act+radius;
	if (act>ref) {
    DisplayEngUnit3(actbuf,"%4.1f%c",act,&sym,&pwr,0,15);
	  DisplayEngUnit4(refbuf,"%4.1f%c",ref,sym,pwr);
	} else {
    DisplayEngUnit3(refbuf,"%4.1f%c",ref,&sym,&pwr,0,15);
	  DisplayEngUnit4(actbuf,"%4.1f%c",act,sym,pwr);
	}
	sprintf(buf, "%c%s", sym, metUnit);
    TextOut(hDC,x1,bot,buf,strlen(buf));
    y=(int) linterp(ybot,bot,ytop,top,ref, true);
  } else {
    ybot=act*usFactor-radius;
    ytop=act*usFactor+radius;
	if (act>ref) {
      DisplayEngUnit3(actbuf,"%4.1f%c",act*usFactor,&sym,&pwr,0,15);
      DisplayEngUnit4(refbuf,"%4.1f%c",ref*usFactor,sym,pwr);
	} else {
      DisplayEngUnit3(refbuf,"%4.1f%c",ref*usFactor,&sym,&pwr,0,15);
      DisplayEngUnit4(actbuf,"%4.1f%c",act*usFactor,sym,pwr);
	}
	sprintf(buf, "%c%s", sym, usUnit);
    TextOut(hDC,x1,bot,buf,strlen(buf));
    y=(int) linterp(ybot,bot,ytop,top,ref*usFactor, true);
  }
  if(y<(top+5))y=top+5;
  if(y>(bot-5))y=bot+8-ch;
  double firstlabel=floor(ybot/minor)*minor;
  double lastlabel=(ceil(ytop/minor)+1)*minor;
  MoveToEx(hDC,x1,y,NULL);
  LineTo(hDC,x2,y);
  TextOut(hDC,x1-2,title_pos,title,strlen(title));
  HRGN hrgn = CreateRectRgn(x1+1,top+1,x2-1,bot-1);
  SelectClipRgn(hDC, hrgn);
  for(double i=firstlabel;i<lastlabel;i+=minor) {
    SetTextColor( hDC, (i>=ref)? WHITE : RED);
    DisplayEngUnit4(buf,"%4.1f%c",i,sym,pwr);
    int yy=(int) linterp(ybot,bot,ytop,top,i, true);
    TextOut(hDC,x1,yy-6,buf,strlen(buf));
  }
  SelectClipRgn(hDC,NULL);
  SelectObject(hDC,blueBrush);
  Rectangle(hDC,x1,y-6,x2,y+ch-3);   // ch is the default char height for the panel (it gets resized according to ship and F8 mode)
  SelectClipRgn(hDC, hrgn);
  SetTextColor(hDC,WHITE);
  TextOut(hDC,x1,y-6,refbuf,strlen(refbuf));
  SelectClipRgn(hDC,NULL);
  Rectangle(hDC,x1,(bot+top)/2-6,x2,(bot+top)/2+ch-3);
  SelectClipRgn(hDC, hrgn);

  SetTextColor( hDC, ((!units) && (fabs(act-ref)*usFactor< minor))? GREEN : ((units) && (fabs(act-ref)*usFactor < minor)) ? GREEN : (act<ref)? RED : WHITE);
  TextOut(hDC,x1,(bot+top)/2-6,actbuf,strlen(actbuf));
  SelectClipRgn(hDC,NULL);
  DeleteObject(hrgn);
  SetTextColor(hDC,WHITE);
  SelectObject(hDC,oldBrush);
  SelectObject(hDC,oldPen);
}

void GlideslopeMFD::VScale(HDC hDC,int pen,char* metFmt,char* usFmt,double usFactor, double ref, double act, double min, double max, int x, double lowarn, double hiwarn) {
  char buf[256];
  int y;
  SelectDefaultPen(hDC,pen);
  int top=line(5);
  int bot=line(21);
  MoveToEx(hDC,x-5,top,NULL);
  LineTo(hDC,x+5,top);
  MoveToEx(hDC,x-5,bot,NULL);
  LineTo(hDC,x+5,bot);
  MoveToEx(hDC,x,top,NULL);
  LineTo(hDC,x,bot);
  y=(int) linterp(min,bot,max,top,ref, true);
  MoveToEx(hDC,x-5,y-5,NULL);
  LineTo(hDC,x,y);
  LineTo(hDC,x-5,y+5);
  if(!units) {
    DisplayEngUnit4(buf,metFmt,ref, ' ', 0);
  } else {
    DisplayEngUnit4(buf,usFmt,ref*usFactor, ' ', 0);
  }
  UINT OldAlign=SetTextAlign(hDC,TA_TOP+TA_RIGHT);
  SetTextColor( hDC, BLUE );
  TextOut(hDC,x-5,y-5,buf,strlen(buf));
  y=(int) linterp(min,bot,max,top,act, true);

//sprintf(oapiDebugString(),"Vscale ref: x1(%4.1f) x2(%4.1f), y1(%4.1f) y2(%4.1f), low(%4.1f) hi(%4.1f) x(%4.1f) ==>y(%4.1f)", min, max, bot, top, lowarn, hiwarn, act, y);  

  MoveToEx(hDC,x+5,y-5,NULL);
  LineTo(hDC,x,y);
  LineTo(hDC,x+5,y+5);
  if(!units) {
    DisplayEngUnit4(buf,metFmt,act, ' ', 0);
  } else {
    DisplayEngUnit4(buf,usFmt,act*usFactor, ' ', 0);
  }
  SetTextAlign(hDC,TA_TOP+TA_LEFT);
  SetTextColor( hDC, ((act<=lowarn) ? RED : (act>=hiwarn) ? WHITE : GREEN ));
  TextOut(hDC,x+5,y-5,buf,strlen(buf));
  SetTextColor( hDC, WHITE );
  SetTextAlign(hDC,OldAlign);
}

void GlideslopeMFD::HScale(HDC hDC,int pen,char* metFmt,char* usFmt,double usFactor, double act, double min, double max, int y, bool top, double lowarn, double hiwarn) {
  char buf[256];
  int x;
  SelectDefaultPen(hDC,pen);
  MoveToEx(hDC,5,y-5,NULL);
  LineTo(hDC,5,y+5);
  MoveToEx(hDC,width-5,y-5,NULL);
  LineTo(hDC,width-5,y+5);
  MoveToEx(hDC,5,y,NULL);
  LineTo(hDC,width-5,y);
  x=(int) linterp(min,5,max,width-5,0, true);
  MoveToEx(hDC,x,y-5,NULL);
  LineTo(hDC,x,y+5);
  x=(int) linterp(min,5,max,width-5,act, true);
  if(x>width-5) {
    SetTextColor(hDC,RED);
    x=width-5;
  }
  if(x<5){
    x=5;
    SetTextColor(hDC,RED);
  }
  MoveToEx(hDC,x-5,y+(top?1:-1)*5,NULL);
  LineTo(hDC,x,y);
  LineTo(hDC,x+5,y+(top?1:-1)*5);
  if(!units) {
    DisplayEngUnit4(buf,metFmt,act, ' ', 0);
  } else {
    DisplayEngUnit4(buf,usFmt,act*usFactor, ' ', 0);
  }
  UINT OldAlign=SetTextAlign(hDC,(top?TA_TOP:TA_BOTTOM)+TA_LEFT);
  SetTextColor(hDC,((act <= lowarn)? RED : (act>= hiwarn) ? YELLOW : GREEN));
  TextOut(hDC,x-5,height-ch,buf,strlen(buf));
//sprintf(oapiDebugString(),"height: %i, Charheight %i, line24: %i", height, ch, line(24));

  SetTextAlign(hDC,OldAlign);
  SetTextColor(hDC,WHITE);
}

void GlideslopeMFD::getMainThrustParm(VESSEL* vessel, double *F, double *isp) {
  *F = 0;
  *isp = 0;
  int nthr=vessel->GetGroupThrusterCount(THGROUP_MAIN);
  for(int i=0; i<nthr; i++) {
    THRUSTER_HANDLE th=vessel->GetGroupThruster(THGROUP_MAIN,i);
    *F += vessel->GetThrusterMax0(th);
    *isp += vessel->GetThrusterIsp(th) * vessel->GetThrusterMax0(th);
  }
  *isp /= *F;
}





bool GlideslopeMFD::ConsumeKeyBuffered (DWORD key) {
  return G->coreButtons[coreMFDref].ConsumeKeyBuffered(this, key);
}

char *GlideslopeMFD::ButtonLabel (int bt) {
  return G->coreButtons[coreMFDref].ButtonLabel(bt);
}

int GlideslopeMFD::ButtonMenu (const MFDBUTTONMENU **menu) const
{
  return  G->coreButtons[coreMFDref].ButtonMenu(menu);
}

bool GlideslopeMFD::ConsumeButton (int bt, int event) {
  return G->coreButtons[coreMFDref].ConsumeButton(this, bt, event);
}

//
// Button Handling Code
//
  // Button handlers
  void GlideslopeMFD::menuConfig(){
 	  jumpmode = mode;
	  mode = 101;
	  G->coreMode[coreMFDref] = mode;
	  G->coreJumpMode[coreMFDref] = jumpmode;
	  G->coreSwitchPage[coreMFDref] = 1;
    G->coreButtons[coreMFDref].SwitchPage(this, 1);
    return;
  };

  void GlideslopeMFD::menuViewBase(){
// 	  jumpmode = mode;    // We jump back to CFG's previous page, not to CFG
	  mode = 104;
	  G->coreMode[coreMFDref] = mode;
//	  G->coreJumpMode[coreMFDref] = jumpmode;
	  G->coreSwitchPage[coreMFDref] = 4;
    G->coreButtons[coreMFDref].SwitchPage(this, 4);
    menuViewBaseReset();
    return;
  };

  void GlideslopeMFD::menuViewBaseReset() {
    G->RunwayBias.x = 0.0;
    G->RunwayBias.y = 0.0;
    G->RunwayBias.s = 1.0;
  };

  void GlideslopeMFD::menuViewBaseZoomOut() { G->RunwayBias.s *= 0.80; };
  void GlideslopeMFD::menuViewBaseZoomIn()  { G->RunwayBias.s *= 1.25; };
  void GlideslopeMFD::menuViewBaseUp()      { G->RunwayBias.y -= 100; };
  void GlideslopeMFD::menuViewBaseDown()    { G->RunwayBias.y += 100; };
  void GlideslopeMFD::menuViewBaseLeft()    { G->RunwayBias.x -= 100; };
  void GlideslopeMFD::menuViewBaseRight()   { G->RunwayBias.x += 100; };

  void GlideslopeMFD::menuOK(){
	  mode = jumpmode;
	  G->coreMode[coreMFDref] = mode;
	  G->coreJumpMode[coreMFDref] = mode;
	  G->coreSwitchPage[coreMFDref] = 0;
    G->coreButtons[coreMFDref].SwitchPage(this, 0);
    return;
  };

  void GlideslopeMFD::menuMode(){
    mode++;
    if(mode>G->maxMod) {
      mode =1;
    }
    if ((mode==5)&&(!G->diagOn)) mode =6;
	  G->coreMode[coreMFDref] = mode;

    if (mode==6) {
	    G->coreSwitchPage[coreMFDref] = 3;
      G->coreButtons[coreMFDref].SwitchPage(this, 3);
    } else {
  	  G->coreSwitchPage[coreMFDref] = 0;
      G->coreButtons[coreMFDref].SwitchPage(this, 0);
    }
    return;
  };

  void GlideslopeMFD::menuUnits(){
    units=!units;
  	G->coreUnit[coreMFDref] = units;
    return;
  };

  void GlideslopeMFD::menuAutopilot(){
    G->steerActive=!G->steerActive;
    //Shut them off here, but just once so that other autopilots can still play
    vessel->SetThrusterGroupLevel(THGROUP_ATT_YAWLEFT,0);
    vessel->SetThrusterGroupLevel(THGROUP_ATT_YAWRIGHT,0);
    vessel->SetThrusterGroupLevel(THGROUP_ATT_PITCHUP,0);
    vessel->SetThrusterGroupLevel(THGROUP_ATT_PITCHDOWN,0);
    vessel->SetThrusterGroupLevel(THGROUP_ATT_BANKLEFT,0);
    vessel->SetThrusterGroupLevel(THGROUP_ATT_BANKRIGHT,0);
    vessel->SetThrusterGroupLevel(THGROUP_ATT_LEFT,0);
    vessel->SetThrusterGroupLevel(THGROUP_ATT_RIGHT,0);
    vessel->SetThrusterGroupLevel(THGROUP_ATT_UP,0);
    vessel->SetThrusterGroupLevel(THGROUP_ATT_DOWN,0);
    vessel->SetThrusterGroupLevel(THGROUP_ATT_FORWARD,0);
    vessel->SetThrusterGroupLevel(THGROUP_ATT_BANKRIGHT,0);
    if (G->steerActive && !G->hasAtmo) G->VacLandHMode = G->VacLandRMode = 0; 
    return;
  };

  void GlideslopeMFD::menuClearGS(){
    G->TrackPtr = 0;
 	  jumpmode = mode;
	  mode = 102;
	  G->coreMode[coreMFDref] = mode;
    G->coreSwitchPage[coreMFDref] = 2;
    G->coreButtons[coreMFDref].SwitchPage(this, 2);
    return;
  };
  
  void GlideslopeMFD::menuSaveGS(){
  	jumpmode = mode;
	  mode = 100;
  	G->coreMode[coreMFDref] = mode;
    G->saveUserGS(units,G->exTrSave);
 	  G->coreSwitchPage[coreMFDref] = 2;
    G->coreButtons[coreMFDref].SwitchPage(this, 2);
    return;
  };

  void GlideslopeMFD::menuHACgeometry(){
	  if (G->hac_open == 1) {
		  G->hac_sign=-G->hac_sign;
		  G->hac_open = 0;
	  } else {
		  G->hac_open = 1;
	  }
    G->calcRunway();
    return;
  };

  void GlideslopeMFD::menuResetHAC(){
	  ManZoomToggle = false;
   	G->coreManZoomToggle[coreMFDref] = ManZoomToggle;
	  G->hacRadius = 15000;
	  G->finalDist=25000;
    G->hac_sign=1;
    if (!G->hasAtmo) {
		  G->hacRadius = 0;
  	  G->finalDist = 0;
    }
    G->calcRunway();
    return;
  };

  void GlideslopeMFD::menuHACRadUp(){
    if (G->hasAtmo) {
      if (G->hacRadius<45000) {		// Max 3x default, else reset to default. 
		    G->hacRadius += 500;
  	    G->finalDist += 800;
	    } else {
	      G->hacRadius = 15000;
	      G->finalDist=25000;
	    }
    } else {
		  G->hacRadius = 0;
  	  G->finalDist = 0;
    }
    G->calcRunway();
    return;
  };

  void GlideslopeMFD::menuHACRadDn(){
    if (G->hasAtmo) {
      if (G->hacRadius>500) G->hacRadius -= 500;	// Smallest is a 500m HAC !! 
	    if (G->finalDist>8000) G->finalDist -= 800;	// Set a minimum final run of 8km
    } else {
		  G->hacRadius = 0;
  	  G->finalDist = 0;
    }
    G->calcRunway();
    return;
  };

  void GlideslopeMFD::menuZoomUp(){
  	ManZoomToggle = true;
	  ZoomMap/=1.1;
    if(ZoomMap<MinZoomMap)ZoomMap=MinZoomMap;
   	G->coreManZoomToggle[coreMFDref] = ManZoomToggle;
   	G->coreZoom[coreMFDref] = ZoomMap;
    return;
  };

  void GlideslopeMFD::menuZoomDn(){
    ManZoomToggle = true;
    ZoomMap*=1.1;
   	G->coreManZoomToggle[coreMFDref] = ManZoomToggle;
   	G->coreZoom[coreMFDref] = ZoomMap;
    return;
  };

  void GlideslopeMFD::menuNextBase(){
    int tmpRwy = G->runway;
    char tmpBase[32];
    strcpy_s(tmpBase,32,G->BaseName[G->RunwayBase[G->runway]]);
    do {
      G->runway++;
      if(G->runway>=G->RunwayCount) G->runway=1;
      if (_stricmp(tmpBase, G->BaseName[G->RunwayBase[G->runway]]) && (!_stricmp(G->base.ref, G->BasePlanet[G->RunwayBase[G->runway]])) && G->BaseMinRwy[G->RunwayBase[G->runway]]>=0) {break;} // Base changed and not an empty runway? If same, try again
    } while (tmpRwy != G->runway);                                         // Loop until we get back to our current runway (in case there is only 1 base)
    menuResetHAC();
    G->calcRunway();
    return;
  };
  void GlideslopeMFD::menuPrevBase(){
    int tmpRwy = G->runway;
    char tmpBase[32];
    strcpy_s(tmpBase,32,G->BaseName[G->RunwayBase[G->runway]]);
    do {
      G->runway--;
      if(G->runway<=0) G->runway=G->RunwayCount-1;
      if (_stricmp(tmpBase, G->BaseName[G->RunwayBase[G->runway]]) && (!_stricmp(G->base.ref, G->BasePlanet[G->RunwayBase[G->runway]])) && G->BaseMinRwy[G->RunwayBase[G->runway]]>=0) {break;} // Base changed? If same, try again
    } while (tmpRwy != G->runway);                                         // Loop until we get back to our current runway (in case there is only 1 base)
    menuResetHAC();
    G->calcRunway();
    return;
  };

  void GlideslopeMFD::menuNextRwy(){
    int tmpRwy = G->runway;
    char tmpBase[32];
    strcpy_s(tmpBase,32,G->BaseName[G->RunwayBase[G->runway]]);
    do {
      G->runway++;
      if(G->runway>=G->RunwayCount) G->runway=0;
      if (!_stricmp(tmpBase, G->BaseName[G->RunwayBase[G->runway]])) {break;} // Base same? We are done. If not - iterate.
    } while (tmpRwy != G->runway);                                         // Loop until we get back to our current runway (in case there is only 1 rwy)
    menuResetHAC();
    G->calcRunway();
    return;
  };

  void GlideslopeMFD::menuPrevRwy(){
    int tmpRwy = G->runway;
    char tmpBase[32];
    strcpy_s(tmpBase,32,G->BaseName[G->RunwayBase[G->runway]]);
    do {
      G->runway--;
      if(G->runway<0) G->runway=G->RunwayCount -1;
      if (!_stricmp(tmpBase, G->BaseName[G->RunwayBase[G->runway]])) {break;} // Base same? We are done. If not - iterate.
    } while (tmpRwy != G->runway);                                         // Loop until we get back to our current runway (in case there is only 1 rwy)
    menuResetHAC();
    G->calcRunway();
    return;
  };

  void GlideslopeMFD::menuNextGS(){
    if (G->GSfileCount == 0) {
      return;                     // Using Internal GS - nothing to change
    }
    G->GSfile++;
    if (G->GSfile >= G->GSfileCount) G->GSfile = 0;
    LoadPreferredGlideslopeConfig(G->GSfileName[G->GSfile]);
    return;
  };
  void GlideslopeMFD::menuPrevGS(){
    if (G->GSfileCount == 0) {
      return;                     // Using Internal GS - nothing to change
    }
    G->GSfile--;
    if (G->GSfile < 0) G->GSfile = G->GSfileCount -1;
    LoadPreferredGlideslopeConfig(G->GSfileName[G->GSfile]);
    return;
  };
  void GlideslopeMFD::menuDiagSel(){
    G->diagOn = !G->diagOn;
    G->maxMod = (G->deorOn)? 6 : (G->diagOn)? 5 : 4;
    return;
  };
  void GlideslopeMFD::menuDeorbitSel(){
    G->deorOn = !G->deorOn;
    if (G->deorOn) {
      G->maxMod = 6;
    } else {
      G->maxMod = (G->diagOn)? 5 : 4;
    }
    return;
  };

  void GlideslopeMFD::menuDeorAB(){
    G->DeorbitExecute = !G->DeorbitExecute;
    if (!G->DeorbitExecute) {
      G->DeorbitMode = 0;
      G->DeorbitFactor = 1.0;
      G->vessel->SetThrusterGroupLevel(THGROUP_MAIN, 0.0);
      G->vessel->SetThrusterGroupLevel(THGROUP_ATT_FORWARD, 0.0);
      G->vessel->SetThrusterGroupLevel(THGROUP_ATT_BACK, 0.0);
      G->vessel->DeactivateNavmode(NAVMODE_PROGRADE);
      G->vessel->DeactivateNavmode(NAVMODE_RETROGRADE);
      if (G->DeoQarOn) {
        fclose(G->DeoQar);
        G->DeoQarOn = false;
      }
    }
    return;
  };

  void GlideslopeMFD::menuExtTrackSave() {
    G->exTrSave = !G->exTrSave;
    return;
  };
  void GlideslopeMFD::menuQuickAccessRecorder() {
    G->GSqarInit = !G->GSqarInit;
    return;
  };

//
//
//  Configuration Load & Parse Code
//
//
void GlideslopeMFD::LoadConfig () {
  FILE* GSCFG;
//  int scancount;
  char buf[256];
  char bufcpy[256];
  buf[255] = '\0';
  char *tok=buf;
  char *strtok=buf;
  char *strtok2=buf;
  double tokf[10];
  int j;
  int ofs;
  int modeTok[3];
  bool unitstok = false;

  char *bp;
  bool prefsFound = false, runwayFound = false, baseFound = false, gsFound = false;

  GSCFG = fopen(".\\Config\\MFD\\Glideslope\\Glideslope.cfg","r");
  if (GSCFG == NULL) {
// Defaults to init code
	  LoadDefaultConfig();
	  return;
  }

  G->maxMod=4;
  G->hacRadius = 15000;
  G->finalDist = 20000;
  G->BaseCount = 0;
  G->RunwayCount = 0;
  G->GSfileCount = 0;

// Default Base/Runway so we always have a target for every location (even if it's landing on the sun!)
  G->hBasePlanet[G->BaseCount] = G->vessel->GetGravityRef();
  oapiGetObjectName(G->hBasePlanet[G->BaseCount], G->BasePlanet[G->BaseCount], 32);
  sprintf(G->BaseName[G->BaseCount],"Surface");
	G->BaseLL[G->BaseCount][0] = 0.0;
	G->BaseLL[G->BaseCount][1] = 0.0;
  G->BaseAlt[G->BaseCount] = 0.0;
  G->BaseMinRwy[G->BaseCount] = 0;
  G->BaseMaxRwy[G->BaseCount] = 0;
  sprintf(G->RunwayName[G->RunwayCount],"Surface");
  for (int i=0; i<12; i++) G->RunwayData[G->RunwayCount][i] = 0.0;
  G->RunwayData[G->RunwayCount][9] = 50.0;
  G->RunwayData[G->RunwayCount][10] = 50.0;
  G->RunwayBase[G->RunwayCount] = 0;
  G->BaseCount++;
  G->RunwayCount++;


  FILE* GSCFGPL=fopen(".\\Config\\MFD\\Glideslope\\Logs\\GScfgParselog.txt","w");
  fprintf(GSCFGPL,"Successfully found and opened Glidselope.cfg\n");

  while (fgets(buf,255,GSCFG)!=NULL) {
	  bp=buf;
	  sprintf(bufcpy,"%s",buf);
	  while (bp[0]==' ' || bp[0]=='\t') bp++;
	  if ((bp[0]=='\0')||(bp[0]=='\n')) {
      fprintf(GSCFGPL,"NUL> %s", buf);
	    continue;
	  }
	  if (bp[0]==';') {
      fprintf(GSCFGPL,"REM> %s", buf);
	    continue;
	  }

	  if (!ParseString(&bp,&tok)) goto badparse;

	  if (_stricmp(tok,"BASE")==0)
	  {
// Parsing BASE "planet" "base name" lon lat
// e.g.    BASE "Earth" "Cape Canaveral" -80.675 +28.5208

	    if (!ParseQuotedString(&bp,&strtok)) goto badparse;
	    sprintf(G->BasePlanet[G->BaseCount],strtok);
      G->hBasePlanet[G->BaseCount] = oapiGetGbodyByName(G->BasePlanet[G->BaseCount]);

	    if (!ParseQuotedString(&bp,&strtok)) goto badparse;
	    sprintf(G->BaseName[G->BaseCount],strtok);
      
      if (strlen(strtok)>32) goto badparse;
	    for (j=0; j<2; j++) {
	      if (!ParseDouble(&bp,&(tokf[j]))) goto badparse;
	    }
	    if (G->BaseCount >= G->BASE_LIMIT) goto badparse;

	    G->BaseLL[G->BaseCount][0] = tokf[1];
	    G->BaseLL[G->BaseCount][1] = tokf[0];
      OBJHANDLE hPlanet = oapiGetGbodyByName(G->BasePlanet[G->BaseCount]);
      G->BaseAlt[G->BaseCount] = oapiSurfaceElevation(hPlanet, G->BaseLL[G->BaseCount][1]*PI/180.0, G->BaseLL[G->BaseCount][0]*PI/180.0);
      G->BaseMinRwy[G->BaseCount] = -1;
      G->BaseMaxRwy[G->BaseCount] = -1;
	    fprintf(GSCFGPL,"BASE LOADED: %s (on %s) Lon:%.3f Lat:%.3f Alt:%.3f\n", G->BaseName[G->BaseCount],G->BasePlanet[G->BaseCount],G->BaseLL[G->BaseCount][1],G->BaseLL[G->BaseCount][0], G->BaseAlt[G->BaseCount]);
	    baseFound = true;
	    G->BaseCount++;
	    continue;
	  }

	  if (_stricmp(tok,"RUNWAY")==0)
	  {
// Parsing RUNWAY "{basename}" "{rwyname}" {end1-1} {end1-3} {end2-1} {end2-3} {papi} {vasi} {alt} {wid} {len} {angle}
// e.g.    RUNWAY "Cape Canaveral" "RWY33" -8220 -600 -12670 -3155 -2000 671 2.6

	    if (!ParseQuotedString(&bp,&strtok)) goto badparse;
	    ofs = G->BaseCount -1;
	    if (_stricmp(strtok,(G->BaseName[ofs]))!=0) goto badparse;

	    if (!ParseQuotedString(&bp,&strtok))  goto badparse;
	    if (strlen(strtok)>12) goto badparse;

	    for (j=0; j<10; j++) {
	      if (!ParseDouble(&bp,&(tokf[j]))) goto badparse;
	    }

      if (G->RunwayCount >= G->RUNWAY_LIMIT) goto badparse;

	    sprintf(G->RunwayName[G->RunwayCount],strtok);
      for (j=0;j<2;j++) G->RunwayData[G->RunwayCount][j] = G->BaseLL[ofs][j];
	    for (j=0;j<10;j++) G->RunwayData[G->RunwayCount][j+2] = tokf[j];
	    G->RunwayBase[G->RunwayCount] = ofs;

      if (G->BaseMinRwy[ofs]==-1) G->BaseMinRwy[ofs] = G->RunwayCount;
      G->BaseMaxRwy[ofs] = G->RunwayCount;
      G->RunwayData[G->RunwayCount][8] += G->BaseAlt[ofs];

	    fprintf(GSCFGPL,"RWY LOADED: %s\'s %s END1(%.0f, %.0f) END2(%.0f, %.0f) PAPI=%.0f VASI=%.0f ALT=%.0f WIDTH=%.0f LENGTH=%.0f ANGLE=%.0f\n", G->BaseName[ofs], G->RunwayName[G->RunwayCount], 
		    G->RunwayData[G->RunwayCount][2], G->RunwayData[G->RunwayCount][3], G->RunwayData[G->RunwayCount][4],
		    G->RunwayData[G->RunwayCount][5], G->RunwayData[G->RunwayCount][6], G->RunwayData[G->RunwayCount][7],G->RunwayData[G->RunwayCount][8],
        G->RunwayData[G->RunwayCount][9], G->RunwayData[G->RunwayCount][10],G->RunwayData[G->RunwayCount][11]);
	    runwayFound = true;

	    G->RunwayCount++;
	    continue;
	  }

	  if (_stricmp(tok,"GLIDESLOPE")==0)
	  {
// Parsing GLIDESLOPE {gsfilname} "gs description"
// e.g.    GLIDESLOPE XR "XR Series"

      if (!ParseString(&bp,&strtok)) goto badparse;
      if (strlen(strtok)>31) goto badparse;

      if (!LoadPreferredGlideslopeConfig(strtok)) goto badparse; // if it loads, then it's good!

      if (!ParseQuotedString(&bp,&tok)) goto badparse;
      if (strlen(strtok)>31) goto badparse;
      if (G->GSfileCount >= G->GSFILE_LIMIT) goto badparse;

      strcpy(G->GSfileName[G->GSfileCount],strtok);
      strcpy(G->GSname[G->GSfileCount],tok);

	    fprintf(GSCFGPL,"GLIDESLOPE LOADED: %s (%s)\n", G->GSname[G->GSfileCount], G->GSfileName[G->GSfileCount]);
	    gsFound = true;
      G->GSfileCount++;
	    continue;
	  }

	  if (!_stricmp(tok,"PREFS"))
	  {

  // Parsing PREFS {mode-L} {mode-R} {mode-X} {units}        {base}  {rwy}   {glideslope}
  // e.g.    PREFS VSIT     HSIT     DATA        {US | METRIC}  "Cape Canaveral" "RWY33" XR

	    if (prefsFound) goto badparse;

      for (int i=0;i<3;i++) {
 	      if (!ParseString(&bp,&tok)) goto badparse;
	      modeTok[i] = (!_stricmp(tok,"VSIT"))? 1 :
                 (!_stricmp(tok,"TAPE"))? 2 :
                 (!_stricmp(tok,"DATA"))? 3 :
                 (!_stricmp(tok,"HSIT"))? 4 :
                 (!_stricmp(tok,"DIAG"))? 5 :
                 (!_stricmp(tok,"DEOR"))? 6 :
                 (!_stricmp(tok,"1"))? 1 :
                 (!_stricmp(tok,"2"))? 2 :
                 (!_stricmp(tok,"3"))? 3 :
                 (!_stricmp(tok,"4"))? 4 :
                 (!_stricmp(tok,"5"))? 5 :
                 (!_stricmp(tok,"5"))? 6 : 7;
        if (modeTok[i]==7) goto badparse;
      }

	    if (!ParseString(&bp,&tok)) goto badparse;
	    if (_stricmp(tok,"US")==0) {
		    unitstok = true;
	    } else if (_stricmp(tok,"METRIC")==0) {
		    unitstok = false;
	    } else goto badparse;
	    if (!ParseQuotedString(&bp,&strtok2)) goto badparse; // Base
      if (!ParseQuotedString(&bp,&strtok)) goto badparse; // Runway
	    for (ofs=0; (ofs<G->RunwayCount) && (_stricmp(strtok2,(G->BaseName[G->RunwayBase[ofs]]))!=0) && (_stricmp(strtok,(G->RunwayName[ofs]))!=0) ; ofs++) {}
	    if (ofs==G->RunwayCount) goto badparse;
	    if (!ParseString(&bp,&tok)) goto badparse;

      if (gsFound) {
        for (j=0; j<G->GSfileCount; j++) {
          if (!_stricmp(tok,G->GSfileName[j])) break;
        }
        if (j<G->GSfileCount) {
          LoadPreferredGlideslopeConfig(tok);
          G->GSfile = j;
        } else goto badparse;
      } else goto badparse;


	    G->ParamsModeL = (modeTok[0]<1)? 1 : (modeTok[0]>G->maxMod) ? G->maxMod : modeTok[0];
	    G->ParamsModeR = (modeTok[1]<1)? 1 : (modeTok[1]>G->maxMod) ? G->maxMod : modeTok[1];
	    G->ParamsModeX = (modeTok[2]<1)? 1 : (modeTok[2]>G->maxMod) ? G->maxMod : modeTok[2];
	    G->ParamsUnits = unitstok;
	    G->ParamsRwy = ofs;
      G->runway = ofs;
	  
	    fprintf(GSCFGPL,"PREFS LOADED: Mode=%i/%i/%i Units=%s Base=%s Runway=%s Glideslope=%s\n", G->ParamsModeL,G->ParamsModeR,G->ParamsModeX, (G->ParamsUnits?"US":"METRIC"), G->BaseName[G->RunwayBase[G->ParamsRwy]], G->RunwayName[G->ParamsRwy], tok );
	    G->BaseCount++;


	    prefsFound = true;
	    continue;
		
     }


badparse:
   fprintf(GSCFGPL,"Bad parse, ignored line>> %s", bufcpy);
   fprintf(GSCFGPL,"(Check for typos, base not previously defined, base>32 chars, runway >12 chars, >64 bases, > 128 rwys, >1 prefs line, etc.)\n\n");
  }

  if (!feof(GSCFG)) {
    fprintf(GSCFGPL,"Bad read on Glideslope.cfg\n");
    LoadDefaultConfig();
    LoadDefaultGlideslopeConfig();
  } else if ((!prefsFound)||(!baseFound)||(!runwayFound)||(!gsFound)) {
    if (!prefsFound) fprintf(GSCFGPL,"No PREFS found ... defaulting to Default Config");
    if (!baseFound) fprintf(GSCFGPL,"No BASE found ... defaulting to Default Config");
    if (!runwayFound) fprintf(GSCFGPL,"No RUNWAY found ... defaulting to Default Config");
    if (!gsFound) fprintf(GSCFGPL,"No GLIDESLOPE found ... defaulting to Default Config");
    LoadDefaultConfig();
    LoadDefaultGlideslopeConfig();
  } else {
    fprintf(GSCFGPL,"Successfully parsed Glideslope.cfg\n");
	}
  LoadRunwayGeometry();
  fclose(GSCFGPL);
  fclose(GSCFG);

  return;
}

void GlideslopeMFD::LoadRunwayGeometry () {
//
// Loads the runway and pad geometry for the visual base mode
//
  const int S0 = 2;
  const int E0 = 3;
  const int S1 = 4;
  const int E1 = 5;
  const int WID = 9;
  const int LEN = 10;
  const int ANG = 11;

  for (int i=0; i<G->RunwayCount; i++) {
    if (G->RunwayData[i][WID] != G->RunwayData[i][LEN]) {
      G->RunwayGeometry[i].type = 'R';
      G->RunwayGeometry[i].os  = (G->RunwayData[i][S1] + G->RunwayData[i][S0])/2.0; 
      G->RunwayGeometry[i].oe  = (G->RunwayData[i][E1] + G->RunwayData[i][E0])/2.0; 
      G->RunwayGeometry[i].ds  = (G->RunwayData[i][S1] - G->RunwayData[i][S0]); 
      G->RunwayGeometry[i].de  = (G->RunwayData[i][E1] - G->RunwayData[i][E0]); 
      G->RunwayGeometry[i].wid = G->RunwayData[i][WID]; 
      G->RunwayGeometry[i].len = sqrt(G->RunwayGeometry[i].ds*G->RunwayGeometry[i].ds + G->RunwayGeometry[i].de*G->RunwayGeometry[i].de);
      G->RunwayGeometry[i].ang = atan2(G->RunwayGeometry[i].ds,G->RunwayGeometry[i].de) * 180 / 3.1415926 + 90.0;


      double ra = G->RunwayGeometry[i].ang * 3.1415926 / 180;
      double ca = cos(ra);
      double sa = sin(ra);
      double x0 = 0.0;
      double y0 = -G->RunwayGeometry[i].len;

      G->RunwayGeometry[i].p[0].x = long(-G->RunwayGeometry[i].wid/2.0 + 0.5);
      G->RunwayGeometry[i].p[0].y = 0;
      G->RunwayGeometry[i].p[1].x = long(-G->RunwayGeometry[i].wid/2.0 + 0.5);
      G->RunwayGeometry[i].p[1].y = long(-G->RunwayGeometry[i].len + 0.5);
      G->RunwayGeometry[i].p[2].x = long(+G->RunwayGeometry[i].wid/2.0 + 0.5);
      G->RunwayGeometry[i].p[2].y = long(-G->RunwayGeometry[i].len + 0.5);
      G->RunwayGeometry[i].p[3].x = long(+G->RunwayGeometry[i].wid/2.0 + 0.5);
      G->RunwayGeometry[i].p[3].y = 0;
      G->RunwayGeometry[i].p[4].x = long(-G->RunwayGeometry[i].wid/2.0 + 0.5);
      G->RunwayGeometry[i].p[4].y = 0;


      G->RunwayGeometry[i].p[5].x = -60;
      G->RunwayGeometry[i].p[5].y = +120;
      G->RunwayGeometry[i].p[6].x = 0;
      G->RunwayGeometry[i].p[6].y = +60;
      G->RunwayGeometry[i].p[7].x = +60;
      G->RunwayGeometry[i].p[7].y = +120;

      G->RunwayGeometry[i].p[8].x = -60;
      G->RunwayGeometry[i].p[8].y = +180;
      G->RunwayGeometry[i].p[9].x = 0;
      G->RunwayGeometry[i].p[9].y = +120;
      G->RunwayGeometry[i].p[10].x = +60;
      G->RunwayGeometry[i].p[10].y = +180;

      G->RunwayGeometry[i].p[11].x = -60;
      G->RunwayGeometry[i].p[11].y = +240;
      G->RunwayGeometry[i].p[12].x = 0;
      G->RunwayGeometry[i].p[12].y = +180;
      G->RunwayGeometry[i].p[13].x = +60;
      G->RunwayGeometry[i].p[13].y = +240;

      for (int j=0; j<14; j++) {
        long xp = long(ca * G->RunwayGeometry[i].p[j].x - sa * G->RunwayGeometry[i].p[j].y + G->RunwayData[i][E0] + 0.5);
        long yp = long(sa * G->RunwayGeometry[i].p[j].x + ca * G->RunwayGeometry[i].p[j].y + G->RunwayData[i][S0] + 0.5);
        G->RunwayGeometry[i].p[j].x = xp;
        G->RunwayGeometry[i].p[j].y = yp;
      }
    } else {
      G->RunwayGeometry[i].type = 'P';
      if (G->RunwayData[i][WID] < 50.0) {
        G->RunwayData[i][WID];
        G->RunwayData[i][LEN];
      }
      G->RunwayGeometry[i].os  = G->RunwayData[i][S1];
      G->RunwayGeometry[i].oe  = G->RunwayData[i][E1];
      G->RunwayGeometry[i].ds  = G->RunwayData[i][WID];
      G->RunwayGeometry[i].de  = G->RunwayData[i][WID];
      G->RunwayGeometry[i].wid = G->RunwayData[i][WID]; 
      G->RunwayGeometry[i].len = G->RunwayData[i][WID];
      G->RunwayGeometry[i].ang = 0;

      double hwid = G->RunwayData[i][WID]/2.0;

      G->RunwayGeometry[i].p[0].x = long(G->RunwayGeometry[i].oe - hwid  + 0.5);
      G->RunwayGeometry[i].p[0].y = long(G->RunwayGeometry[i].os - hwid  + 0.5);
      G->RunwayGeometry[i].p[1].x = long(G->RunwayGeometry[i].oe + hwid  + 0.5);
      G->RunwayGeometry[i].p[1].y = long(G->RunwayGeometry[i].os + hwid  + 0.5);
    }
  }
}


void GlideslopeMFD::LoadDefaultConfig () {

  double DefaultRunwayData[2][12]= {
//               Base lat, deg   Base lon, deg   RW near south, m   RW near east, m, RW far south, m, RW far east, m, Disp VASI, m,  Disp PAPI, m,   Alt, m   Wid, m  Len, m    Angle
	  /*KSC33*/{      28.5208,       -80.675,           -8220,           -600,          -12670,            -3155,          671,         -2000,          2.6,    100.0,  5131.0,   331.0},
	  /*KSC15*/{      28.5208,       -80.675,           -12670,          -3155,         -8220,             -600,           671,         -2000,          2.6,    100.0,  5131.0,   151.0}
  
  };
  
  strcpy(G->BasePlanet[0],"Earth");
  G->hBasePlanet[0] = oapiGetGbodyByName("Earth");
  strcpy(G->BaseName[0],"Cape Canaveral (Default)");
  strcpy(G->RunwayName[0], "KSC33 (def)");
  strcpy(G->RunwayName[1], "KSC15 (def)");

  for (int i=0; i<12; i++) {
	  G->RunwayData[0][i] = DefaultRunwayData[0][i];
	  G->RunwayData[1][i] = DefaultRunwayData[1][i];
  }
  G->RunwayBase[0] = 0;
  G->RunwayBase[1] = 0;
  G->BaseMinRwy[0] = 0;
  G->BaseMaxRwy[0] = 1;
  G->RunwayCount = 2;
  G->BaseCount = 1;
  G->ParamsModeL = 2;
  G->ParamsModeR = 3;
  G->ParamsModeX = 4;
  G->ParamsUnits = false;
	G->ParamsRwy = 1;

/*
sprintf(oapiDebugString(),"Default rwy init Base(%s) Rwy(%s) LatLon(%4.1f %4.1f) Near(%4.1f %4.1f) Far(%4.1f %4.1f) TDP PAPI(%4.1f %4.1f)",
BaseName[RunwayBase[G->runway]], RunwayName[G->runway],
RunwayData[G->runway][0], RunwayData[G->runway][1], 
RunwayData[G->runway][2], RunwayData[G->runway][3], 
RunwayData[G->runway][4], RunwayData[G->runway][5], 
RunwayData[G->runway][6], RunwayData[G->runway][7]); 
*/

  return;
}



bool GlideslopeMFD::LoadPreferredGlideslopeConfig (char *PrefFile) {
  FILE* GSCFG;
  char buf[256];
  char bufcpy[256];
  buf[255] = '\0';
  char *tok=buf;
  char *strtok=buf;
  int i, j;
  int modetok = 0;
  bool unitstok = false;

  double gsparams[G->GLIDESLOPE_POINTS_LIMIT][6];
  int gsrow=0;

  char *bp;
  bool unitsFound = false, gsFound = false, inGSblock = false;
  bool unitMode = false;

  sprintf(buf,".\\Config\\MFD\\Glideslope\\Glideslope_%s.cfg",PrefFile);

  GSCFG = fopen(buf,"r");
  if (GSCFG == NULL) {
// Defaults to init code
	  return false;
  }
  sprintf(buf,".\\Config\\MFD\\Glideslope\\Logs\\Glideslope_%s_ParseLog.txt",PrefFile);

  FILE* GSCFGPL=fopen(buf,"w");
  fprintf(GSCFGPL,"Successfully found and opened %s\n",buf);

  while (fgets(buf,255,GSCFG)!=NULL) {
	  bp=buf;
	  sprintf(bufcpy,"%s",buf);
	  while (bp[0]==' ' || bp[0]=='\t') bp++;
	  if ((bp[0]=='\0')||(bp[0]=='\n')) {
      fprintf(GSCFGPL,"NUL> %s", buf);
	    continue;
	  }
	  if (bp[0]==';') {
      fprintf(GSCFGPL,"REM> %s", buf);
	    continue;
	  }

	  if (!ParseString(&bp,&tok)) goto badparse;

	  if (_stricmp(tok,"UNITS")==0)
	  {
  // Parsing UNITS METRIC or UNITS US

	    if (!ParseString(&bp,&strtok)) goto badparse;
  	  if (_stricmp(strtok,"METRIC")==0) {
		    unitsFound = true;
		    unitMode = false;
        fprintf(GSCFGPL,"UNITS are Metric\n");
	      continue;
	    } else if (_stricmp(strtok,"US")==0) {
		    unitsFound = true;
		    unitMode = true;
        fprintf(GSCFGPL,"UNITS are US\n");
		    continue;
	    } else goto badparse;
	  }

	  if (_stricmp(tok,"BEGIN")==0)
	  {
  // Parsing BEGIN GLIDESLOPE

	    if (!ParseString(&bp,&strtok)) goto badparse;
  	  if (_stricmp(strtok,"GLIDESLOPE")!=0) goto badparse;
	    if (!unitsFound) goto badparse;
	    if (inGSblock) goto badparse;
	    inGSblock = true;
	    continue;
	  }

	  if (_stricmp(tok,"END")==0)
	  {
  // Parsing END GLIDESLOPE

	    if (!ParseString(&bp,&strtok)) goto badparse;
  	  if (_stricmp(strtok,"GLIDESLOPE")!=0) goto badparse;
	    if (!inGSblock) goto badparse;
	    inGSblock = false;
	    continue;
	  }

  // To get here .... we should be in the GS details 
    if (!inGSblock) goto badparse;
    if (gsrow>=G->GLIDESLOPE_POINTS_LIMIT) goto badparse;

	  gsparams[gsrow][0] = atoi(tok);
	  for (j=1;j<5;j++) {
      if (!ParseDouble(&bp,&(gsparams[gsrow][j]))) goto badparse;
	  }
	  gsparams[gsrow][5] = 0;
    fprintf(GSCFGPL,"Loaded glideslope point> (RNG:%.0f, ALT:%.1f, TAS:%.1f, VSP:%.1f, AOA:%.1f)\n", gsparams[gsrow][0],gsparams[gsrow][1],gsparams[gsrow][2],gsparams[gsrow][3],gsparams[gsrow][4]);
	  gsrow++;
    continue;

badparse:
    fprintf(GSCFGPL,"Bad parse, ignored line>> %s", bufcpy);
    fprintf(GSCFGPL,"(Check for typos, no UNITS line, badly formed GLIDESLOPE data etc.)\n\n");
  }

  if (!feof(GSCFG)) {
    fprintf(GSCFGPL,"Bad read on Glideslope glideslope points file\n");
    fclose(GSCFGPL);
    fclose(GSCFG);
    return false;
  } else {
    if (gsrow<2) {
      fprintf(GSCFGPL,"Need glideslope points in a glideslope points file ... defaulting to Default Config");
      fclose(GSCFGPL);
      fclose(GSCFG);
      return false;
    } else {
      fprintf(GSCFGPL,"Successfully parsed Glideslope glideslope points file\n");
	  }
  }

// Transfer points into master structure
  const int pmap[6] = {2, 3, 0, 4, 1, 5};		// Order from points file different to order in the internal table. Fix it up
  const char colname[6][4] = {"RNG","ALT","TAS","VSP","AOA","ZRO"};
  const double cnvM[6] = {1000, 1000, 1, 1, RAD, 0};	// Conversion for metric points entry Rng, Alt, TAS, Vspd, AOA, 0
  const double cnvU[6] = {1852, 304.8, 0.3048, 0.3048, RAD, 0};	// Conversion for US points entry Rng, Alt, TAS, Vspd, AOA, 0
  double cnv[6];

  for (j=0;j<6;j++) {cnv[j] = (unitMode? cnvU[j] : cnvM[j]);};

  G->RefSlopePts = gsrow;
  for (i=0;i<gsrow;i++) {
    fprintf(GSCFGPL,"Glideslope point loaded: ");
	  for (j=0; j<6; j++) {
      G->RefSlope[i][pmap[j]] = gsparams[i][j]*cnv[j]; 
      fprintf(GSCFGPL,"%s %.3f (%.3f * %.3f) ",colname[j], G->RefSlope[i][pmap[j]], gsparams[i][j], cnv[j] );
    }
    fprintf(GSCFGPL,"\n");
  }
  fprintf(GSCFGPL,"Completed\n");
  fclose(GSCFGPL);
  fclose(GSCFG);

  G->base.alt = G->RefSlope[0][3] / 1000.0;
  G->base.ang = atan2(-G->RefSlope[0][4], G->RefSlope[0][0]);
  double checkang = G->base.ang * 180.0 / PI;
  G->base.ant = (G->RefSlope[0][2])/oapiGetSize(vessel->GetGravityRef());
  double checkant = G->base.ant * 180.0 / PI;
  G->base.alt = G->RefSlope[0][3] / 1000.0;

  return true;
}

void GlideslopeMFD::LoadDefaultGlideslopeConfig () {

  int i, j;
	const double DefSlope[44][6]={


  // New Ref Slope in Meters
  //           TAS m/s         AoA deg        Range, m              Alt, m      dAlt, m/s         Segment length
  //-----------------------------------------------------------
 
  {                   7501,    41.0*RAD,        8145000,             120000,           -95,         0},
  {                   7504,    41.0*RAD,        8000000,             118000,           -95,         0},
  {                   7520,    41.5*RAD,        7000000,             105000,           -94,         0}, 
  {                   7533,    41.5*RAD,        6000000,              93000,           -89,         0},
  {                   7534,    42.5*RAD,        5500000,              87000,           -85,         0},
  {                   7524,    42.5*RAD,        5000000,              81500,           -79,         0},
  {                   7491,    42.0*RAD,        4500000,              76500,           -70,         0},
  {                   7416,    42.0*RAD,        4000000,              72000,           -60,         0},
  {                   7285,    42.0*RAD,        3500000,              68500,           -55,         0},
  {                   7078,    43.0*RAD,        3000000,              65500,           -55,         0}, 
  {                   6774,    43.0*RAD,        2500000,              62000,           -55,         0},
  {                   6245,    40.5*RAD,        2000000,              56000,           -55,         0},
  {                   5352,    40.5*RAD,        1500000,              50500,           -55,         0},
  {                   4176,    36.5*RAD,        1000000,              46500,           -50,         0},
  {                   3652,    36.5*RAD,         800000,              44600,           -50,         0},
  {                   2987,    35.5*RAD,         600000,              41000,           -50,         0},
  {                   2567,    34.0*RAD,         500000,              39000,           -50,         0},
  {                   2294,    33.0*RAD,         450000,              38500,           -50,         0},
  {                   2109,    33.0*RAD,         400000,              37600,           -60,         0}, 
  {                   1908,    33.0*RAD,         350000,              35700,           -90,         0},
  {                   1645,    33.0*RAD,         300000,              33100,           -90,         0},
  {                   1491,    33.0*RAD,         275000,              31800,           -80,         0},
  {                   1320,    33.0*RAD,         250000,              30400,           -70,         0},
  {                   1135,    33.0*RAD,         225000,              28900,           -80,         0},
  {                    919,    33.0*RAD,         200000,              26400,           -80,         0},
  {                    754,    29.0*RAD,         180000,              23900,           -80,         0}, 
  {                    666,    25.0*RAD,         170000,              22700,           -80,         0},
  {                    592,     0.0*RAD,         160000,              22700,            40,         0},
  {                    567,     4.0*RAD,         150000,              22700,           -20,         0},
  {                    552,     4.0*RAD,         140000,              22400,           -28,         0},
  {                    414,    12.0*RAD,         120000,              21100,           -24,         0},
  {                    327,    13.0*RAD,         100000,              20600,           -25,         0},
  {                    275,    12.0*RAD,          80000,              18300,           -25,         0},
  {                    238,     9.0*RAD,          60000,              15000,           -25,         0},
  {                    221,     8.0*RAD,          50000,              13900,           -25,         0},
  {                    222,     6.0*RAD,          40000,              12600,           -35,         0},
  {                    238,     2.0*RAD,          30000,              10600,           -56,         0},
  {                    264,     1.0*RAD,          20000,               7000,           -80,         0},
  {                    204,     1.0*RAD,          10000,               2770,           -60,         0},
  {                    188,     1.0*RAD,           2000,                200,           -33,         0},
  {                    166,     2.0*RAD,              0,                  0,            -2,         0},
  {                    155,     0.0*RAD,          -1000,                  0,             0,         0},
  {                    116,     0.0*RAD,          -2000,                  0,             0,         0},
  {                      0,     0.0*RAD,          -3300,                  0,             0,         0}

	};

  G->RefSlopePts = 44;
  for (i=0;i<44;i++) {
    for (j=0; j<6; j++) {
      G->RefSlope[i][j] = DefSlope[i][j];
    }
  }

  G->GSfile = 0;
  G->GSfileCount = 0;
  strcpy(G->GSfileName[G->GSfileCount],"<<internal default>>");
  strcpy(G->GSname[G->GSfileCount],"Emergency Default");
	return;
}