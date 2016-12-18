/* Copyright (c) 2007 Steve Arch
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in
** all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
** THE SOFTWARE.*/

#include "BurnTime.hpp"

using namespace std;

BurnTime::BurnTime(){}
BurnTime::~BurnTime(){}

double BurnTime::GetBurnStart(VESSEL *vessel, THGROUP_TYPE thGroupType, double instantaneousBurnTime, double deltaV)
{
	double startBurn = instantaneousBurnTime + GetBurnTimeVariadic(vessel, thGroupType, deltaV);
	return startBurn;
}

double BurnTime::GetBurnTimeVariadic(VESSEL *vessel, THGROUP_TYPE thGroupType, double deltaV)
{
	double thrust = 0, isp = 0;
	const int numThrusters = vessel->GetGroupThrusterCount(thGroupType);
	for(int i = 0; i < numThrusters; ++i)
	{
		THRUSTER_HANDLE thruster = vessel->GetGroupThruster(thGroupType,i);
		thrust += vessel->GetThrusterMax0(thruster);
		isp += vessel->GetThrusterIsp0(thruster);
	}

	double mass = GetStackMass(vessel);
	double startAccel = thrust / mass;
	double mdot = thrust / isp;	// mass flow rate (rate of change of mass)
	// jerk is rate of change of acceleration - differentiate F/(M - dm * t) using chain rule
	// jerk is not constant, but assume it is and take the starting value (at t=0)
	double jerk = thrust * mdot / (mass * mass);
	double totalBurnTime = GetBurnTime(vessel, deltaV);

	// magic formula calculated from equations of motion of an object under non-uniform acceleration but uniform jerk
	double startBurn = (startAccel * totalBurnTime) / (2 * startAccel + jerk * totalBurnTime) - totalBurnTime;

	return startBurn;
}

double BurnTime::GetBurnTime(VESSEL *vessel, double deltaV)
{
	// Returns the time to burn to the required deltaV. Calculates via rocket equation
	if(deltaV < 0)
		deltaV = -deltaV;
    double T = 0, isp = 0;
	const int numThrusters = vessel->GetGroupThrusterCount(THGROUP_MAIN);
	for(int i = 0; i < numThrusters; ++i)
	{
		THRUSTER_HANDLE thruster = vessel->GetGroupThruster(THGROUP_MAIN,i);
		T += vessel->GetThrusterMax0(thruster);
		isp += vessel->GetThrusterIsp0(thruster);
	}
	return - (isp * GetStackMass(vessel) / T * (exp(-deltaV / isp) - 1.0));
}

void BurnTime::AddVesselToStack(VESSEL *vessel, vector<VESSEL*> &stack)
{
	// Is the vessel in the stack
	for(vector<VESSEL*>::iterator it = stack.begin(); it != stack.end(); it++)
		if(*it == vessel)
			return;	// return early as the vessel is already in the stack

	if(vessel)
	{
		stack.push_back(vessel);

		// Add all the docked vessels to the stack
		for(unsigned int i = 0; i < vessel->DockCount(); i++)
			if(OBJHANDLE dockedVessel = vessel->GetDockStatus(vessel->GetDockHandle(i)))
				AddVesselToStack(oapiGetVesselInterface(dockedVessel), stack);
	}
}

double BurnTime::GetStackMass(VESSEL *vessel)
{
	// Create a list with all the vessels in the stack contained in the list
	vector<VESSEL*> stack;
	AddVesselToStack(vessel, stack);

	// Get the total mass of all the vessels in the list.
	double stackMass = 0.0;
	for(vector<VESSEL*>::iterator it = stack.begin(); it != stack.end(); it++)
		stackMass += (*it)->GetMass();
	return stackMass;
}
