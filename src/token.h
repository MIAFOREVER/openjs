#ifndef _TOKEN_H_
#define _TOKEN_H_
#include <string>
#include <vector>
#include <regex>
#include <map>

namespace openjs{
class Token{
public:
std::vector<std::string> token_map = {
    "TOKEN_SCOPE_BEGIN",
    "TOKEN_SCOPE_END",
    "TOKEN_FUNCTION",
    "TOKEN_ID",
    "TOKEN_NUMBER",
    "TOKEN_SMALL_SCOPE_BEGIN",
    "TOKEN_SMALL_SCOPE_END",
    "TOKEN_IF",
    "TOKEN_ELSE",
    "TOKEN_ORERATOR",
    "TOKEN_RETURN",
    "TOKEN_BOOL_TRUE",
    "TOKEN_BOOL_FALSE",
    "TOKEN_UNKNOWN",
};
enum token{
    TOKEN_SCOPE_BEGIN,
    TOKEN_SCOPE_END,
    TOKEN_FUNCTION,
    TOKEN_ID,
    TOKEN_NUMBER,
    TOKEN_SMALL_SCOPE_BEGIN,
    TOKEN_SMALL_SCOPE_END,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_ORERATOR,
    TOKEN_RETURN,
    TOKEN_BOOL_TRUE,
    TOKEN_BOOL_FALSE,
    TOKEN_UNKNOWN,
};
    
    void SetFilename(std::string filename);
    void Run();
    std::vector<std::string> GetToken();
    std::vector<token> GetTokenType();
private:
    // Input string stream
    void SetOperatorCode();
    bool InOperatorCode(char s);

    void PreProcess(std::string s);
    void ReadFile(std::string filename);
    
    // tokenize
    void Tokenizer();
    void SetRegex();

    //debug
    void DebugPrintToken();

    std::vector<std::string> tk;
    std::vector<token> token_type;
    std::vector<std::string> operator_code;
    std::string filename;
    std::string file_stream;
    std::vector<std::pair<std::regex, token>> reg;
};
}
#endif