
#include "ast.h"
#include "log/log.h"

using namespace llvm;
namespace CodeGen{
    
    std::unique_ptr<LLVMContext> TheContext;
    std::unique_ptr<Module> TheModule;
    std::unique_ptr<IRBuilder<>> Builder;
    std::map<std::string, Value *> NamedValues;
};

Value *LogErrorV(const char *Str) {
    LOG::ERROR(Str);
    return nullptr;
}

Value* openjs::NumberExpAst::codegen() {
    return ConstantFP::get(*CodeGen::TheContext, APFloat(val));
}

Value* openjs::VariableExpAst::codegen() {
    // Look this variable up in the function.
    Value *V = CodeGen::NamedValues[Name];
    if (!V)
        return LogErrorV("Unknown variable name");
    return V;
}

Value* openjs::BinaryExpAst::codegen() {
    Value *L = LHS->codegen();
    Value *R = RHS->codegen();
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

Value* openjs::AssignExpAst::codegen(){

    std::vector<AllocaInst *> OldBindings;

    Function *TheFunction = Builder->GetInsertBlock()->getParent();

    // Register all variables and emit their initializer.
    for (unsigned i = 0, e = VarNames.size(); i != e; ++i) {
        const std::string &VarName = VarNames[i].first;
        ExprAST *Init = VarNames[i].second.get();

        // Emit the initializer before adding the variable to scope, this prevents
        // the initializer from referencing the variable itself, and permits stuff
        // like this:
        //  var a = 1 in
        //    var a = a in ...   # refers to outer 'a'.
        Value *InitVal;
        if (Init) {
        InitVal = Init->codegen();
        if (!InitVal)
            return nullptr;
        } else { // If not specified, use 0.0.
        InitVal = ConstantFP::get(*TheContext, APFloat(0.0));
        }

        AllocaInst *Alloca = CreateEntryBlockAlloca(TheFunction, VarName);
        Builder->CreateStore(InitVal, Alloca);

        // Remember the old variable binding so that we can restore the binding when
        // we unrecurse.
        OldBindings.push_back(NamedValues[VarName]);

        // Remember this binding.
        NamedValues[VarName] = Alloca;
    }

    // Codegen the body, now that all vars are in scope.
    Value *BodyVal = Body->codegen();
    if (!BodyVal)
        return nullptr;

    // Pop all our variables from scope.
    for (unsigned i = 0, e = VarNames.size(); i != e; ++i)
        NamedValues[VarNames[i].first] = OldBindings[i];

    // Return the body computation.
    return BodyVal;
}

Value* openjs::MutiExpAst::codegen(){
    for(int i = 0; i < (int)exp.size(); i++)
        exp[i]->codegen();
    return nullptr;
}

Value* openjs::IfExpAst::codegen(){
    Value *CondV = condition->codegen();
    if (!CondV)
        return nullptr;

    // Convert condition to a bool by comparing non-equal to 0.0.
    CondV = CodeGen::Builder->CreateFCmpONE(
        CondV, ConstantFP::get(*CodeGen::TheContext, APFloat(0.0)), "ifcond");

    Function *TheFunction = CodeGen::Builder->GetInsertBlock()->getParent();

    // Create blocks for the then and else cases.  Insert the 'then' block at the
    // end of the function.
    BasicBlock *ThenBB = BasicBlock::Create(*CodeGen::TheContext, "then", TheFunction);
    BasicBlock *ElseBB = BasicBlock::Create(*CodeGen::TheContext, "else");
    BasicBlock *MergeBB = BasicBlock::Create(*CodeGen::TheContext, "ifcont");

    CodeGen::Builder->CreateCondBr(CondV, ThenBB, ElseBB);

    // Emit then value.
    CodeGen::Builder->SetInsertPoint(ThenBB);

    Value *ThenV = if_exp->codegen();
    if (!ThenV)
        return nullptr;

    CodeGen::Builder->CreateBr(MergeBB);
    // Codegen of 'Then' can change the current block, update ThenBB for the PHI.
    ThenBB = CodeGen::Builder->GetInsertBlock();

    // Emit else block.
    TheFunction->getBasicBlockList().push_back(ElseBB);
    CodeGen::Builder->SetInsertPoint(ElseBB);

    Value *ElseV = else_exp->codegen();
    if (!ElseV)
        return nullptr;

    CodeGen::Builder->CreateBr(MergeBB);
    // Codegen of 'Else' can change the current block, update ElseBB for the PHI.
    ElseBB = CodeGen::Builder->GetInsertBlock();

    // Emit merge block.
    TheFunction->getBasicBlockList().push_back(MergeBB);
    CodeGen::Builder->SetInsertPoint(MergeBB);
    PHINode *PN = CodeGen::Builder->CreatePHI(Type::getDoubleTy(*CodeGen::TheContext), 2, "iftmp");

    PN->addIncoming(ThenV, ThenBB);
    PN->addIncoming(ElseV, ElseBB);
    return PN;

}

Value* openjs::CallExpAst::codegen() {
    // Look up the name in the global module table.
    Function *CalleeF = CodeGen::TheModule->getFunction(Callee);
    if (!CalleeF)
        return LogErrorV("Unknown function referenced");

    // If argument mismatch error.
    if (CalleeF->arg_size() != Args.size())
        return LogErrorV("Incorrect # arguments passed");

    std::vector<Value *> ArgsV;
    for (unsigned i = 0, e = Args.size(); i != e; ++i) {
        ArgsV.push_back(Args[i]->codegen());
        if (!ArgsV.back())
        return nullptr;
    }

    CodeGen::Builder->CreateCall(CalleeF, ArgsV, "calltmp");
    return nullptr;
}

Function* openjs::PrototypeAst::codegen() {
    // Make the function type:  double(double,double) etc.
    std::vector<Type *> Doubles(Args.size(), Type::getDoubleTy(*CodeGen::TheContext));
    FunctionType *FT =
        FunctionType::get(Type::getDoubleTy(*CodeGen::TheContext), Doubles, false);

    //FunctionType * function_type = FunctionType::get();

    Function *F =
        Function::Create(FT, Function::ExternalLinkage, Name, CodeGen::TheModule.get());

    // Set names for all arguments.
    unsigned Idx = 0;
    for (auto &Arg : F->args())
        Arg.setName(Args[Idx++]);

    return F;
}

Function* openjs::FunctionAst::codegen() {
    // First, check for an existing function from a previous 'extern' declaration.
    Function *TheFunction = CodeGen::TheModule->getFunction(Proto->getName());

    if (!TheFunction)
        TheFunction = Proto->codegen();

    if (!TheFunction)
        return nullptr;

    // Create a new basic block to start insertion into.
    BasicBlock *BB = BasicBlock::Create(*CodeGen::TheContext, "entry", TheFunction);
    CodeGen::Builder->SetInsertPoint(BB);

    // Record the function arguments in the NamedValues map.
    CodeGen::NamedValues.clear();
    for (auto &Arg : TheFunction->args())
        CodeGen::NamedValues[std::string(Arg.getName())] = &Arg;

    if (Value *RetVal = Body->codegen()) {
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
