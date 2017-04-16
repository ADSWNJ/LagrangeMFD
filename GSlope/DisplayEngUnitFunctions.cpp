//
// Display Eng Unit Helper Functions
//
// Purpose ... factor floating point numbers into engineering format (kilo, mega,etc.)
//
// (c) Andrew Stokes (ADSWNJ) 2012-2017
//
// All rights reserved
//

#include <math.h>
#include <stdio.h>

// DisplayEngUnit ... Simple conversion of value into engineering format
void DisplayEngUnit(char* buf, char* fmt, double val) {
  char engUnit[12] = "pnum kMGTPE";
  double cnvVal = val;
  int i = 4;

  if (fabs(cnvVal) < 1) {
	  while ((fabs(cnvVal) < 1) && (i>0)) {
		  i--;
		  cnvVal *= 1000;
	  }
  } else if (fabs(cnvVal) >= 1000) {
    while ((fabs(cnvVal) >= 1000) && (i< 10)) {
		  i++;
		  cnvVal /= 1000;
	  }
  }
  sprintf_s(buf, 256, fmt, cnvVal, engUnit[i]);
}

// DisplayEngUnit2 ... Conversion of value into engineering format, adding metric suffix if units==false , or
// convert to US units, add US suffix if units == true
void DisplayEngUnit2(char* buf, char* metricFmt, char* usFmt, char* metricSfx, char* usSfx, double cnvFactor, bool units, double val) {
  char tmpFmt[256];
  char engUnit[12] = "pnum kMGTPE";
  double cnvVal = val;
  int i = 4;
  
  if (units) {
    sprintf_s(tmpFmt, 256, "%s%s", usFmt, usSfx); 
	cnvVal = val * cnvFactor; 
  } else {
	sprintf_s(tmpFmt, 256, "%s%s", metricFmt, metricSfx);
	cnvVal = val; 
  }

  if (fabs(cnvVal) < 1) {
	  while ((fabs(cnvVal) < 1) && (i>0)) {
		  i--;
		  cnvVal *= 1000;
	  }
  } else if (fabs(cnvVal) >= 1000) {
    while ((fabs(cnvVal) >= 1000) && (i< 10)) {
		  i++;
		  cnvVal /= 1000;
	  }
  }
  sprintf_s(buf, 256, tmpFmt, cnvVal, engUnit[i]);
}

// DisplayEngUnit3 ... Simple conversion of value into engineering format, returning sym and pwr to lock the scale for DisplayEngUnit4
void DisplayEngUnit3(char* buf, char* fmt, double val, char* sym, int* pwr, int minpwr, int maxpwr) {
  char engUnit[12] = "pnum kMGTPE";
  double cnvVal = val;
  int i = 4;
  int actpwr = 0;

  if (fabs(cnvVal) < 1) {
	  while ((fabs(cnvVal) < 1) && (i>0) && (actpwr > minpwr)) {
		  i--;
		  actpwr -= 3;
		  cnvVal *= 1000;
	  }
  } else if (fabs(cnvVal) >= 1000) {
    while ((fabs(cnvVal) >= 1000) && (i< 10) && (actpwr < maxpwr)) {
		  i++;
		  actpwr += 3;
		  cnvVal /= 1000;
	  }
  }
  sprintf_s(buf, 256, fmt, cnvVal, engUnit[i]);
  *sym = engUnit[i];
  *pwr = (i-4) *3;
}

// DisplayEngUnit4 ... Fixed conversion of value into engineering format, using sym and pwr set in DisplayEngUnit3
void DisplayEngUnit4(char* buf, char* fmt, double val, char sym, int pwr) {
  double cnvVal = val;
  int i = pwr; 

  if (i<0) {
    while (i++) {
		  cnvVal *= 10;
	  }
  } else if (i>0) {
	  while (i--) {
		  cnvVal /= 10;
	  }
  }
  sprintf_s(buf, 256, fmt, cnvVal, sym);

}


// DisplayEngUnit5 ... Same as DisplayEngUnit2, but with lower bound (0= unit, -1 = milli, -2 = micro, -3 = nano, -4 = pico)
void DisplayEngUnit5(char* buf, char* metricFmt, char* usFmt, char* metricSfx, char* usSfx, double cnvFactor, bool units, double val, int loB) {
  char tmpFmt[256];
  char engUnit[12] = "pnum kMGTPE";
  double cnvVal = val;
  int i = 4;

  if (loB<-4) loB = -4;
  if (loB>0) loB = 0;
  loB +=4;
  
  if (units) {
    sprintf_s(tmpFmt, 256, "%s%s", usFmt, usSfx); 
	cnvVal = val * cnvFactor; 
  } else {
	sprintf_s(tmpFmt, 256, "%s%s", metricFmt, metricSfx);
	cnvVal = val; 
  }

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
  sprintf_s(buf, 256, tmpFmt, cnvVal, engUnit[i]);
}

// DisplayEngUnit6 ... As DisplayEngUnit5, with extra boolean for displaying value or --- (e.g. for no values)

void DisplayEngUnit6(char* buf, char* metricFmt, char* usFmt, char* metricSfx, char* usSfx, double cnvFactor, bool units, double val, int loB, bool dispval) {
  if (dispval) {
	  DisplayEngUnit5(buf, metricFmt, usFmt, metricSfx, usSfx, cnvFactor, units, val, loB);
	  return;
  }
  sprintf_s(buf, 256, "%s", "     ---");
}