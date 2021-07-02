#ifndef _AST_H_
#define _AST_H_

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"

#include "tokenizer.h"
#include "token.h"
#include "log/log.h"
#include "defination.h"
#include "error-process.h"
namespace openjs{
class ExpAst{
public:
    virtual ~ExpAst() {}
    virtual llvm::Value *codegen() = 0;    
};

class NumberExpAst : public ExpAst{
    double val;
public:
    NumberExpAst(double _val){ val = _val;}

    llvm::Value *codegen() override;
};

class VariableExpAst : public ExpAst {
    std::string Name;

public:
    VariableExpAst(const std::string &Name) : Name(Name) {}

    llvm::Value *codegen() override;
};

/// BinaryExpAst - Expression class for a binary operator.
class BinaryExpAst : public ExpAst {
    char Op;
    std::unique_ptr<ExpAst> LHS, RHS;

public:
    BinaryExpAst(char op, std::unique_ptr<ExpAst> LHS,
                std::unique_ptr<ExpAst> RHS)
    : Op(op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}

    llvm::Value *codegen() override;
};

class AssignExpAst : public ExpAst{
    std::unique_ptr<VariableExpAst> var;
    std::unique_ptr<ExpAst> RHS;

public:
    AssignExpAst(std::unique_ptr<VariableExpAst> _var, std::unique_ptr<ExpAst> _RHS){
        var = std::move(_var);
        RHS = std::move(_RHS);
    }

    llvm::Value *codegen() override;
};

class MutiExpAst : public ExpAst {
    std::vector<std::unique_ptr<ExpAst>> exp;

public:
    MutiExpAst(std::vector<std::unique_ptr<ExpAst>> _exp): exp(std::move(_exp)){}

    llvm::Value *codegen() override;
};

class IfExpAst : public ExpAst {
    std::unique_ptr<ExpAst> condition;
    std::unique_ptr<ExpAst> if_exp;
    std::unique_ptr<ExpAst> else_exp;

public:
    IfExpAst(std::unique_ptr<ExpAst> _condition,
    std::unique_ptr<ExpAst> _if_exp,
    std::unique_ptr<ExpAst> _else_exp){
        condition = std::move(_condition);
        if_exp = std::move(_if_exp);
        else_exp = std::move(_else_exp);
    }

    llvm::Value *codegen() override;
};

/// CallExpAst - Expression class for function calls.
class CallExpAst : public ExpAst {
    std::string Callee;
    std::vector<std::unique_ptr<ExpAst>> Args;

    public:
    CallExpAst(const std::string &Callee,
                std::vector<std::unique_ptr<ExpAst>> Args)
        : Callee(Callee), Args(std::move(Args)) {}

    llvm::Value *codegen() override;
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

    llvm::Function *codegen();
};

/// FunctionAst - This class represents a function definition itself.
class FunctionAst {
    std::unique_ptr<PrototypeAst> Proto;
    std::unique_ptr<MutiExpAst> Body;

    public:
    FunctionAst(std::unique_ptr<PrototypeAst> _Proto,
                std::unique_ptr<MutiExpAst> _Body)
        : Proto(std::move(_Proto)), Body(std::move(_Body)) {}

    llvm::Function *codegen();
};

class BuildTree {
public:
    std::unique_ptr<Tokenizer> tokenizer;
    void SetTokenizer(std::unique_ptr<Tokenizer> _tokenizer){
        tokenizer = std::move(_tokenizer);
    }

    void CheckAndNextToken(Token tk, Defination::token_type type){
        if(tk.GetTokenType() != type)
            error_process->PrintError(tk, "Parser " + std::string(Defination::GetInstance().token_map[type]) + " error");
        NextToken();
    }

    bool Check(Token tk, Defination::token_type type){
        if(tk.GetTokenType() != type)
            return false;
        return true;
    }

    void Run(){   
        error_process = std::make_unique<ErrorProcess>();
        // Get token from Token class
        token = tokenizer->GetToken();

        error_process->SetSouceCode(tokenizer->GetSourceCode());
        while(GetToken().GetTokenType() != Defination::TOKEN_EOF){
            auto token_tmp = GetToken();
            if(token_tmp.GetTokenType() == Defination::TOKEN_FUNCTION){
                HandleFunction();
            }
            else if(token_tmp.GetTokenType() == Defination::TOKEN_ID){
                HandleCallFunction();
            }
            else if(Check(GetToken(), Defination::TOKEN_EXTERN)){
                HandleExternExp();
            }
        }
    }

    std::unique_ptr<ExpAst> HandleCallFunction(){
        if(!(GetToken().GetTokenType() == Defination::TOKEN_ID))
            error_process->PrintError(GetToken(), "Parser call token id error");
        std::string function_name = GetToken().GetTokenName();

        NextToken();
        if(!(GetToken().GetTokenType() == Defination::TOKEN_SMALL_SCOPE_BEGIN))
           error_process->PrintError(GetToken(), "Parser call '(' error");

        std::vector<std::unique_ptr<ExpAst>> args;
        NextToken();
        auto exp = HandlePrimaryExp();
        args.push_back(std::move(exp));

        while(true){
            if(!Check(GetToken(), Defination::TOKEN_COMMA))
                break;
            NextToken();
            exp = HandlePrimaryExp();
            args.push_back(std::move(exp));
        }
        CheckAndNextToken(GetToken(), Defination::TOKEN_SMALL_SCOPE_END);
        return std::make_unique<CallExpAst>(function_name, std::move(args));
    }

    std::unique_ptr<ExpAst> HandlePrimaryExp(){
        if(GetToken().GetTokenType() == Defination::TOKEN_ID && 
        GetToken(1).GetTokenType() == Defination::TOKEN_EQUAL){
            return HandleAssignmentExp();
        }
        else if(GetToken().GetTokenType() == Defination::TOKEN_NUMBER &&
        GetToken(1).GetTokenType() == Defination::TOKEN_OPERATOR){
            return HandleBinaryExp();
        }
        else if((GetToken().GetTokenType() == Defination::TOKEN_NUMBER || GetToken().GetTokenType() == Defination::TOKEN_ID) &&
        GetToken(1).GetTokenType() != Defination::TOKEN_OPERATOR){
            return HandleSingleExp();
        }
        return nullptr;
    }

    std::unique_ptr<ExpAst> HandleSingleExp(){
        // Parser a single exp
        if(Check(GetToken(), Defination::TOKEN_ID)){
            std::string var = GetToken().GetTokenName();
            NextToken();
            return std::make_unique<VariableExpAst>(var);
        }
        else if(Check(GetToken(), Defination::TOKEN_NUMBER)){
            double num = atof(GetToken().GetTokenName().c_str());
            NextToken();
            return std::make_unique<NumberExpAst>(num);
        }
        else
            return nullptr;
    }

    std::unique_ptr<AssignExpAst> HandleAssignmentExp(){
        
        // var 
        auto var = std::make_unique<VariableExpAst>(GetToken().GetTokenName());
        NextToken();

        CheckAndNextToken(GetToken(), Defination::TOKEN_EQUAL);

        if(GetToken(1).GetTokenType() == Defination::TOKEN_OPERATOR){
            auto exp = HandleBinaryExp();
            return std::make_unique<AssignExpAst>(std::move(var), std::move(exp));
        }
        else{
            auto exp = HandleSingleExp();
            return std::make_unique<AssignExpAst>(std::move(var), std::move(exp));
        }
    }

    std::unique_ptr<BinaryExpAst> HandleBinaryExp(){
        std::unique_ptr<ExpAst> LHS = nullptr;

        // LHS
        if(Check(GetToken(), Defination::TOKEN_ID)){
            LHS = std::make_unique<VariableExpAst>(GetToken().GetTokenName());
            NextToken();
        }
        else if(Check(GetToken(), Defination::TOKEN_NUMBER)){
            LHS = std::make_unique<NumberExpAst>(atof(GetToken().GetTokenName().c_str()));
            NextToken();
        }

        // LHS opcode
        Check(GetToken(), Defination::TOKEN_OPERATOR);
        NextToken();
        char op = GetToken().GetTokenName()[0];

        // LHS opcode RHS
        std::unique_ptr<ExpAst> RHS = nullptr;
        if(Check(GetToken(), Defination::TOKEN_ID)){
            RHS = std::make_unique<VariableExpAst>(GetToken().GetTokenName());
            NextToken();
        }
        else if(Check(GetToken(), Defination::TOKEN_NUMBER)){
            RHS = std::make_unique<NumberExpAst>(atof(GetToken().GetTokenName().c_str()));
            NextToken();
        }

        return std::make_unique<BinaryExpAst>(op, std::move(LHS), std::move(RHS));
    }

    std::unique_ptr<FunctionAst> HandleFunction(){
        CheckAndNextToken(GetToken(), Defination::TOKEN_FUNCTION);

        auto proto = HandleProtoType();

        CheckAndNextToken(GetToken(), Defination::TOKEN_SCOPE_BEGIN);

        auto exp = HandleExp();

        CheckAndNextToken(GetToken(), Defination::TOKEN_SCOPE_END);
        
        return std::make_unique<FunctionAst>(std::move(proto), std::move(exp));
    }

    std::unique_ptr<MutiExpAst> HandleExp(){
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
            else
                break;
        }

        return std::make_unique<MutiExpAst>(std::move(exp));
    }

    std::unique_ptr<IfExpAst> HandleIfExp(){
        // if
        CheckAndNextToken(GetToken(), Defination::TOKEN_IF);

        // if (
        CheckAndNextToken(GetToken(), Defination::TOKEN_SMALL_SCOPE_BEGIN);

        // if ( condition
        auto condition = HandleBinaryExp();

        // if ( condition )
        CheckAndNextToken(GetToken(), Defination::TOKEN_SMALL_SCOPE_END);

        // if ( condition ) {
        CheckAndNextToken(GetToken(), Defination::TOKEN_SCOPE_BEGIN);

        // if ( condition ) {
        //   exp
        auto if_exp = HandleExp();

        // if ( condition ) {
        //   exp
        // }
        CheckAndNextToken(GetToken(), Defination::TOKEN_SCOPE_END);

        // if ( condition ) {
        //   exp
        // } else
        CheckAndNextToken(GetToken(), Defination::TOKEN_ELSE);

        // if ( condition ) {
        //   exp
        // } else {
        CheckAndNextToken(GetToken(), Defination::TOKEN_SCOPE_BEGIN);

        // if ( condition ) {
        //   exp
        // } else {
        //   exp
        auto else_exp = HandleExp();

        // if ( condition ) {
        //   exp
        // } else {
        //   exp
        // }
        CheckAndNextToken(GetToken(), Defination::TOKEN_SCOPE_END);

        // return if-else exp
        return std::make_unique<IfExpAst>(std::move(condition), std::move(if_exp), std::move(else_exp));
    }

    std::unique_ptr<PrototypeAst> HandleExternExp(){
        CheckAndNextToken(GetToken(), Defination::TOKEN_EXTERN);
        
        auto proto = HandleProtoType();
        
        return proto;
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
        std::string function_name = GetToken().GetTokenName();

        CheckAndNextToken(GetToken(), Defination::TOKEN_ID);

        CheckAndNextToken(GetToken(), Defination::TOKEN_SMALL_SCOPE_BEGIN);

        while(Check(GetToken(), Defination::TOKEN_ID)){
            args.push_back(GetToken().GetTokenName());
            NextToken();

            if(!Check(GetToken(), Defination::TOKEN_COMMA))
                break;
            NextToken();
        }

        CheckAndNextToken(GetToken(), Defination::TOKEN_SMALL_SCOPE_END);

        return std::make_unique<PrototypeAst>(function_name, std::move(args));
    }

    Token GetToken(int i = 0){
        if(!TokenIsEnd(i))
            return *token[token_index + i];
        else
            return Token();
    }

    void NextToken(){
        token_index ++;
        // std::cout << "TOKEN:\t" << (*token[token_index]).GetTokenName() << std::endl;
    }

    bool TokenIsEnd(int i = 0){
        if(token_index + i <= (int)token.size() - 1)
            return false;
        return true;
    }
    int token_index = 0;
    std::vector<std::shared_ptr<Token>> token;
    std::unique_ptr<ErrorProcess> error_process;
};

}

#endif