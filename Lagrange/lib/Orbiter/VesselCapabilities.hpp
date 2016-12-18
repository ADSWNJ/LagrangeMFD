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
// Steve "agentgonzo" Arch (IsTailSiter)
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

#ifndef VESSELCAPABILITIES_H
#define VESSELCAPABILITIES_H

#include "Engine.hpp"

#include <orbitersdk.h>

class VesselCapabilities
{
    public:
        VesselCapabilities();
        virtual ~VesselCapabilities();

        double GetThrusterIsp( const VESSEL * vessel, OBJHANDLE Thr) const;
        Engine EstimateMainThrustParm( const VESSEL * v, double level = 1) const;
        Engine GetMainThrustParm( const VESSEL * v ) const;
        /// Checks if this ship is a tail sitter
        /** Tail sitters are where there is no main thruster and is is flown with
        the hover thruster (eg Apollo LM) */
        // By Agentgonzo
        bool IsTailSitter( const VESSEL * v ) const;
        bool CheckVesselAirControlSurfaces( const VESSEL * vessel ) const;
        THGROUP_HANDLE GetMainEnginesHandle( const VESSEL * v ) const;

    protected:
    private:
};

#endif // VESSELCAPABILITIES_H
