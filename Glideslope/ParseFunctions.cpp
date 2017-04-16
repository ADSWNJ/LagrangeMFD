//
// Parse Helper Functions
//
// Purpose ... tokenize strings, pull out floats, deal with quoted strings, etc
//
// (c) Andrew Stokes (ADSWNJ) 2012-2017
//
// All rights reserved
//

#include <math.h>
#include <stdio.h>
#include <string.h>

//
// ParseQuotedString:
// Pull out a quoted string from the buffer pointer to by bp into ret
// After the function, ret points to the quoted string (quotes stripped), and bp points to the rest of the string
// Note 1 ... the buffer is modified (nuls added to terminate strings). 
// Note 2 ... escaped characters are not handled (e.g. \" terminates the string and leaves a \). 
// Return status reflects whether the task was successful. 
//

  bool ParseQuotedString(char **bp, char **ret) {
	  char *b = *bp;
	  while ((*b==' ')||(*b=='\t')) b++;
	  if (*b!='\"') return false;
	  b++;
	  *ret = b;
	  while ((*b!='\"')&&(*b!='\0')&&(*b!='\n')) b++;
	  if (*b!='\"') return false;
	  *b = '\0';
	  *bp = b+1;
	  return true;
  }
//
// ParseString:
// Same as ParaseQuotedString except does not want quotes and stops at the first whitespace
//

  bool ParseString(char **bp, char **ret) {

	  char *b = *bp;
	  while ((*b==' ')||(*b=='\t')) b++;
	  *ret = b;
	  while ((*b!='\"')&&(*b!='\0')&&(*b!='\n')&&(*b!=' ')&&(*b!='\'')&&(*b!='\t')) b++;
	  if ((*b=='\"')||(*b=='\'')) return false; // No quotes allowed in string
	  if (*b!='\0') {
		*b = '\0';
		*bp=b+1;
	  } else {
		*bp = b;	// if we hit EOL, point to EOL not one beyond
	  }
	  return true;
  }
// 
// ParseDouble:
// Same as ParaseString except it pulls out a double floating point
//

  bool ParseDouble(char **bp, double *ret) {
	  char *b = *bp;
	  char *t;
	  int i;

	  while ((*b==' ')||(*b=='\t')) b++;
	  t = b;
	  i = strspn(t, "+-0123456789e.");
	  b = t+i;
	  if ((*b!=' ')&&(*b!='\t')&&(*b!='\n')&&(*b!='\0')) return false; // End of parse must be whitespace
	  if (*b!='\0') {
		*b = '\0';
		*bp=b+1;
	  } else {
		*bp = b;	// if we hit EOL, point to EOL not one beyond
	  }
	  if (i==0) return false;
	  *ret = atof(t);
	  return true;
  }
// 
// ParseInt:
// Same as ParaseString except it pulls out an integer
//
  bool ParseInt(char **bp, int *ret) {
	  double f=0.0;
	  if (!ParseDouble(bp,&f)) return false;
	  *ret=int(f);
	  return true;
  }
// 
// ParseBool:
// Same as ParaseString except it pulls out a true or false
//
  bool ParseBool(char **bp, bool *ret) {
	  char *bufp;
	  if (!ParseString(bp,&bufp)) return false;
	  if (_stricmp(bufp,"FALSE")==0) {
		  *ret = false;
		  return true;
	  }
	  if (_stricmp(bufp,"TRUE")==0) {
		  *ret = true;
		  return true;
	  }
	  return false;
  }

//
// ParseWhiteSpace:
// Skips leading space, tabs or comments (; found in whitespace parse)
// Returns false if all whitespace
//
  bool ParseWhiteSpace(char **bp, char **ret) {

	  char *b = *bp;
	  while ((*b==' ')||(*b=='\t')) b++;
	  *ret = b;
    *bp = b;
    if ((*b==';')||(*b=='\0')) return false;
    return true;
  }
