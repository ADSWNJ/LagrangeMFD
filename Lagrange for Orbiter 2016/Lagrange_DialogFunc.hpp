// ==============================================================
//
//	Lagrange (Dialog Handling Code)
//	===============================
//
//	Copyright (C) 2016	Andrew (ADSWNJ) Stokes and Keith (Keithth G) Gelling
//                   All rights reserved
//
//	See Lagrange.cpp
//
// ==============================================================


#ifndef __LAGRANGE_DIALOGFunc
#define __LAGRANGE_DIALOGFunc
#include "Lagrange_LCore.hpp"

class Lagrange_DialogFunc
{
    public:
      static bool clbkITR(void *id, char *str, void *usrdata);
      static bool clbkTSP(void *id, char *str, void *usrdata);
      static bool clbkENT(void *id, char *str, void *usrdata);
protected:
    private:
      static bool skipWS(char **p);
      static bool spanStr(char **p);
};

#endif // Lagrange_DIALOGTGT