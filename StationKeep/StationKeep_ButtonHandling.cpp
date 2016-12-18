// ==============================================================
//
//	StationKeep (Button Handling Code)
//	============================
//
//	Copyright (C) 2013	Andrew (ADSWNJ) Stokes
//                   All rights reserved
//
//	See StationKeep.cpp
//
// ==============================================================

#include "StationKeep.hpp"
#include "StationKeep_DialogFunc.hpp"
#include <EnjoLib/ModuleMessagingExt.hpp>

// ==============================================================
// MFD button hooks to Button Page library
//
char* StationKeep::ButtonLabel (int bt)
{
	return LC->B.ButtonLabel(bt);
}

// Return button menus
int StationKeep::ButtonMenu (const MFDBUTTONMENU **menu) const
{
	return LC->B.ButtonMenu(menu);
}

// Return clicked button
bool StationKeep::ConsumeButton (int bt, int event) {
  return LC->B.ConsumeButton(this, bt, event);
}

// Return pressed keystroke
bool StationKeep::ConsumeKeyBuffered (DWORD key) {
  return LC->B.ConsumeKeyBuffered(this, key);
}



// ==============================================================
// MFD Button Handler Callbacks
//



// GV1 = Get Vector 1
void StationKeep::Button_GV1() {
	VECTOR3 vec;
	if (EnjoLib::ModuleMessagingExt().ModMsgGet("LagrangerMFD","TPos", &vec, VC->v)) {
		VC->tpos = vec;
	} else {
		VC->tpos = _V(0.0,0.0,0.0);
	}
  return;
};

// GV2 = Get Vector 2
void StationKeep::Button_GV2() {
  VECTOR3 vec;
  if (EnjoLib::ModuleMessagingExt().ModMsgGet("LagrangerMFD", "TVel", &vec, VC->v)) {
    VC->tvel = vec;
  } else {
    VC->tvel = _V(0.0, 0.0, 0.0);
  }
  return;
};

