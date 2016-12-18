// ==============================================================
//
//	Parse Functions
//	===============
//
//	Copyright (C) 2014	Andrew (ADSWNJ) Stokes
//                   All rights reserved
//
//	See ParseFunctions.cpp
//
// ==============================================================

#ifndef __ParseFunctins
#define __ParseFunctins
#include <stdio.h>

//
// ParseQuotedString:
// Pull out a quoted string from the buffer pointer to by bp into ret
// After the function, ret points to the quoted string (quotes stripped), and bp points to the rest of the string
// Note 1 ... the buffer is modified (nuls added to terminate strings). 
// Note 2 ... escaped characters are not handled (e.g. \" terminates the string and leaves a \). 
// Return status reflects whether the task was successful. 
//
  bool ParseQuotedString(char **bp, char **ret);
//
// ParseString:
// Same as ParseQuotedString except does not want quotes and stops at the first whitespace
//
  bool ParseString(char **bp, char **ret);
// 
// ParseDouble:
// Same as ParseString except it pulls out a double doubleing point
//
  bool ParseDouble(char **bp, double *ret);
// 
// ParseInt:
// Same as ParseString except it pulls out an integer
//
  bool ParseInt(char **bp, int *ret);

// 
// ParseBool:
// Same as ParseString except it pulls out a true or false
//
  bool ParseBool(char **bp, bool *ret);
//
// ParseWhiteSpace:
// Skips leading space, tabs or comments (; found in whitespace parse)
// Returns false if all whitespace
//
  bool ParseWhiteSpace(char **bp, char **ret);

  #endif // __ParseFunctions