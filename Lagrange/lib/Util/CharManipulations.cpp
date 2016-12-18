/*
Modified BSD License

This file originates from:
http://sf.net/projects/enjomitchsorbit

Copyright (c) 2002, David "Daver" Rowbotham
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

#include <cstdlib>
#include <sstream>
#include <stdexcept>
#include "CharManipulations.hpp"
using namespace EnjoLib;

/*
void CharManipulations::char_replace_s(char *src, char lookfor, char replacewith, size_t maxlength)
{
    char *src_ptr = src;
    unsigned counter = 0;
    while( *src_ptr != NULL && counter < maxlength )
    {
        if(*src_ptr == lookfor)
            *src_ptr = replacewith;
        ++src_ptr;
        ++counter;
    }
}
*/

std::string CharManipulations::replace(const std::string & in, const std::string & lookFor, const std::string & replaceWith )
{
    std::string out = in;
    size_t pos;
    while ( (pos = out.find(lookFor)) != std::string::npos )
    {
        out.replace(pos, lookFor.size(), replaceWith);
    }
    return out;
}

std::string CharManipulations::trim(const std::string & in )
{
    return replace(in, " ", "");
}

std::string CharManipulations::ToStr(double d, unsigned precision) const
{
    std::ostringstream ss;
    ss.setf(std::ios::fixed);
    ss.precision(precision);
    ss << d;
    return ss.str();
}

std::string CharManipulations::ToStr(int i) const
{
    std::ostringstream ss;
    ss << i;
    return ss.str();
}

bool CharManipulations::ToDouble(const std::string & in, double * d) const
{
    return ToNumber<double>(in, d);
}

double CharManipulations::ToDouble(const std::string & in) const
{
    return ToNumber<double>(in);
}

bool CharManipulations::ToInt(const std::string & in, int * i) const
{
    return ToNumber<int>(in, i);
}

int CharManipulations::ToInt(const std::string & in) const
{
    return ToNumber<int>(in);
}
