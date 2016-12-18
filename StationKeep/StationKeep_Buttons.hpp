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



#ifndef _StationKeep_BUTTON_CLASS
#define _StationKeep_BUTTON_CLASS
#include "MFDButtonPage.hpp"

class StationKeep;

class StationKeep_Buttons : public MFDButtonPage<StationKeep>
{
  public:
    StationKeep_Buttons();
  protected:
    bool SearchForKeysInOtherPages() const;
  private:
};
#endif // _StationKeep_BUTTON_CLASS

