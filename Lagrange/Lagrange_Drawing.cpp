// ==============================================================
//
//	Lagrange (Color management)
//	================================
//
//	Copyright (C) 2017	Andrew (ADSWNJ) Stokes
//                   All rights reserved
//
//	See Lagrange.cpp
//
// ==============================================================

#include "Lagrange_Drawing.hpp"

Lagrange_Drawing::Lagrange_Drawing() {
}
Lagrange_Drawing::~Lagrange_Drawing() {
  Reset();
}

void Lagrange_Drawing::Reset() {
  _color.clear();
  for (auto& it : _pen) {
    oapiReleasePen(it.second);
  }
  _pen.clear();
}

oapi::Pen* Lagrange_Drawing::GetPen(const string s) {
  string key = s;
  if (!_pen.count(key)) std::invalid_argument("Key not found");
  return _pen[key];
}

DWORD Lagrange_Drawing::GetMFDColor(const string s) {
  if (!_color.count(s)) std::invalid_argument("Color not found");
  return _color[_mfdcol[s]];
}

void Lagrange_Drawing::DefColor(const string s, const int r, const int g, const int b) {
  DWORD bgr = b * 256 * 256 + g * 256 + r;
  _color[s] = bgr;
}

void Lagrange_Drawing::DefPlot(const string s, const string col, const bool solid) {
  string key = s;
  if (!_color.count(col)) std::invalid_argument("Color not found");
  if (!_pen.count(key)) {
    _pen[key] = oapiCreatePen(solid? 1 : 2, 1, _color[col]);
  } else {
    throw std::invalid_argument("Attempted redefinition");
  }
}

void Lagrange_Drawing::DefMFDCol(string s, const string col) {
  if (!_color.count(col)) throw std::invalid_argument("Color not found");
  if (!_mfdcol.count(s)) {
    _mfdcol[s] = col;
  } else {
    throw std::invalid_argument("Attempted redefinition");
  }
}

bool Lagrange_Drawing::GoodInit() {
  if (!_mfdcol.count("DEF")) return false;
  if (!_mfdcol.count("HI")) return false;
  if (!_mfdcol.count("WARN")) return false;
  if (!_pen.count("Sun")) return false;
  if (!_pen.count("Earth")) return false;
  if (!_pen.count("Moon")) return false;
  if (!_pen.count("LP")) return false;
  if (!_pen.count("VL")) return false;
  if (!_pen.count("VP")) return false;
  return true;
}