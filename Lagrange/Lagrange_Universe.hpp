// ===========================================================================================
//
//	LagrangeUniverse (Master Mathematical Definitions for all Lagrange Points in this MFD)
//	=========================================================================================
//
//  Master configuration for all LP modeled in this MFD. We define each Lagrangian Point
//  including a series of pre-computed reference values, and provide an interface to query
//  the Lagrangian Point list
//
//	Copyright (C) 2016	Keith (Keithth G) Gelling and Andrew (ADSWNJ) Stokes
//                   All rights reserved
//
//	See Lagrange.cpp
//
// ===========================================================================================

using namespace std;

#ifndef _LAGRANGE_UNIVERSE
#define _LAGRANGE_UNIVERSE

#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include "orbitersdk.h"

#define S4INT_ENTITIES 5
#define ORB_PLOT_COUNT 1000
#define ORB_MAX_LINES 4
#define ORB_PEN_WHITE 0
#define ORB_PEN_YELLOW 1
#define ORB_PEN_ORANGE 2
#define ORB_PEN_RED 3
#define ORB_PEN_MAGENTA 4
#define ORB_PEN_DASHED_AQUA 5
#define ORB_PEN_DASHED_BLUE 6
#define ORB_PEN_DASHED_VIOLET 7
#define ORB_PEN_LIGHT_GREEN 8
#define ORB_PEN_DASHED_GREEN 9
#define ORB_PEN_BRIGHT_GREEN 10
#define ORB_PEN_BRIGHT_YELLOW 11

#define COUNT_LP 10
#define COUNT_BODY 4

#define LU_SUN 0
#define LU_EARTH 1
#define LU_MOON 2
#define LU_EARTHMOONBARY 3

// 2D vector for display calculations
typedef union {
  double data[2];                  // < array data interface
  struct { double x, y;  };        // < named data interface
} VECTOR2;

// Core data streuctures for the Lagrange Universe

struct QP_struct {
  VECTOR3 Q;                                                // Position (per Keith: use Q for position vector)
  VECTOR3 P;                                                // Velocity (per Keith: use P for velocity vector)

  QP_struct& operator=(const QP_struct& x);
};

struct Lagrange_ves_s4i {                                   // Important data for each vessel and timestep
  QP_struct ves;                                            // Absolute global QP of the vessel
  QP_struct vesLP;                                          // Relative QP of the vessel w.r.t. the current lagrange point
  double mass;                                              // Vessel mass (as it changes post-burn)
  double dQ;                                                // Scalar distance to the LP
  double dP;                                                // Scalar dV to the LP
};

class Lagrange_vdata {                                      // Main vessel data structure (act/wkg)
public:
  VESSEL *v;                                                // Allows us to get the vessel mass
  int block_scan;                                           // Either 0, 1, 2. 0 = scan for minimum enc dist. 1 or 2 ... not enough points yet.
  vector<Lagrange_ves_s4i> vs4i;                            // Vessel plots (vector = number of steps)
  vector<VECTOR2> orb_km;                                   // Distance in km from center of the major entity in this LP system
  vector<VECTOR2> orb_plot;                                 // Line segments for plot lines for orb display for this vessel
  double cmass;                                             // Current mass
  double dmass;                                             // Delta to mass from burn (via rocket's main engine isp)
  bool burnArmed;                                           // Tells the integrator to use this burn data
  double burnMJD;                                           // date of hypotetical or real burn
  VECTOR3 burndV;                                           // hypothetical or real burn (prograde, out, plane)
  double enc_Q;                                             // Min encounter distance
  double enc_P;                                             // Min encounter dV
  int enc_typ;                                              // -1 if constantly growing encounter, +1 if constantly decreasing, 0 if enc found
  int enc_ix;                                               // s4i index of min encounter
  int refEnt;                                               // entity code for the frame reference for orbits and burns
  VECTOR2 orb_plot_origin;                                  // location of major entity on the screen 
  VECTOR2 orb_plot_ves_enc;                                 // X Y positions for the vessel at closest vessel encounter
  vector<VECTOR2> orb_plot_body_enc;                        // X Y positions for each body at closest vessel encounter
  Lagrange_vdata& operator=(const Lagrange_vdata& x);       // Helper function to assign vdata data in the swap function
};


struct Lagrange_s4i {                                       // Main symplectic integrator data structure
  QP_struct LP;                                             // 4th order symplectic integration of global position
  QP_struct body[COUNT_BODY];                               // Integration of each celestial body
  double sec;                                               // list of seconds into the simulation for each s4i
  double MJD;                                               // list of MJD's into the simulation for each s4i
};

struct Lagrange_orb_disp {
  vector<VECTOR2> orb_km[ORB_MAX_LINES];                    // distances in km raltive to the major entity of this LP system
  vector<VECTOR2> orb_plot[ORB_MAX_LINES];                  // Line segments for plot lines for orb display in fraction of H / W (0 = MAJ, 1 = MIN, 2 = LP)
};

class LagrangeUniverse
{
  public:
    LagrangeUniverse();                                     
    ~LagrangeUniverse();                                    
    int selectNextLP();                                     // TGT selection next LP
    int selectPrevLP();                                     // TGT selection prev LP
    void selectLP(int i);                                   // TGT selection of LP by index
    int getLP();                                            // Return LP index
    void updateUniverse();                                  // Update current positions and velocities for our LP system
    void threadCtrlMain();                                  // Main interface to worker
    void threadCtrlWorker();                                // Worker interface to main
    void threadCtrlPauseToggle();                           // Causes the worker to freeze or unfreeze
    char threadWorkerState() { return s4i_wstate; }         // Display worker state
    void integrateUniverse();                               // Calculate projected positions for our LP system (including vessels)
    void lp123(const int n, const int s);                   // Routine for calculating L1, L2, L3 lagrange points for the nth integration
    void lp45(const int n, const int s);                    // Routine for calculating L4, L5 lagrange points for the nth integration
    void lp_ves(const int s, const int i, const int w);     // Convert LP into vessel relative coords and TransX style dV, for vessel s, timestep i, working set w
    VECTOR3 s4iforce_ves(const int s, const int i);         // Force function for integrator on vessel s, iteration i
    VECTOR3 s4iforce(const int e, const int i);             // Force function for integrator 


    struct LagrangeUniverse_Body {
    public:
      int ix;                                               // Index of current body
      char name[32];                                        // Display names of each body
      double mu;                                            // GGRAV * mass of body (or of sum of bodies in bary)
      OBJHANDLE hObj;                                       // Handles to each body
      double mass;                                          // Mass of each body
      bool isBary;                                          // Is the body a barycenter?
      int b_e[COUNT_BODY+1];                                // If barycenter, composed of these bodies
    } body[COUNT_BODY];


    struct LagrangeUniverse_LP_Def {
    public:
      int nxix;                                             // Next index (propagates at the next worker thread sync)
      int ix;                                               // Index of current LP
      char name[32];                                        // Display name for each LP
      int ref;                                              // index of the refernce body for orientation
      int maj;                                              // index of the major body for this LP
      int min;                                              // index of the min body for this LP
      double alpha;                                         // Pre-calculated alpha for each LP per http://www.orbiter-forum.com/showthread.php?t=36110
      double gm1;                                           // GM for major                                       
      double gm2;                                           // GM for minor                                       
      double gm;                                            // = gm1 + gm2                                       
      double mu1;                                           // = gm1 / gm
      double mu2;                                           // = gm2 / gm
      int bodyIx[COUNT_BODY+1];                             // body indices for this system (-1 terminated)
      int baryIx[COUNT_BODY+1];                             // barycenter indices for this system (-1 terminated)
      int Lnum;                                             // Lagrange number (1-5)
      int plotix[ORB_MAX_LINES];                            // Index of current LP, or -1 if not used. Note: lporb[0] is LP, both with ix = -2 (not used)
      int plotixpen[ORB_MAX_LINES];                         // Pen color inxex
    } lptab[COUNT_LP];

    struct LagrangeUniverse_LP {
    public:
      int nxix;                                             // Next index (propagates at the next worker thread sync)
      int ix;                                               // Index of current LP
      char name[32];                                        // Display name for each LP
      int ref;                                              // index of the reference body for orientation
      int maj;                                              // index of the major body for this LP
      int min;                                              // index of the min body for this LP
      int Lnum;                                             // Lagrange number (1-5)
      double alpha;                                         // Pre-calculated alpha for each LP per http://www.orbiter-forum.com/showthread.php?t=36110
      double gm1;                                           // GM for major                                       
      double gm2;                                           // GM for minor                                       
      double gm;                                            // = gm1 + gm2                                       
      double mu1;                                           // = gm1 / gm
      double mu2;                                           // = gm2 / gm
      int bodyIx[COUNT_BODY + 1];                           // body indices for this system (-1 terminated)
      int baryIx[COUNT_BODY + 1];                           // barycenter indices for this system (-1 terminated)
      int plotix[ORB_MAX_LINES];                            // Index of current LP, or -1 if not used. Note: lporb[0] is vessel, and lporb[1] is LP, both with ix = -2 (not used)
      int plotixpen[ORB_MAX_LINES];                         // Pen color inxex
      LagrangeUniverse_LP& operator=(const LagrangeUniverse_LP_Def& x);
    } LP;

    unsigned int s4int_count[2];                            // Iteration count for inegrator
    double s4int_timestep[2];                               // Iteration timestap for integrator

    vector<Lagrange_s4i> s4i[2];                            // 4th Order symplectic integrator (vector = number of steps)
    vector<Lagrange_vdata> vdata[2];                        // Vessel 4th order integrator data (vector = number of vessels)
    Lagrange_orb_disp l_orb[2];                             // Lagrange orbit display structure (holds the plots of the bodies to plot)


    double s4int_refresh;                                   // wait time between s4i runs

    double dbg[2][7];                                       // Debug vars (wkg/act first param, diags for 2nd)
    char buf[80];
    double next_s4i_time;

    // Public thread interface vars
    atomic<int> act;
    atomic<int> wkg;
    atomic<bool> s4i_pause;
    atomic<bool> s4i_canstart;
    atomic<bool> s4i_valid;
    atomic<bool> s4i_waitrel;
    atomic<bool> dmp_log;
    atomic<bool> dmp_enc;


  protected:
  private:

    void defBody(LagrangeUniverse_Body *body, int p0, char* p1);                                 // Sets initial constants for selected body
    void defBary(LagrangeUniverse_Body *bary, int p0, char* p1, int maj, int min);               // Sets initial constants for 2-body barycenter
    void defLP(LagrangeUniverse_LP_Def *lpdef, int p0, char *p1, int Lnum, double a, int ref, int maj, int min,
                                               int oth1=-1,int oth2=-1);     // Sets initial constants for selected LP

    void defOrbPlot(LagrangeUniverse_LP_Def *lptab, int lppen, int b1 = -1, 
                                                               int b2 = -1, int b2pen = 0,
                                                               int b3 = -1, int b3pen = 0);     // Sets plot lines and colors for each LP
                                         
    // Private thread interface vars
    atomic<bool> s4i_finished;
    atomic<bool> s4i_wkill;
    mutex s4i_trafficlight[2];
    mutex s4i_pauselight;
    atomic<char> s4i_mstate, s4i_wstate;

};

VECTOR3 safe_unit(const VECTOR3 &a);

VECTOR3 safe_crossp (const VECTOR3 &a, const VECTOR3 &b);


#endif // _LAGRANGE_UNIVERSE