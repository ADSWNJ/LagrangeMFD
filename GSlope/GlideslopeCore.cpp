//
// Glideslope Core Functions
//
// Purpose ... persistent class supporting the core calculations
// (The Glideslope class is ephemeral ... it gets destroyed and reinstantiated each time you change view, etc, so persist the core into this class)
//
//
// (c) Andrew Stokes (ADSWNJ) 2012-2017
//
// All rights reserved
//

#include <stdio.h>
#include <assert.h>
#include "GlideslopeCore.hpp"



void GlideslopeCore::calcRunway() {

  if (_stricmp(base.ref, BasePlanet[RunwayBase[runway]])) {                 // If our planet is wrong, drop our planet into the default base (offset 0), and hunt for first available base
    strcpy(BasePlanet[0], base.ref);
    hBasePlanet[0] = oapiGetGbodyByName(base.ref);
    runway = 1;
    while (true) {
      if (runway>=RunwayCount) runway=0;
      if (!_stricmp(base.ref, BasePlanet[RunwayBase[runway]])) break;
      runway++;
    }
  }

  strcpy(base.name, BaseName[RunwayBase[runway]]);                          // Update the export structure
  base.lat = RunwayData[runway][0]*DEG2RAD;
  base.lon = RunwayData[runway][1]*DEG2RAD;
  base.alt = RefSlope[0][3] / 1000.0;
  base.ang = atan2(-RefSlope[0][4], RefSlope[0][0]);
  double checkang = base.ang * 180.0 / PI;
  base.ant = RefSlope[0][2]/re;
  double checkant = base.ant * 180.0 / PI;
  base.alt = RefSlope[0][3] / 1000.0;

  bool check = ModMsgPutByRef("GlideslopeTarget",2,base);                   // Put our structure into MMExt

  double COG_elev = vessel->GetCOG_elev();                                  // We land when the Center of Gravity is at this value above the runway!
// Calculate base position in XYZ and LLR
  rrwy = re+RunwayData[runway][8];                                          // Radius of runway from core of planet. Used in place of 're' to adjust for runways above 0.0 Alt.
  tdFullH = rrwy + COG_elev;                                                // Full height of touchdown point. Used for Alt calculations. 
  llr_base=_V(RunwayData[runway][0],RunwayData[runway][1],0)*PI/180.0 +_V(0,0,tdFullH);
                                                                            // Create Lat-Long-Radius coords for base. Note the vector multiplication and addition helpers 
                                                                            // here converting degree Lat/Lon to radians and then adding radius of the earth to the Alt
                                                                            // (no oblate spheroid conversion here - simple sphere) 

  xyz_base=llr_to_xyz(llr_base);                                            // Convert base coords to XYZ ECEF cartesian coordinates
  VECTOR3 checklla_base = xyz_to_lladeg(xyz_base,re);                       // Check coordinates 


// Get runway near & far end offsets from base
  nre_near_base=_V(-RunwayData[runway][2],rrwy,RunwayData[runway][3]);      // Create a North-Radial-East offset for Near End of Rwy from Base
  nre_near_base=nre_near_base/length(nre_near_base)*rrwy;                   // Scale the vector to match the runway altitude (tiny correction for the lat/lon distance)
  nre_far_base=_V(-RunwayData[runway][4],rrwy,RunwayData[runway][5]);       // Create a North-Radial-East offset for Near End of Rwy from Base
  nre_far_base=nre_far_base/length(nre_far_base)*rrwy;                      // Scale vector to match the runway altitude
  rwy_length=length(nre_near_base-nre_far_base);                            // Rwy length - subtract the relative offsets

// Set up transform from base to convert near and far to XYZ
  M_base=xyz_to_matrix(xyz_base);                                            // Create NRE:XYZ transform matrix from Base XYZ ECEF coords
  xyz_near=tmul(M_base,nre_near_base);                                      // Transpose-Multiply NRE_NEAR to XYZ coords                                     
// VECTOR3 checklla_near = xyz_to_lladeg(xyz_near,re);                       // Check coordinates 
  xyz_far=tmul(M_base,nre_far_base);                                        // Same for NRE_FAR to XYZ coords
//  VECTOR3 checklla_far = xyz_to_lladeg(xyz_far,re);                         // Check coordinates 


// Set up transform from far end to create far to near NRE, Azimuth and Range
  M_far=xyz_to_matrix(xyz_far);                                              // Create NRE:XYZ transform matrix for NRE_FAR
  nre_near_far=mul(M_far,xyz_near);                                         // Multiply (n.b. not TMUL) XYZ_NEAR into NRE relative to far end
  ar_near_far=nre_to_ar(nre_near_far,rrwy);                           // ... giving us azimuth and range from far end down rwy

//  VECTOR3 check_newnear_llr = _V(28.635137,-80.707620,0)*PI/180.0+_V(0,0,rrwy);
//  VECTOR3 check_newnear_xyz = llr_to_xyz(check_newnear_llr);
//  MATRIX3 check_M_near = xyz_to_matrix(check_newnear_xyz);
//  VECTOR3 check_llr = _V(28.632760,-80.706064,0)*PI/180.0+_V(0,0,rrwy);
//  VECTOR3 check_xyz = llr_to_xyz(check_llr);
//  VECTOR3 check_nre_near = mul(check_M_near,check_xyz);
//  check_nre_near = check_nre_near/(check_nre_near.y)*rrwy;
//  AZ_RANGE check_ar =nre_to_ar(check_nre_near,rrwy);


// Calculate PAPI XYZ, and NRE/AR relative to far end
  disp_papi=-RunwayData[runway][6];                                         // Get the PAPI offset from the Runway Data
  if (!hasAtmo) disp_papi = 0.1;                                            // In a vacuum, no PAPI needed
  ar_papi_far=ar_near_far;                                                  
  ar_papi_far.range+=disp_papi;                                             // Extend the AR_NEAR_FAR by the PAPI distance
  nre_papi_far=ar_to_nre(ar_papi_far,rrwy);                                 // Convert to NRE relative to far end
  xyz_papi=tmul(M_far,nre_papi_far);                                        // Transpose-multiply NRE into XYZ coords for PAPI
//  VECTOR3 checklla_papi = xyz_to_lladeg(xyz_papi,re);                       // Check coordinates 

// Calculate Touchdown Point XYZ, and NRE/AR relative to far end
  disp_tdp=-RunwayData[runway][7];                                          // Get the touchdown point from the Runway Data (VASI light point)
  ar_tdp_far=ar_near_far;
  ar_tdp_far.range+=disp_tdp;                                               // Shrink AR_NEAR_FAR by the touchdown point
  nre_tdp_far=ar_to_nre(ar_tdp_far,rrwy);                                   // Convert to NRE relative to far end
  xyz_tdp=tmul(M_far,nre_tdp_far);                                          // Transpose-muliply NRE into XYZ coords for TDP
  llr_tdp = xyz_to_llr(xyz_tdp);                                            // Get a lat/lon back (in radians)

  strcpy(base.name, BaseName[RunwayBase[runway]]);                          // Update the export structure
  base.lat = llr_tdp.y;
  base.lon = llr_tdp.x;


 // OBJHANDLE hPlanet = oapiGetGbodyByName(G->BasePlanet[G->BaseCount]);
 // G->BaseAlt[G->BaseCount] = oapiSurfaceElevation(hPlanet, G->BaseLL[G->BaseCount][1] * PI / 180.0, G->BaseLL[G->BaseCount][0] * PI / 180.0);

  ModMsgPutByRef("GlideslopeTarget",1,base);                                // Put our structure into MMExt - e.g. for BaseSync

  //  VECTOR3 checklla_tdp = xyz_to_lladeg(xyz_tdp,re);                         // Check coordinates 

// Calculate WP2 XYZ, and NRE/AR relative to far end
  if (!hasAtmo) finalDist = 0;                                            // No final glide in vacuum 
  disp_wp2=finalDist;                                                       // Length of final (WP2 to rwy) set by HAC geometry (default 20km)
  if (!hasAtmo) disp_wp2 = 0.1;                                            // No final glide in vacuum 
  ar_wp2_far=ar_near_far;
  ar_wp2_far.range+=disp_wp2;                                               // Extend AR_NEAR_FAR by the final distance
  nre_wp2_far=ar_to_nre(ar_wp2_far,rrwy);                             // Convert to NRE relative to far end
  xyz_wp2=tmul(M_far,nre_wp2_far);                                          // Transpose-muliply NRE into XYZ coords for TDP
//  VECTOR3 checklla_wp2 = xyz_to_lladeg(xyz_wp2,re);                         // Check coordinates 

// Calculate HAC Center XYZ, and NRE/AR relative to far end
  if (!hasAtmo) hacRadius = 0;                                            // No HAC turn in a vacuum
  base_radius_hac=hacRadius;                                                // HAC radius set by HAC geometry (default 15km)
  if (!hasAtmo) base_radius_hac = 0.1;                                            // No HAC turn in a vacuum
  radius_hac = base_radius_hac; 
  ra_wp2_far=ar_wp2_far.range/rrwy;
  ra_hac=radius_hac/rrwy;
  ra_hac_far=acos(cos(ra_wp2_far)*cos(ra_hac));            
  ar_hac_far.range=ra_hac_far*rrwy;
  daz_hac_far=acos(tan(ra_wp2_far)/tan(ra_hac_far));

  ar_hac_far.az=ar_wp2_far.az+daz_hac_far*hac_sign;
  nre_hac_far=ar_to_nre(ar_hac_far,rrwy);
  xyz_hac=tmul(M_far,nre_hac_far);                                          // Create XYZ point for HAC Center
//  VECTOR3 checklla_hac = xyz_to_lladeg(xyz_hac,re);                         // Check coordinates 


// Set up transform matrix from WP1, to create azimuth and range WP2 from WP1
  M_hac=xyz_to_matrix(xyz_hac);
  nre_wp2_hac=mul(M_hac,xyz_wp2);
  ar_wp2_hac=nre_to_ar(nre_wp2_hac,rrwy);




// rwy_pole used for crossrange calcs 
  rwy_pole=crossp(xyz_far,xyz_near);                                        // Crossproduct creates a vector normal to either end of the runway
  rwy_pole=rwy_pole/length(rwy_pole);                                       // ... make it unit length, for crossrange error calcs. 

/*sprintf(oapiDebugString(),"Runway data Base(%s) Rwy(%s) LatLon(%4.1f %4.1f) Near(%4.1f %4.1f) Far(%4.1f %4.1f) TDP PAPI(%4.1f %4.1f)",
BaseName[RunwayBase[runway]], RunwayName[runway],
RunwayData[runway][0], RunwayData[runway][1], 
RunwayData[runway][2], RunwayData[runway][3], 
RunwayData[runway][4], RunwayData[runway][5], 
RunwayData[runway][6], RunwayData[runway][7] 
);
*/

// Tidy up the glideslope to land at our runway. Works for altitudes up to 100m. (Gee - wonder what a Denver landing would look like!!)
  for (int i=0;i<RefSlopePts;i++) {
    if (RefSlope[i][3]<100.0) {RefSlope[i][3] = RunwayData[runway][8];}
  }
// Reset arming flags
  finalArmed = false;
  wp2Armed = false;
  leg=1;
  maxleg=1;



}

void GlideslopeCore::calcRange() {
                                                //This part depends on the spacecraft position
  nre_wp2_sc=mul(M_sc,xyz_wp2);								  // Get NRE of WP2 relative to ship (waypoint 2 = exit HAC into top of final descent)
  double scrossrange=dotp(rwy_pole,xyz_sc)/length(xyz_sc);	// Calculate scaled crossrange
  crossrange_angle=asin(scrossrange);						// Calculate crossrange angle
  crossrange=crossrange_angle*rrwy;							// Calculate crossrange (perpendicular left-right distance) to extended runway centerline

  nre_sc_far=mul(M_far,xyz_sc);								  // Get NRE vector of ship relative to far end of rwy
  nre_far_sc=mul(M_sc,xyz_far);								  // Get NRE vector of far end of rwy relative to ship
  ar_sc_far=nre_to_ar(nre_sc_far,rrwy);		      // Azimuth & Range ship from far end runway
  nre_near_sc=mul(M_sc,xyz_near);							  // Vector near end of runway from ship
  nre_hac_sc=mul(M_sc,xyz_hac);								  // Vector HAC center from ship

// Assess leg 3 (final) entry criteria
  if (( ( fabs(crossrange) < 2000) && (finalArmed || (( Altitude < 8000 )&&(ar_sc_far.range<50000)))) || (maxleg>2)) { // 
															                  // On final when sub 2km crossrange and final armed from leg2,
                                                // or if we skipped the hac, sub 8km alt within 50km of the end of the runway will do
    leg=3;													            // If we reach here, we are on leg 3, past the HAC on final approach
	  maxleg=3;									            			// If on final, lock us on final
    VECTOR3 nre_tdp_sc=mul(M_sc,xyz_tdp);				// Vector from ship to touchdown point
    AZ_RANGE ar_tdp_sc=nre_to_ar(nre_tdp_sc,rrwy);		// Calc azimuth & range to touchdown point from relative vector
    az_tgt_sc=ar_tdp_sc.az;									    // Azimuth to touchdown point
    range_papi_sc=ar_tdp_sc.range+disp_tdp-disp_papi;	// Range ship to the PAPI ... tdp less the VASI distance (start or rwy) less the PAPI (e.g. 20000 - 671 - 2000)
    final_range=ar_sc_far.range-rwy_length-disp_tdp;				// Actual range .. range to far end minus rwy length plus the VASI dist. (Disp TDP is negative, so this is correct!)
    range=final_range;
    opt_hac_alt = (ar_wp2_far.range-ar_tdp_far.range) * tan(20*RAD);
//sprintf(oapiDebugString(),"Leg 3 Range(FINAL)(%4.1f) Range(CRS, Angle deg)(%4.1f,%5.2f°) Range(TDP, dtdp, dpapi, far, len)(%4.1f %4.1f %4.1f %4.1f %4.1f)", range, fabs(crossrange), crossrange_angle * RAD2DEG,
//	ar_tdp_sc.range, disp_tdp, disp_papi, ar_sc_far.range, rwy_length);
    return;
  }

  // Leg 1 (descent) or 2 (in hac) common calcs
  final_range=ar_wp2_far.range-ar_tdp_far.range;// For On-HAC or Approach HAC, start with the range from HAC exit to rwy touchdown point
  range=final_range;                            // Start range calc with final glideslope
  opt_hac_alt = final_range * tan(20*RAD);      // Start optimal hac altitude with the height that gives a 20 degree final glideslope
  nre_sc_hac=mul(M_hac,xyz_sc);								  // calculate HAC center vector to ship (multiply the xyz position by the hac matrix)
  ar_sc_hac=nre_to_ar(nre_sc_hac,rrwy);			    // calculate the ship's range to the HAC center
    
// Assess leg 2 entry criteria
  if((ar_sc_hac.range<(radius_hac*1.10)) || (maxleg>1)) {	// we assess on-HAC when we get within 10% of the hac radius on this approach (or we have been here already!)
    leg=2;						            							// If we reach here, we are on leg 2, past descent phase. 
	  maxleg=2;		      						      				// If on HAC, lock out descent phase
    double az_wp2_hac=ar_wp2_hac.az*hac_sign;		// if HAC sign is inverted, we will go the opposite way round the HAC
    double az_sc_hac=ar_sc_hac.az*hac_sign;			// so do this consistently for ship's azimuth as well as WP2
    while (az_sc_hac< az_wp2_hac) az_sc_hac+=2*PI;	// make sure ship azimuth is higher (more radians) than WP2 by adding a circle's worth (2 PI) on 
    hac_arc=az_sc_hac-az_wp2_hac;							  // HAC arc is now the radian segment round the HAC from ship to wp2
	  if ((hac_arc > 1.5 * PI) && !wp2Armed) {			// We enter the HAC with WP2 exit disarmed, so we make sure we do a lap ...
														                  	//  ... without this, the arc range goes to zero as we roll round from WP1 to WP2 and then jumps up alarmingly by a full lap
		  wp2Armed = true;									          //  ... so we check if we have gone past the WP2 point (hac_arc jumps from 0 to just under 2 * PI) and if so  arm WP2 exit
 	  } else {												
		  if (!wp2Armed) hac_arc += 2*PI;						  //  ... else if not, then we need to add on a lap's worth of arc. 
	  }
      if ((hac_arc < 0.5 * PI) && wp2Armed) {		 	// We arm the finals code block (entry to leg 3) when we are within a quarter turn of completion of the hac
		  finalArmed = true;									        //  ... and WP2 exit is armed.  
 	  }

	  arc_range=hac_arc*(radius_hac+ar_sc_hac.range)/2;		// Arc range is the hac_arc angle round the circle, times the mid-point of the reference hac radius and our actual radius 
    range+=arc_range;										        // range starts with WP2 to TDP, now gets the arc range added
    opt_hac_alt += arc_range * tan(10*RAD);     // add a 10 degree hac glideslope ... this is the optimal alt for a 10 degree HAC then 20 degree final
    VECTOR3 nre_tdp_sc=mul(M_sc,xyz_tdp);				// Vector ship to the tdp
    AZ_RANGE az_range_far_sc=nre_to_ar(nre_tdp_sc,rrwy); // Convert to azimuth and range
    range_papi_sc=range+disp_tdp-disp_papi;			// This PAPI range is correct ... range to tdp less dist_tdp (which is -ve) less dist papi (which is +ve) ... 10000 - 671 - 2000
    az_tgt_sc=az_sc_hac+PI/2*-hac_sign;					// Azimuth to target is a quarter turn round from current hac (for plotting track)
//sprintf(oapiDebugString(),"Leg 2 Range(HAC, HACdeg, FINAL)(%4.1f,%4.1f,%4.1f) RAD(HAC,CUR)(%4.1f, %4.1f) ARM(WP2,FIN)(%i,%i)", arc_range, hac_arc*RAD2DEG, ar_wp2_far.range-ar_tdp_far.range, radius_hac, ar_sc_hac.range, wp2Armed?1:0, finalArmed?1:0);
    return;
  }

  // Fall through to leg 1
  leg=1;													              //If we reach here, we are on leg 1, approaching the HAC
  maxleg=1;													            // Lock us on descent phase for now
  
  double A=acos(tan(ra_hac)/tan(ar_sc_hac.range/rrwy));
  double delta_az_wp1=acos(sin(A)*cos(ra_hac))*hac_sign;	// Calculate delta azimuth for WP1 HAC edge from HAC center
  double ra_wp1_sc=atan(sin(ra_hac)*tan(A));
  if(ra_wp1_sc<0) ra_wp1_sc=PI+ra_wp1_sc;
  if(fabs(DelAz)>PI/2) ra_wp1_sc=2*PI-ra_wp1_sc;
  range_wp1_sc=ra_wp1_sc*rrwy;								    // Calculate range from the ship to WP1
  nre_hac_sc=mul(M_sc,xyz_hac);								  // Vector ship to the hac
  ar_hac_sc=nre_to_ar(nre_hac_sc,rrwy);			// Azimuth and range to the hac
  ar_wp1_sc.az=ar_hac_sc.az+delta_az_wp1;				// Azimuth to WP1 relative to our current bearing
  ar_wp1_sc.range=range_wp1_sc;								  // Range is simply the range to WP1
  az_tgt_sc=ar_wp1_sc.az;									      // Azimuth to target is the WP1 azimuth
  nre_wp1_sc=ar_to_nre(ar_wp1_sc,rrwy);			// Vector ship to WP1
  xyz_wp1=tmul(M_sc,nre_wp1_sc);							  // Absolute coords of WP1
  nre_wp1_hac=mul(M_hac,xyz_wp1);						  	// Vector HAC center to WP1
  nre_wp1_far=mul(M_far,xyz_wp1);						  	// Vector far end of rwy to WP1
  ar_wp1_hac=nre_to_ar(nre_wp1_hac,rrwy);		// Azimuth & range HAC to WP1
  double az_wp2_hac=ar_wp2_hac.az*hac_sign;			// Azimuth of Waypoint 2 (in radians). This is exit point from the HAC. 
  double az_wp1_hac=ar_wp1_hac.az*hac_sign;			// Azimuth of Waypoint 1 (in radians). This is the entry point onto the HAC circle.
  while (az_wp1_hac< az_wp2_hac) az_wp1_hac+=2*PI;	// Make WP1 bigger in radians than WP2
  hac_arc=az_wp1_hac-az_wp2_hac;							  // HAC arc is the arc segment distance from WP1 to WP2
  if (hac_open==0) {										        // If the HAC is not open ... we do a lap round then the WP1-WP2 arc then exit, so
		hac_arc +=2*PI;										          // Add on 2 PI to the HAC ARC
		wp2Armed = false;									          // And make sure the WP2 is not armed (keep HAC closed)
  } else {
	  wp2Armed = true;	        									// HAC is open, so arm WP2
  }
  arc_range=hac_arc*radius_hac*1.05;						// ARC RANGE is the distance round the HAC. We do WP1 to WP2, then 2*PI (a lap) times the radius. (Leg 2 kicks in at 1.1x HAC, averaging to 1.05 radius)
  range+=arc_range;						        					// Add to the final range
  opt_hac_alt += arc_range * tan(10*RAD);       // add a 10 degree hac glideslope ... this is now the entry alt to the HAC turn
  range+=range_wp1_sc;								      		// Add current distance to WP1
  range_papi_sc=range+disp_tdp-disp_papi;				// This PAPI range is correct ... range to tdp less dist_tdp (which is -ve) less dist papi (which is +ve) ... 10000 - 671 - 2000

//sprintf(oapiDebugString(),"Leg 1 Range(WP, HAC, HACdeg, FINAL)(%4.1f,%4.1f,%4.1f, %4.1f) Range(toHAC, L2trig)(%4.1f,%4.1f) RadHAC(BASE,ACT)(%4.1f,%4.1f) AZ1,2(%4.1f,%4.1f)", range_wp1_sc, arc_range, hac_arc*RAD2DEG, (ar_wp2_far.range-ar_tdp_far.range), ar_sc_hac.range, radius_hac*1.10,	1.0*base_radius_hac, 1.0*radius_hac, az_wp1_hac*RAD2DEG, az_wp2_hac*RAD2DEG);



}

bool GlideslopeCore::calcVacuumLand(double simt) {

// Do vacuum landing calculations:
// Leg 0 = Wait
// Leg 1 = Retro burn, with hover as needed, to above target at 2km
// Leg 2 = (HAC / not used)
// Leg 3 = Final (vertical descent)

  VECTOR3 LandD, LandV, LandA, LandT;
  double LandM = M2;
  double t;
  double BaseD; 
  double HFloor;
  bool FloorHold = true;
  double HFloorThr = 0.5;
  double Ag;
  double Ac;
  double altFl;
  double HneedA;
  double HoptA;
  bool Htrig = false;
  bool dumpToFile = (simt < 1.0); // false;
  double retroLim = 0.75;    // We usually will slow down at 0.75 thrust, except when we are close to min distance, when we will allow up to 0.95
  bool addedFirst;

  double gsdec;
  double gstgt;
  double htgt = 2000.0;

  errno_t err;
  FILE* ouf = nullptr;
//  char buf[256];
  
  if (dumpToFile) {
    if (err = fopen_s(&ouf, ".\\Config\\MFD\\Glideslope\\Diags\\VacuumCalc.csv","w") != 0) {
	    dumpToFile = false;
    } else {
	    fprintf(ouf,"t,LandD.x,LandV.x, LandA.x, LandD.y,LandD.y-BaseD,LandV.y,Ag,Ac,LandA.y,LandM,LandT.x,LandT.y,Alt,altFl,HFloor,HneedA,HoptA,HFloorThr,HFloorThr * LandAA.z,LandAA.y, HTrig,TTod,THvr,TOvh,TLand\n");
    }
  }

  VECTOR3 LandAA; // Land Acceleration Authority ... X is retro, Y is hover, Z = hover + centripital less gravity
  VECTOR3 LandMAX = _V(ThMAX[THGROUP_RETRO], ThMAX[THGROUP_HOVER], ThMAX[THGROUP_HOVER]);
  VECTOR3 LandMFR = _V(ThMFR[THGROUP_RETRO], ThMFR[THGROUP_HOVER], ThMFR[THGROUP_HOVER]);

  if (LandMAX.x == 0.0) {
    ThrustOrientation = -1;  // Aligns everything backwards for retro on mains. 
    LandMAX = _V(ThMAX[THGROUP_MAIN], ThMAX[THGROUP_HOVER], ThMAX[THGROUP_HOVER]);
    LandMFR = _V(ThMFR[THGROUP_MAIN], ThMFR[THGROUP_HOVER], ThMFR[THGROUP_HOVER]);
  }
  
  double tmpGC = oapiOrthodome(llr_tdp.x, llr_tdp.y, Lon, Lat);
  double GCangD = tmpGC * RAD2DEG;
  LandD = _V(0.0, FullH, 0);  // Start of calc run ... projecting V and H over time to a controlled landing
  if (tmpGC < TgtGC) {
    TgtDist = tmpGC * LandD.y;
  } else {
    TgtDist = (2 * PI - tmpGC) * LandD.y;
  }

  if ((0.5 * Groundspeed * Groundspeed / TgtDist) > 0.95 * LandMAX.x / LandM) {
    TgtDist = (2 * PI - tmpGC) * LandD.y;
  }
  while ((0.5 * Groundspeed * Groundspeed / TgtDist) > 0.95 * LandMAX.x / LandM) {
    TgtDist += (2 * PI) * LandD.y;
  }
  TgtGC = tmpGC;

  if ((0.5 * Groundspeed * Groundspeed / (TgtDist-1100.0) ) > 0.75 * LandMAX.x / LandM) {
    retroLim = (0.5 * Groundspeed * Groundspeed / (TgtDist-1100.0)) / (LandMAX.x / LandM);
  }

// Calc the correction burn
  DelAz=az_tgt_sc-Bearing;
  while(DelAz<-PI)DelAz+=2*PI;
  while(DelAz>PI)DelAz-=2*PI;
  Txbrn = 0.0;

  double Vp = sin(DelAz)*Groundspeed; 
  if (abs(Vp)>5.0 && (retroLim <= 0.75)) { // We need to do a 90 roll and then hover burn
    Txbrn = (Vp * M2) / (ThMAX[THGROUP_HOVER]+0.5*ThMFR[THGROUP_HOVER]);  //  v1 - v0 = F t / (m0 - 0.5*(m1-m0))
                                                                              //  (v1 - v0) m0 = Ft + 0.5 MFR t
                                                                              //  t = (v1 - v0) m0 / (F + 0.5 MFR)
    LandM -= Txbrn * ThMFR[THGROUP_HOVER];
  } // else leave to vectoring on retro
  
  LandD = _V(0.0, FullH, 0);  // Start of calc run ... projecting V and H over time to a controlled landing
  BaseD = tdFullH; // Base Distance from the body center
  LandV = _V(Groundspeed, vspd, 0); // Velocity ... assumes we have nulled out the DelAz, so it's a 2D problem now. ***NOTE*** x here is retro (i.e. ship's z), and y is alt. 
  LandA = _V(-retroLim * LandMAX.x / LandM, (LandV.x*LandV.x)/LandD.y - GtimesM1/(LandD.y*LandD.y), 0); //    Centripetal accel less the gravitational accel
  LandT = _V(0.0,0.0,0.0);
  HFloor = BaseD + 2001.0;
  t = 0.0;


  VECTOR3 xyz_sc = llr_to_xyz(llr_sc);

/*  if (TgtGC == 0.0) {
    TgtGC = tmpGC;
    TgtDT = simt;
    TgtDD = xyz_sc - xyz_tdp;
    TgtDV = AirspeedVec;
    TgtDA = _V(0,0,0);
    return false;
  } */

  TTod = 0.0;
  THvr = 0.0;
  TOvh = 0.0;
  TLand = 0.0;
  DTod = 0.0;
  RthrF = 0.0;
  RthrP = 0.0;
  RthrA = 0.0;
  HthrF = 0.0;
  HthrP = 0.0;
  HthrA = 0.0;

  TgtDA = (AirspeedVec - TgtDV) / (simt - TgtDT);
  TgtDV = AirspeedVec;
  TgtDD = xyz_sc - xyz_tdp;
  TgtDDl = length(TgtDD);
  TgtDVl = length(TgtDV);
  TgtDT = simt;

  if (Groundspeed > 1000.0) {
    gsdec = Groundspeed / 100.0;
  } else if (Groundspeed > 500.0) {
    gsdec = Groundspeed / 50.0;
  } else if (Groundspeed > 100.0) {
    gsdec = Groundspeed / 10.0;
  } else {
    gsdec = 1.0;
  }
  gstgt = Groundspeed;
  RefSlopePts = 0;

  if ((0.5 * Groundspeed * Groundspeed / TgtDist) < 0.75 * LandMAX.x / LandM) {
    RefSlope[RefSlopePts][0] = LandV.x;             // Ground speed
    RefSlope[RefSlopePts][1] = 0.0;                 // AoA - always 0 for vaclanding
    RefSlope[RefSlopePts][2] = 0.0;                 // Range
    RefSlope[RefSlopePts][3] = LandD.y - BaseD;     // Alt
    RefSlope[RefSlopePts][4] = LandV.y;             // vspd
    RefSlope[RefSlopePts][5] = 0.0;                 // always 0
    RefThrust[RefSlopePts][0][0] = 0.0;             // retro thrust (or main thrust reversed)
    RefThrust[RefSlopePts][0][1] = 0.0;             // retro thrust %
    RefThrust[RefSlopePts][1][0] = 0.0;             // hover thrust
    RefThrust[RefSlopePts][1][1] = 0.0;             // hover thrust %
    RefTime[RefSlopePts] = t;
    RefSlopePts++;
    addedFirst = true;
  } else {
    addedFirst = false;
  }


  while ((LandD.y-BaseD > 0.2 || LandV.x > 0.1) && t < elT) {
    if ((LandV.x <= gstgt) || (LandD.y - BaseD < htgt)) {
      RefSlope[RefSlopePts][0] = LandV.x;
      RefSlope[RefSlopePts][1] = 0.0;
      RefSlope[RefSlopePts][2] = -LandD.x;
      RefSlope[RefSlopePts][3] = LandD.y - BaseD;
      RefSlope[RefSlopePts][4] = LandV.y;
      RefSlope[RefSlopePts][5] = 0.0;
      RefThrust[RefSlopePts][0][0] = LandT.x * LandMAX.x;
      RefThrust[RefSlopePts][0][1] = LandT.x;
      RefThrust[RefSlopePts][1][0] = LandT.y * LandMAX.y;
      RefThrust[RefSlopePts][1][1] = LandT.y;
      RefTime[RefSlopePts] = t;
      RefSlopePts++;
      if (LandV.x <= gstgt) gstgt -= gsdec;
      if (LandD.y - BaseD < htgt) htgt -= 100.0;
    }


    LandD.x = LandD.x + LandV.x + 0.5 * LandA.x; // assumes dt is always 1.0
    LandV.x += LandA.x; // assumes dt is always 1.0

    LandD.y = LandD.y + LandV.y + 0.5 * LandA.y; 
    LandV.y += LandA.y;

    LandM -= LandMFR.x * LandT.x + LandMFR.y * LandT.y;
    t += 1.0;

    if (LandV.x < 5.0 && TOvh == 0.0) {
        HFloor = BaseD;
        HFloorThr = 0.15;
        FloorHold = false;
        Htrig = false;
        TOvh = t;
    }

    Ag = - GtimesM1/(LandD.y*LandD.y);
    Ac = (LandV.x*LandV.x)/LandD.y;
    LandAA = LandMAX / LandM;
    LandAA.z += Ag + Ac;

    if (LandV.x < 4 * LandAA.x) {                     // Binary chop it down to zero
      LandT.x = 0.5 * LandV.x / LandAA.x;
    } else {
      LandT.x = retroLim;                             // over 4s of full thrust ... leave a bit of thrust for corrections
    }

    LandT.x = LandT.x > retroLim ? retroLim : LandT.x < 0.0 ? 0.0 : LandT.x;
    LandA.x = - LandT.x * LandMAX.x / LandM;

    altFl = LandD.y - HFloor;
    if (altFl < 2.0 && HFloor > BaseD + 2000.0 && THvr == 0.0) {
      THvr = t;
    }

    HneedA = LandV.y*LandV.y/(2*altFl);
    if (altFl * LandV.y > 0.0) {
      HneedA = (altFl > 0.0)? - HneedA - 0.01 : - HneedA + 0.01;
    }
    HoptA = HneedA -Ag - Ac;

    if (!Htrig) {
      Htrig = HneedA > HFloorThr * LandAA.z || altFl < 1.0;  // Triggers start of retros, when the vvel gets to the Floor threshhold acceleration limit
      if (Htrig && THvr == 0.0) THvr = t;
    }

    if (LandD.y - BaseD < 0.2) {          // Land achieved
      Htrig = false;
      LandT.y = 0.0;
      if (TLand == 0.0) {
        TLand = t; 
      }
    } else if (abs(altFl)<5.0 && abs(LandV.y) < 0.2) {
      LandT.y = (-(Ag+Ac)/LandAA.y);
      if (altFl > 1.00) {
        LandT.y *= 0.99;
      } else if (altFl < 1.00) {
        LandT.y *= 1.01;
      }
    } else if (Htrig) {
      LandT.y = HoptA / LandAA.y;       // Modulate acceleration to hit velocity and distance goals
    } else {
      LandT.y = 0;
    }

    LandT.y = LandT.y > 1.0 ? 1.0 : LandT.y < 0.0 ? 0.0 : LandT.y;
    LandA.y = LandT.y * LandAA.y + Ag + Ac;
    LandM = LandM - LandT.x * LandMFR.x - LandT.y * LandMFR.y;

    if (dumpToFile) {
      fprintf(ouf,

      "%f, %.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%s,%.3f,%.3f,%.3f,%.3f\n",

      t,LandD.x,LandV.x, LandA.x, LandD.y,LandD.y-BaseD,LandV.y,Ag,Ac,LandA.y,LandM,LandT.x,LandT.y,
      LandD.y - BaseD,altFl,HFloor,HneedA,HoptA,HFloorThr,HFloorThr * LandAA.z, LandAA.y, (Htrig?"Y":"N"), TTod,THvr,TOvh,TLand);
    }


  }

  if ((0.5 * Groundspeed * Groundspeed / TgtDist) > 0.74 * LandMAX.x / LandM) {
      DTod = 0.0; // Already at or past TOD
  } else {
      DTod = TgtDist - LandD.x - 1100.0; // Aim to end 1100m short of target at 2000m alt, then descend to overhead at 100m, then 100m straight down
  }
  TTod = simt + DTod / Groundspeed; 
  THvr += TTod;
  TOvh += TTod;
  TLand += TTod;
  if (origTTod == 0.0) origTTod = TTod;
  if (origTHvr == 0.0) origTTod = THvr;
  if (origTOvh == 0.0) origTTod = TOvh;
  if (origTLand == 0.0) origTTod = TLand;


//  sprintf(oapiDebugString(), "TgtDist: %.3f   GCang: %.0f (%.2f)  Delta: Dist: %.0f X: %.0f Z: %0.f dX: %.3f dZ: %.3f aX: %.3f aZ: %.3f TTOD: %.0f  THLD: %.0f  TEXH: %.0f  THVR: %.0f  TLND: %.0f", 
//    TgtDist/1000, TgtGC * RAD2DEG, (2 * PI - TgtGC) * RAD2DEG, TgtDDl, TgtDD.x, TgtDD.z, TgtDV.x, TgtDV.z, TgtDA.x, TgtDA.z, Ttod, Thld, Texh, Thvr, Tlnd);

  for (int i=0; i<RefSlopePts; i++) {
    double alt = RefSlope[i][3];
    RefSlope[i][2] += LandD.x + ((alt>2000.0)? 1100.0 : 1100.0 * alt/2000.0);
    if (RefSlope[i][0]<5.00) RefSlope[i][0] = (alt>2000.0)? 5.0 : 5.0 * alt/2000.0;
    RefTime[i] += TTod;
  }
  RefSlope[0][2] = TgtDist+0.0001;
  RefTime[0] = simt - 0.000001;

  RefSlope[RefSlopePts][0] = 0.0;
  RefSlope[RefSlopePts][1] = 0.0;
  RefSlope[RefSlopePts][2] = 0.0;
  RefSlope[RefSlopePts][3] = 0.0;
  RefSlope[RefSlopePts][4] = 0.0;
  RefSlope[RefSlopePts][5] = 0.0;
  RefThrust[RefSlopePts][0][0] = 0.0;
  RefThrust[RefSlopePts][0][1] = 0.0;
  RefThrust[RefSlopePts][1][0] = 0.0;
  RefThrust[RefSlopePts][1][1] = 0.0;
  RefTime[RefSlopePts] = TLand;
  RefSlopePts++;
  RefSlope[RefSlopePts][0] = 0.0;
  RefSlope[RefSlopePts][1] = 0.0;
  RefSlope[RefSlopePts][2] = 0.0;
  RefSlope[RefSlopePts][3] = 0.0;
  RefSlope[RefSlopePts][4] = 0.0;
  RefSlope[RefSlopePts][5] = 0.0;
  RefThrust[RefSlopePts][0][0] = 0.0;
  RefThrust[RefSlopePts][0][1] = 0.0;
  RefThrust[RefSlopePts][1][0] = 0.0;
  RefThrust[RefSlopePts][1][1] = 0.0;
  RefTime[RefSlopePts] = TLand+20000; // Locks the slope to zero way past landing time
  RefSlopePts++;

  if (dumpToFile) {
	    fprintf(ouf,"\n\nt,TAS,AoA,RNG,ALT,VSP,R(N),R%%,H(N),H%%\n");
      for (int i=0; i<RefSlopePts; i++) {
  	    fprintf(ouf,"%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,\n",
          RefTime[i],
          RefSlope[i][0],          RefSlope[i][1],          RefSlope[i][2],          RefSlope[i][3],          RefSlope[i][4],
          RefThrust[i][0][0],RefThrust[i][0][1],RefThrust[i][1][0],RefThrust[i][1][1]
          );
      }
      fprintf(ouf,"-End-\n");

  }


  if (TTod > origTTod && origTTod != 0.0) {
    TTod = origTTod;
  } else {
    origTTod = TTod;
  }
  if (THvr > origTHvr && origTHvr != 0.0 && simt > origTHvr) {
    THvr = origTHvr;
  } else {
    origTHvr = THvr;
  }
  if (TOvh > origTOvh && origTOvh != 0.0 && simt > origTOvh) {
    TOvh = origTOvh;
  } else {
    origTOvh = TOvh;
  }
  if (TLand > origTLand && origTLand != 0.0 && simt > origTLand) {
    TLand = origTLand;
  } else {
    origTLand = TLand;
  }
  if (dumpToFile) {
    fclose(ouf);
  }
  VacLandCalculated = true;
  return true;
}

void GlideslopeCore::vacLand(double simt) {

  double fallHeight;
  hvrThrP = 0.0;
  retThrP = 0.0;
  double Vxr = 0.0;
  double Tp = 0.0;
  double MaxAr = (ThrustOrientation > 0.0) ? ThMAX[THGROUP_RETRO] / M2 : ThMAX[THGROUP_MAIN] / M2;
  double MaxAh = ThMAX[THGROUP_HOVER] / M2;
  double dVr = MaxAr * simMinDT;
  double dVh = MaxAh * simMinDT;
  double DesiredAr = 0.0;
  double DesiredAh = 0.0;
  double AgAc = 0.0;
  hvrFloor = tdFullH + 6000.5;
  double xrng = 0.0;
  double zrng = 0.0; 
  double betaBias = 0.0;
  double azErr = 0.0;
  double azErrRate = 0.0;
  double yawErr = 0.0;
  bool alignLockout = false;
  double yaw = vessel->GetYaw();
  while (yaw < -PI) yaw += 2 * PI;
  while (yaw > +PI) yaw -= 2 * PI;

  if (Altitude > 20.0) vacLanded = false;

  if ((VacLandRMode != VACLANDR_INAC) && oapiGetTimeAcceleration() > 100.0) oapiSetTimeAcceleration(10.0);
  //if (Altitude < 150.0 && oapiGetTimeAcceleration() > 1.0 && VacLandRMode == VACLANDR_PLND) oapiSetTimeAcceleration(1.0);

  if (steerActive) {
    // Ensure we default to nothing at all is thrusting
    vessel->SetThrusterGroupLevel(THGROUP_ATT_PITCHUP,0.0);
    vessel->SetThrusterGroupLevel(THGROUP_ATT_PITCHDOWN,0.0);
    vessel->SetThrusterGroupLevel(THGROUP_ATT_YAWLEFT,0.0);
    vessel->SetThrusterGroupLevel(THGROUP_ATT_YAWRIGHT,0.0);
    vessel->SetThrusterGroupLevel(THGROUP_ATT_BANKLEFT,0.0);
    vessel->SetThrusterGroupLevel(THGROUP_ATT_BANKRIGHT,0.0);
    vessel->SetThrusterGroupLevel(THGROUP_ATT_FORWARD,0.0);
    vessel->SetThrusterGroupLevel(THGROUP_ATT_BACK,0.0);
    vessel->SetThrusterGroupLevel(THGROUP_ATT_LEFT,0.0);
    vessel->SetThrusterGroupLevel(THGROUP_ATT_RIGHT,0.0);
    vessel->SetThrusterGroupLevel(THGROUP_ATT_UP,0.0);
    vessel->SetThrusterGroupLevel(THGROUP_ATT_DOWN,0.0);
    vessel->SetThrusterGroupLevel(THGROUP_MAIN,0.0);
    vessel->SetThrusterGroupLevel(THGROUP_RETRO,0.0);
    vessel->SetThrusterGroupLevel(THGROUP_HOVER,0.0);
  }

  nre_land=mul(M_sc,xyz_tdp);									// Vector touchdown point from ship
  ar_land=nre_to_ar(nre_land,tdFullH);								// Azimuth & range touchdown point, from ship

  alphaErr = alpha;            // nose flat


  if (VacLandRMode==VACLANDR_PLND) {
    azErr = ar_land.az - yaw;   // When in precision land, we want to track bearing only
    yawErr = azErr;
  } else {
    azErr = ar_land.az - Bearing;
    yawErr = beta - Bearing;
    if (ThrustOrientation < 0) yawErr -= PI;
  }
  while (yawErr < -PI) yawErr += 2 * PI;
  while (yawErr > +PI) yawErr -= 2 * PI;
  while (azErr < -PI) azErr += 2 * PI;
  while (azErr > +PI) azErr -= 2 * PI;
  Vxr = sin(azErr)*Groundspeed;                                                  // Looking at whether to do a pop-up burn
  Tp = abs(Vxr / MaxAh);

  azErrRate = (azErr - lastAZerr) / simMinDT;
  xrng = sin(azErr)*TgtDist;
  zrng = cos(azErr)*TgtDist;

  if (VacLandRMode==VACLANDR_PLND) {
    xrng = sin(azErr)*ar_land.range;
    zrng = cos(azErr)*ar_land.range;
  }

  if (xzrngcnt == -1) {   // One-off inits needed

    for (int i=0; i<STEER_ANTI_HMAX; i++) {
      SteerAntiHammerHist[0][i] = _V(0.0,0.0,0.0);
      SteerAntiHammerHist[1][i] = _V(0.0,0.0,0.0);
    }
    for (int i=0; i<10; i++) {
      xzrnghist[i] = _V(xrng,0,zrng);
    }
    SteerAHcnt[0] = 0;
    SteerAHcnt[1] = 0;
    xzrngcnt++;
  }

  xzrnghist[xzrngcnt] = _V(xrng,simt,zrng);
  xzrngcnt = (xzrngcnt+1) % 10;
  xrngRate = (xrng - xzrnghist[xzrngcnt].x) / (simt-xzrnghist[xzrngcnt].y); 
  zrngRate = (zrng - xzrnghist[xzrngcnt].z) / (simt-xzrnghist[xzrngcnt].y); 

  lastAZerr = azErr;
  lastXrng = xrng;
  lastZrng = zrng;

  betaErr = yawErr;       // aligned to the target
  while (abs(betaErr) > PI) betaErr = betaErr<0.0? betaErr += 2*PI  : betaErr -= 2*PI;
  gammaErr = -gamma;            // no roll
  TgtDist = ar_land.range;
  range = TgtDist;

  lastBetaRate = betaRate;
  betaRate = (betaErr - lastBetaErr) / simMinDT;
  if (abs(betaErr)>1e10) betaErr = 0.0;
  if (abs(betaRate)>1e10) betaRate = 0.0;
  if (abs(betaRate)>10.0 * abs(lastBetaRate) && abs(lastBetaRate) > 1e-6) betaRate = lastBetaRate; // Throw out single cyle rate anomalies




  //
  // Retro Controls
  //
  switch (VacLandRMode) {
  case VACLANDR_INAC:               // Inactive (> 3 mins to TOD, or landed)
    if (TTod+Tp > simt+300.0) {                                                                    // Too far away ... go quiet for now
      VacLandRMode = VACLANDR_INAC;
      VacLandHMode = VACLANDH_INAC;
    } else {
      VacLandRMode = VACLANDR_ARMD;
    }
    alignLockout = true;
    break;
  case VACLANDR_ARMD:               // Armed (< 3 mins TOD)
    if (TTod+Tp > simt + 320.0) {
      VacLandRMode = VACLANDR_INAC;
      break;
    }
    if (TTod < simt) {
      VacLandRMode = VACLANDR_RTRO; // Trigger retro phase at TOD (calculated from calcVacuumLand()).
      VacLandHMode = VACLANDH_ARMD; // Switch to descend burn mode
    } else if ((abs(Vxr)>5.0) && (TTod + Tp > simt + 10.0)) {
      popupBurn = true;
      popupDir = (Vxr > 0) ? 1.0 : -1.0;
      VacLandHMode = VACLANDH_POPA;
    }
    break;

  case VACLANDR_RTRO:               // Retro burn (slow to zero groundspeed overhead target)
    if (Groundspeed < 5.0) {
      VacLandRMode = VACLANDR_PLND; // Switch to position hold (hold over target whilst hover engine controls descent)
      VacLandHMode = VACLANDH_DSND; // Switch to descend burn mode
    } else {
      DesiredAr = 0.5 * Groundspeed * Groundspeed / (TgtDist-1100.0);
      if (DesiredAr < 0.0) {
        DesiredAr = 0.5 * Groundspeed * Groundspeed / (TgtDist+0.01);
      }
      if (DesiredAr > 0.5 * MaxAr) {
        if (dVr > Groundspeed) {
          retThrP = Groundspeed / dVr;
        } else {
          retThrP = DesiredAr / MaxAr;
        }
        if (retThrP > 1.0) retThrP = 1.0;
      }
    }

    if (TgtDist < 10000.0) break;                                                           // No more beta bias within the last 10km
    if (xrngRetroWideBand && abs(azErr)<0.6*DEG2RAD && abs(azErrRate)<0.1*DEG2RAD) break;   // If we are in wide band mode, then allow xrng to drift up to 10m
    xrngRetroWideBand = false;                        // ... else tighten it again to 1m
    if (!xrngRetroWideBand && abs(azErr)<0.12*DEG2RAD && abs(azErrRate)<0.03*DEG2RAD) {
      xrngRetroWideBand = true;
      break;
    }
    betaBias = 1.0 * azErr + 8.0 * azErrRate; // drive the yaw to take out the crossrange error. PD control loop, damping on rate to reduce overshoot. 
    if (betaBias > 15.0*DEG2RAD) betaBias = 15.0*DEG2RAD;
    if (betaBias < -15.0*DEG2RAD) betaBias = -15.0*DEG2RAD;
    betaErr +=  betaBias;
    break;

  case VACLANDR_PLND:               // Precision Landing mode (track X and Z errors to zero, regulating hover floor)
    if (TgtDist > 1000.00) {
      hvrFloor = tdFullH + 6000.0;
    } else if (TgtDist > 100.00) {
      hvrFloor = tdFullH + 2000.0 + 4000.0 * ((TgtDist - 100) * (TgtDist - 100) / (900.0 * 900.0));
    } else if (TgtDist > 10.00) {
      hvrFloor = tdFullH + 200.0 + 1800.0 * ((TgtDist-10) * (TgtDist - 10) / (90.0 * 90.0));
    } else if (TgtDist > 1.00) {
      hvrFloor = tdFullH + 2.0 * ((TgtDist-1)*(TgtDist - 1) /(9.0 * 9.0));
    } else {
      hvrFloor = tdFullH - 5.0;
    }

    if (Altitude < 5.0 && VacLandHMode == VACLANDH_DSND && abs(vspd)<0.01) {
      VacLandRMode = VACLANDR_INAC;
      VacLandHMode = VACLANDH_INAC;
      vessel->SetThrusterGroupLevel(THGROUP_ATT_PITCHUP, 0.0);
      vessel->SetThrusterGroupLevel(THGROUP_ATT_PITCHDOWN, 0.0);
      vessel->SetThrusterGroupLevel(THGROUP_ATT_YAWLEFT, 0.0);
      vessel->SetThrusterGroupLevel(THGROUP_ATT_YAWRIGHT, 0.0);
      vessel->SetThrusterGroupLevel(THGROUP_ATT_BANKLEFT, 0.0);
      vessel->SetThrusterGroupLevel(THGROUP_ATT_BANKRIGHT, 0.0);
      vessel->SetThrusterGroupLevel(THGROUP_ATT_FORWARD, 0.0);
      vessel->SetThrusterGroupLevel(THGROUP_ATT_BACK, 0.0);
      vessel->SetThrusterGroupLevel(THGROUP_ATT_LEFT, 0.0);
      vessel->SetThrusterGroupLevel(THGROUP_ATT_RIGHT, 0.0);
      vessel->SetThrusterGroupLevel(THGROUP_ATT_UP, 0.0);
      vessel->SetThrusterGroupLevel(THGROUP_ATT_DOWN, 0.0);
      vessel->SetThrusterGroupLevel(THGROUP_MAIN, 0.0);
      vessel->SetThrusterGroupLevel(THGROUP_RETRO, 0.0);
      vessel->SetThrusterGroupLevel(THGROUP_HOVER, 0.0);
      vacLanded = true;
      steerActive = false;
      break;
    } else {
      if (steerActive) vacLanded = false;
    }

    PlndTog = !PlndTog;
    beta = 0.0;   // Stops spinning round when we are on top of the target
    betaErr = 0.0;
    if (PlndTog && ((abs(alphaErr)<2.0*DEG2RAD) || (abs(betaRate)<0.02*DEG2RAD) || (abs(gammaErr)<2.0*DEG2RAD))) {
      alignLockout = false;
    } else {
      alignLockout = true;
      if (abs(xrng)>0.5 || abs(zrng)>0.5 || abs(xrngRate)>0.1 || abs(zrngRate)>0.1)
        thrust_sc(simt, xrng, xrngRate, zrng, zrngRate);    
    }
    break;
  default:
      VacLandRMode = VACLANDR_INAC; 
      VacLandHMode = VACLANDH_INAC;
  }

  //
  // Hover and alignment controls
  //
  switch (VacLandHMode) {
  case VACLANDH_INAC:               // Inactive (> 3 mins to TOD, or landed)
    break;

  case VACLANDH_POPA:               // Align for Pop-Up Burn
    popupBurn = (TTod + Tp > simt + 30.0); // Don't attempt the burn inside 30 secs (need to be ready for the retro phase)
    if (popupBurn) {
      gammaErr = (Vxr < 0.0) ? ThrustOrientation * 90.0 * DEG2RAD - gamma : ThrustOrientation * -90.0 * DEG2RAD - gamma;                     // Figure out which way to roll
      if (abs(alphaErr) < 1.0 * DEG2RAD && abs(betaErr) < 5.0 * DEG2RAD && abs(gammaErr) < 1.2 * DEG2RAD) {
        VacLandHMode = VACLANDH_XBRN;
      } else {
        break;
      }
    } else {
      VacLandHMode = VACLANDH_ARMD;
      break;
    }

  case VACLANDH_XBRN:               // Pup-Up Burn (to eliminate Crossrange delta)
    popupBurn = (abs(Vxr) > 0.05) && (TTod + Tp > simt + 10.0) && (Vxr * popupDir > 0.0);            // Terminate if we are close to 0 Vp, or close to Top Descent, or Vp has just flipped direction
    gammaErr = (Vxr < 0.0) ? ThrustOrientation * 90.0 * DEG2RAD - gamma : ThrustOrientation * -90.0 * DEG2RAD - gamma;                     // Figure out which way to roll
    if (popupBurn) {
      if (abs(Vxr) > 100 * dVh) {
        hvrThrP = 1.0;
      } else if (abs(Vxr) > 30 * dVh) {
        hvrThrP = 0.50;
      } else {
        hvrThrP = 0.10 + (0.40 / 50.0) * (abs(Vxr) / (dVh));
      }
    } else {
      VacLandHMode = VACLANDH_ARMD; // Align for retro, plus vspeed protection
    }
    break;

  case VACLANDH_ARMD:               // Armed waiting for 0.5 x MaxAh trigger
    if (abs(FullH - hvrFloor) < 1.0) {
      VacLandHMode = VACLANDH_AHLD;
    } else {
      DesiredAh = 0.5 * vspd * vspd / (FullH - hvrFloor);
      AgAc = (GtimesM1 / FullH / FullH )-(Groundspeed * Groundspeed / FullH);
      DesiredAh += AgAc;
      if (DesiredAh > 0.5 * MaxAh) {
        VacLandHMode = VACLANDH_DSND;
        hvrThrP = DesiredAh / MaxAh;
      }
    }
    break;

  case VACLANDH_DSND:               // Descent Burn to control vertical speed
    fallHeight = FullH - hvrFloor;
    if (fallHeight < 5.00 && Altitude > 25.0) {
      VacLandHMode = VACLANDH_AHLD;
    } else {
      double dHeight = (FullH - hvrFloor);
      double desiredVspd = 0.2 * dHeight;
      if (VacLandRMode == VACLANDR_PLND) {
        if (dHeight > 1000.0) { desiredVspd = 15.0; }
        else if (dHeight > 10.0) { desiredVspd = dHeight * 15.0 / 1000.0; }
        else { desiredVspd = 0.08; };
      }
      if (desiredVspd < 0.05) desiredVspd = 0.05;
      double brakeAh;
      if (fallHeight > 100.0) {
        brakeAh = (vspd >= 0.0) ? 0.0 : 0.5 * vspd * vspd / (FullH - hvrFloor + 10.0);
      } else if (fallHeight > 10.0) {
        brakeAh = (vspd >= 0.0) ? 0.0 : 0.5 * vspd * vspd / (FullH - hvrFloor + 1.0);
      } else if (fallHeight > 1.0) {
        brakeAh = (vspd >= 0.0) ? 0.0 : 0.5 * vspd * vspd / (FullH - hvrFloor + 0.10);
      } else {
        brakeAh = (vspd >= 0.0) ? 0.0 : 0.5 * vspd * vspd / (FullH - hvrFloor - 0.04);
      }
      double brakeVspd = sqrt(2.0 * brakeAh * dHeight);
      double MinAh = 0.0;
      if (dHeight < 2000.0) MinAh = 0.1 * MaxAh;
      AgAc = (GtimesM1 / FullH / FullH )-(Groundspeed * Groundspeed / FullH);

      if (brakeAh + AgAc > 0.8 * MaxAh) {
        DesiredAh = brakeAh + AgAc;
      } else {
        DesiredAh = AgAc * (-vspd) /desiredVspd;
        if (DesiredAh < MinAh) DesiredAh = MinAh;
      }
      hvrThrP = DesiredAh / MaxAh;
      if (hvrThrP > 1.0) hvrThrP = 1.0;
    }
    break;

  case VACLANDH_AHLD:               // Altitude hold - i.e. counterbalance gravity only
    AgAc = (GtimesM1 / FullH / FullH )-(Groundspeed * Groundspeed / FullH);
    if (FullH - hvrFloor > 2.0) {
      DesiredAh = AgAc * 0.95;
    } else if (FullH - hvrFloor < -2.0) {
      DesiredAh = AgAc * 1.05;
    } else {
      DesiredAh = AgAc;
    }
    if (vspd < -dVh) {
      hvrThrP = 1.0;
    } else if (abs(vspd) <= dVh) {
      hvrThrP = (vspd / -dVh) + (DesiredAh / MaxAh); 
    } else {
      hvrThrP = DesiredAh / MaxAh; 
    }
    if (hvrThrP > 1.0) hvrThrP = 1.0;
    if (hvrThrP < 0.0) hvrThrP = 0.0;
    if (FullH - hvrFloor > 1.00) {
      VacLandHMode = VACLANDH_DSND;
    }
    break;
  }


  // Align and Thrust if we are active and not yet landed
  if (steerActive) {
    if ((VacLandHMode != VACLANDH_INAC) || (VacLandRMode != VACLANDR_INAC)) {
      if (!alignLockout) align_sc(simt, alpha, alphaErr, alphaRate, beta, betaErr, betaRate, -gamma, gammaErr, -gammaRate);                              
      vacThrust(ThrustOrientation * retThrP, true, hvrThrP, true); 
    }
  }
  
  if (DBG_VAC_LAND) {
    char rmodestr[4][5] = {"INAC", "ARMD", "RTRO", "PLND"};
    char hmodestr[6][5] = {"INAC","POPA","XBRN","ARMD","DSND","AHLD"};

    //sprintf(oapiDebugString(), "TgtDist(km): %.3f RMode: %s HMode: %s TTOD: %.0f THvr: %.0f  TOvh: %.0f TLAND: %.0f Perr: %+.2f Yerr: %+.2f Rerr: %+.2f ret%%: %.0f hvr%%: %.0f  Brng: %.1f TgtAz: %.1f, BetaErr: %0.3f, BetaRate: %0.3f, Vxr: %+.3f, XRng: %+.1f", 
    //  TgtDist/1000, rmodestr[VacLandRMode], hmodestr[VacLandHMode], TTod, THvr, TOvh, TLand, (abs(alphaErr) < 1e-3? 0: alphaErr*RAD2DEG), (abs(betaErr) < 1e-3? 0: betaErr*RAD2DEG),
    //  (abs(gammaErr) < 1e-3? 0: gammaErr*RAD2DEG), retThrP*100.0, hvrThrP*100.0, Bearing*RAD2DEG, az_tgt_sc*RAD2DEG, betaErr*RAD2DEG,betaRate*RAD2DEG, Vxr, xrng);

    if (!dbgVacLandOpen) {
      if (fopen_s(&vacLandouf, ".\\Config\\MFD\\Glideslope\\Diags\\GSvacLandAP.csv","w")==0) {
        dbgVacLandOpen = true;
	      fprintf(vacLandouf,"T,dT,dtAvg,  Ttod, THvr, TOvh, TLand, RMode, HMode, Alpha, Beta, Gamma, Vxr, Tp, dVr, dVh, hvrFlrAlt, hvrThrP, retThrP,  Aerr, Berr, Gerr, AZerr, azErrRate, YawErr, BetaBias, AZTgt,   VSpd, Alt, GrndSpd, TgtDist, Racc, Mass, DesAr, MaxAr, DesAh, AgAc, Des2Ah, MaxAh, Brng, TgtAz, simt, BetaErr, BetaRate, Ctrl, BetaBias, Xrng, ZRng, TgtDist, ARLandAZ, ARLandRng\n");
      }
    }

    if (dbgVacLandOpen && (VacLandHMode != VACLANDH_INAC || VacLandRMode != VACLANDR_INAC)) fprintf(vacLandouf,
		
"%.3f,%.3f,%.3f, \
%.1f,%.1f,%.1f,%.1f, \
%s, %s, \
\
%.3f, %.3f, %.3f, %.3f, %.3f, \
%.3f, %.3f, %.1f, %.2f, %.2f, \
%.3f, %.3f, %.3f, %.3f, %.3f, %.3f,  \
\
%.3f, %.2f, %.5f, %.2f, %.3f,  \
%.1f, %.1f,  %.1f, %.1f,  \
\
%.3f, %.0f, %.1f, %.1f,%.1f,%.1f,%.1f, \
%.1f,%.1f,%.1f, \
%.5f,%.5f,%.5f,%.5f, \
\
%+.4f, %+.4f, %+.2f, \
\n",

simt, simMinDT, simMinDTavg, 
TTod, THvr, TOvh, TLand, 
rmodestr[VacLandRMode], hmodestr[VacLandHMode], 

alpha*RAD2DEG, beta*RAD2DEG, gamma*RAD2DEG, Vxr, Tp,
dVr, dVh, hvrFloor - re, hvrThrP, retThrP, 
alphaErr*RAD2DEG, betaErr*RAD2DEG, gammaErr*RAD2DEG, azErr*RAD2DEG, azErrRate*RAD2DEG, yawErr*RAD2DEG,

betaBias*RAD2DEG, az_tgt_sc*RAD2DEG, vspd, Altitude, Groundspeed,
TgtDist, dVr/simMinDT, M2, DesiredAr,

MaxAr, DesiredAh - AgAc, AgAc, DesiredAh, MaxAh, Bearing*RAD2DEG, az_tgt_sc*RAD2DEG,
simt, betaErr*RAD2DEG, betaRate*RAD2DEG, 
betaErr*RAD2DEG, betaBias*RAD2DEG, xrng, zrng,

TgtDist, ar_land.az*RAD2DEG, ar_land.range/1000);
  }
}

void GlideslopeCore::MinorCycle(double simt, double simdt) {

  simMinDT = simdt;

  if (simMinDTpopulated) {
    double simdtlim = (simdt>1.2 * simMinDTavg? 1.2 * simMinDTavg : simdt);
    simMinDTtot = simMinDTtot - simMinDTtrack[simMinDTctr] + simdtlim;
    simMinDTtrack[simMinDTctr] = simdtlim;
    simMinDTavg = simMinDTtot/50.0;
    simMinDTctr++;
    if (simMinDTctr==50) simMinDTctr=0;
  } else {
    simMinDTtrack[simMinDTctr++] = simdt;
    if (simMinDTctr==1) {
      simMinDTtot = simdt;
    } else {
      simMinDTtot += simdt;
    }
    simMinDTavg = simMinDTtot/simMinDTctr;
    if (simMinDTctr == 50) {
      simMinDTpopulated = true;
      simMinDTctr = 0;
    }
  }

  if (hasAtmo) {
    alpha=vessel->GetAOA();
    beta=vessel->GetSlipAngle();
    gamma=vessel->GetBank();
  } else {
    alpha=vessel->GetPitch();
    beta=vessel->GetYaw();
    gamma=vessel->GetBank();
    if (lastBeta > 1.5*PI && beta < 0.5*PI) {
      lastBeta -= 2*PI;
    } else if (lastBeta < 0.5*PI && beta > 1.5*PI) {
      lastBeta += 2*PI;
    }
  }
  alphaRate=(alpha-lastAlpha)/lastSimMinDT;
  betaRate=(beta-lastBeta)/lastSimMinDT;
  gammaRate=(gamma-lastGamma)/lastSimMinDT;
  
  lastAlpha=alpha;
  lastBeta=beta;
  lastGamma=gamma;


  vessel->GetEquPos(Lon,Lat,FullH);
  vessel->GetElements(el,elmjd);
  elT = 2 * PI * sqrt(el.a*el.a*el.a/GtimesM1);
  vs2.version=2;
  vs2.flag=0;
  vessel->GetStatusEx(&vs2);
  llr_sc=_V(Lat,Lon,FullH);
  xyz_sc=llr_to_xyz(llr_sc);							    	// llr_sc is Lat/Long/Radius of spacecraft. Convert to xyz coords.
  M_sc=xyz_to_matrix(xyz_sc);							  	  // create spacecraft transform matrix around our current position
  Altitude= FullH - tdFullH;
  AtmDensity = vessel->GetAtmDensity();

  vessel->GetHorizonAirspeedVector(AirspeedVec);
  vspd=AirspeedVec.y;

  Groundspeed=sqrt(AirspeedVec.x*AirspeedVec.x+AirspeedVec.z*AirspeedVec.z);
  if (vs2.status==0) { // status 0 is in flight, 1 is landed
    Bearing=atan2(AirspeedVec.x,AirspeedVec.z);
  } else {
    Bearing=vs2.surf_hdg;
  }
  while(Bearing<0)Bearing+=2*PI;


  vessel->GetRelativeVel(ourPlanet,InertialVel);
  InertialSpd=length(InertialVel);
  Mach=vessel->GetMachNumber();
  M2 = vessel->GetMass();

  if (simt>nextMajorCycle || simt<5.0) {
    MajorCycle(simt);
    nextMajorCycle=simt+1.00;
  }
  if (!VacRotOK) {
    VacRotOK = VacRateCalibrate(simMinDT);
    if (VacRotOK) VacRateStore();
  }
  if (DeorbitActive) {
    DeoBurn(simt);
  } else {

    // ALL BAR DEORBIT CONTROL ... 

    if (!hasAtmo) {

      // Moon or non-atmo planet ...

      if (VacRotOK) {
        if (okVac) {
          vacLand(simt);
        } else {
          VacLandRMode = 0;       // Disable landing modes if steering is off or not initialized yet
          VacLandHMode = 0;
          if (dbgVacAPOpen) { fclose(vacAPouf); dbgVacAPOpen = false;}
          if (dbgVacLandOpen) { fclose(vacLandouf); dbgVacLandOpen = false;}
        }
      }
    } else {

      // At an Atmospheric Planet ...

      double YawCmd=0;
      double PitchCmd=0;
      double RollCmd=0;
      double TrimCmd=0;
      double KPYaw=-5;
      double KDYaw=-25;
      double KPPitch=-5;
      double KDPitch=-25;
      double KPRoll=0;
      double KDRoll=5;
      if(Altitude>BETA_GUIDE_OFF_ALT) {
        double PYaw=beta;
        double DYaw=betaRate;
        YawCmd=KPYaw*PYaw+KDYaw*DYaw;
        double PPitch=alpha-Refalpha;
        double DPitch=alphaRate;
        PitchCmd=KPPitch*PPitch+KDPitch*DPitch;
        double PRoll=0;
        double DRoll=gammaRate;
        RollCmd=KPRoll*PRoll+KDRoll*DRoll;
        double ManualRoll=vessel->GetManualControlLevel(THGROUP_ATT_BANKRIGHT,MANCTRL_ANYDEVICE,MANCTRL_ROTMODE)-vessel->GetManualControlLevel(THGROUP_ATT_BANKLEFT,MANCTRL_ANYDEVICE,MANCTRL_ROTMODE);
        if(fabs(ManualRoll)>0.1)RollCmd=ManualRoll;
      }
      steer(PitchCmd,Altitude>BETA_GUIDE_OFF_ALT,YawCmd,Altitude>BETA_GUIDE_OFF_ALT,RollCmd,Altitude>BETA_GUIDE_OFF_ALT);
    }
  }
  lastSimMinDT = simMinDT;
}

void GlideslopeCore::MajorCycle(double simt) {

  simDT = simt - lastSimT;

  TrimGain=linterp(8,1,2.5,0,Mach,false);
  if(TrimGain>1)TrimGain=1;
  if(TrimGain<0)TrimGain=0;
  //sprintf(oapiDebugString(),"Mach %f TrimGain %f",Mach,TrimGain);
  Airspeed=vessel->GetAirspeed();
  DynPres=vessel->GetDynPressure();
  HeatFlux=DynPres*Airspeed;
  calcRange();
  if (!hasAtmo && (simt>nextVacCalc || simt < 5.00)) {
    okVac = calcVacuumLand(simt);
    if (okVac) nextVacCalc=simt+5.00;
  }

  DelAz=az_tgt_sc-Bearing;
  while(DelAz<-PI)DelAz+=2*PI;
  while(DelAz>PI)DelAz-=2*PI;
  if (range>RefSlope[0][2]) {
    RangeSeg=-1;
  } else {
    for(RangeSeg=0;range<RefSlope[RangeSeg][2] && RangeSeg<RefSlopePts;RangeSeg++);
  }
  RangeSeg--;
  hasRefSlope=(RangeSeg>=0);
  if(hasRefSlope) {
    if (RangeSeg==RefSlopePts-1) {
      Refalpha=   RefSlope[RangeSeg][1];
      RefAltitude= RefSlope[RangeSeg][3];
      RefAirspeed= RefSlope[RangeSeg][0];
    } else {
      Refalpha=   linterp(RefSlope[RangeSeg+1][2],RefSlope[RangeSeg+1][1],RefSlope[RangeSeg][2],RefSlope[RangeSeg][1],range, false);
      RefAltitude=linterp(RefSlope[RangeSeg+1][2],RefSlope[RangeSeg+1][3],RefSlope[RangeSeg][2],RefSlope[RangeSeg][3],range, false);
      RefAirspeed=linterp(RefSlope[RangeSeg+1][2],RefSlope[RangeSeg+1][0],RefSlope[RangeSeg][2],RefSlope[RangeSeg][0],range, false);
    }
  } else {
    RefVspd=0;
    Refalpha=40*RAD;
    RefAltitude=tdFullH-re;
    RefAirspeed=0;
  }
  PE=(GtimesM1 * M2 / (rrwy)) - (GtimesM1 * M2 / (tdFullH + Altitude));
  KE=0.5 * M2 * Airspeed * Airspeed;
  TE=PE+KE;
  RefPE=(GtimesM1 * M2 / (rrwy)) - (GtimesM1 * M2 / (tdFullH + RefAltitude));
  RefKE=0.5 * M2 * RefAirspeed * RefAirspeed;
  RefTE=RefPE+RefKE;

  GroundspeedRate=(Groundspeed-lastGroundspeed)/(simt-lastSimT);
  AirspeedRate=(Airspeed-lastAirspeed)/(simt-lastSimT);
  RefAirspeedRate=(RefAirspeed-lastRefAirspeed)/(simt-lastSimT);
  RefVspd=(RefAltitude-lastRefAltitude)/(simt-lastSimT);
  RefVspdRate=(RefVspd-lastRefVspd)/(simt-lastSimT);
  RefTERate=(RefTE-lastRefTE)/(simt-lastSimT);

  if (okVac) {
    RefAirspeedRate=(RefSlope[RangeSeg+1][0] - RefSlope[RangeSeg][0]) / (RefTime[RangeSeg+1] - RefTime[RangeSeg]);
    RefVspd=linterp(RefTime[RangeSeg],RefSlope[RangeSeg][4],RefTime[RangeSeg+1],RefSlope[RangeSeg+1][4],simt, true);
    RefVspdRate=(RefSlope[RangeSeg][4] - RefSlope[RangeSeg+1][4]) / (RefTime[RangeSeg+1] - RefTime[RangeSeg]);
  }

  if (abs(Bearing-lastBearing) <= PI) {
    BearingRate=(Bearing-lastBearing)/(simt-lastSimT);
  } else {
    if ((Bearing-lastBearing) > PI ) {
      BearingRate=(Bearing-lastBearing- (2* PI))/(simt-lastSimT);
    } else {
      BearingRate=((2*PI) + Bearing-lastBearing)/(simt-lastSimT);
    }
  }
  VspdRate=(vspd-lastVspd)/(simt-lastSimT);
  TERate=(TE-lastTE)/(simt-lastSimT);
  DelAzRate=(DelAz-lastDelAz)/(simt-lastSimT); 
  lastSimT=simt;
  lastTE=TE;
  lastRefTE=RefTE;
  lastVspd=vspd;
  lastRefVspd=RefVspd;
  lastBearing=Bearing;
  lastAirspeed=Airspeed;
  lastGroundspeed = Groundspeed;
  lastAltitude=Altitude;
  lastRefAirspeed=RefAirspeed;
  lastRefAltitude=RefAltitude;
  lastDelAz = DelAz;
  TrackT[TrackPtr]=simt;
  TrackD[TrackPtr]=range;
  TrackV[TrackPtr]=okVac?Groundspeed:Airspeed;
  TrackA[TrackPtr]=Altitude;
  TrackVspd[TrackPtr]=vspd;
  TrackAlpha[TrackPtr]=alpha;
  TrackBeta[TrackPtr]=beta;
  TrackGamma[TrackPtr]=gamma;
  TrackRefAlpha[TrackPtr]=Refalpha;
  Trackvreldot[TrackPtr]=AirspeedRate;
  TrackRefvreldot[TrackPtr]=RefAirspeedRate;
  xyz_track[TrackPtr]=xyz_sc;
  TrackPtr++;
  if(TrackPtr>10000) TrackPtr=0;

/*  // On HAC approach, adjust Radius_HAC dynamically according to Alt error and TAS error
  if (leg==1) {
    radius_hac = base_radius_hac + ((2 * ((Airspeed-RefAirspeed)/RefAirspeed)) * base_radius_hac) + ((0.2 * ((Altitude-RefAltitude)/RefAltitude)) * base_radius_hac);
    if (radius_hac < base_radius_hac /2) radius_hac = base_radius_hac /2;

//sprintf(oapiDebugString(),"radius hac (%4.1f) = base(%4.1f) as(%4.1f) ref(%4.1f) factor(%6.3f)  alt(%4.1f) ref(%4.1f) factor(%6.3f)",
//radius_hac, base_radius_hac, Airspeed, RefAirspeed, (2 * ((Airspeed-RefAirspeed)/RefAirspeed)), 
//Altitude, RefAltitude, (0.2 * ((Altitude-RefAltitude)/RefAltitude)));

  }
*/

// QAR code
  if (GSqarInit && !GSqarOn) {    // Toggle start of QAR
    errno_t err;
  
    if ((err = fopen_s(&GSqar, ".\\Config\\MFD\\Glideslope\\Diags\\GSqar.csv","w")) != 0) {
      GSqarInit = false;
    } else {
      GSqarOn = true;
      char buf[512];
      sprintf_s(buf, 512, "SimT,Mass-kg,AirSpeed-mps,Altitude-m,AoA-deg,Bank-deg,BearingRate-degs,TurnRadius-m");
      fprintf(GSqar,"%s\n",buf);
    }
  } else if (!GSqarInit && GSqarOn) { // Toggle stop of QAR
    fclose(GSqar);
    GSqarOn = false; 
  }
  
  if (GSqarOn) {
    char buf[512];
    sprintf_s(buf, 512, "%.2f,%.0f,%.0f,%.0f,%.0f,%.0f,%.2f,%.0f\n",simt,M2,Airspeed,Altitude,vessel->GetAOA()*RAD2DEG,
      vessel->GetBank()*RAD2DEG,BearingRate*RAD2DEG,(fabs(BearingRate) < 0.00000001? 1000000.0 : Groundspeed/BearingRate)
      );
    fprintf(GSqar,"%s",buf);
  }

}

void GlideslopeCore::DeoBurn(double simt) {
  // DEORBIT CONTROL ...
  double cyc_left = 0.0;

  if (!BaseSyncConnected) {
    bool BS_1 = EnjoLib::ModuleMessagingExt().ModMsgGetByRef("BaseSyncMFD", "BaseSyncTarget", 2, &BS_trgt, vessel);
    bool BS_2 = EnjoLib::ModuleMessagingExt().ModMsgGetByRef("BaseSyncMFD", "BaseSyncMode", 1, &BS_mode, vessel);
    bool BS_3 = EnjoLib::ModuleMessagingExt().ModMsgGetByRef("BaseSyncMFD", "BaseSyncSolution", 2, &BS_sol, vessel);
    bool BS_4 = EnjoLib::ModuleMessagingExt().ModMsgGetByRef("BaseSyncMFD", "BaseSyncDeorbit", 4, &BS_deo, vessel);
    bool BS_5 = EnjoLib::ModuleMessagingExt().ModMsgGetByRef("BaseSyncMFD", "BaseSyncBurn", 1, &BS_burn, vessel);
    if (BS_1 && BS_2 && BS_3 && BS_4 && BS_5) BaseSyncConnected = true;
  }
  if (!BaseSyncConnected) return;
  if (DeorbitMode < 2) {
    if (BS_deo->dataValid) {
      DeorbitDV = BS_deo->dV;
      DeorbitTgtV = Groundspeed - BS_deo->dV;
      DeorbitT0 = simt + BS_deo->tToBurn;
    } else {
      DeorbitDV = 0.0;
      DeorbitTgtV = Groundspeed;
      DeorbitT0 = simt + 10000.0;
    }
  }
  DeorbitDT = DeorbitT0 - simt;
  if (DeorbitMode == 5 && abs(DeorbitDV) > 0.5) {
    DeorbitMode = 0;
  }

  if (!DeorbitExecute) return;
  if (base.lat != BS_trgt->lat) return;
  if (base.lon != BS_trgt->lon) return;
  if (!BS_deo->dataValid) return;
  if (DeorbitMode == 5) return;
  
  if (!DeoQarOn) {
    if (fopen_s(&DeoQar, ".\\Config\\MFD\\Glideslope\\Diags\\DeoBurn.csv","w") == 0) {
	    DeoQarOn = true;
	    fprintf(DeoQar,"simt,dTavg,TtBurn,dV,DeoFac,DeoMode,MThr%%,FThr%%,BThr%%,CycLeft\n");
    }
  }

  if (BS_deo->tToBurn > 120.0 && ( DeorbitMode == 0 || DeorbitMode == 5)) {  // Wait to T=2mins for retro AP
    DeorbitMode = 0;
    return;
  }

  if (DeorbitMode == 0) {       // Exit mode 0 ... activate retro AP
    if (BS_deo->tToBurn < 0.0) return;
    if (oapiGetTimeAcceleration()>10.0) oapiSetTimeAcceleration(10.0);
    DeorbitMode++;
    vessel->ActivateNavmode(NAVMODE_RETROGRADE);
  }

  if (DeorbitMode == 1 && BS_deo->tToBurn <= 5.0) { // At T-5.0s, take over time and dV from baseSync
    DeorbitMode++;
  }

  if (DeorbitMode == 2 && simt >= DeorbitT0) { // Trigger exactly on burn time
    DeorbitMode++;
  }

  if (DeorbitMode == 3) {     // Main Burn Mode
    if (oapiGetTimeAcceleration()>1.0) oapiSetTimeAcceleration(1.0);
    DeorbitDV = Groundspeed - DeorbitTgtV;
    if (DeorbitDV <= 0.02) {  // If 2cm/s left or less, move to Trim Mode
      DeorbitMode++;
      DeorbitFactor = 1.0;
    } else {                  // Burn off the dV
      double a = simMinDTavg * DeorbitFactor * ThMAX[THGROUP_MAIN]/M2;
      cyc_left = DeorbitDV/a;
      while (cyc_left > 20.0 && DeorbitFactor < 1.0) {
        DeorbitFactor *= 2.0;
        cyc_left /= 2.0;
      }
      if (DeorbitFactor > 1.0) DeorbitFactor = 1.0;
      while (cyc_left < 10.0 && DeorbitFactor > 0.12) {
        DeorbitFactor /= 2.0;
        cyc_left *= 2.0;
      }
      vessel->SetThrusterGroupLevel(THGROUP_MAIN,DeorbitFactor*(cyc_left>1.0? 1.0 : cyc_left <0.0 ? 0.0 : cyc_left));
    }
  }

  if (DeorbitMode == 4) {     // Trim burn mode. Tidy up the burn using DCS thrust only
    oapiSetTimeAcceleration(0.1);
    DeorbitDV = Groundspeed - DeorbitTgtV;
    vessel->SetThrusterGroupLevel(THGROUP_MAIN,0);
    if (abs(DeorbitDV) <= 0.0005) {  // Trimming to 0.5mm
      DeorbitMode++;
      oapiSetTimeAcceleration(1.0);
      vessel->SetThrusterGroupLevel(THGROUP_ATT_BACK,0.0);
      vessel->SetThrusterGroupLevel(THGROUP_ATT_FORWARD,0.0);
      vessel->ActivateNavmode(NAVMODE_PROGRADE);
  	  if (DeoQarOn) {
        double tm = vessel->GetThrusterGroupLevel(THGROUP_MAIN) * 100.0;
        double tf = vessel->GetThrusterGroupLevel(THGROUP_ATT_FORWARD) * 100.0;
        double tb = vessel->GetThrusterGroupLevel(THGROUP_ATT_BACK) * 100.0;
        fprintf(DeoQar,"%.3f,%.3f,%.3f,%.4f,%.3f,%d,%.1f,%.1f,%.1f,0.0\n", simt, simMinDTavg, DeorbitDT, DeorbitDV, DeorbitFactor, DeorbitMode, tm, tf, tb );
        fclose(DeoQar);
        DeoQarOn = false;
      }
      DeorbitExecute = false;
    } else {                  // Burn off the dV
      double a = simMinDTavg * DeorbitFactor * (DeorbitDV > 0.0 ? ThMAX[THGROUP_ATT_FORWARD] : ThMAX[THGROUP_ATT_BACK] )/M2;
      cyc_left = abs(DeorbitDV)/a;
      while (cyc_left > 8.0 && DeorbitFactor < 1.0) {
        DeorbitFactor *= 2.0;
        cyc_left /= 2.0;
      }
      if (DeorbitFactor > 1.0) DeorbitFactor = 1.0;
      while (cyc_left < 4.0 && DeorbitFactor > 0.49) {
        DeorbitFactor /= 2.0;
        cyc_left *= 2.0;
      }
      if (DeorbitDV > 0) {
        vessel->SetThrusterGroupLevel(THGROUP_ATT_FORWARD,DeorbitFactor*(1.2*cyc_left>1.0? 1.0 : 1.2*cyc_left));
        vessel->SetThrusterGroupLevel(THGROUP_ATT_BACK,0.0);
      } else {
        vessel->SetThrusterGroupLevel(THGROUP_ATT_BACK,DeorbitFactor*(1.2*cyc_left>1.0? 1.0 : 1.2*cyc_left));
        vessel->SetThrusterGroupLevel(THGROUP_ATT_FORWARD,0.0);
      }
    }
  }
  if (DeoQarOn && simt > (DeorbitT0 - 6.0)) {
    double tm = vessel->GetThrusterGroupLevel(THGROUP_MAIN) * 100.0;
    double tf = vessel->GetThrusterGroupLevel(THGROUP_ATT_FORWARD) * 100.0;
    double tb = vessel->GetThrusterGroupLevel(THGROUP_ATT_BACK) * 100.0;
    fprintf(DeoQar,"%.3f,%.3f,%.3f,%.4f,%.3f,%d,%.1f,%.1f,%.1f, %.3f\n", simt, simMinDTavg, DeorbitDT, DeorbitDV, DeorbitFactor, DeorbitMode, tm, tf, tb, cyc_left );
  }
  return;
}

void GlideslopeCore::align_sc(double simt, double pC, double pP, double pR, double yC, double yP, double yR, double rC, double rP, double rR) {

  if (abs(rP) > 75.0*RAD || abs(rR) > 5.0*RAD) {
    // Damp down X and Y whilst focusing on z.
    pC = pP = yC = yP = 0.0;
  }

  double paramP[3] = {-5,-5,-5};
  double paramR[3] = {-25,-25,-25};

  MATRIX3 rotZ = {cos(rC), sin(rC), 0,   -sin(rC), cos(rC), 0,   0, 0, 1};

  VECTOR3 origpos  = {pC, yC, rC};
  VECTOR3 origposE = {pP, yP, rP};
  VECTOR3 origrate = {pR, yR, rR};
  VECTOR3 origmaxE = {90.0*RAD, 180.0*RAD, 180.0*RAD};

  VECTOR3 pos = mul(rotZ,origpos);
  VECTOR3 posE = mul(rotZ, origposE);
  VECTOR3 rate = mul(rotZ, origrate);
  VECTOR3 maxE = mul(rotZ,origmaxE);

  VECTOR3 needR = origrate;
  VECTOR3 cmd = {0.0, 0.0, 0.0};

  VECTOR3 posD = pos * RAD2DEG; 
  VECTOR3 posED = posE * RAD2DEG; 
  VECTOR3 rateD = rate * RAD2DEG;
  VECTOR3 needA = {0,0,0};
  VECTOR3 maxA = {0,0,0};
  VECTOR3 needRD = {0,0,0};
  VECTOR3 needAD = {0,0,0};
  VECTOR3 maxAD = {0,0,0};
  int k[3] = {2,0,1};

  for (int j=0; j<3; j++) {
    int i = k[j];

    double maxR = VacRotRate.data[i] * 1.0;
    double maxBias;
    if (abs(posE.data[i]) < maxR * 2.0) maxR = abs(posE.data[i])/2.0;
    if (abs(posE.data[i]) < VacRotRate.data[i]) maxR = VacRotRate.data[i] * 0.5;
    maxA.data[i] = VacRotRate.data[i] * simMinDTavg;

    if ((abs(posE.data[i] + rate.data[i]) < 60 * maxA.data[i]) ||
        (abs(posE.data[i]) < 100 * maxA.data[i])){
      needR.data[i] = (-posE.data[i] - 5.0 * rate.data[i])/4;
      maxBias = 0.25;
    } else {
      needR.data[i] = (-1.0 * posE.data[i] - 8.0 * rate.data[i]);
      maxBias = 1.0;
    }

    if (abs(needR.data[i]) > maxR) needR.data[i] = needR.data[i]>0? maxR : -maxR;
    needA.data[i] = maxBias * (needR.data[i] - rate.data[i]);
    if ( ( (abs(rate.data[i]) > (1-align_sc_deadband.data[i]) * abs(needR.data[i])) ||
           (abs(rate.data[i]) > abs(needR.data[i]-0.02*RAD))
      
         ) && (
             (abs(rate.data[i]) < (1+align_sc_deadband.data[i]) * abs(needR.data[i])) ||
             (abs(rate.data[i]) < abs(needR.data[i]+0.02*RAD))
         ) && (
             (needR.data[i] * rate.data[i] > 0) ||
             (abs(needR.data[i]) < 0.005)
         )
        ) {
      needA.data[i] = 0.0;                 // When we get close to target, 
      align_sc_deadband.data[i] = 0.25 * DEG2RAD;     // ... widen deadband
    } else {
      align_sc_deadband.data[i] = 0.04 * DEG2RAD;    // Else re-acquire the tighter deadband
    }

    cmd.data[i] = needA.data[i] / maxA.data[i];
    cmd.data[i] = (cmd.data[i] > maxBias ? maxBias :  cmd.data[i] < -maxBias ? -maxBias : cmd.data[i]);


    if (j==1) {
      double check1 = 180.0*DEG2RAD - abs(posE.y);
      double check2 = 10.0*DEG2RAD;
      double check1d = check1 * RAD2DEG;
      double check2d = check2 * RAD2DEG;
      if (180.0*DEG2RAD - abs(posE.y) < 10.0*DEG2RAD) {
        cmd.y = 1.0; // Anti gimbal lock. Within 10 degrees of 180, force right to clean it out. 
      }
    }

  }

  for (int i=1; i<8; i++) vessel->DeactivateNavmode(i);

  if (oapiGetTimeAcceleration() > 10.0) oapiSetTimeAcceleration(10.0);
  if (oapiGetTimeAcceleration() == 10.0) {
    steer(cmd.data[0]/8.0, true, cmd.data[1]/8.0, true, cmd.data[2]/8.0, true);
  } else {
    steer(cmd.data[0], true, cmd.data[1], true, cmd.data[2], true);
  }

  double mpU = vessel->GetThrusterGroupLevel(THGROUP_ATT_PITCHUP);
  double mpD = vessel->GetThrusterGroupLevel(THGROUP_ATT_PITCHDOWN);
  double myL = vessel->GetThrusterGroupLevel(THGROUP_ATT_YAWLEFT);
  double myR = vessel->GetThrusterGroupLevel(THGROUP_ATT_YAWRIGHT);
  double mrL = vessel->GetThrusterGroupLevel(THGROUP_ATT_BANKLEFT);
  double mrR = vessel->GetThrusterGroupLevel(THGROUP_ATT_BANKRIGHT);

  VECTOR3 origposD = origpos * RAD2DEG;
  VECTOR3 origposED = origposE * RAD2DEG;
  VECTOR3 origrateD = origrate * RAD2DEG;
  posD = pos * RAD2DEG;
  posED = posE * RAD2DEG;
  rateD = rate * RAD2DEG;
  needRD = needR * RAD2DEG;
  needAD = needA * RAD2DEG;
  maxAD = maxA * RAD2DEG;

  if (DBG_VAC_AP && !dbgVacAPOpen) {
    if (fopen_s(&vacAPouf, ".\\Config\\MFD\\Glideslope\\Diags\\GSvacLandAlign.csv","w")==0) {
      dbgVacAPOpen = true;
	    fprintf(vacAPouf,"T,dT,dtavg,  curX,errX,rateX,curX',errX',rateX',needX,accX,maxaX,bandX,ctrlX,    curY,errY,rateY,curY',errY',rateY',needY,accY,maxaY,bandY,ctrlY,    curZ,errZ,rateZ,curZ',errZ',rateZ',needZ,accZ,maxaZ,bandZ,ctrlZ,  pU(F),pD(B),yL,yR,rL(L),rR(R),  Az,Rng,Yaw\n");
    }
  }

  if (dbgVacAPOpen) fprintf(vacAPouf,"%.3f,%.5f,%.5f,   %.3f,%.3f,%.4f,%.3f,%.3f,%.4f,%.4f,%.5f,%.5f,%.5f,%.5f,   %.3f,%.3f,%.4f,%.3f,%.3f,%.4f,%.4f,%.5f,%.5f,%.5f,%.5f,   %.3f,%.3f,%.4f,%.3f,%.3f,%.4f,%.4f,%.5f,%.5f,%.5f,%.5f,     %.5f,%.5f,%.5f,%.5f,%.5f,%.5f, %.3f,%.3f,%.3f\n",
    simt, simMinDT, simMinDTavg,
    origposD.x, origposED.x, origrateD.x, posD.x, posED.x, rateD.x, needRD.x, needAD.x, maxAD.x, align_sc_deadband.x, cmd.x,
    origposD.y, origposED.y, origrateD.y, posD.y, posED.y, rateD.y, needRD.y, needAD.y, maxAD.y, align_sc_deadband.y, cmd.y,
    origposD.z, origposED.z, origrateD.z, posD.z, posED.z, rateD.z, needRD.z, needAD.z, maxAD.z, align_sc_deadband.z, cmd.z,
    mpU,mpD,myL,myR,mrL,mrR, ar_land.az*RAD2DEG, ar_land.range,vessel->GetYaw()*RAD2DEG);


}

void GlideslopeCore::thrust_sc(double simt, double xP, double xR, double zP, double zR) {

  VECTOR3 pos = _V(0.0,0.0,0.0);
  VECTOR3 posE = _V(-xP, 0.0, -zP);
  VECTOR3 rate = _V(-xR, 0.0, -zR);

  VECTOR3 needR = _V(0.0,0.0,0.0);
  VECTOR3 cmd = {0.0, 0.0, 0.0};
  VECTOR3 needA = {0,0,0};
  VECTOR3 maxA = {0,0,0};

  double rqRt[][2] = { 100.00,  5.00, 
                        50.00,  4.00,
                        30.00,  3.00, 
                        10.00,  2.00,
                         4.00,  1.00, 
                         1.00,  0.50,
                         0.25,  0.10,
                         0.10,  0.05,
                         0.00,  0.02 };

  int k[6] = {THGROUP_ATT_RIGHT,0,THGROUP_ATT_FORWARD,THGROUP_ATT_LEFT,0,THGROUP_ATT_BACK};

  for (int i=0; i<3; i+=2) {    // Skipping Y axis as this is still on hover
    int thg = k[i];
    int thoppg = k[i+3];

    maxA.data[i] = ThMAX[thg] / M2;
    int n=0;
    while (abs(posE.data[i]) < rqRt[n][0]) n++;
    needR.data[i] = (posE.data[i] >= 0.0) ? -rqRt[n][1] : rqRt[n][1]; 

    if (needR.data[i] < 0) {
      maxA.data[i] = maxA.data[i] * ThMAX[thoppg] / ThMAX[thg];
    }

    needA.data[i] = needR.data[i] - rate.data[i];
    if ( ( (abs(rate.data[i]) > 0.9 * abs(needR.data[i])) &&
           (abs(rate.data[i]) < 1.1 * abs(needR.data[i])) &&
           (rate.data[i] * needR.data[i] >= 0.0)     
         ) 
       ) {
      needA.data[i] = 0.0;                 // When we get to +- 10% of target, it's enough
    }


    double brakeA = 0.5 * rate.data[i] * rate.data[i] / posE.data[i];
    if (rate.data[i] * posE.data[i] > 0.0) brakeA = (rate.data[i]<0.0)? maxA.data[i] : -maxA.data[i];
    if (abs(brakeA)/ maxA.data[i] > 0.8) needA.data[i] = brakeA;

    cmd.data[i] = needA.data[i] / maxA.data[i];
    cmd.data[i] = (cmd.data[i] > 1.0 ? 1.0 :  cmd.data[i] < -1.0 ? -1.0 : cmd.data[i]);

    if (cmd.data[i]>=0) {
      vessel->SetThrusterGroupLevel((THGROUP_TYPE) thg,cmd.data[i]);
      vessel->SetThrusterGroupLevel((THGROUP_TYPE) thoppg, 0.0);
    } else {
      vessel->SetThrusterGroupLevel((THGROUP_TYPE) thoppg,-cmd.data[i]);
      vessel->SetThrusterGroupLevel((THGROUP_TYPE) thg, 0.0);
    }
  }

  double mF = vessel->GetThrusterGroupLevel(THGROUP_ATT_FORWARD);
  double mB = vessel->GetThrusterGroupLevel(THGROUP_ATT_BACK);
  double mL = vessel->GetThrusterGroupLevel(THGROUP_ATT_LEFT);
  double mR = vessel->GetThrusterGroupLevel(THGROUP_ATT_RIGHT);


  if (dbgVacAPOpen) fprintf(vacAPouf,"%.3f,%.5f,%.5f,   X,X,X,%.3f,%.3f,%.4f,%.4f,%.5f,%.5f,X,%.5f,   X,X,X,%.3f,X,X,%.3f,X,X,X,X,   X,X,X,%.3f,%.3f,%.4f,%.4f,%.5f,%.5f,X,%.5f,     %.5f,%.5f,X,X,%.5f,%.5f,  %.3f,%.3f,%.3f\n",
    simt, simMinDT, simMinDTavg,
    pos.x, posE.x, rate.x, needR.x, needA.x, maxA.x, cmd.x,
    Altitude, hvrFloor - tdFullH,
    pos.z, posE.z, rate.z, needR.z, needA.z, maxA.z, cmd.z,
    mF,mB,mL,mR, ar_land.az*RAD2DEG,ar_land.range,vessel->GetYaw()*RAD2DEG);


}

void GlideslopeCore::steerAntiHammer(VECTOR3 &thrusts, bool rot) {
//
// I didn't like the thrust hammering from +ve to -ve excessively, so I wanted an anti-hammer function. This code stores a rolling history of
// the last 10 thrusts (either for ROT or LIN). The thrust is modulated by the ratio of the sum of the +ve and -ve thrust history. If, for example
// 60% of the thrust has been +ve (e.g. 1,-1,-1,-1,-1,1,1,1,1,1) then a +ve thrust should be at 60% and a -ve thrust at 40%. We want some hysteresis
// too, so if the ratio is over 80% or under 20%, then clamp to 100% or 0% respectively. This means that a random opposite thrust in a stream in
// one direction will be zeroed, to prevent the hammer. 
//

  int m = (rot==true)? 0 : 1; 
  VECTOR3 SteerAHpos = _V(0.0,0.0,0.0);
  VECTOR3 SteerAHneg = _V(0.0,0.0,0.0);
  VECTOR3 SteerAHposRatio = _V(0.0,0.0,0.0);

  for (int j=0; j<3; j++) {
    if (SteerAHcnt[m]>=STEER_ANTI_HMAX || SteerAHcnt[m]<0) SteerAHcnt[m] = 0;
    SteerAntiHammerHist[m][SteerAHcnt[m]].data[j] = thrusts.data[j];
    for (int i=0; i<STEER_ANTI_HMAX; i++) {
      if (SteerAntiHammerHist[m][i].data[j]<0.0) {
        SteerAHneg.data[j] += -SteerAntiHammerHist[m][i].data[j];
      } else {
        SteerAHpos.data[j] += SteerAntiHammerHist[m][i].data[j];
      }
    }
    if (SteerAHpos.data[j] == SteerAHneg.data[j]) {
      SteerAHposRatio.data[j] = 0.50;
    } else if (SteerAHpos.data[j] == 0) {
      SteerAHposRatio.data[j] = 0.00;
    } else if (SteerAHneg.data[j] == 0) {
      SteerAHposRatio.data[j] = 1.00;
    } else {
      SteerAHposRatio.data[j] = SteerAHpos.data[j]/SteerAHneg.data[j];
      if (SteerAHposRatio.data[j] >= 0.80) {
        SteerAHposRatio.data[j] = 1.00;
      } else if (SteerAHposRatio.data[j] <= 0.20) {
        SteerAHposRatio.data[j] = 0.00;
      }
    }
  }
  SteerAHcnt[m] = (SteerAHcnt[m] + 1) % STEER_ANTI_HMAX;

  for (int i=0; i<3; i++) {
      thrusts.data[i] *= (thrusts.data[i]>0.0) ? SteerAHposRatio.data[i] : 1 - SteerAHposRatio.data[i];
      if (abs(thrusts.data[i]) < 0.001) thrusts.data[i] = 0.00;
  }
  return;
}


void GlideslopeCore::steer(double PitchCmd, bool PitchActive, double YawCmd, bool YawActive, double RollCmd, bool RollActive) {

  THGROUP_TYPE tgPos[3] = {THGROUP_ATT_PITCHUP, THGROUP_ATT_YAWRIGHT, THGROUP_ATT_BANKRIGHT };
  THGROUP_TYPE tgNeg[3] = {THGROUP_ATT_PITCHDOWN, THGROUP_ATT_YAWLEFT, THGROUP_ATT_BANKLEFT };

  VECTOR3 thrusts = _V(PitchCmd, YawCmd, RollCmd);
  bool Active[3] = {PitchActive, YawActive, RollActive};

  if (!steerActive) return;

  steerAntiHammer(thrusts, true);

  for (int i=0; i<3; i++) {
    if (Active[i]) {
      if (thrusts.data[i] > 0.0) {
        vessel->SetThrusterGroupLevel(tgNeg[i],0.0);
        vessel->SetThrusterGroupLevel(tgPos[i],thrusts.data[i]);
      } else {
        vessel->SetThrusterGroupLevel(tgNeg[i],-thrusts.data[i]);
        vessel->SetThrusterGroupLevel(tgPos[i],0.0);
      }
    }
  }

  if(PitchActive) {
    PitchCmd = thrusts.x;
    if((TrimGain>0.5)&&(hasAtmo)) {
      double simt=oapiGetSimTime();
      double TrimSpd=PitchCmd*(simt-TrimLastSimT)*TrimGain;
      TrimPos+=PitchCmd*(simt-TrimLastSimT);
      TrimLastSimT=simt;
      if(TrimPos>1.0)TrimPos=1.0;
      if(TrimPos<-1.0)TrimPos=-1.0;
      vessel->SetControlSurfaceLevel(AIRCTRL_ELEVATORTRIM,TrimPos);
    }
  }
}

void GlideslopeCore::vacThrust(double RetCmd, bool RetActive, double HvrCmd, bool HvrActive) {
  if(RetActive && steerActive) {
    if(RetCmd>0) {
      vessel->SetThrusterGroupLevel(THGROUP_MAIN,0);
      vessel->SetThrusterGroupLevel(THGROUP_RETRO,RetCmd);
    } else {
      vessel->SetThrusterGroupLevel(THGROUP_MAIN,-RetCmd);
      vessel->SetThrusterGroupLevel(THGROUP_RETRO,0);
    }
  }
  if(HvrActive && steerActive) {
    if(HvrCmd>0) {
      vessel->SetThrusterGroupLevel(THGROUP_HOVER,HvrCmd);
    } else {
      vessel->SetThrusterGroupLevel(THGROUP_HOVER,0);
    }
  }
}




void GlideslopeCore::saveUserGS(bool units, bool extendedTrackSave) {
//
// Save out User Glide Slope
//
  errno_t err;
  FILE* ouf;
  
  if ((err = fopen_s(&ouf, ".\\Config\\MFD\\Glideslope\\Glideslope_UserSave.cfg","w")) != 0) {
	  return;
  };
  double FindRangeM[50] = { 8155, 8005, 7505, 7005, 6505, 6005, 5505, 5005, 4505, 4005,
	                          3505, 3005, 2505, 2005, 1505, 1005,  905,  805,  705,  605,
		              					 505,  455,  405,  355,  305,  275,  255,  225,  205,  195,
							               185,  175,  165,  155,  145,  135,  125,  105,   92,   82,
							                72,   62,   52,   42,   32,   22,   11,    2,   -1,   -2};
  int fr=0, maxfr=50, i=1;


  if (ouf!=NULL) {
	  fprintf(ouf,";\n");
	  fprintf(ouf,"; Glideslope 2 User-Saved Glideslope Data\n");
	  fprintf(ouf,";\n");
	  fprintf(ouf,"UNITS %s\n",(units?"US":"METRIC"));
	  fprintf(ouf,"BEGIN GLIDESLOPE\n");
	  fprintf(ouf,  ";   Range       Alt          TAS       VSpd        AoA\n");
	  if (!units) {
  	  fprintf(ouf,";   km          km           m/s       m/s          °\n");
	  } else {
	    fprintf(ouf,";   NMi         kft          ft/s      ft/s         °\n");
	  }
	  fprintf(ouf,  ";    ------      -----      -----     -----        -----\n");

// Find max distance on final approach. This is needed to avoid multi-orbit glideslopes.
// Method ... start at the last record, and work back to 100km (to avoid glitches caused by runway/HAC change, etc.),
//            then track back to where the distance flips to a small range (i.e. previous orbit). Ensure i stays >= 1.
i = TrackPtr-1;
double MinDist = (!units) ? 1000 * 1000.0 : 1000 * 1852;
while (TrackD[i] < MinDist && i > 1) { i--; };
while (TrackD[i-1] > TrackD[i] && i > 1) { i--; };

// Find starting range
	  while ((fr<maxfr)&&(FindRangeM[fr]*1000>TrackD[i])) {
//fprintf(ouf,"; FSR fr %i FindRangeM[fr]*1000 %10.1f, TrackD[1] %10.1f\n", fr,FindRangeM[fr]*1000,TrackD[1]);
 	    fr++;
    };

    if (extendedTrackSave) {
// Skip stationary on the ground (Range is screwy until bearing comes alive)
      while ((TrackD[i]>FindRangeM[fr]*1000) && (i<TrackPtr) && (TrackV[i] = 0)) {
        i++;
      }
// save every 10 secs until start of range
		  while ((TrackD[i]>FindRangeM[fr]*1000) && (i<TrackPtr)) {
//fprintf(ouf,"; DEL i %i TrackD[i]  %10.2f >FindRangeM[fr]*1000 %10.2f\n", i, TrackD[i], FindRangeM[fr]*1000);
        if (i % 10 == 5) {
			    if (!units) {
			      fprintf(ouf,"%10.2f %10.2f %10.0f %10.1f %10.1f   ; Time %i secs (Ext Track)\n",
			        TrackD[i]/1000,TrackA[i]/1000,TrackV[i],TrackVspd[i],TrackAlpha[i]*RAD2DEG, (int) TrackT[i]);
			    } else {
			      fprintf(ouf,"%10.2f %10.2f,%10.0f %10.1f %10.1f   ; Time %i secs (Ext Track)\n",
			        TrackD[i]/1852,TrackA[i]/304.8,TrackV[i]/0.3048,TrackVspd[i]/0.3048,TrackAlpha[i]*RAD2DEG, (int) TrackT[i]);
			    }
		    }
			  i++;
      }
    }

    // Dump each line immediately below range
	  do {
		  while ((TrackD[i]>FindRangeM[fr]*1000) && (i<TrackPtr)) {
//fprintf(ouf,"; DEL i %i TrackD[i]  %10.2f >FindRangeM[fr]*1000 %10.2f\n", i, TrackD[i], FindRangeM[fr]*1000);
        if (extendedTrackSave) {
          if (i % 10 == 5) {
			      if (!units) {
			        fprintf(ouf,"%10.2f %10.2f %10.0f %10.1f %10.1f   ; Time %i secs (Ext Track)\n",
			          TrackD[i]/1000,TrackA[i]/1000,TrackV[i],TrackVspd[i],TrackAlpha[i]*RAD2DEG, (int) TrackT[i]);
			      } else {
			        fprintf(ouf,"%10.2f %10.2f,%10.0f %10.1f %10.1f   ; Time %i secs (Ext Track)\n",
			          TrackD[i]/1852,TrackA[i]/304.8,TrackV[i]/0.3048,TrackVspd[i]/0.3048,TrackAlpha[i]*RAD2DEG, (int) TrackT[i]);
			      }
		      }
        }
			
			  i++;
      }
		  if (i<TrackPtr) {
			  if (!units) {
			    fprintf(ouf,"%10.2f %10.2f %10.0f %10.1f %10.1f   ; Time %i secs\n",
			      TrackD[i]/1000,TrackA[i]/1000,TrackV[i],TrackVspd[i],TrackAlpha[i]*RAD2DEG, (int) TrackT[i]);
			  } else {
			    fprintf(ouf,"%10.2f %10.2f,%10.0f %10.1f %10.1f   ; Time %i secs\n",
			      TrackD[i]/1852,TrackA[i]/304.8,TrackV[i]/0.3048,TrackVspd[i]/0.3048,TrackAlpha[i]*RAD2DEG, (int) TrackT[i]);
			  }
			  fr++; i++;
		  }
    } while ((fr<maxfr)&&(i<TrackPtr));

	  fprintf(ouf,"END GLIDESLOPE\n");

// Debug full dump
//    if (extendedTrackSave) {
//    	for (i=0;i<TrackPtr;i++) {
//  	    fprintf(ouf,";     RNG %10.2f ALT %10.2f TAS %10.0f VSP %10.1f AOA %10.1f   ; Time %i secs, Row %i\n",
//	        TrackD[i],TrackA[i],TrackV[i],TrackVspd[i],TrackAlpha[i]*RAD2DEG, (int) TrackT[i], i);
//   	}
//    }
    fclose(ouf);
  }
}

void GlideslopeCore::reset(VESSEL *v) {
  vessel=v;
  runway=0;
  DeorbitActive = false;
  DeorbitExecute = false;
  BaseSyncConnected = false;
  DeorbitMode = 0;
  DeorbitFactor = 1.0;
  TrackPtr=0;
  hac_sign=1;
  hac_open=0;
  TgtDist = 0.0;
  TgtDT = 0.0;
  TgtDD = _V(0,0,0);
  TgtDV = _V(0,0,0);
  TgtDA = _V(0,0,0);
  diagOn = false;
  deorOn = false;
  DeoQarOn = false;

  nextVacCalc = 0.0;
  PlndTog = true;
  VacLandCalculated = false;
  origTTod = TTod = 0.0;
  DTod = 0.0;
  origTHvr = THvr = 0.0;
  origTOvh = TOvh = 0.0;
  origTLand = TLand = 0.0;
  RthrF = 0.0;
  RthrP = 0.0;
  RthrA = 0.0;
  HthrF = 0.0;
  HthrP = 0.0;
  HthrA = 0.0;
  nextMajorCycle = -1.0;
  okVac = false;
  popupBurn = false;
  VacCalibrationMode = 0;
  VacRotOK = false;
  VacRotRate = _V(0.0,0.0,0.0);
  for (int i=0; i<3; i++) {
    VacRotCalCount[i] = 0;
  }
  VacRotOK = VacRateFetch();
  dbgVacAPOpen = false;
  dbgVacLandOpen = false;
  lastXrng = lastZrng = xrngRate = zrngRate = 0.0;
  xrngRetroWideBand = false;

  steerActive=false;
  isInit=true;
  ourPlanet=vessel->GetGravityRef();
  oapiGetObjectName(ourPlanet, base.ref, 32);
  re=oapiGetSize(ourPlanet);
  GtimesM1=GGRAV*oapiGetMass(ourPlanet);
  runway = ParamsRwy;

  atmConst = oapiGetPlanetAtmConstants(ourPlanet);
  hasAtmo = (atmConst != NULL);

  calcRunway();
  wp2Armed = false;
  finalArmed = false;
  maxleg = 1;

  lastAlpha = 0.0;
  lastBeta = 0.0;
  lastGamma = 0.0;
  lastAZerr = 0.0;
  betaErr = lastBetaErr = betaRate = lastBetaRate = 0.0;

  lastSimMinDT = 0.1;
  simMinDTctr = 0;
  simMinDTpopulated = false;
  simMinDTavg = 0.0;
  simMinDTtot = 0.0;
  align_sc_deadband = _V(.02,.02,.02);
  for (int i=0; i<10; i++) { xzrnghist[i] = _V(0,0,0); }
  xzrngcnt = -1;
  ThrustOrientation = 1.0;

  // Scan thruster groups for force and flow rates for non-atmo landings
  ThID[0] = THGROUP_MAIN;
  ThID[1] = THGROUP_RETRO;
  ThID[2] = THGROUP_HOVER;
  ThID[3] = THGROUP_ATT_UP;
  ThID[4] = THGROUP_ATT_DOWN;
  ThID[5] = THGROUP_ATT_LEFT;
  ThID[6] = THGROUP_ATT_RIGHT;
  ThID[7] = THGROUP_ATT_FORWARD;
  ThID[8] = THGROUP_ATT_BACK;


  for (int i=0; i<9; i++) {
    THRUSTER_HANDLE th;
    int thc = 0;
    THGROUP_TYPE thname = ThID[i];
    ThMAX[thname] = 0.0;
    ThISP[thname] = 0.0;
    for (DWORD k=0; k<v->GetGroupThrusterCount(thname); k++) {
      th = v->GetGroupThruster(thname, k);
      if (thc==0) {
        ThISP[thname] = v->GetThrusterIsp0(th);
        thc++;
      } else {
        ThISP[thname] = (ThISP[thname]*thc + v->GetThrusterIsp0(th))/(thc+1);
        thc++;
      }
      ThMAX[thname] += v->GetThrusterMax0(th);
    }
    ThMFR[thname] = ThMAX[thname]/ThISP[thname];
  }




  if (!hasAtmo) {
    altEI = 0.0;
    return;
  }
  
  altEI = atmConst->altlimit;
  ATMPARAM prm;

  oapiGetPlanetAtmParams(ourPlanet,0.0,0.0,0.0,&prm);
  if (prm.p < 0.0024) { // treat for all intents and purposes as if no atmo if mean zero alt has so little atmo
//  if (prm.p < 620.0) { // Treat Mars as zero atmo (just for a test)
    hasAtmo = false;
    altEI = 0.0;
    return;
  }
  
  oapiGetPlanetAtmParams(ourPlanet,altEI,0.0,0.0,&prm);
  if (prm.p > 0.0024) { // Edge of atmo is our target density
    altEI = altEI;
    return;
  }
  
  // Binary chop until atmo is close to 0.0024 density (approx 120km for Earth);
  double dAlt = altEI / 4.0;
  double alt = altEI / 2.0;
  oapiGetPlanetAtmParams(ourPlanet,alt,0.0,0.0,&prm);

  while (abs(prm.p-0.0024)>1e-6) {
    alt = (prm.p > 0.0024) ? alt+dAlt : alt-dAlt;
    dAlt /= 2.0;
    oapiGetPlanetAtmParams(ourPlanet,alt,0.0,0.0,&prm);
  }
  altEI = alt;


}

bool GlideslopeCore::VacRateFetch() {
  // Fetch calibration data for this ship class, if found
  FILE* rf;
  char clName[128];
  char buf[256];
  char *tok;
  char *bp;
  bool goodFetch = false;
  double params[3];
  int i;
  errno_t err;

  strcpy_s(clName,128,vessel->GetClassName());

  err = fopen_s(&rf, ".\\Config\\MFD\\Glideslope\\Glideslope-Rates.cfg","r");
  if (err != 0) return false;

  while (fgets(buf,255,rf)!=NULL) {
    bp = buf;
    if (!ParseWhiteSpace(&bp,&tok)) continue;   // Skip leading tabs and any comments
    if (!ParseQuotedString(&bp,&tok)) continue; // Bad parse
    if (_stricmp(tok,clName)!=0) continue;      // Not our ship
    for (i=0; i<3;i++) {
      if (!ParseDouble(&bp,&(params[i]))) break; // Bad parse
    }
    if (i<3) continue;                         // Bad parse

    for (i=0; i<3;i++) {
      VacRotRate.data[i] = params[i] * RAD;
    }

    VacRotOK = true;
    goodFetch = true;                           // Found and successfully loaded a ship rate file (note - keep going in case user has calibrated more)
  }

  fclose(rf);
  return goodFetch;
}

void GlideslopeCore::VacRateStore() {
  // Store calibration data for this ship class
  FILE* rf;
  char clName[128];
  errno_t err; 

  err = fopen_s(&rf, ".\\Config\\MFD\\Glideslope\\Glideslope-Rates.cfg","a+");
  if (err != 0) return; 

  strcpy_s(clName,128,vessel->GetClassName());
  fprintf(rf,"\"%s\"\t", clName);
  for (int i=0; i<3; i++) {
    fprintf(rf,"\t%.6f", VacRotRate.data[i] * RAD2DEG);
  }
  fprintf(rf," ; Counts: ");
  for (int i=0; i<3; i++) {
    fprintf(rf," %d", VacRotCalCount[i]);
  }
  fprintf(rf,"\n");
  fclose(rf);
  return;
}

bool GlideslopeCore::VacRateCalibrate(double simMinDT) {

  if (!simMinDTpopulated) return false;
  if (vessel->GetAtmPressure() > 0.0025) return false;
  if (vessel->GetDynPressure() > 0.0025) return false;
  VESSELSTATUS vs; 
  vessel->GetStatus(vs);
  if (vs.status != 0) return false;
  for (unsigned int i=0; i<vessel->DockCount(); i++) {
    if (vessel->DockingStatus(i) == 1) return false;
  }

  VECTOR3 curvel;
  double computedacc;
  vessel->GetAngularVel(curvel);
  vessel->SetAngularVel(_V(0.0,0.0,0.0));
  vessel->SetThrusterGroupLevel(THGROUP_ATT_BANKRIGHT,0.0);
  vessel->SetThrusterGroupLevel(THGROUP_ATT_BANKLEFT,0.0);
  vessel->SetThrusterGroupLevel(THGROUP_ATT_PITCHDOWN,0.0);
  vessel->SetThrusterGroupLevel(THGROUP_ATT_PITCHUP,0.0);
  vessel->SetThrusterGroupLevel(THGROUP_ATT_YAWRIGHT,0.0);
  vessel->SetThrusterGroupLevel(THGROUP_ATT_YAWLEFT,0.0);

  vessel->SetThrusterGroupLevel(THGROUP_ATT_RIGHT,0.0);
  vessel->SetThrusterGroupLevel(THGROUP_ATT_LEFT,0.0);
  vessel->SetThrusterGroupLevel(THGROUP_ATT_UP,0.0);
  vessel->SetThrusterGroupLevel(THGROUP_ATT_DOWN,0.0);
  vessel->SetThrusterGroupLevel(THGROUP_ATT_FORWARD,0.0);
  vessel->SetThrusterGroupLevel(THGROUP_ATT_BACK,0.0);

  vessel->SetThrusterGroupLevel(THGROUP_MAIN,0.0);
  vessel->SetThrusterGroupLevel(THGROUP_RETRO,0.0);
  vessel->SetThrusterGroupLevel(THGROUP_HOVER,0.0);


  switch (VacCalibrationMode) {
  case 0:
    VacRotRate = _V(0.0,0.0,0.0);
    VacRotCalCount[0] = 0;
    VacRotCalCount[1] = 0;
    VacRotCalCount[2] = 0;
    vessel->SetThrusterGroupLevel(THGROUP_ATT_PITCHUP,1.0);
    VacCalibrationMode++;
    break;
  case 1:
    computedacc = abs(curvel.x)/simMinDTavg;
    VacRotRate.x = (VacRotRate.x * VacRotCalCount[0] + computedacc) / (VacRotCalCount[0]+1);
    VacRotCalCount[0]++;
    vessel->SetThrusterGroupLevel(THGROUP_ATT_PITCHDOWN,1.0);
    VacCalibrationMode++;
    break;
  case 2:
    computedacc = abs(curvel.x)/simMinDTavg;
    VacRotRate.x = (VacRotRate.x * VacRotCalCount[0] + computedacc) / (VacRotCalCount[0]+1);
    VacRotCalCount[0]++;
    if (VacRotCalCount[0]==1000) {
      vessel->SetThrusterGroupLevel(THGROUP_ATT_YAWLEFT,1.0);
      VacCalibrationMode++;
    } else {
      vessel->SetThrusterGroupLevel(THGROUP_ATT_PITCHUP,1.0);
      VacCalibrationMode--;
    }
    break;
  case 3:
    computedacc = abs(curvel.y)/simMinDTavg;
    VacRotRate.y = (VacRotRate.y * VacRotCalCount[1] + computedacc) / (VacRotCalCount[1]+1);
    VacRotCalCount[1]++;
    vessel->SetThrusterGroupLevel(THGROUP_ATT_YAWRIGHT,1.0);
    VacCalibrationMode++;
    break;
  case 4:
    computedacc = abs(curvel.y)/simMinDTavg;
    VacRotRate.y = (VacRotRate.y * VacRotCalCount[1] + computedacc) / (VacRotCalCount[1]+1);
    VacRotCalCount[1]++;
    if (VacRotCalCount[1]==1000) {
      vessel->SetThrusterGroupLevel(THGROUP_ATT_BANKLEFT,1.0);
      VacCalibrationMode++;
    } else {
      vessel->SetThrusterGroupLevel(THGROUP_ATT_YAWLEFT,1.0);
      VacCalibrationMode--;
    }
    break;
  case 5:
    computedacc = abs(curvel.z)/simMinDTavg;
    VacRotRate.z = (VacRotRate.z * VacRotCalCount[2] + computedacc) / (VacRotCalCount[2]+1);
    VacRotCalCount[2]++;
    vessel->SetThrusterGroupLevel(THGROUP_ATT_BANKRIGHT,1.0);
    VacCalibrationMode++;
    break;
  case 6:
    computedacc = abs(curvel.z)/simMinDTavg;
    VacRotRate.z = (VacRotRate.z * VacRotCalCount[2] + computedacc) / (VacRotCalCount[2]+1);
    VacRotCalCount[2]++;
    if (VacRotCalCount[2]==1000) {
      VacCalibrationMode++;
      sprintf(oapiDebugString(), "");
    } else {
      vessel->SetThrusterGroupLevel(THGROUP_ATT_BANKLEFT,1.0);
      VacCalibrationMode--;
    }
    break;
  case 7:
    break;
  }
  if (VacCalibrationMode < 7) {
    sprintf(oapiDebugString(), "ONE-OFF ROT CALIBRATION FOR THIS VESSEL TYPE: Do not touch thrusters please! %d %d %d", VacRotCalCount[0], VacRotCalCount[1], VacRotCalCount[2]);
    return false;
  } else {
    sprintf(oapiDebugString(), "");
    return true;
  }
}