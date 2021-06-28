#ifndef _TOKEN_H_
#define _TOKEN_H_

#include <vector>
#include <string>

#include "defination.h"

namespace openjs{
class Token{
public:
    Token();
    Token(std::string token_name);
    // Set Token name
    void SetTokenName(std::string _name);

    // Set Token type
    void SetTokenType(Defination::token_type t);
    
    void SetPosition(int x, int y);

    std::string GetTokenName();
    Defination::token_type GetTokenType();
    std::pair<int,int> GetPosition();

private:
    Defination::token_type token_type = Defination::TOKEN_UNKNOWN; 
    std::string name;
    int position_line = 0;
    int position_char = 0;
};
}
#endif