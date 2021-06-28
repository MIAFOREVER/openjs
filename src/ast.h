#ifndef _AST_H_
#define _AST_H_
#include "tokenizer.h"
#include "token.h"
#include "log/log.h"
#include "defination.h"
#include "error-process.h"
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
    MutiExpAst(std::vector<std::unique_ptr<ExpAst>> _exp): exp(std::move(_exp)){}
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
    FunctionAst(std::unique_ptr<PrototypeAst> _Proto,
                std::unique_ptr<ExpAst> _Body)
        : Proto(std::move(_Proto)), Body(std::move(_Body)) {}
};

class BuildTree {
public:
    std::unique_ptr<Tokenizer> tokenizer;
    void SetTokenizer(std::unique_ptr<Tokenizer> _tokenizer){
        tokenizer = std::move(_tokenizer);
    }
    void Run(){   
        error_process = std::move(std::make_unique<ErrorProcess>());
        // Get token from Token class
        token = tokenizer->GetToken();

        error_process->SetSouceCode(tokenizer->GetSourceCode());
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
        if(!(GetToken().GetTokenType() == Defination::TOKEN_ID))
            error_process->PrintError(GetToken(), "Parser call token id error");

        NextToken();
        if(!(GetToken().GetTokenType() == Defination::TOKEN_SMALL_SCOPE_BEGIN))
           error_process->PrintError(GetToken(), "Parser call '(' error");

        NextToken();
        auto exp = HandlePrimaryExp();

        while(exp){
            if(!(GetToken().GetTokenType() == Defination::TOKEN_COMMA)){
                NextToken();
                break;
            }
        }
        return nullptr;
    }

    std::unique_ptr<ExpAst> HandlePrimaryExp(){
        return nullptr;
    }

    std::unique_ptr<FunctionAst> HandleFunction(){
        auto proto = HandleProtoType();
        if(!proto)
            error_process->PrintError(GetToken(), "Parser function proto type error");
        if(!(GetToken().GetTokenType() == Defination::TOKEN_SCOPE_BEGIN))
            error_process->PrintError(GetToken(), "Parser function scope begin error");
        NextToken();
        auto exp = HandleExp();
        if(!(GetToken().GetTokenType() == Defination::TOKEN_SCOPE_END)){
            error_process->PrintError(GetToken(), "Parser function scope end error");
        }
        NextToken();
        
        LOG::INFO("Parser function sucessfully");
        return std::make_unique<FunctionAst>(std::move(proto), std::move(exp));
    }

    std::unique_ptr<ExpAst> HandleExp(){
        std::vector<std::unique_ptr<ExpAst>> exp;

        while((GetToken().GetTokenType() == Defination::TOKEN_IF) 
            ||(GetToken().GetTokenType() == Defination::TOKEN_ID)
            ||(GetToken().GetTokenType() == Defination::TOKEN_NUMBER)
        ){
            if(GetToken().GetTokenType() == Defination::TOKEN_IF){
                auto exp_primary = HandleIfExp();
                exp.push_back(std::move(exp_primary));
            }
            else if(GetToken().GetTokenType() == Defination::TOKEN_ID
                ||GetToken().GetTokenType() == Defination::TOKEN_NUMBER){
                auto exp_primary = HandlePrimaryExp();
                exp.push_back(std::move(exp_primary));
            }
        }

        NextToken();
        return std::make_unique<MutiExpAst>(std::move(exp));
    }

    std::unique_ptr<ExpAst> HandleIfExp(){
        if(GetToken().GetTokenType() != Defination::TOKEN_IF)
            error_process->PrintError(GetToken(), "Parser if exp error");
        NextToken();

        if(GetToken().GetTokenType() != Defination::TOKEN_SMALL_SCOPE_BEGIN)
            error_process->PrintError(GetToken(), "Parser '(' error");
        NextToken();

        return nullptr;
    }

    std::unique_ptr<ExpAst> HandleExternExp(){
        return nullptr;
    }

    std::unique_ptr<ExpAst> ParserNumber(){
        double num = std::stod(GetToken().GetTokenName());
        NextToken();
        return std::make_unique<NumberExpAst>(num);
    }

    std::unique_ptr<ExpAst> ParserVariable(){
        std::string var_name = GetToken().GetTokenName();
        NextToken();
        return std::make_unique<VariableExpAst>(var_name);
    }

    std::unique_ptr<PrototypeAst> HandleProtoType(){

        std::vector<std::string> args;

        if(!(GetToken().GetTokenType() == Defination::TOKEN_ID))
            error_process->PrintError(GetToken(), "Parser proto id error");
        // store function name
        std::string function_name = GetToken().GetTokenName();
        NextToken();

        if(!(GetToken().GetTokenType() == Defination::TOKEN_SMALL_SCOPE_BEGIN))
            error_process->PrintError(GetToken(), "Parser proto scope begin");
        NextToken();
        
        while(GetToken().GetTokenType() == Defination::TOKEN_ID){
            args.push_back(GetToken().GetTokenName());
            NextToken();

            if(!(GetToken().GetTokenType() == Defination::TOKEN_COMMA))
                break;
            NextToken();
        }

        if(!(GetToken().GetTokenType() == Defination::TOKEN_SMALL_SCOPE_END))
            error_process->PrintError(GetToken(), "Parser proto scope end");
        NextToken();

        LOG::INFO("Parser proto type successful");
        return std::make_unique<PrototypeAst>(function_name, std::move(args));
    }

    Token GetToken(){
        if(!TokenIsEnd())
            return *token[token_index];
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
    std::vector<std::shared_ptr<Token>> token;
    std::unique_ptr<ErrorProcess> error_process;
};

}

#endif