#ifndef TOKENIZE_H
#define TOKENIZE_H

#include <vector>
#include <string>
#include <istream>

namespace EnjoLib
{
class Tokenizer
{
    public:
        Tokenizer();
        virtual ~Tokenizer();
        std::vector<std::string> Tokenize(const std::string & line, char token = ' ') const;
        std::vector<std::string> GetLines(const char * fileName, bool excludeFirstLine = false) const;
        std::vector<std::string> GetLines(std::istream & is, bool excludeFirstLine = false) const;
    protected:
    private:
};
}
#endif // TOKENIZE_H
