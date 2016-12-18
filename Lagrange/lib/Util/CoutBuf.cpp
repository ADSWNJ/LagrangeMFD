#include "CoutBuf.hpp"
#include <iostream>

using namespace std;

CoutBuf::CoutBuf(bool verbose)
: m_verbose(verbose)
{
}

CoutBuf::~CoutBuf()
{
    if (m_verbose)
    {
        cout << this->str();
    }
}
