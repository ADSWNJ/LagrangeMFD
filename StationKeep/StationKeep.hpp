// ==============================================================
//
//	Bare Bones Auto Pilot (StationKeep)
//	============================
//
//	Copyright (C) 2014	Andrew (ADSWNJ) Stokes
//                   All rights reserved
//
//	See StationKeep.cpp
//
// ==============================================================


#ifndef __StationKeep_H
#define __StationKeep_H

#include "StationKeep_GCore.hpp"   
#include "StationKeep_VCore.hpp" 
#include "StationKeep_LCore.hpp" 

extern "C" 
class StationKeep: public MFD2
{
public:
	StationKeep (DWORD w, DWORD h, VESSEL *vessel, UINT mfd);
	~StationKeep ();

  char *ButtonLabel (int bt);
	int ButtonMenu (const MFDBUTTONMENU **menu) const;
  bool ConsumeKeyBuffered (DWORD key);
  bool ConsumeButton (int bt, int event);
  
  bool Update (oapi::Sketchpad *skp);
  static int MsgProc (UINT msg, UINT mfd, WPARAM wparam, LPARAM lparam);

  // Button Press Handlers
  void Button_GV1();
  void Button_GV2();

  const char* GetModuleName() const;

  // Persistence functions
  void ReadStatus(FILEHANDLE scn);
  void WriteStatus(FILEHANDLE scn) const;

  // Unit conversions
  double id( double d ) const;
  double ed( double d ) const;

protected:
  StationKeep_GCore* GC;
  StationKeep_LCore* LC;
  StationKeep_VCore* VC;

  int Line( int row );
  int Col( int pos );
  int Col2( int pos );
  void ShowMessage(oapi::Sketchpad *skp);

  oapi::Font *font;


};

#endif // !__StationKeep_H
