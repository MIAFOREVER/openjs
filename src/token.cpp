#include "token.h"

namespace openjs{

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

}