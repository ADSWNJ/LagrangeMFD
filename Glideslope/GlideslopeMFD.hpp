//
// Glideslope 2 Ephemeral Class header
//
// Purpose ... class definitions and defined constants for Glideslope
//
// (c) Andrew Stokes (ADSWNJ) 2012-2017
//
// All rights reserved
//


#ifndef RED
#define RED RGB(255, 0, 0)
#define GREEN RGB(0, 255, 0)
#define YELLOW RGB(255, 255, 0)
#define DARK_YELLOW RGB(128, 128, 0)
#define DARK_GREEN RGB(0, 128, 0)
#define WHITE RGB(255, 255, 255)
#define BLACK RGB(0, 0, 0)
#define BLUE RGB(0, 150, 255)
#define DARK_BLUE RGB(0, 77, 162)
#define GRAY RGB(160, 160, 160)
#define BRIGHTERGRAY RGB(200, 200, 200)
#endif

#include "GSMiscFunctions.hpp"
#include "GlideslopeCore.hpp"

#ifndef _GS_EPH_MAIN_CLASS
#define _GS_EPH_MAIN_CLASS
class GlideslopeMFD: public MFD {
public:
  VESSEL *vessel;
  int lapId;
  
  //Core persistent class
  GlideslopeCore* G;

  GlideslopeMFD (DWORD w, DWORD h, VESSEL *v, UINT im);
  ~GlideslopeMFD ();
  char *ButtonLabel (int bt);
  int ButtonMenu (const MFDBUTTONMENU **menu) const;
  bool units;

  bool ConsumeKeyBuffered (DWORD key);
  bool ConsumeButton (int bt, int event);
  void Update (HDC hDC);
  void WriteStatus (FILEHANDLE scn) const;
  void ReadStatus (FILEHANDLE scn);

  void LoadConfig();
  void LoadDefaultConfig();
  void LoadRunwayGeometry();
  void LoadDefaultGlideslopeConfig (); 
  bool LoadPreferredGlideslopeConfig (char *PrefFile);
  void getMainThrustParm(VESSEL* vessel, double *F, double *isp);
  void VScale(HDC hDC,int pen,char* metFmt,char* usFmt,double usFactor, double ref, double act, double min, double max, int x, double lowarn, double hiwarn);
 
  void VTape(HDC hDC,char *title,char *metUnit,char* usUnit,int radius, int minor,double usFactor,double ref,double act,int x1, int x2);
  void HScale(HDC hDC,int pen,char* metUnit,char* usUnit,double usFactor, double act, double min, double max, int y, bool top, double lowarn, double hiwarn);
  bool checkThGroup(THGROUP_TYPE thGp);

  // Button handlers
  void menuConfig();
  void menuMode();
  void menuUnits();
  void menuAutopilot();
  void menuClearGS();
  void menuSaveGS();
  void menuHACgeometry();
  void menuResetHAC();
  void menuHACRadUp();
  void menuHACRadDn();
  void menuZoomUp();
  void menuZoomDn();
  void menuNextBase();
  void menuPrevBase();
  void menuNextRwy();
  void menuPrevRwy();
  void menuViewBase();
  void menuNextGS();
  void menuPrevGS();
  void menuDiagSel();
  void menuDeorbitSel();
  void menuOK();
  void menuDeorAB();
  void menuExtTrackSave();
  void menuQuickAccessRecorder();

  void menuViewBaseReset();
  void menuViewBaseZoomOut();
  void menuViewBaseZoomIn();
  void menuViewBaseUp();
  void menuViewBaseDown();
  void menuViewBaseLeft();
  void menuViewBaseRight();

  // variables
  int width,height,mid, col1, col2, col3;
  int line( int );
  int imfd; 
  int coreMFDref;
  int mode;
  int jumpmode;



  int MapX(double x);
  int MapY(double y);
  int MapX(AZ_RANGE a);
  int MapY(AZ_RANGE a);
  double ZoomX,OfsX,ZoomY,OfsY,ZoomMap;
  bool ManZoomToggle;
  int FlashToggle;
  double MinZoomX,MinZoomY,MinZoomMap;

  //Logging
  void LogTitle();
  void LogLine();


  HBRUSH blueBrush;
  HBRUSH dkblueBrush;
  HBRUSH blackBrush;
  HBRUSH grayBrush;
  HBRUSH greenBrush;

  HPEN whitePen;
  HPEN yellowPen;
//  HPEN ltBluePen;
//  HPEN dkBluePen;
};
#endif // _GS_EPH_MAIN_CLASS