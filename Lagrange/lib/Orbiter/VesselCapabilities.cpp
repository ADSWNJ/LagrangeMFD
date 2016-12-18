// ==============================================================
//                 ORBITER MODULE: LaunchMFD
//                  Part of the ORBITER SDK
//
// Copyright (C) 2004      rjcroy                      - robust time based pitch autopilot (borrowed code)
// Copyright (C) 2004      Dave "Daver" Rowbotham      - conversion of rjcroy's autopolot to C++ (borrowed code)
// Copyright (C) 2004      Erik H. "Sputnik" Anderson  - conversion of the autopilot to energy based (borrowed code)
// Copyright (C) 2007      "Vanguard"                  - dressing up azimuth calcualtions into an MFD (author)
// Copyright (C) 2007      Pawel "She'da'Lier" Stiasny - yaw error visual representation (contributor)
// Copyright (C) 2008      Mohd "Computerex" Ali       - borrowed his code (multiple vessels support) (borrowed code)
// Copyright (C) 2008      Chris "Kwan" Jeppesen       - borrowed his code (peg guidance) (borrowed code)
// Copyright (C) 2008      Steve "agentgonzo" Arch     - peg integration, offplane correction, compass, hud display (co-developer)
// Copyright (C) 2007-2012 Szymon "Enjo" Ender         - everything else ;> (author and maintainer)
//                         All rights reserved
//
// Authors:
// Chris "Kwan" Jeppesen (original code)
// Szymon "Enjo" Ender (cleanups)
//
// This module calculates the appropriate launch azimuth given
// desired orbital inclination and desired orbit altitude. This
// MFD takes the planets rotation into account, which provides a
// much more accurate azimuth. The calculations are performed
// 'on the fly' (technically and methaphorically), meaning that
// you get info about necessary course corrections.
//
// This file is part of LaunchMFD.
//
// LaunchMFD is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// LaunchMFD is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with LaunchMFD.  If not, see <http://www.gnu.org/licenses/>.
// ==============================================================


#include "VesselCapabilities.hpp"

VesselCapabilities::VesselCapabilities()
{
}

VesselCapabilities::~VesselCapabilities()
{
}

Engine VesselCapabilities::EstimateMainThrustParm(const VESSEL * v, double level) const
{
    Engine eng;
    if ( level < 0 || level > 1 )
        level = 1;
    if(v==NULL) return eng;
    THGROUP_TYPE thrustersGroup = IsTailSitter(v) ? THGROUP_HOVER : THGROUP_MAIN;
    int nthr=v->GetGroupThrusterCount(thrustersGroup);
    bool hasMain = nthr != 0;
    if ( ! hasMain )
    {
        nthr=v->GetThrusterCount();
        if (nthr==0) return eng;
    }
    //Assume that under max thrust of all main drive engines along the line from the center of thrust
    //to the center of gravity, the rocket is balanced, ie no rotation.
    VECTOR3 ThrBalance=_V(0,0,0),thisCoT;
    for(int i=0; i<nthr; i++)
    {
        THRUSTER_HANDLE th;
        th = hasMain ? v->GetGroupThruster(thrustersGroup, i) : v->GetThrusterHandleByIndex(i);
        if(th!=NULL)
        {
            double thisF=v->GetThrusterMax(th) * level;

            if(thisF>15000)
            {
                v->GetThrusterRef(th,thisCoT);
                eng.F+=thisF;
                eng.isp += GetThrusterIsp(v,th) * thisF;
                if(length(thisCoT)>0) ThrBalance+=thisCoT*thisF/length(thisCoT);
            }
        }
    }
    if(length(ThrBalance)>0)
        eng.ThrAngle=-atan2(ThrBalance.y,-ThrBalance.z);
    else
        eng.ThrAngle=0;

    eng.isp /= eng.F;

	return eng;
}

Engine VesselCapabilities::GetMainThrustParm( const VESSEL * v) const
{
    Engine eng;
	VECTOR3 F,M,TotalF=_V(0,0,0);
	if(v==NULL) return eng;
	int nthr=v->GetThrusterCount();
	if (nthr==0) return eng;
	//Assume that under max thrust of all main drive engines along the line from the center of thrust
	//to the center of gravity, the rocket is balanced, ie no rotation.
	VECTOR3 ThrBalance=_V(0,0,0),thisCoT;
	for(int i=0; i<nthr; i++) {
		THRUSTER_HANDLE th=v->GetThrusterHandleByIndex(i);
		if(th!=NULL) {
			double thisMax=v->GetThrusterMax(th);
			if(thisMax>15000) {
				v->GetThrusterMoment(th,F,M);
				v->GetThrusterRef(th,thisCoT);
				TotalF+=F;
				eng.isp += GetThrusterIsp(v,th) * length(F);
				if(length(thisCoT)>0) ThrBalance+=thisCoT*length(F)/length(thisCoT);
			}
		}
	}
	if(length(ThrBalance)>0)
		eng.ThrAngle=-atan2(ThrBalance.y,-ThrBalance.z);
	else
		eng.ThrAngle=0;

	eng.F=length(TotalF);
	eng.isp /= eng.F;

	return eng;
}

double VesselCapabilities::GetThrusterIsp( const VESSEL * vessel, OBJHANDLE Thr) const
{
	OBJHANDLE Prop=vessel->GetThrusterResource(Thr);
	if(Prop)
		return vessel->GetThrusterIsp0(Thr)*vessel->GetPropellantEfficiency(Prop);
	else
		return vessel->GetThrusterIsp0(Thr);
}

bool VesselCapabilities::IsTailSitter( const VESSEL * vessel ) const
{
    // If the main thruster has a zero maximum power (or does not exist), then it must be a tail sitter.
    THGROUP_HANDLE main = vessel->GetThrusterGroupHandle(THGROUP_MAIN);
    if(!main)	// no main thruster group, so it's a tail sitter
        return true;
    int thrusters = vessel->GetGroupThrusterCount(main);
    for(int i = 0; i < thrusters; ++i)
    {
        THRUSTER_HANDLE thruster = vessel->GetGroupThruster(main, i);
        if(vessel->GetThrusterMax0(thruster) > 0)
            return false;	// we have found a main thruster with >0 power. It is not a tail sitter
    }

    // if we have got this far, then we have not found a powered main thruster, so it's a tail sitter
    return true;
}

// TODO: Doesn't work in Orbiter BETA
bool VesselCapabilities::CheckVesselAirControlSurfaces( const VESSEL * vessel ) const
{
/*
    bool newAtmosphericModel;
#ifdef ORB2006
    // Not so easy for Orbiter 2006
    newAtmosphericModel = true;
#else
    double prevTrim = vessel->GetControlSurfaceLevel(AIRCTRL_ELEVATORTRIM);
    vessel->SetControlSurfaceLevel(AIRCTRL_ELEVATORTRIM, 0, true);
    vessel->SetControlSurfaceLevel(AIRCTRL_ELEVATORTRIM, 1, true);

    if ( vessel->GetControlSurfaceLevel(AIRCTRL_ELEVATORTRIM) > 0 )
        newAtmosphericModel = true;
    else
        newAtmosphericModel = false;
    // Restore previous value
    vessel->SetControlSurfaceLevel(AIRCTRL_ELEVATORTRIM, prevTrim > 0 ? 1 : prevTrim, true);
#endif
    return newAtmosphericModel;
*/
    //return vessel->GetControlSurfaceLevel(AIRCTRL_ELEVATORTRIM) != 0;
    //return vessel->Version() > 1;
    return true; // tried many things ...
}

THGROUP_HANDLE VesselCapabilities::GetMainEnginesHandle( const VESSEL * v ) const
{
    if (!v)
        return NULL;
    THGROUP_TYPE thrustersGroup = IsTailSitter(v) ? THGROUP_HOVER : THGROUP_MAIN;
    THGROUP_HANDLE h = v->GetThrusterGroupHandle( thrustersGroup );
    return h;
}
