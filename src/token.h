#ifndef _TOKEN_H_
#define _TOKEN_H_

#include <vector>
#include <string>

#include "defination.h"

namespace openjs{
class Token{
public:
    void SetTokenName(std::string _name);
    void SetTokenType(Defination::token_type t);
    std::string GetTokenName();
    Defination::token_type GetTokenType();

private:
    Defination::token_type token_type; 
    std::string name;
};
}
#endif