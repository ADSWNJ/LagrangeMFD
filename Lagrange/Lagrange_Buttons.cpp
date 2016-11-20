// ==============================================================
//
//	Lagrange (MFD Button Management)
//	================================
//
//	Copyright (C) 2016	Andrew (ADSWNJ) Stokes and Keith (Keithth G) Gelling
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
    RegisterFunction("MOD", OAPI_KEY_I, &Lagrange::Button_MOD);
    RegisterFunction("TGT", OAPI_KEY_D, &Lagrange::Button_TGT);
    RegisterFunction("FRM", OAPI_KEY_V, &Lagrange::Button_FRM);
    RegisterFunction("PRJ", OAPI_KEY_1, &Lagrange::Button_PRJ);
    RegisterFunction("ZM+", OAPI_KEY_2, &Lagrange::Button_ZMU);
    RegisterFunction("ZM-", OAPI_KEY_3, &Lagrange::Button_ZMD);
    RegisterFunction("UP", OAPI_KEY_4, &Lagrange::Button_MUP);
    RegisterFunction("DN", OAPI_KEY_5, &Lagrange::Button_MDN);
    RegisterFunction("<", OAPI_KEY_5, &Lagrange::Button_MLF);
    RegisterFunction(">", OAPI_KEY_5, &Lagrange::Button_MRG);

    // Plan Mode
    static const MFDBUTTONMENU mnu1[] =
    {
      { "Mode Select", 0, 'M' },
      { "Set MJD", 0, 'T' },
      { "Set Prograde", 0, 'F' },
      { "Set Outward", 0, 'O' },
      { "Set Plane Change", 0, 'P' },
      { "Enter Value", 0, 'E' },
      { "Increase Increment", 0, 'U' },
      { "Reduce Increment", 0, 'D' },
      { "Increment Value", 0, '+' },
      { "Decrement Value", 0, '-' }
    };
    RegisterPage(mnu1, sizeof(mnu1) / sizeof(MFDBUTTONMENU));
    RegisterFunction("MOD", OAPI_KEY_I, &Lagrange::Button_MOD);
    RegisterFunction("MJD", OAPI_KEY_D, &Lagrange::Button_MJD);
    RegisterFunction("PRO", OAPI_KEY_V, &Lagrange::Button_PRO);
    RegisterFunction("OUT", OAPI_KEY_1, &Lagrange::Button_OUT);
    RegisterFunction("PLC", OAPI_KEY_2, &Lagrange::Button_PLC);
    RegisterFunction("ENT", OAPI_KEY_3, &Lagrange::Button_ENT);
    RegisterFunction("ADJ", OAPI_KEY_4, &Lagrange::Button_ADJ);
    RegisterFunction("-AJ", OAPI_KEY_5, &Lagrange::Button_ADM);
    RegisterFunction("++", OAPI_KEY_5, &Lagrange::Button_AUP);
    RegisterFunction("--", OAPI_KEY_5, &Lagrange::Button_ADN);

    // Burn Mode
    static const MFDBUTTONMENU mnu2[] =
    {
      { "Mode Select", 0, 'M' },
      { "Set AutoBurn", 0, 'B' },
      { "Set AutoCenter", 0, 'C' },
      { "Set AutoTrim", 0, 'T' }
    };
    RegisterPage(mnu2, sizeof(mnu2) / sizeof(MFDBUTTONMENU));
    RegisterFunction("MOD", OAPI_KEY_I, &Lagrange::Button_MOD);
    RegisterFunction("AB", OAPI_KEY_D, &Lagrange::Button_BAB);
    RegisterFunction("AC", OAPI_KEY_V, &Lagrange::Button_BAC);
    RegisterFunction("AT", OAPI_KEY_1, &Lagrange::Button_BAT);

    // Position Mode
    static const MFDBUTTONMENU mnu3[] =
    {
      { "Mode Select", 0, 'M' },
      { "Set AutoHold", 0, 'H' }
    };
    RegisterPage(mnu3, sizeof(mnu3) / sizeof(MFDBUTTONMENU));
    RegisterFunction("MOD", OAPI_KEY_I, &Lagrange::Button_MOD);
    RegisterFunction("AH", OAPI_KEY_D, &Lagrange::Button_PAH);

    // Diags Mode
    static const MFDBUTTONMENU mnu4[] =
    {
      { "Mode Select", 0, 'M' }
    };
    RegisterPage(mnu4, sizeof(mnu4) / sizeof(MFDBUTTONMENU));
    RegisterFunction("MOD", OAPI_KEY_I, &Lagrange::Button_MOD);
    RegisterFunction("LT", OAPI_KEY_L, &Lagrange::Button_LT);
    RegisterFunction("ITR", OAPI_KEY_I, &Lagrange::Button_ITR);
    RegisterFunction("TSP", OAPI_KEY_I, &Lagrange::Button_TSP);

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
      { "NXT",0, 'N' },  
      { "PRV",0, 'P' },  
    };
    RegisterPage(mnu6, sizeof(mnu6) / sizeof(MFDBUTTONMENU));
    RegisterFunction("OK", OAPI_KEY_K, &Lagrange::Button_TOK);
    RegisterFunction("PRV", OAPI_KEY_P, &Lagrange::Button_PRV);
    RegisterFunction("NXT", OAPI_KEY_N, &Lagrange::Button_NXT);
    return;
}

bool Lagrange_Buttons::SearchForKeysInOtherPages() const
{
    return false;
}