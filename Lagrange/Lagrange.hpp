// ==============================================================
//
//	Lagrange MFD Headers
//	====================
//
//	Copyright (C) 2016	Andrew (ADSWNJ) Stokes and Keith (Keithth G) Gelling
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
  bool DisplayOrbitMode(oapi::Sketchpad *skp);
  bool DisplayPlanMode(oapi::Sketchpad *skp);
  bool DisplayBurnMode(oapi::Sketchpad *skp);
  bool DisplayLPMode(oapi::Sketchpad *skp);
  bool DisplayS4IMode(oapi::Sketchpad *skp);
  bool DisplayTgtMode(oapi::Sketchpad *skp);
  bool DisplayMessageMode(oapi::Sketchpad *skp);

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
  void Button_BAB();
  void Button_BAC();
  void Button_BAT();
  void Button_PAH();
  void Button_OK();
  void Button_NotImplementedYet();
  void Button_S4IARM();
  void Button_TOK();
  void Button_NXT();
  void Button_PRV();
  void Button_ITR();
  void Button_TSP();

  void Lagrange::ButtonHelper_AdjVar(double adj);

  const char* GetModuleName() const { return "Lagrange"; };

  // Persistence functions
  void ReadStatus(FILEHANDLE scn);
  void WriteStatus(FILEHANDLE scn) const;

protected:
  Lagrange_GCore* GC;
  Lagrange_LCore* LC;
  Lagrange_VCore* VC;

  int Line( int row );
  int Col( int pos );
  int Col2( int pos );
  void Lagrange::SkpFormatText(oapi::Sketchpad *skp, int col, int line, const char* fmt, ...);
  void Lagrange::SkpFmtColText(oapi::Sketchpad *skp, int col, int line, bool test, DWORD truecol, DWORD falsecol, const char* fmt, ...);
  void Lagrange::SkpFmtEngText(oapi::Sketchpad *skp, int col, int line, const char* fmt, char* sfx, double val, int loB);
  void ShowMessage(oapi::Sketchpad *skp);

  oapi::Font *font;
  oapi::Pen *pen[12];

};

#endif // !__LAGRANGE_H