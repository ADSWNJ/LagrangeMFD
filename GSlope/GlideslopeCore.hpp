//
// Glideslope Core Functions
//
// Purpose ... persistent class supporting the core calculations for Glideslope
// (The Glideslope class is ephemeral ... it gets destroyed and reinstantiated each time you change view, etc, so persist the core into this class)
//
//
// (c) Andrew Stokes (ADSWNJ) 2012-2017
//
// All rights reserved
//

#include "windows.h"
#include "orbitersdk.h"
#include "DisplayEngUnitFunctions.h"
#include "ParseFunctions.h"
#include "GSMiscFunctions.hpp"
#include "MFDButtonPage.hpp"
#include "GSmfdButtons.hpp"
#include "GlideslopeExports.hpp"
#include "EnjoLib/ModuleMessagingExtPut.hpp"
#include "EnjoLib/ModuleMessagingExt.hpp"
#include "BaseSyncExports.hpp"

#ifndef _GS_CORE_CLASS
#define _GS_CORE_CLASS

const double DEG2RAD   = RAD;      ///< factor to map degrees to radians
const double RAD2DEG   = DEG;      ///< factor to map radians to degrees

class GlideslopeCore : public EnjoLib::ModuleMessagingExtPut {
  public:

  const char* ModuleMessagingGetModuleName() const { return "GS2"; }
  struct GlideslopeExportTgtStruct base;

  VESSEL *vessel;
  VESSELSTATUS2 vs2;
  //Tracking variables
  int TrackPtr;														      	// Current track point
  double TrackT[10000];											  		// Simulation time
  double TrackD[10000];											  		// Range (distance)
  double TrackV[10000];											  		// Velocity (TAS)
  double TrackA[10000];											  		// Altitude
  double TrackAlpha[10000];									  		// AoA (Alpha)
  double TrackRefAlpha[10000];											
  double TrackVspd[10000];
  double TrackBeta[10000];
  double TrackGamma[10000];
  double Trackvreldot[10000];
  double TrackRefvreldot[10000];
  VECTOR3 xyz_track[10000];
  void calcRunway();
  void calcRange(); //Range to runway touchdown point via waypoints and HAC as necessary
  bool calcVacuumLand(double simt); // landing calcs to e.g. the Moon
  void MinorCycle(double simt, double simdt);
  void MajorCycle(double simt);
  void reset(VESSEL *v);
  void align_sc(double simt, double pC, double pP, double pR, double yC, double yP, double yR, double rC, double rP, double rR);
  void thrust_sc(double simt, double xP, double xR, double zP, double zR);
  void steer(double PitchCmd, bool PitchActive, double YawCmd, bool YawActive,double RollCmd, bool RollActive);
  void steerAntiHammer(VECTOR3 &thrusts, bool rot);
  void vacThrust(double RetCmd, bool RetActive,double HvrCmd, bool HvrActive);
  void vacLand(double simt);
  void DeoBurn(double simt);
  void saveUserGS(bool units, bool extendedTrackSave);
  bool VacRateFetch();
  void VacRateStore();
  bool VacRateCalibrate(double simMinDT);
  bool exTrSave;
  bool isInit;
  int coreMFDcount;
  static const int CORE_MFD_LIMIT = 10;
  int coreMFDint[CORE_MFD_LIMIT];
  int coreMode[CORE_MFD_LIMIT];
  int coreJumpMode[CORE_MFD_LIMIT];
  int coreSwitchPage[CORE_MFD_LIMIT];
  bool coreUnit[CORE_MFD_LIMIT];
  bool coreManZoomToggle[CORE_MFD_LIMIT];
  double coreZoom[CORE_MFD_LIMIT];
  GSmfdButtons coreButtons[CORE_MFD_LIMIT];

  static const int RUNWAY_LIMIT=512;
  static const int BASE_LIMIT=256;
  static const int GSFILE_LIMIT=64;
  static const int GLIDESLOPE_POINTS_LIMIT=1024;
  static const int BETA_GUIDE_OFF_ALT=20000; //meters, ~60kft
  // static const int EI_ALT=120000; // used to be 121920



  bool steerActive;
  bool diagOn;
  bool deorOn;
  int runway;
  int GSfile;
  double tdFullH; 

  int RunwayCount;
  int BaseCount;
  int GSfileCount;
  double RunwayData[RUNWAY_LIMIT][12];
  int RunwayBase[RUNWAY_LIMIT];
  char RunwayName[RUNWAY_LIMIT][12];
  char BasePlanet[BASE_LIMIT][32];
  OBJHANDLE hBasePlanet[BASE_LIMIT];
  char BaseName[BASE_LIMIT][32];
  int BaseMinRwy[BASE_LIMIT];
  int BaseMaxRwy[BASE_LIMIT];

  struct {
      char type;
      double os;
      double oe;
      double ds;
      double de;
      double len;
      double ang;
      double wid;
      POINT p[14];
  } RunwayGeometry[RUNWAY_LIMIT];

  struct {
    double x,y,s;
  } RunwayBias;

  char GSfileName[GSFILE_LIMIT][32];
  char GSname[GSFILE_LIMIT][32];
  double BaseLL[BASE_LIMIT][2];
  double BaseAlt[BASE_LIMIT];
  int ParamsModeL, ParamsModeR, ParamsModeX, ParamsRwy;
  bool ParamsUnits; 
  int RefSlopePts;
  double RefTime[GLIDESLOPE_POINTS_LIMIT];
  double RefSlope[GLIDESLOPE_POINTS_LIMIT][6];
  double RefThrust[GLIDESLOPE_POINTS_LIMIT][2][2]; // [0][x] = retro-thrust, [1][x] = hover, [x][0] = thrust force, [x][1] = thrust pct
  int maxMod;
  int hacRadius;
  int finalDist;
  double re;													//Radius of the focus planet (usually Earth)
  double rrwy;                        //Radius of the runway fron the core of the focus planet (e.g. re + 2.6 meters for KSC33). 



  //HAC runway constants
  //All vectors are of the form xxx_yyy_zzz where
  //xxx is llr latitude, longitude, radius (no zzz)
  //       xyz ECF cartesian (no zzz)
  //       nre north, radial, east relative to zzz
  //       ar  relative to zzz
  //       M   xyz-to-nre matrix for this point (no zzz)
  //yyy is the name of this point
  //zzz is the name of the reference point

  //Base coordinates
  VECTOR3 llr_base,xyz_base;									//Coordinates of our base rwy in LLR (lat,lon,radius) and XYZ format (ECEF Cartesian Coords)
  MATRIX3 M_base;												//Mapping matrix from NRE vectors to XYZ coords (e.f. for near/far ends of base rwy)
  //Runway near threshold points
  VECTOR3 xyz_near, nre_near_base, nre_near_far, nre_near_sc;	// XYZ absolue, then NRE relative to base, relative to far end and relative to spacecraft
  AZ_RANGE ar_near_far;
  //Runway far threshold points
  VECTOR3 xyz_far, nre_far_base, nre_far_sc;
  MATRIX3 M_far;
  //PAPI point
  double disp_papi;
  VECTOR3 nre_papi_far,xyz_papi;
  AZ_RANGE ar_papi_far;
  //Touchdown point
  double disp_tdp;
  VECTOR3 nre_tdp_far,xyz_tdp, llr_tdp;
  AZ_RANGE ar_tdp_far;
  //WP2 point
  double disp_wp2,ra_wp2_far;
  VECTOR3 nre_wp2_far,xyz_wp2,nre_wp2_hac,nre_wp2_sc;
  AZ_RANGE ar_wp2_far,ar_wp2_hac;
  //Runway constants
  double rwy_length;
  VECTOR3 rwy_pole;
  //HAC
  double base_radius_hac, radius_hac,ra_hac,ra_hac_far,daz_hac_far;
  VECTOR3 nre_hac_far,xyz_hac;
  AZ_RANGE ar_hac_far;
  MATRIX3 M_hac;

  VECTOR3 nre_land;
  AZ_RANGE ar_land;

  int hac_sign;													// 1 for Anti-Clockwise, -1 for clockwise
  int hac_open;													// 0 for Closed (do a lap before arming WP2), 1 for Open (less than a lap)
  int RangeSeg;													// -1 whilst outside ref glideslope
  //HAC spacecraft constants
  //Spacecraft
  VECTOR3 llr_sc,xyz_sc,nre_sc_far,nre_sc_hac,nre_hac_sc;
  AZ_RANGE ar_sc_hac,ar_hac_sc,ar_sc_far;
  MATRIX3 M_sc;
  //WP1
  AZ_RANGE ar_wp1_sc,ar_wp1_hac;
  VECTOR3 nre_wp1_sc,nre_wp1_far,nre_wp1_hac,xyz_wp1;
  double crossrange,crossrange_angle,az_tgt_sc,range_papi_sc,range;
  int leg, maxleg;
  double range_wp1_sc;
  double arc_range, hac_arc,final_range,opt_hac_alt;
  bool wp2Armed, finalArmed; 
  OBJHANDLE ourPlanet;
  double GtimesM1;
  bool hasAtmo;
  const ATMCONST* atmConst;
  double altEI; 

  ELEMENTS el;
  double elmjd;
  double elT;

  double M2;
  VECTOR3 AirspeedVec,InertialVel;
  double Lat,Lon,FullH,Altitude,AtmDensity, vspd,Groundspeed,Bearing,Mach,Airspeed,AirspeedRate,GroundspeedRate,RefAirspeedRate,DynPres,HeatFlux,InertialSpd,DelAz,alpha,beta,gamma;
  double simDT,simMinDT,lastSimMinDT,lastSimT,lastTE,lastRefTE,lastRefVspd,lastVspd,lastGroundspeed,lastBearing,lastAirspeed,lastRefAltitude,lastRefAirspeed,lastAltitude,lastDelAz,nextMajorCycle;
  double PE,KE,TE,RefPE,RefKE,RefTE,RefVspdRate,RefTERate,BearingRate,VspdRate,TERate,DelAzRate;
  double RefVspd,Refalpha,RefAirspeed,RefAltitude;
  double lastAlpha,lastBeta,lastGamma;
  double lastXrng, lastZrng, xrngRate, zrngRate;

  int simMinDTctr;
  bool simMinDTpopulated;
#define SIM_MIN_DTCTR_LIM 50
  double simMinDTtrack[SIM_MIN_DTCTR_LIM];
  double simMinDTavg;
  double simMinDTtot;
  double retThrP, hvrThrP;
  double curRetThrP, curHvrThrP;
  double curRetThr, curHvrThr;
  double maxRetThr, maxHvrThr;
  bool hasRefSlope;

  //Trim control
  double TrimPos,TrimSpd,TrimLastSimT,TrimGain;


  // Deorbit BaseSync alignment
  const struct BaseSyncExportTgtStruct *BS_trgt;
	const struct BaseSyncExportModeStruct *BS_mode;
	const struct BaseSyncExportSolStruct *BS_sol;
	const struct BaseSyncExportDeorbitStruct *BS_deo;
	const struct BaseSyncExportBurnStruct *BS_burn;

  //Deorbit Control
  bool   DeorbitActive;
  bool   DeorbitExecute;
  int    DeorbitMode;
  double DeorbitTgtV;
  double DeorbitDV;
  double DeorbitDT;
  double DeorbitT0;
  double DeorbitFactor;
  bool BaseSyncConnected;
  FILE* DeoQar;
  bool DeoQarOn;



  // Quick Access Recorder file
  FILE* GSqar;
  bool GSqarOn;
  bool GSqarInit;


  // Vacuum reentry data
  bool okVac;
  THGROUP_TYPE ThID[9];
  double ThISP[15];
  double ThMAX[15];
  double ThMFR[15];

  double TgtGC, TgtDist;        // Target great circle angle and great circle distance 
  double Txbrn;                 // Time for the xburn
  double origTTod, TTod, DTod;  // Time & distance to top of descent
  double origTHvr, THvr;        // Time to firing the hover thruster
  double origTOvh, TOvh;        // Time to getting overhead
  double origTLand, TLand;      // Time to land
  double RthrF, RthrP, RthrA;   // Predicted Retro thrust force, percent, resultant acceleration
  double HthrF, HthrP, HthrA;   // Predicted Retro thrust force, percent, resultant acceleration (net of gravity)
  VECTOR3 TgtDD, TgtDV, TgtDA;  // XYZ coordinate tracking to TDP
  double TgtDDl, TgtDVl;        // Lengths of TgtDD, TgtDV
  double TgtDT;
  int VacLandRMode;             // 0 = Inactive, 1 = Armed, 2 = RetroBurn & Align, 3 = PrecisionLand
  int VacLandHMode;             // 0 = Inactive, 1 = Pop-Up Align, 2 = XRngBrn, 3 = Aremd to Check Descent, 4 = Dscnd Burn, 5 = AltHld
  bool vacLanded; 
  double lastAZerr;

#define VACLANDR_INAC 0
#define VACLANDR_ARMD 1
#define VACLANDR_RTRO 2
#define VACLANDR_PLND 3
#define VACLANDH_INAC 0
#define VACLANDH_POPA 1
#define VACLANDH_XBRN 2
#define VACLANDH_ARMD 3
#define VACLANDH_DSND 4
#define VACLANDH_AHLD 5


  double alphaErr, betaErr, gammaErr; // Erros on each axis alignment
  double alphaRate, betaRate, gammaRate;
  double lastBetaRate, lastBetaErr;
  bool popupBurn;               // true if we are burning off a DelAz using a normal or antinormal burn
  double popupDir;                // Direction of burn ... prevents 180 flip when we just go over the zero point

  VECTOR3 VacRotRate;           // Radians/sec rates rotating on each axis
  int VacRotCalCount[3];        // How many calibration runs on each axis
  bool VacRotOK;                // Good rates loaded
  int VacCalibrationMode;      // Calibration of rot rates

  static const bool DBG_VAC_AP = true;
  static const bool DBG_VAC_LAND = true;
  FILE* vacAPouf;
  bool dbgVacAPOpen;
  FILE* vacLandouf;
  bool dbgVacLandOpen;
  VECTOR3 align_sc_deadband;    // Hysteresis for deadband... track to within 2%, then dead to 20%, etc.
  double hvrFloor;

  VECTOR3 xzrnghist[10];        // xrng in .x, simt in .y, zrng in .z
  int xzrngcnt;
  double ThrustOrientation;
  bool xrngRetroWideBand;       // Once xrng < 10m, wideband is true at 10m, and false to drive 1m 

  // Anti-hammer variables (prevents oscillating thrusts)
#define STEER_ANTI_HMAX 24
  VECTOR3 SteerAntiHammerHist[2][STEER_ANTI_HMAX];
  int SteerAHcnt[2];
  bool PlndTog;
  bool VacLandCalculated;
  double nextVacCalc;
};
#endif // _GS_CORE_CLASS