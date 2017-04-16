//
// Display Eng Unit Helper Functions
//
// Purpose ... factor floating point numbers into engineering format (kilo, mega,etc.)
//
// (c) Andrew Stokes (ADSWNJ) 2012-2017
//
// All rights reserved
//

// DisplayEngUnit ... Simple conversion of value into engineering format
void DisplayEngUnit(char* buf, char* fmt, double val);

// DisplayEngUnit2 ... Conversion of value into engineering format, adding metric suffix if units==false , or
// convert to US units, add US suffix if units == true
void DisplayEngUnit2(char* buf, char* metricFmt, char* usFmt, char* metricSfx, char* usSfx, double cnvFactor, bool units, double val);

// DisplayEngUnit3 ... Simple conversion of value into engineering format, returning sym and pwr to lock the scale for DisplayEngUnit4
void DisplayEngUnit3(char* buf, char* fmt, double val, char* sym, int* pwr, int minpwr, int maxpwr);

// DisplayEngUnit4 ... Fixed conversion of value into engineering format, using sym and pwr set in DisplayEngUnit3
void DisplayEngUnit4(char* buf, char* fmt, double val, char sym, int pwr);

// DisplayEngUnit5 ... Same as DisplayEngUnit2, but with lower bound
void DisplayEngUnit5(char* buf, char* metricFmt, char* usFmt, char* metricSfx, char* usSfx, double cnvFactor, bool units, double val, int loB);

// DisplayEngUnit6 ... As DisplayEngUnit2, with extra boolean for displaying value or --- (e.g. for no values)
void DisplayEngUnit6(char* buf, char* metricFmt, char* usFmt, char* metricSfx, char* usSfx, double cnvFactor, bool units, double val, int loB, bool dispval);
