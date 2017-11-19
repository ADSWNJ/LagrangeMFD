// ==============================================================
//
//	Lagrange (Dialog Handling Code)
//	===============================
//
//	Copyright (C) 2016-2017	Andrew (ADSWNJ) Stokes
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
      static bool clbkOPC(void *id, char *str, void *usrdata);
      static bool clbkITR(void *id, char *str, void *usrdata);
      static bool clbkTSP(void *id, char *str, void *usrdata);
      static bool clbkRNG(void *id, char *str, void *usrdata);
      static bool clbkENC(void *id, char *str, void *usrdata);
      static bool clbkRCT(void *id, char *str, void *usrdata);
      static bool clbkHYS(void *id, char *str, void *usrdata);
      static bool clbkENT(void *id, char *str, void *usrdata);
      static bool clbkWT(void *id, char *str, void *usrdata);
      static bool clbkRVA(void *id, char *str, void *usrdata);
protected:
    private:
      static bool skipWS(char **p);
      static bool spanStr(char **p);
};

#endif // Lagrange_DIALOGTGT