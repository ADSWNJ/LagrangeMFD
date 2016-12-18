// ==============================================================
//
//	StationKeep (MFD Update)
//	=================
//
//	Copyright (C) 2013	Andrew (ADSWNJ) Stokes
//                   All rights reserved
//
//	See StationKeep.cpp
//
// ==============================================================

#include "StationKeep.hpp"

bool StationKeep::Update (oapi::Sketchpad *skp)
{

  Title (skp, "StationKeep MFD v1.0");
	// Draws the MFD title

  int l = 4;
	skp->SetTextAlign (oapi::Sketchpad::LEFT, oapi::Sketchpad::BOTTOM);
	skp->SetTextColor (0x00FFFF);

  char buf[128];


	return true;
}

// MFD Line formatting helper
void StationKeep::ShowMessage(oapi::Sketchpad *skp) {

  char localMsg[750];
  strcpy_s(localMsg,750, LC->Message.c_str());
  char *bp = localMsg;
  char *bp2 = localMsg;
  char *bp3;
  char c1, c2;
  int i = 0;
  int j;
  int l = 4;
  bool eol = false;

  do {
    if ((*bp2 == '\n') || (*bp2 == '\0')) {     // Look for user newline or end of buffer
      eol = true;
      c1 = *bp2;
      *bp2 = '\0';
    } else {
      if (i==34) {                              // 34 chars no newline ... need to break the line
        eol=true;
        bp3 = bp2;
        for (j=34; j>20; j--) {                 // look for a space from 21 to 34
          if (*bp3==' ') break;
          bp3--;
        }
        if (j>20) {                             // space found
          bp2 = bp3;
          c1 = *bp2;
          *bp2 = '\0';
        } else {                                // no space ... insert hyphen
          bp3 = bp2 + 1;
          c1 = *bp2;
          c2 = *bp3;
          *bp2 = '-';
          *bp3 = '\0';
        }
      } else {                                  // Scan forward      
        i++;
        bp2++;
      }
    }

    if (eol) {                                  // EOL flag ... write out buffer from bp to bp2.
  	  skp->Text (Col(0), Line(l++), bp, strlen(bp));
      eol = false;
      if (c1 == '\0') {
        bp = bp2;     // End of buffer
      } else if ((c1 == '\n') || (c1 == ' ')) {
        bp = bp2+1;   // Reset for next line of the buffer
        bp2++;
        i=0;
      } else {
        bp = bp2;     // Put back the chars we stomped
        *bp2 = c1;
        *bp3 = c2;
        i=0;
      }
    }
  } while (*bp);

  return;
}


// MFD Positioning Helper Functions
int StationKeep::Line( int row ) {  // row is 0-24, for 24 rows. e.g. Line(12)
  int ret;
  ret = (int) ((H-(int)(ch/4)) * row / 25) + (int) (ch/4);
  return ret;
};

int StationKeep::Col( int pos ) {  // pos is 0-5, for 6 columns. Eg Col(3) for middle
  int ret = (int) ((W-(int)(cw/2)) * pos / 6) + int (cw/2);
  return ret;
};

int StationKeep::Col2( int pos ) {  // pos is 0-11, for 12 columns. Eg Col(6) for middle
  int ret = (int) ((W-(int)(cw/2)) * pos / 12) + int (cw/2);
  return ret;
};

