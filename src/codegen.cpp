
#include "ast.h"
#include "log/log.h"
namespace CodeGen{
    
    std::unique_ptr<llvm::LLVMContext> TheContext;
    std::unique_ptr<llvm::Module> TheModule;
    std::unique_ptr<llvm::IRBuilder<>> Builder;
    std::map<std::string, llvm::Value *> NamedValues;
};

llvm::Value *LogErrorV(const char *Str) {
    LOG::ERROR(Str);
    return nullptr;
}

llvm::Value* openjs::NumberExpAst::codegen() {
    return llvm::ConstantFP::get(*CodeGen::TheContext, llvm::APFloat(val));
}

llvm::Value* openjs::VariableExpAst::codegen() {
    // Look this variable up in the function.
    llvm::Value *V = CodeGen::NamedValues[Name];
    if (!V)
        return LogErrorV("Unknown variable name");
    return V;
}

llvm::Value* openjs::BinaryExpAst::codegen() {
    llvm::Value *L = LHS->codegen();
    llvm::Value *R = RHS->codegen();
    if (!L || !R)
        return nullptr;

    switch (Op) {
    case '+':
        return CodeGen::Builder->CreateFAdd(L, R, "addtmp");
    case '-':
        return CodeGen::Builder->CreateFSub(L, R, "subtmp");
    case '*':
        return CodeGen::Builder->CreateFMul(L, R, "multmp");
    case '/':
        return CodeGen::Builder->CreateUDiv(L, R, "divtmp");
    //   case '<':
    //     L = Builder->CreateFCmpULT(L, R, "cmptmp");
    //     // Convert bool 0/1 to double 0.0 or 1.0
    //     return Builder->CreateUIToFP(L, Type::getDoubleTy(*TheContext), "booltmp");
    default:
        return LogErrorV("invalid binary operator");
    }
}

llvm::Value* openjs::AssignExpAst::codegen(){
    // var = std::move(RHS->codegen());
    return nullptr;
}

llvm::Value* openjs::MutiExpAst::codegen(){
    for(int i = 0; i < (int)exp.size(); i++)
        exp[i]->codegen();
    return nullptr;
}

llvm::Value* openjs::IfExpAst::codegen(){
    llvm::Value *CondV = condition->codegen();
    if (!CondV)
        return nullptr;

    // Convert condition to a bool by comparing non-equal to 0.0.
    CondV = CodeGen::Builder->CreateFCmpONE(
        CondV, llvm::ConstantFP::get(*CodeGen::TheContext, llvm::APFloat(0.0)), "ifcond");

    llvm::Function *TheFunction = CodeGen::Builder->GetInsertBlock()->getParent();

    // Create blocks for the then and else cases.  Insert the 'then' block at the
    // end of the function.
    llvm::BasicBlock *ThenBB = llvm::BasicBlock::Create(*CodeGen::TheContext, "then", TheFunction);
    llvm::BasicBlock *ElseBB = llvm::BasicBlock::Create(*CodeGen::TheContext, "else");
    llvm::BasicBlock *MergeBB = llvm::BasicBlock::Create(*CodeGen::TheContext, "ifcont");

    CodeGen::Builder->CreateCondBr(CondV, ThenBB, ElseBB);

    // Emit then value.
    CodeGen::Builder->SetInsertPoint(ThenBB);

    llvm::Value *ThenV = if_exp->codegen();
    if (!ThenV)
        return nullptr;

    CodeGen::Builder->CreateBr(MergeBB);
    // Codegen of 'Then' can change the current block, update ThenBB for the PHI.
    ThenBB = CodeGen::Builder->GetInsertBlock();

    // Emit else block.
    TheFunction->getBasicBlockList().push_back(ElseBB);
    CodeGen::Builder->SetInsertPoint(ElseBB);

    llvm::Value *ElseV = else_exp->codegen();
    if (!ElseV)
        return nullptr;

    CodeGen::Builder->CreateBr(MergeBB);
    // Codegen of 'Else' can change the current block, update ElseBB for the PHI.
    ElseBB = CodeGen::Builder->GetInsertBlock();

    // Emit merge block.
    TheFunction->getBasicBlockList().push_back(MergeBB);
    CodeGen::Builder->SetInsertPoint(MergeBB);
    llvm::PHINode *PN = CodeGen::Builder->CreatePHI(llvm::Type::getDoubleTy(*CodeGen::TheContext), 2, "iftmp");

    PN->addIncoming(ThenV, ThenBB);
    PN->addIncoming(ElseV, ElseBB);
    return PN;

}

llvm::Value* openjs::CallExpAst::codegen() {
    // Look up the name in the global module table.
    llvm::Function *CalleeF = CodeGen::TheModule->getFunction(Callee);
    if (!CalleeF)
        return LogErrorV("Unknown function referenced");

    // If argument mismatch error.
    if (CalleeF->arg_size() != Args.size())
        return LogErrorV("Incorrect # arguments passed");

    std::vector<llvm::Value *> ArgsV;
    for (unsigned i = 0, e = Args.size(); i != e; ++i) {
        ArgsV.push_back(Args[i]->codegen());
        if (!ArgsV.back())
        return nullptr;
    }

    CodeGen::Builder->CreateCall(CalleeF, ArgsV, "calltmp");
    return nullptr;
}

llvm::Function* openjs::PrototypeAst::codegen() {
    // Make the function type:  double(double,double) etc.
    std::vector<llvm::Type *> Doubles(Args.size(), llvm::Type::getDoubleTy(*CodeGen::TheContext));
    llvm::FunctionType *FT =
        llvm::FunctionType::get(llvm::Type::getDoubleTy(*CodeGen::TheContext), Doubles, false);

    //llvm::FunctionType * function_type = llvm::FunctionType::get();

    llvm::Function *F =
        llvm::Function::Create(FT, llvm::Function::ExternalLinkage, Name, CodeGen::TheModule.get());

    // Set names for all arguments.
    unsigned Idx = 0;
    for (auto &Arg : F->args())
        Arg.setName(Args[Idx++]);

    return F;
}

llvm::Function* openjs::FunctionAst::codegen() {
    // First, check for an existing function from a previous 'extern' declaration.
    llvm::Function *TheFunction = CodeGen::TheModule->getFunction(Proto->getName());

    if (!TheFunction)
        TheFunction = Proto->codegen();

    if (!TheFunction)
        return nullptr;

    // Create a new basic block to start insertion into.
    llvm::BasicBlock *BB = llvm::BasicBlock::Create(*CodeGen::TheContext, "entry", TheFunction);
    CodeGen::Builder->SetInsertPoint(BB);

    // Record the function arguments in the NamedValues map.
    CodeGen::NamedValues.clear();
    for (auto &Arg : TheFunction->args())
        CodeGen::NamedValues[std::string(Arg.getName())] = &Arg;

    if (llvm::Value *RetVal = Body->codegen()) {
        // Finish off the function.
        // Builder->CreateRet(RetVal);

        // Validate the generated code, checking for consistency.
        verifyFunction(*TheFunction);

        return TheFunction;
    }

    // Error reading body, remove function.
    TheFunction->eraseFromParent();
    return nullptr;
}
