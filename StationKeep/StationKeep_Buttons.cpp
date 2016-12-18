// ==============================================================
//
//	StationKeep (MFD Button Management)
//	============================
//
//	Copyright (C) 2013	Andrew (ADSWNJ) Stokes
//                   All rights reserved
//
//	See StationKeep.cpp
//
// ==============================================================

#include "MFDButtonPage.hpp"
#include "StationKeep_Buttons.hpp"
#include "StationKeep.hpp"


StationKeep_Buttons::StationKeep_Buttons() 
{
    // The menu descriptions of all buttons
    static const MFDBUTTONMENU mnu0[] =
    {
      {"Get Vec1", 0, '1'},
      {"Get Vec2", 0, '2' }
    };

    RegisterPage(mnu0, sizeof(mnu0) / sizeof(MFDBUTTONMENU));

    RegisterFunction("GV1", OAPI_KEY_2, &StationKeep::Button_GV1);
    RegisterFunction("GV2", OAPI_KEY_3, &StationKeep::Button_GV2);

    return;
}

bool StationKeep_Buttons::SearchForKeysInOtherPages() const
{
    return false;
}



