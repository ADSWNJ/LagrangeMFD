#ifndef COUTBUF_H
#define COUTBUF_H

#include <sstream>

class CoutBuf : public std::ostringstream
{
    public:
        CoutBuf(bool verbose);
        virtual ~CoutBuf();
    protected:
    private:
        bool m_verbose;
};

#endif // COUTBUF_H
