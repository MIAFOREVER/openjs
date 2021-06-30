#ifndef _DEFINATION_H_
#define _DEFINATION_H_

#include <string>
#include <vector>

namespace openjs{

class Defination
{
private:
    /* data */
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
        "TOKEN_OPERATOR",
        "TOKEN_RETURN",
        "TOKEN_BOOL_TRUE",
        "TOKEN_BOOL_FALSE",
        "TOKEN_COMMA",
        "TOKEN_EQUAL",
        "TOKEN_EOF",
        "TOKEN_EXTERN",
        "TOKEN_UNKNOWN",
    }; 
    enum token_type{
        TOKEN_SCOPE_BEGIN,
        TOKEN_SCOPE_END,
        TOKEN_FUNCTION,
        TOKEN_ID,
        TOKEN_NUMBER,
        TOKEN_SMALL_SCOPE_BEGIN,
        TOKEN_SMALL_SCOPE_END,
        TOKEN_IF,
        TOKEN_ELSE,
        TOKEN_OPERATOR,
        TOKEN_RETURN,
        TOKEN_BOOL_TRUE,
        TOKEN_BOOL_FALSE,
        TOKEN_COMMA,
        TOKEN_EQUAL,
        TOKEN_EOF,
        TOKEN_EXTERN,
        TOKEN_UNKNOWN,
    };

    static Defination& GetInstance(){
        static Defination instance;
        return instance;
    }
};

}
#endif