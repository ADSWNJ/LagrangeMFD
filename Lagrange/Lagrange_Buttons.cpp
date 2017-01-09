// ==============================================================
//
//	Lagrange (MFD Button Management)
//	================================
//
//	Copyright (C) 2016-2017	Andrew (ADSWNJ) Stokes
//                   All rights reserved
//
//	See Lagrange.cpp
//
// ==============================================================

#include "MFDButtonPage.hpp"
#include "Lagrange_Buttons.hpp"
#include "Lagrange.hpp"


Lagrange_Buttons::Lagrange_Buttons() 
{
    // Orbit Mode
    static const MFDBUTTONMENU mnu0[] =
    {
      {"Mode Select", 0, 'M'},
      {"Set Target", 0, 'T'},
      {"Ref Frame", 0, 'F'},
      {"Projection", 0, 'P'},
      {"Zoom In", 0, 'I'},
      {"Zoom Out", 0, 'O'},
      {"Move Up", 0, 'U'}, 
      {"Move Down", 0, 'D' },
      {"Move Left", 0, '<' },
      {"Move Right", 0, '>' }
    };
    RegisterPage(mnu0, sizeof(mnu0) / sizeof(MFDBUTTONMENU));
    RegisterFunction("MOD", OAPI_KEY_M, &Lagrange::Button_MOD);
    RegisterFunction("TGT", OAPI_KEY_T, &Lagrange::Button_TGT);
    RegisterFunction("FRM", OAPI_KEY_F, &Lagrange::Button_FRM);
    RegisterFunction("PRJ", OAPI_KEY_P, &Lagrange::Button_PRJ);
    RegisterFunction("ZM+", OAPI_KEY_I, &Lagrange::Button_ZMU);
    RegisterFunction("ZM-", OAPI_KEY_O, &Lagrange::Button_ZMD);
    RegisterFunction("UP",  OAPI_KEY_U, &Lagrange::Button_MUP);
    RegisterFunction("DN",  OAPI_KEY_D, &Lagrange::Button_MDN);
    RegisterFunction("<",   OAPI_KEY_COMMA, &Lagrange::Button_MLF);
    RegisterFunction(">",   OAPI_KEY_PERIOD, &Lagrange::Button_MRG);

    // Plan Mode
    static const MFDBUTTONMENU mnu1[] =
    {
      { "Mode Select", 0, 'M' },
      { "Arm/Disarm Plan", 0, 'A' },
      { "Set Burn MJD", 0, 'D' },
      { "Set Prograde", 0, 'F' },
      { "Set Total Delta-V", 0, 'T' },
      { "Lock Total Delta-V", 0, 'L' },
      { "Increase Increment", 0, 'U' },
      { "Increment Value", 0, '+' },
      { "Decrement Value", 0, '-' },
      { "Enter Value", 0, 'E' },
      { "Set Plane Change", 0, 'P' },
      { "Set Outward", 0, 'O' }
    };
    RegisterPage(mnu1, sizeof(mnu1) / sizeof(MFDBUTTONMENU));
    RegisterFunction("MOD", OAPI_KEY_M, &Lagrange::Button_MOD);
    RegisterFunction("ARM", OAPI_KEY_A, &Lagrange::Button_BURNARM);
    RegisterFunction("MJD", OAPI_KEY_D, &Lagrange::Button_MJD);
    RegisterFunction("PRO", OAPI_KEY_F, &Lagrange::Button_PRO);
    RegisterFunction("TDV", OAPI_KEY_F, &Lagrange::Button_TDV);
    RegisterFunction("LDV", OAPI_KEY_F, &Lagrange::Button_LDV);
    RegisterFunction("ADJ", OAPI_KEY_1, &Lagrange::Button_ADJ);
    RegisterFunction("++",  OAPI_KEY_3, &Lagrange::Button_AUP);
    RegisterFunction("--",  OAPI_KEY_4, &Lagrange::Button_ADN);
    RegisterFunction("ENT", OAPI_KEY_E, &Lagrange::Button_ENT);
    RegisterFunction("PLC", OAPI_KEY_P, &Lagrange::Button_PLC);
    RegisterFunction("OUT", OAPI_KEY_O, &Lagrange::Button_OUT);

    // Burn Mode
    static const MFDBUTTONMENU mnu2[] =
    {
      { "Mode Select", 0, 'M' },
      { "Set AutoBurn", 0, 'B' },
      { "Set AutoCenter", 0, 'C' },
      { "Set AutoTrim", 0, 'T' }
    };
    RegisterPage(mnu2, sizeof(mnu2) / sizeof(MFDBUTTONMENU));
    RegisterFunction("MOD", OAPI_KEY_M, &Lagrange::Button_MOD);
    RegisterFunction("AB",  OAPI_KEY_B, &Lagrange::Button_BAB);
    RegisterFunction("AC",  OAPI_KEY_C, &Lagrange::Button_BAC);
    RegisterFunction("AT",  OAPI_KEY_T, &Lagrange::Button_BAT);

    // Position Mode
    static const MFDBUTTONMENU mnu3[] =
    {
      { "Mode Select", 0, 'M' },
      { "Set AutoHold", 0, 'H' }
    };
    RegisterPage(mnu3, sizeof(mnu3) / sizeof(MFDBUTTONMENU));
    RegisterFunction("MOD", OAPI_KEY_M, &Lagrange::Button_MOD);
    RegisterFunction("AH",  OAPI_KEY_H, &Lagrange::Button_PAH);

    // S4I Mode
    static const MFDBUTTONMENU mnu4[] =
    {
      { "Mode Select", 0, 'M' },
      { "Arm/Disarm S4I", 0, 'A' },
      { "S4I Iterations", 0, 'I' },
      { "S4I Delta Time", 0, 'T' },
      { "Iter Wait Time", 0, 'W' },
      { "Dump Logs", 0, 'L' },
      { "Dump Encounter", 0, 'E' }
    };
    RegisterPage(mnu4, sizeof(mnu4) / sizeof(MFDBUTTONMENU));
    RegisterFunction("MOD", OAPI_KEY_M, &Lagrange::Button_MOD);
    RegisterFunction("ARM", OAPI_KEY_A, &Lagrange::Button_S4IARM);
    RegisterFunction("ITR", OAPI_KEY_I, &Lagrange::Button_ITR);
    RegisterFunction("DT",  OAPI_KEY_T, &Lagrange::Button_TSP);
    RegisterFunction("WT",  OAPI_KEY_W, &Lagrange::Button_WT);
    RegisterFunction("DML", OAPI_KEY_W, &Lagrange::Button_DML);
    RegisterFunction("DME", OAPI_KEY_W, &Lagrange::Button_DME);

    // Acknowledge Message Mode
    static const MFDBUTTONMENU mnu5[] =
    {
      { "Acknowledge", 0, 'K' }
    };
    RegisterPage(mnu5, sizeof(mnu5) / sizeof(MFDBUTTONMENU));
    RegisterFunction("OK", OAPI_KEY_K, &Lagrange::Button_OK);

   // Target Mode
    static const MFDBUTTONMENU mnu6[] =
    {
      { "OK", 0, 'K' },
      { "Previous",0, 'P' },
      { "Next",0, 'N' },
      { "Put Me There",0, 'X' },
    };
    RegisterPage(mnu6, sizeof(mnu6) / sizeof(MFDBUTTONMENU));
    RegisterFunction("OK", OAPI_KEY_K, &Lagrange::Button_TOK);
    RegisterFunction("PRV", OAPI_KEY_P, &Lagrange::Button_PRV);
    RegisterFunction("NXT", OAPI_KEY_N, &Lagrange::Button_NXT);
    RegisterFunction("PMT", OAPI_KEY_N, &Lagrange::Button_PMT);

    // Frame Mode
    static const MFDBUTTONMENU mnu7[] =
    {
      { "OK", 0, 'K' },
      { "Previous",0, 'P' },
      { "Next",0, 'N' }
    };
    RegisterPage(mnu7, sizeof(mnu7) / sizeof(MFDBUTTONMENU));
    RegisterFunction("OK", OAPI_KEY_K, &Lagrange::Button_TOK);
    RegisterFunction("PRV", OAPI_KEY_P, &Lagrange::Button_PRV);
    RegisterFunction("NXT", OAPI_KEY_N, &Lagrange::Button_NXT);
    return;
}

bool Lagrange_Buttons::SearchForKeysInOtherPages() const
{
    return false;
}