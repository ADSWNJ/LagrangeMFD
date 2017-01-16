// ==========================================================================
//
//	Lagrange (Sketchpad Functions)
//	==============================
//
//	Copyright (C) 2016-2017	Andrew (ADSWNJ) Stokes
//                   All rights reserved
//
//  A collection of helper functions to simplify writing on the sketchpad
//	See Lagrange.cpp
//
// ===========================================================================

#include "Lagrange.hpp"
#include "Lagrange_Universe.hpp"
#include "DisplayEngUnitFunctions.h"
#include <math.h>
#include <stdarg.h>

#define CLR_YELLOW 0x00FFFF
#define CLR_WHITE 0xFFFFFF

// MFD Positioning Helper Functions
int Lagrange::_Line(const int row) {  // row is 0-24, for 24 rows. e.g. Line(12)
  int ret;
  ret = (int)((H - (int)(ch / 4)) * row / 25) + (int)(ch / 4);
  return ret;
};

int Lagrange::_Col(const int pos) {  // pos is 0-5, for 6 columns. Eg Col(3) for middle
  int ret = (int)((W - (int)(cw / 2)) * pos / 6) + int(cw / 2);
  return ret;
};

int Lagrange::_Col2(const int pos) {  // pos is 0-11, for 12 columns. Eg Col(6) for middle
  int ret = (int)((W - (int)(cw / 2)) * pos / 12) + int(cw / 2);
  return ret;
};


// MFD Format and print helper
void Lagrange::skpFormatText(const int col, const int line, const char* fmt, ...) {
  LC->skpColPix = _Col(col);
  LC->skpLinePix = _Line(line);
  va_list args;
  va_start(args, fmt);
  vsprintf_s(LC->skpBuf, 128, fmt, args);
  va_end(args);
  LC->skp->Text(LC->skpColPix, LC->skpLinePix, LC->skpBuf, strlen(LC->skpBuf));
}

void Lagrange::skpFmtColText(const int col, const int line, const bool test, const DWORD truecol, const DWORD falsecol, const char* fmt, ...) {
  LC->skp->SetTextColor(test ? truecol : falsecol);
  LC->skpColPix = _Col(col);
  LC->skpLinePix = _Line(line);
  va_list args;
  va_start(args, fmt);
  vsprintf_s(LC->skpBuf, 128, fmt, args);
  va_end(args);
  LC->skp->Text(LC->skpColPix, LC->skpLinePix, LC->skpBuf, strlen(LC->skpBuf));
  LC->skp->SetTextColor(falsecol);
}

void Lagrange::skpColor(const DWORD col) {
  LC->skp->SetTextColor(col);
}

void Lagrange::skpFmtEngText(const int col, const int line, const char* fmt, const char* sfx, const double val) {
  LC->skpColPix = _Col(col);
  LC->skpLinePix = _Line(line);
  char engUnit[12] = "pnum kMGTPE";
  double cnvVal = val;
  int i = 4;
  int loB = LC->skpLoB;

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
    sprintf_s(LC->skpFmtBuf, 128, "%s%s", fmt, sfx);
    sprintf_s(LC->skpBuf, 128, LC->skpFmtBuf, cnvVal);
  } else {
    sprintf_s(LC->skpFmtBuf, 128, "%s%%c%s", fmt, sfx);
    sprintf_s(LC->skpBuf, 128, LC->skpFmtBuf, cnvVal, engUnit[i]);
  }
  LC->skp->Text(LC->skpColPix, LC->skpLinePix, LC->skpBuf, strlen(LC->skpBuf));
}


void Lagrange::skpFmtEngText(const int col, const int line, const char* fmt, const unsigned char* sfx, const double val) {
  LC->skpColPix = _Col(col);
  LC->skpLinePix = _Line(line);
  char engUnit[12] = "pnum kMGTPE";
  double cnvVal = val;
  int i = 4;
  int loB = LC->skpLoB;

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
    sprintf_s(LC->skpFmtBuf, 128, "%s%s", fmt, sfx);
    sprintf_s(LC->skpBuf, 128, LC->skpFmtBuf, cnvVal);
  } else {
    sprintf_s(LC->skpFmtBuf, 128, "%s%%c%s", fmt, sfx);
    sprintf_s(LC->skpBuf, 128, LC->skpFmtBuf, cnvVal, engUnit[i]);
  }
  LC->skp->Text(LC->skpColPix, LC->skpLinePix, LC->skpBuf, strlen(LC->skpBuf));
}




void Lagrange::skpTitle(const char *title) {
  Title(LC->skp, title);
  LC->skp->SetTextAlign(oapi::Sketchpad::LEFT, oapi::Sketchpad::BOTTOM);
  LC->skp->SetTextColor(CLR_WHITE);
}