#ifndef _AST_H_
#define _AST_H_
#include "tokenizer.h"
#include "token.h"
#include "log/log.h"
#include "defination.h"
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

class MutiExpAst : public ExpAst {
    std::vector<std::unique_ptr<ExpAst>> exp;

public:
    MutiExpAst(std::vector<std::unique_ptr<ExpAst>> _exp): exp(_exp){}
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
        auto t = std::make_unique<Tokenizer>();   
        // Get token from Token class
        token = t->GetToken();
        while(!TokenIsEnd()){
            auto token_tmp = GetToken();
            if(token_tmp.GetTokenType() == Defination::TOKEN_FUNCTION){
                NextToken();
                HandleFunction();
            }
            else if(token_tmp.GetTokenType() == Defination::TOKEN_ID){
                NextToken();
                HandleCallFunction();
            }
        }
    }

    std::unique_ptr<ExpAst> HandleCallFunction(){
        // if()

    }

    std::unique_ptr<FunctionAst> HandleFunction(){
        auto proto = HandleProtoType();
        if(!proto)
            LOG::ERROR("Parser function proto type error");
        if(!GetToken().GetTokenType() == Defination::TOKEN_SCOPE_BEGIN){
            LOG::ERROR("Parser function scope begin error");
        }
        NextToken();
        auto exp = HandleExp();
        if(!GetToken().GetTokenType() == Defination::TOKEN_SCOPE_END){
            LOG::ERROR("Parser function scope end error");
        }
        NextToken();
        
        LOG::INFO("Parser function sucessfully");
        return std::make_unique<FunctionAst>(proto, exp);
    }

    std::unique_ptr<MutiExpAst> HandleExp(){
        std::vector<std::unique_ptr<ExpAst>> exp;

        
        NextToken();
        return std::make_unique<MutiExpAst>(exp);
    }

    std::unique_ptr<ExpAst> HandleExternExp(){

    }

    std::unique_ptr<ExpAst> ParserNumber(){

    }

    std::unique_ptr<ExpAst> ParserId(){
        
    }

    std::unique_ptr<PrototypeAst> HandleProtoType(){
        if(!GetToken().GetTokenType() == Defination::TOKEN_ID)
            LOG::ERROR("Parser proto id error");
        // store function name
        std::string function_name = GetToken().GetTokenName();
        NextToken();
        if(!GetToken().GetTokenType() == Defination::TOKEN_SMALL_SCOPE_BEGIN)
            LOG::ERROR("Parser proto scope begin");
        std::vector<std::string> args;
        while(GetToken().GetTokenType() == Defination::TOKEN_ID){
            args.push_back(GetToken().GetTokenName());
            NextToken();
            if(GetToken().GetTokenType() != Defination::TOKEN_COMMA)
                break;
        }
        if(!GetToken().GetTokenType() == Defination::TOKEN_SMALL_SCOPE_END)
            LOG::ERROR("Parser proto scope end");
        
        LOG::INFO("Parser proto type successful");
        // consume this token.
        NextToken();
        return std::make_unique<PrototypeAst>(function_name, args);
    }

    Token GetToken(){
        if(!TokenIsEnd())
            return token[token_index];
        else
            return Token();
    }

    void NextToken(){
        token_index ++;
    }

    bool TokenIsEnd(){
        if(token_index < token.size() - 1)
            return false;
        return true;
    }
    int token_index = 0;
    std::vector<Token> token;
};

}

#endif