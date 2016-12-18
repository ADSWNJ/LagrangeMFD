#ifndef ENUMSTRINGMAP_H
#define ENUMSTRINGMAP_H

#include <map>
#include <string>

class EnumStringMap : public std::map<unsigned int, std::string>
{
    public:
        EnumStringMap();
        virtual ~EnumStringMap();
    protected:
        /// Use this to add all elements of the enum in your class
        void Add( unsigned int enumId, const std::string & name );
    private:
};

#endif // ENUMSTRINGMAP_H
