/*
Modified BSD License

PID controller classes
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
DISCLAIMED. IN NO EVENT SHALL SZYMON ENDER BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "PID.hpp"
using namespace EnjoLib;

PID::PID(double kp, double kd, double ki)
{
    m_kp = kp;
    m_kd = kd;
    m_ki = ki;
    Reset();
}

double PID::Update(double error, double dt)
{
    m_currValue = m_kp * error +
                  m_kd * m_derivator.Derive(error, dt) +
                  m_ki * m_integrator.IntegrateTrapezoidal(error, dt);

    return m_currValue;
}

double PID::GetCurrentValue() const
{
    return m_currValue;
}

void PID::Reset()
{
    m_currValue = 0;
    m_integrator.Reset();
    m_derivator.Reset();
}

void PID::SetKp(double kp)
{
    m_kp = kp;
}
void PID::SetKd(double kd)
{
    m_kd = kd;
}
void PID::SetKi(double ki)
{
    m_ki = ki;
}

double PID::GetKp() const
{
	return m_kp;
}
double PID::GetKd() const
{
	return m_kd;
}
double PID::GetKi() const
{
	return m_ki;
}


