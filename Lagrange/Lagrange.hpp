// ==============================================================
//
//	Lagrange MFD Headers
//	====================
//
//	Copyright (C) 2016-2017	Andrew (ADSWNJ) Stokes
//                   All rights reserved
//
//	See Lagrange.cpp
//
// ==============================================================


#ifndef __LAGRANGE_H
#define __LAGRANGE_H

#include "Lagrange_GCore.hpp"   
#include "Lagrange_VCore.hpp" 
#include "Lagrange_LCore.hpp" 

extern "C" 
class Lagrange: public MFD2
{
public:
	Lagrange (DWORD w, DWORD h, VESSEL *vessel, UINT mfd);
	~Lagrange ();

  char *ButtonLabel (int bt);
	int ButtonMenu (const MFDBUTTONMENU **menu) const;
  bool ConsumeKeyBuffered (DWORD key);
  bool ConsumeButton (int bt, int event);
  
  bool Update (oapi::Sketchpad *skp);
  bool DisplayOrbitMode();
  bool DisplayPlanMode();
  bool DisplayAPMode();
  bool DisplayLPMode();
  bool DisplayS4IMode();
  bool DisplayFrmMode();
  bool DisplayTgtMode();
  bool DisplayMessageMode();

  // Button Press Handlers
  void Button_MOD();
  void Button_BURNARM();
  void Button_MJD();
  void Button_TDV();
  void Button_LDV();
  void Button_PRO();
  void Button_OUT();
  void Button_PLC();
  void Button_ENT();
  void Button_ADJ();
  void Button_ADM();
  void Button_AUP();
  void Button_ADN();
  void Button_TGT();
  void Button_FRM();
  void Button_PRJ();
  void Button_ZMU();
  void Button_ZMD();
  void Button_MUP();
  void Button_MDN();
  void Button_MLF();
  void Button_MRG();
  void Button_AAB();
  void Button_AAC();
  void Button_AAH();
  void Button_OK();
  void Button_NotImplementedYet();
  void Button_S4IARM();
  void Button_TOK();
  void Button_NXT();
  void Button_PRV();
  void Button_PMT();
  void Button_ITR();
  void Button_TSP();
  void Button_WT();
  void Button_DML();
  void Button_DME();

  void ButtonHelper_AdjVar(double adj);

  const char* GetModuleName() const { return "Lagrange"; };

  // Persistence functions
  void ReadStatus(FILEHANDLE scn);
  void WriteStatus(FILEHANDLE scn) const;

protected:
  Lagrange_GCore* GC;
  Lagrange_LCore* LC;
  Lagrange_VCore* VC;

  int _Line(const int row );
  int _Col(const int pos );
  int _Col2(const int pos );
  void skpFormatText(const int col, const int line, const char* fmt, ...);
  void skpFmtColText(const int col, const int line, const bool test, const DWORD truecol, const DWORD falsecol, const char* fmt, ...);
  void skpFmtEngText(const int col, const int line, const char* fmt, const char* sfx, const double val);
  void skpFmtEngText(const int col, const int line, const char* fmt, const unsigned char* sfx, const double val);
  void skpTitle(const char* title);
  void skpColor(DWORD col);
  void showMessage();

  oapi::Font *font;
  oapi::Pen *pen[12];

};

#endif // !__LAGRANGE_H