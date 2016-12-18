/*
Modified BSD License

This file is a part of Math package and originates from:
http://sf.net/projects/enjomitchsorbit

Copyright (c) 2013, Szymon "Enjo" Ender
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
DISCLAIMED. IN NO EVENT SHALL SZYMON ENDER BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "RK4.hpp"

using namespace EnjoLib;

RK4::RK4( const RK4AccelerationBase & rk4AccelerationFunction )
: m_RK4AccelerationFunction(rk4AccelerationFunction)
{
}

RK4::~RK4()
{
}

StateVect3 RK4::Integrate(const StateVect3 & state, double dt) const
{
     const StateVect3 & a = Derive(state, 0, StateVect3());
     const StateVect3 & b = Derive(state, dt*0.5, a);
     const StateVect3 & c = Derive(state, dt*0.5, b);
     const StateVect3 & d = Derive(state, dt, c);

     const Vect3 & dxdt = (a.pos + (b.pos + c.pos) * 2 + d.pos) * 1/6.0;
     const Vect3 & dvdt = (a.vel + (b.vel + c.vel) * 2 + d.vel) * 1/6.0;

     StateVect3 output;
     output.pos = state.pos + dxdt * dt;
     output.vel = state.vel + dvdt * dt;

     return output;
}

StateVect3 RK4::Derive(const StateVect3 & initial, double dt, const StateVect3 & derivative) const
{
     StateVect3 state;
     state.pos = initial.pos + derivative.pos*dt;
     state.vel = initial.vel + derivative.vel*dt;

     StateVect3 newDerivative;
     newDerivative.pos = state.vel;
     newDerivative.vel = m_RK4AccelerationFunction(state);
     return newDerivative;
}
