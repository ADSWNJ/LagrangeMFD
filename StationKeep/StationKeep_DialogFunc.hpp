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


#ifndef __StationKeep_DIALOGFunc
#define __StationKeep_DIALOGFunc

class StationKeep_DialogFunc
{
    public:
      static bool clbkSIL(void *id, char *str, void *usrdata);
      static bool clbkSDL(void *id, char *str, void *usrdata);
      static bool clbkSVL(void *id, char *str, void *usrdata);
      static bool clbkSSN(void *id, char *str, void *usrdata);
    protected:
    private:

};

#endif // StationKeep_DIALOGTGT
