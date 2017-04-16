// ==============================================================
//
//	MFDPersist
//	==========
//
//	Copyright (C) 2013-2016	Andrew (ADSWNJ) Stokes
//                   All rights reserved
//
//	Description, Credits and Copyright Notice: see MFDPersist.cpp
//
//	Release History:
//
//	V1.00	Initial Release
//
// ==============================================================


#ifndef __MFDPersist
#define __MFDPersist

#include "windows.h"
#include "orbitersdk.h"
#include <list>
using namespace std;


class MFDPersistLookup {
  public:
    VESSEL *v;
    UINT  mfd;
    void* core;
};

class MFDPersist {
  public:
    MFDPersist();
    void* findVC(VESSEL *v);
    void* findLC(VESSEL *v, UINT mfd);
    void  addVC(VESSEL *v, void *VC);
    void  addLC(VESSEL *v, UINT mfd, void *LC);
    void  delVC(VESSEL *v);
    void  delLC(VESSEL *v);
    void* firstVC();
    void* nextVC();

  private:
    list<MFDPersistLookup*>  VCL;
    list<MFDPersistLookup*>  LCL;
    void* find(list<MFDPersistLookup*> &L, VESSEL *v, UINT mfd);
    void add(list<MFDPersistLookup*> &L, VESSEL *v, UINT mfd, void *C);
    void del(list<MFDPersistLookup*> &L, VESSEL* vin);
    list<MFDPersistLookup*>::iterator iterVC;
};

#endif // __MFDPersist