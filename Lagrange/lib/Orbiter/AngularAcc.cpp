/*
Modified BSD License

This file is a part of Math package and originates from:
http://sf.net/projects/enjomitchsorbit

Copyright (c) 2011, Szymon "Enjo" Ender
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

#include "AngularAcc.hpp"

VECTOR3 AngularAcc::GetMaxAngAcc( const VESSEL * v ) const
{
	VECTOR3 pmi;
	v->GetPMI(pmi);
	const double & mass = v->GetMass();
	const double pitchTorque = GetMaxTorqueFromTHGroup( v, THGROUP_ATT_PITCHUP );
	const double yawTorque = GetMaxTorqueFromTHGroup( v, THGROUP_ATT_YAWLEFT );
	const double bankTorque = GetMaxTorqueFromTHGroup( v, THGROUP_ATT_BANKLEFT );

	const double pitchAcc = pitchTorque / (mass * pmi.x);
	const double yawAcc = yawTorque / (mass * pmi.y);
	const double bankAcc = bankTorque / (mass * pmi.z);

	VECTOR3 ret;
	ret.x = pitchAcc;
	ret.y = yawAcc;
	ret.z = bankAcc;

	return ret;
}

double AngularAcc::GetMaxTorqueFromTHGroup( const VESSEL * v, THGROUP_TYPE thGroupType ) const
{
	double torqueSum = 0;
	for (unsigned i = 0; i < v->GetGroupThrusterCount(thGroupType); ++i)
	{
		VECTOR3 thrusterPos;
		THRUSTER_HANDLE th = v->GetGroupThruster(thGroupType, i);
		v->GetThrusterRef(th, thrusterPos);
		double torque = v->GetThrusterMax(th) * length(thrusterPos);
		torqueSum += torque;
	}
	return torqueSum;
}

VECTOR3 AngularAcc::GetSpecificTorque( const VESSEL * v ) const
{
    VECTOR3 SpecificTorque = _V(0,0,0);
    const int nThrusters= v->GetThrusterCount();
    const double P=v->GetThrusterGroupLevel(THGROUP_ATT_PITCHUP)-v->GetThrusterGroupLevel(THGROUP_ATT_PITCHDOWN);
    const double Y=v->GetThrusterGroupLevel(THGROUP_ATT_YAWRIGHT)-v->GetThrusterGroupLevel(THGROUP_ATT_YAWLEFT);
    const double R=v->GetThrusterGroupLevel(THGROUP_ATT_BANKRIGHT)-v->GetThrusterGroupLevel(THGROUP_ATT_BANKLEFT);
    VECTOR3 TotalM=_V(0,0,0);
    for(int i=0; i<nThrusters; i++)
    {
        VECTOR3 F,M;
        v->GetThrusterMoment(v->GetThrusterHandleByIndex(i),F,M);
        TotalM=TotalM+M;
    }
    if(P!=0) SpecificTorque.x=fabs(TotalM.x/P);
    if(Y!=0) SpecificTorque.y=fabs(TotalM.y/Y);
    if(R!=0) SpecificTorque.z=fabs(TotalM.z/R);
    return SpecificTorque;
}
