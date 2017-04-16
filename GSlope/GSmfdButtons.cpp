//
// Glideslope Button Control ... leveraging Enjo's MFDButtons helper library
//
// Purpose ... sets up the menus and supports the context page switching (main, config, deorbit menus)
//
// (c) Andrew Stokes (ADSWNJ) 2012-2017
//
// All rights reserved
//

#include "MFDButtonPage.hpp"
#include "GSmfdButtons.hpp"
#include "GlideslopeMFD.hpp" 

GSmfdButtons::GSmfdButtons()
{
    // The menu descriptions of all buttons
    static const MFDBUTTONMENU mnu0[] =
    {
      {"Switch Mode", 0, 'M'},
      {"Config Screen", 0, 'C'},
      {"Switch Units", 0, 'U'},
      {"AutoPilot Toggle", 0, 'B'},
      {"Clear Track History", 0, 'T'},
      {"Save User Glideslope", 0, 'V'},

	    {"Switch HAC", 0, 'H'},
      {"Reset HAC & Zoom", 0, 'R'},
	    {"HAC Radius++", 0, '='},
      {"HAC Radius--", 0, '-'},
      {"Manual Zoom++", 0, '\''},
      {"Manual Zoom--", 0, ';'}
    };

    RegisterPage(mnu0, sizeof(mnu0) / sizeof(MFDBUTTONMENU));

    RegisterFunction("MOD", OAPI_KEY_M, &GlideslopeMFD::menuMode);
    RegisterFunction("CFG", OAPI_KEY_C, &GlideslopeMFD::menuConfig);
    RegisterFunction("UNT", OAPI_KEY_U, &GlideslopeMFD::menuUnits);
    RegisterFunction("AUT", OAPI_KEY_B, &GlideslopeMFD::menuAutopilot);
    RegisterFunction("CLR", OAPI_KEY_T, &GlideslopeMFD::menuClearGS);
    RegisterFunction("SAV", OAPI_KEY_V, &GlideslopeMFD::menuSaveGS);

  	RegisterFunction("HAC", OAPI_KEY_H, &GlideslopeMFD::menuHACgeometry);
    RegisterFunction("RST", OAPI_KEY_R, &GlideslopeMFD::menuResetHAC);
    RegisterFunction("HR+", OAPI_KEY_EQUALS, &GlideslopeMFD::menuHACRadUp);
    RegisterFunction("HR-", OAPI_KEY_MINUS, &GlideslopeMFD::menuHACRadDn);
    RegisterFunction("ZM+", OAPI_KEY_APOSTROPHE, &GlideslopeMFD::menuZoomUp);
    RegisterFunction("ZM-",  OAPI_KEY_SEMICOLON, &GlideslopeMFD::menuZoomDn);


    static const MFDBUTTONMENU mnu1[] =
    {
      {"Return", 0, 'K'},
      {"Previous Base", 0, 'A'},
      {"Next Base", 0, 'S'},
      {"Previous Rwy", 0, 'Q'},
      {"Next Rwy", 0, 'W'},
      {"View Base", 0, 'V'},

      {"Deorbit Mode Enable", 0, 'O'},
      {"Previous Glideslope", 0, 'E'},
      {"Next Glideslope", 0, 'R'},
	    {"Diagnostic Mode Enable", 0, 'D'},
	    {"Extended Track Save", 0, 'X'},
      {"Quick Access Recorder", 0, 'Q'}
    };

    RegisterPage(mnu1, sizeof(mnu1) / sizeof(MFDBUTTONMENU));

    RegisterFunction("OK", OAPI_KEY_K, &GlideslopeMFD::menuOK);
    RegisterFunction("PB", OAPI_KEY_A, &GlideslopeMFD::menuPrevBase);
    RegisterFunction("NB", OAPI_KEY_S, &GlideslopeMFD::menuNextBase);
    RegisterFunction("PR", OAPI_KEY_Q, &GlideslopeMFD::menuPrevRwy);
    RegisterFunction("NR", OAPI_KEY_W, &GlideslopeMFD::menuNextRwy);
    RegisterFunction("VB", OAPI_KEY_V, &GlideslopeMFD::menuViewBase);

    RegisterFunction("DEO",OAPI_KEY_O, &GlideslopeMFD::menuDeorbitSel);
    RegisterFunction("PG", OAPI_KEY_E, &GlideslopeMFD::menuPrevGS);
    RegisterFunction("NG", OAPI_KEY_R, &GlideslopeMFD::menuNextGS);
    RegisterFunction("DIA", OAPI_KEY_D, &GlideslopeMFD::menuDiagSel);
    RegisterFunction("XTS", OAPI_KEY_X, &GlideslopeMFD::menuExtTrackSave);
    RegisterFunction("QAR", OAPI_KEY_X, &GlideslopeMFD::menuQuickAccessRecorder);

    static const MFDBUTTONMENU mnu2[] =
    {
      {"Return", 0, 'K'},
    };

    RegisterPage(mnu2, sizeof(mnu2) / sizeof(MFDBUTTONMENU));

    RegisterFunction("OK", OAPI_KEY_K, &GlideslopeMFD::menuOK);

    static const MFDBUTTONMENU mnu3[] =
    {
      {"Switch Mode", 0, 'M'},
      {"Deorbit HP++", 0, '='},
      {"Deorbit HP+", 0, '\''},
      {"Deorbit HP-", 0, ';'},
      {"Deorbit HP--", 0, '-'},
    };

    RegisterPage(mnu3, sizeof(mnu3) / sizeof(MFDBUTTONMENU));

    RegisterFunction("MOD",  OAPI_KEY_M, &GlideslopeMFD::menuMode);
    RegisterFunction("AB",   OAPI_KEY_A, &GlideslopeMFD::menuDeorAB);

    static const MFDBUTTONMENU mnu4[] =
    {
      {"Return", 0, 'K'},
      {"Previous Base", 0, 'A'},
      {"Next Base", 0, 'S'},
      {"Previous Rwy", 0, 'Q'},
      {"Next Rwy", 0, 'W'},
      {"Reset View", 0, 'R'},
      {"Zoom Out", 0, 'X'},
      {"Zoom In", 0, 'Z'},
      {"View Up", 0, '-'},
      {"View Down", 0, '='},
      {"View Left", 0, '<'},
      {"View Right", 0, '>'}
    };

    RegisterPage(mnu4, sizeof(mnu4) / sizeof(MFDBUTTONMENU));

    RegisterFunction("OK", OAPI_KEY_K, &GlideslopeMFD::menuOK);
    RegisterFunction("PB", OAPI_KEY_A, &GlideslopeMFD::menuPrevBase);
    RegisterFunction("NB", OAPI_KEY_S, &GlideslopeMFD::menuNextBase);
    RegisterFunction("PR", OAPI_KEY_Q, &GlideslopeMFD::menuPrevRwy);
    RegisterFunction("NR", OAPI_KEY_W, &GlideslopeMFD::menuNextRwy);

    RegisterFunction("RST", OAPI_KEY_R, &GlideslopeMFD::menuViewBaseReset);
    RegisterFunction("ZM-", OAPI_KEY_X, &GlideslopeMFD::menuViewBaseZoomOut);
    RegisterFunction("ZM+", OAPI_KEY_Z, &GlideslopeMFD::menuViewBaseZoomIn);
    RegisterFunction("UP", OAPI_KEY_MINUS, &GlideslopeMFD::menuViewBaseUp);
    RegisterFunction("DN", OAPI_KEY_EQUALS, &GlideslopeMFD::menuViewBaseDown);
    RegisterFunction("<", OAPI_KEY_LBRACKET, &GlideslopeMFD::menuViewBaseLeft);
    RegisterFunction(">", OAPI_KEY_RBRACKET, &GlideslopeMFD::menuViewBaseRight);

    return;
}

bool GSmfdButtons::SearchForKeysInOtherPages() const
{
    return false;
}
