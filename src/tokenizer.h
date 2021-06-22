#ifndef _TOKENIZER_H_
#define _TOKENIZER_H_

#include "token.h"
#include "defination.h"

#include <string>
#include <vector>
#include <regex>
#include <map>

namespace openjs{
class Tokenizer{
    
public:    
    void SetFilename(std::string filename);
    void Run();
    std::vector<Token> GetToken();
    
private:
    // Input string stream
    void SetOperatorCode();
    bool InOperatorCode(char s);

    void PreProcess(std::string s);
    void ReadFile(std::string filename);
    
    // Tokenize
    void RunTokenizer();
    void SetRegex();

    // Debug for print
    void DebugPrintToken();

    std::vector<std::string> tk;
    std::vector<Defination::token_type> token_type;
    std::vector<std::string> operator_code;
    std::string filename;
    std::string file_stream;
    std::vector<std::pair<std::regex, Defination::token_type>> reg;

    // return for public api to build ast tree.
    std::vector<Token> public_token;
};
}
#endif