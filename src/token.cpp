#include "token.h"

namespace openjs{

Token::Token(){}

Token::Token(std::string token_name){
    SetTokenName(token_name);
}

void Token::SetPosition(int x, int y){
    position_line = x;
    position_char = y;
}

void Token::SetTokenName(std::string _name){
    name = _name;
}
void Token::SetTokenType(Defination::token_type t){
    token_type = t;
}

std::string Token::GetTokenName(){
    return name;
}

Defination::token_type Token::GetTokenType(){
    return token_type;
}

std::pair<int,int> Token::GetPosition(){
    return std::make_pair(position_line,position_char);
}

}