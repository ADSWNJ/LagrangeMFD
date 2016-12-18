/*
Modified BSD License

This file is a part of Math package and originates from:
http://sf.net/projects/enjomitchsorbit

Copyright (c) 2012, Szymon "Enjo" Ender
Copyright (c) 2008, Steve "agentgonzo" Arch - GetHeadBearing()

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "SpaceMathOrbiter.hpp"
#include "Math/VectorMath.hpp"
#include "SystemsConverterOrbiter.hpp"
#include <Systems/Vect3.hpp>

using namespace EnjoLib;

double SpaceMathOrbiter::GetPlaneAngle( const VESSEL * v, const OBJHANDLE hTarget ) const
{
    if ( ! hTarget )
        return 0;
    const OBJHANDLE hRef = v->GetSurfaceRef();
    const OBJHANDLE hVessel = v->GetHandle();
    const Vect3 & planeVessel = SystemsConverterOrbiter(GetPlaneAxis(hVessel, hRef)).GetVector();
    const Vect3 & planeTarget = SystemsConverterOrbiter(GetPlaneAxis(hTarget, hRef)).GetVector();
    const double planeAngle = VectorMath().angle(planeVessel, planeTarget);

    return planeAngle;
}

VECTOR3 SpaceMathOrbiter::GetPlaneAxis( const OBJHANDLE hObj, const OBJHANDLE hRef ) const
{
    VECTOR3 pos, vel;
    oapiGetRelativePos(hObj, hRef, &pos);
    oapiGetRelativeVel(hObj, hRef, &vel);
    const VECTOR3 & axis = crossp(pos, vel);

    return axis;
}

// Rotate to get a state vector in equatorial frame
VECTOR3 SpaceMathOrbiter::ToEquatorial( const VECTOR3 & in, const OBJHANDLE hRef ) const
{
    MATRIX3 rot;
    oapiGetPlanetObliquityMatrix(hRef, &rot);
    return tmul(rot, in);
}

VECTOR3 SpaceMathOrbiter::SwapCoordinateSystem( const VECTOR3 & in ) const
{
    return _V(in.x, in.z, in.y);
}

///////////////////////////////////////////////////////////////
// Tool functions
double SpaceMathOrbiter::getdeterminant(const MATRIX3 & mat) const
{
	return   mat.m11 * (mat.m22 * mat.m33 - mat.m23 * mat.m32)
		   - mat.m12 * (mat.m21 * mat.m33 - mat.m23 * mat.m31)
		   + mat.m13 * (mat.m21 * mat.m32 - mat.m22 * mat.m31);
}

MATRIX3 SpaceMathOrbiter::getinvmatrix(const MATRIX3 & mat) const
{
	MATRIX3 out = {0,0,0,0,0,0,0,0,0};
	const double det = getdeterminant(mat);
	if(det == 0)
		return out; // prevent devide by zero

	out.m11 =  (mat.m22 * mat.m33 - mat.m23 * mat.m32) / det;
	out.m21 = -(mat.m21 * mat.m33 - mat.m23 * mat.m31) / det;
	out.m31 =  (mat.m21 * mat.m32 - mat.m22 * mat.m31) / det;
	out.m12 = -(mat.m12 * mat.m33 - mat.m13 * mat.m32) / det;
	out.m22 =  (mat.m11 * mat.m33 - mat.m13 * mat.m31) / det;
	out.m32 = -(mat.m11 * mat.m32 - mat.m12 * mat.m31) / det;
	out.m13 =  (mat.m12 * mat.m23 - mat.m13 * mat.m22) / det;
	out.m23 = -(mat.m11 * mat.m23 - mat.m13 * mat.m21) / det;
	out.m33 =  (mat.m11 * mat.m22 - mat.m12 * mat.m21) / det;

	return out;
}

void SpaceMathOrbiter::getinvrotmatrix(VECTOR3 arot, MATRIX3 *invrotmatrix) const //arot not really a vector - see arot defn from vessel struct
{
	double tcos=cos(arot.z);
	double tsin=sin(arot.z);
	MATRIX3 z={0,0,0,0,0,0,0,0,1};
	z.m11=z.m22=tcos;
	z.m12=-tsin;
	z.m21=tsin;
	tcos=cos(arot.y);
	tsin=sin(arot.y);
	MATRIX3 y={0,0,0,0,1,0,0,0,0};
	y.m11=y.m33=tcos;
	y.m13=tsin;
	y.m31=-tsin;
	tcos=cos(arot.x);
	tsin=sin(arot.x);
	MATRIX3 x={1,0,0,0,0,0,0,0,0};
	x.m22=x.m33=tcos;
	x.m32=tsin;
	x.m23=-tsin;
	MATRIX3 temp = mul(z, y);
	*invrotmatrix = mul(temp, x);
}

VECTOR3 SpaceMathOrbiter::GetRotationToTarget(VESSEL * vessel, const VECTOR3 & target) const
{
    VECTOR3 trtarget;
	VESSELSTATUS status;
    vessel->GetStatus(status);
	VECTOR3 arot=status.arot;
	MATRIX3 rotmatrix;
	getinvrotmatrix(arot,&rotmatrix);
	trtarget = mul(rotmatrix, target);

	return trtarget;
}

double SpaceMathOrbiter::GetHeadBearing( const VESSEL * v ) const
{
	VECTOR3 globalrot;
	v->GetGlobalOrientation(globalrot);
	const OBJHANDLE refbody = v->GetSurfaceRef();
	MATRIX3 refrot;
	oapiGetPlanetObliquityMatrix(refbody, &refrot);
	const MATRIX3 & refRotInv = getinvmatrix(refrot);
	const VECTOR3 & localrot = mul(refRotInv, globalrot);
	double lat, lng, rad;
	v->GetEquPos(lng, lat, rad);
	VECTOR3 head = {0, 1, 0};	// the unit vector coming out of the pilot's head (vessel frame)
	// Set R as the rotation matrix from the vessel to the global space
	MATRIX3 R;
	v->GetRotationMatrix(R);
	// rotate the head vector into the global framec.Y(
	head = mul(R, head);
	// rotate this head vector to the planet's frame
	head = mul(refRotInv, head);
	// Take account for the vessel's longitude and the planet's rotation
	double planetRotation = oapiGetPlanetCurrentRotation(refbody) + lng;
	MATRIX3 planetrot = {cos(planetRotation), 0, sin(planetRotation),
						 0,					  1, 0,
						 -sin(planetRotation),0, cos(planetRotation)};
	head = mul(planetrot, head);
	// Take account for the planet's latitude
	MATRIX3 latRot = {cos(lat), sin(lat), 0,
					 -sin(lat), cos(lat), 0,
					  0,		0,		  1};
	head = mul(latRot, head);
	// The vector head now represents a vector pointing from arse to head as if the vessel was positioned at lat = 0, lng = 0
	// in the body's own frame of reference.
	// Use this to calculate the bearing of the head vector
	return atan2(head.z, head.y);
}

// Borrowed from dr Martin Schweiger's ScnEditor
void SpaceMathOrbiter::Crt2Pol (VECTOR3 & pos, VECTOR3 & vel) const
{
    // position in polar coordinates
    const double r      = sqrt  (pos.x*pos.x + pos.y*pos.y + pos.z*pos.z);
    const double lng    = atan2 (pos.z, pos.x);
    const double lat    = asin  (pos.y/r);
    // derivatives in polar coordinates
    const double drdt   = (vel.x*pos.x + vel.y*pos.y + vel.z*pos.z) / r;
    const double dlngdt = (vel.z*pos.x - pos.z*vel.x) / (pos.x*pos.x + pos.z*pos.z);
    const double dlatdt = (vel.y*r - pos.y*drdt) / (r*sqrt(r*r - pos.y*pos.y));
    pos.data[0] = r;
    pos.data[1] = lng;
    pos.data[2] = lat;
    vel.data[0] = drdt;
    vel.data[1] = dlngdt;
    vel.data[2] = dlatdt;
}

// Borrowed from dr Martin Schweiger's ScnEditor
void SpaceMathOrbiter::Crt2Pol (VECTOR3 &pos) const
{
    // position in polar coordinates
    const double r      = sqrt  (pos.x*pos.x + pos.y*pos.y + pos.z*pos.z);
    const double lng    = atan2 (pos.z, pos.x);
    const double lat    = asin  (pos.y/r);
    pos.data[0] = r;
    pos.data[1] = lng;
    pos.data[2] = lat;
}


