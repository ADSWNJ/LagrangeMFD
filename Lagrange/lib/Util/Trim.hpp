#ifndef TRIM_H
#define TRIM_H

#include <string>

namespace EnjoLib
{
class Trim
{
    public:
        std::string & ltrim(std::string &s) const;
        std::string & rtrim(std::string &s) const;
        std::string & trim(std::string &s) const;
    protected:
    private:
};
}

#endif // TRIM_H
