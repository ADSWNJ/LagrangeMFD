#include "EnumStringMap.h"

EnumStringMap::EnumStringMap()
{
    //ctor
}

EnumStringMap::~EnumStringMap()
{
    //dtor
}

void EnumStringMap::Add( unsigned int enumId, const std::string & name )
{
    this->operator[]( enumId ) = name;
}
