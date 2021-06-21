#ifndef _AST_H_
#define _AST_H_
#include "token.h"
namespace openjs{
class ExpAst{
    
};

class NumberExpAst : public ExpAst{
    double val;
public:
    NumberExpAst(double _val){ val = _val;}
};

class VariableExpAst : public ExpAst {
    std::string Name;

public:
    VariableExpAst(const std::string &Name) : Name(Name) {}
};

/// BinaryExpAst - Expression class for a binary operator.
class BinaryExpAst : public ExpAst {
    char Op;
    std::unique_ptr<ExpAst> LHS, RHS;

public:
    BinaryExpAst(char op, std::unique_ptr<ExpAst> LHS,
                std::unique_ptr<ExpAst> RHS)
    : Op(op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}
};

/// CallExpAst - Expression class for function calls.
class CallExpAst : public ExpAst {
    std::string Callee;
    std::vector<std::unique_ptr<ExpAst>> Args;

    public:
    CallExpAst(const std::string &Callee,
                std::vector<std::unique_ptr<ExpAst>> Args)
        : Callee(Callee), Args(std::move(Args)) {}
};

/// which captures its name, and its argument names (thus implicitly the number
/// of arguments the function takes).
class PrototypeAst {
    std::string Name;
    std::vector<std::string> Args;

    public:
    PrototypeAst(const std::string &name, std::vector<std::string> Args)
        : Name(name), Args(std::move(Args)) {}

    const std::string &getName() const { return Name; }
};

/// FunctionAst - This class represents a function definition itself.
class FunctionAst {
    std::unique_ptr<PrototypeAst> Proto;
    std::unique_ptr<ExpAst> Body;

    public:
    FunctionAst(std::unique_ptr<PrototypeAst> Proto,
                std::unique_ptr<ExpAst> Body)
        : Proto(std::move(Proto)), Body(std::move(Body)) {}
};

class BuildTree {

    void Run(){
        auto t = std::make_unique<Token>();   
        token_type = t->GetTokenType();
        while(!TokenIsEnd()){
            auto token_tmp = GetToken();
            if(token_tmp == Token::TOKEN_FUNCTION)
                HandleFunction();
            else if(token_tmp == Token::TOKEN_ID)
                HandleCallFunction();
        }
    }

    void HandleCallFunction(){
        // if()

    }

    void HandleFunction(){
        
    }

    Token::token GetToken(){
        return token_type[token_index];
    }

    void NextToken(){
        token_index ++;
    }

    bool TokenIsEnd(){
        if(token_index < token_type.size() - 1)
            return false;
        return true;
    }
    int token_index = 0;
    std::vector<Token::token> token_type;
};

}

#endif