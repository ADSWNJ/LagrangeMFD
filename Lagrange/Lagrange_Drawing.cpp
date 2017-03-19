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
    if (it.first[0] != '\t') oapiReleasePen(it.second);
  }
  _pen.clear();
}

oapi::Pen* Lagrange_Drawing::GetPen(const char* key, const bool force_solid ) {
  string _key = key;
  if (force_solid) _key = (string) "\t=" + key;
  if (!_pen.count(_key)) throw std::invalid_argument("Key not found");
  return _pen[_key];
}

DWORD Lagrange_Drawing::GetMFDColor(const char* key) {
  if (!_mfdcol.count(key)) throw std::invalid_argument("Color not found");
  if (!_color.count(_mfdcol[key])) throw std::invalid_argument("Color not found");
  return _color[_mfdcol[key]];
}

bool Lagrange_Drawing::DefColor(const char* key, const int r, const int g, const int b) {
  if (!_color.count(key)) {
    int _r = r < 0 ? 0 : r > 255 ? 255 : r;
    int _g = g < 0 ? 0 : g > 255 ? 255 : g;
    int _b = b < 0 ? 0 : b > 255 ? 255 : b;
    DWORD bgr = _b * 256 * 256 + _g * 256 + _r;
    _color[key] = bgr;
    return true;
  } else {
    return false;
  }

}

bool Lagrange_Drawing::DefPlot(const char* key, const char* col, const bool solid) {
  if (!_color.count(col)) return false;
  string solid_key = "\t=" + (string)key;
  if (!_pen.count(key)) {
    _pen[key] = oapiCreatePen(solid? 1 : 2, 1, _color[col]);
    _plotcol[key] = (solid? "" : "Dashed ") + (string) col;
    if (!solid) {
      _pen[solid_key] = oapiCreatePen(1, 1, _color[col]);
    } else {
      _pen[solid_key] = _pen[key];
    }
  } else {
    return false;
  }
  return true;
}

bool Lagrange_Drawing::DefMFDCol(const char* key, const char* col) {
  if (!_color.count(col)) return false;
  if (!_mfdcol.count(key)) {
    _mfdcol[key] = col;
  } else {
    return false;
  }
  return true;
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

const char* Lagrange_Drawing::GetPlotColor(const char* key) {
  if (!_plotcol.count(key)) {
    throw std::invalid_argument("Color not found");
  }
  return _plotcol[key].c_str();
}