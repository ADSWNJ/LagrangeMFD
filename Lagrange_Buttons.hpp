// ==============================================================
//
//	Lagrange (Button Handling Headers)
//	==================================
//
//	Copyright (C) 2016-2017	Andrew (ADSWNJ) Stokes
//                   All rights reserved
//
//	See Lagrange.cpp
//
// ==============================================================



#ifndef _LAGRANGE_BUTTON_CLASS
#define _LAGRANGE_BUTTON_CLASS
#include "MFDButtonPage.hpp"

class Lagrange;

class Lagrange_Buttons : public MFDButtonPage<Lagrange>
{
  public:
    Lagrange_Buttons();
  protected:
    bool SearchForKeysInOtherPages() const;
  private:
};
#endif // _LAGRANGE_BUTTON_CLASS

