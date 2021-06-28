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
    explicit Tokenizer(std::string);
    void SetFilename(std::string filename);
    void Run();
    std::vector<std::shared_ptr<Token>> GetToken();
    std::vector<std::string> GetSourceCode();
    
private:
    // Input string stream
    void SetOperatorCode();
    bool InOperatorCode(char s);

    // Split function
    // This function is use for spilt string to vector<token>
    // Maybe it still has bugs
    // The logical needs to complete
    void Split(std::string s);
    void ReadFile(std::string filename);
    
    // Tokenize
    void RunTokenizer();
    void SetRegex();

    // Debug for print
    void DebugPrintToken();

    std::vector<std::shared_ptr<Token>> tk;

    std::vector<std::string> operator_code;
    std::string filename;
    std::string file_stream;
    std::vector<std::pair<std::regex, Defination::token_type>> reg;

    std::vector<std::string> source_code;

    int position_line = 0;
    int position_char = 0;
};
}
#endif