#include "Trim.hpp"

#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>

using namespace EnjoLib;

// trim from start
std::string & Trim::ltrim(std::string &s) const
{
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
        return s;
}

// trim from end
std::string & Trim::rtrim(std::string &s) const
{
        s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
        return s;
}

// trim from both ends
std::string & Trim::trim(std::string &s)  const
{
        return ltrim(rtrim(s));
}
