#include "Tokenizer.hpp"

#include <sstream>
#include <fstream>
using namespace std;
using namespace EnjoLib;

Tokenizer::Tokenizer()
{
}

Tokenizer::~Tokenizer()
{
}

vector<string> Tokenizer::Tokenize(const std::string & line, char token) const
{
    vector<string> ret;
    stringstream ss(line);
    string s;

    while (getline(ss, s, token))
    {
        ret.push_back(s);
    }
    return ret;
}

vector<string> Tokenizer::GetLines(const char * fileName, bool excludeFirstLine) const
{
    ifstream file(fileName);
    return GetLines(file, excludeFirstLine);
}

vector<string> Tokenizer::GetLines(istream & is, bool excludeFirstLine) const
{
    string line;
    vector<string> lines;
    bool first = true;
    while (getline(is, line))
    {
        if (excludeFirstLine && first)
        {
            first = false;
            continue;
        }
        lines.push_back(line);
    }
    return lines;
}
