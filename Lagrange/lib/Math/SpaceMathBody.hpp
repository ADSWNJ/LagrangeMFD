/*
Modified BSD License

This file is a part of Math package and originates from:
http://sf.net/projects/enjomitchsorbit

Copyright (c) 2012, Szymon "Enjo" Ender
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

#ifndef SpaceMathBody_HPP
#define SpaceMathBody_HPP

namespace EnjoLib
{
class SpaceMathBody
{
    public:
        SpaceMathBody(double mass, double radius = 0);
        virtual ~SpaceMathBody();

        double GetHohmannDVCircularise( double rp, double ra) const;
        double GetHohmannDVExtend( double rp, double ra) const;
        double CalcA( double rp, double ra) const;
        double CalcEcc( double rp, double ra) const;
        double CalcH( double rp, double ra ) const;
        double CalcVelAtRadius( double rp, double ra, double rTest ) const;
        double CalcPeriod( double rp, double ra) const;
        double CalcEnergyFromVelRadius( double vel, double r ) const;
        double CalcEnergyFromRadii( double rp, double ra ) const;
        double GetMi() const;

    protected:
    private:
        double CalcMi( double mass ) const;
        void PrepareData(double & alt1, double & alt2) const;

        const double m_mass, m_radius, m_mi;
};
}

#endif // SpaceMathBody_HPP
