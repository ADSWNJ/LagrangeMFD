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



#ifndef _LAGRANGE_DRAWING_CLASS
#define _LAGRANGE_DRAWING_CLASS

#include <map>
using namespace std;
#include "orbitersdk.h"

class Lagrange_Drawing
{
public:
  Lagrange_Drawing();
  ~Lagrange_Drawing();
  oapi::Pen* GetPen(const string s);
  DWORD GetMFDColor(const string s); 

  void DefColor(const string s, const int r, const int g, const int b);
  void DefPlot(const string s, const string col, const bool solid);
  void DefMFDCol(const string s, const string col);
  bool GoodInit();

  void Reset();
protected:
private:
  map<string, DWORD> _color;
  map<string, oapi::Pen*> _pen;
  map<string, string> _mfdcol;
};
#endif // _LAGRANGE_DRAWING_CLASS