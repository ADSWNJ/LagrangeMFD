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
  oapi::Pen* GetPen(const char* key, const bool force_solid = false);
  DWORD GetMFDColor(const char* key);
  const char* GetPlotColor(const char* key);

  bool DefColor(const char* key, const int r, const int g, const int b);
  bool DefPlot(const char* key, const char* col, const bool solid);
  bool DefMFDCol(const char* key, const char* col);
  bool GoodInit();

  void Reset();
protected:
private:
  map<string, DWORD> _color;
  map<string, oapi::Pen*> _pen;
  map<string, string> _plotcol;
  map<string, string> _mfdcol;
};
#endif // _LAGRANGE_DRAWING_CLASS