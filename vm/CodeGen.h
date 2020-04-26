#pragma once
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"

#include "Parser.h"
#include "expression.h"

namespace begonia {
//class 
class CodeGen {
public:
    using ValueTypeSize = int8_t;
    enum class ValueType: ValueTypeSize{
        String,
        Int,
        Double,
        Bool,
        Unkown,
    };
    struct Environment {
        std::map<std::string, llvm::Value*>         declared_variable;
        std::map<std::string, llvm::FunctionType *> declared_prototype;
        llvm::BasicBlock*                           block;
    };
    using GeneratorHandler = std::function<llvm::Value*(AstPtr,std::list<Environment>&)>;

    CodeGen();
    int initialize();
    int generate(AstPtr ast );

private:
    llvm::LLVMContext                   _context;
    llvm::IRBuilder<>                   _builder;
    std::unique_ptr<llvm::Module>       _module;
    std::map<std::string, ValueType>    _basic_variable_type;
    std::map<AstType, GeneratorHandler> _generator;
    Environment                         _global_env;
    llvm::legacy::PassManager           _pass;
    std::string                         _outFilename = "output.o";
    std::shared_ptr<llvm::raw_fd_ostream> _outDestPtr;


    llvm::Type* getValueType(std::string type_name);

    llvm::Value* declareProtoGen(AstPtr, std::list<Environment>&);
    llvm::Value* assignGen(AstPtr, std::list<Environment>&);
    llvm::Value* funcallGen(AstPtr, std::list<Environment>&);
    llvm::Value* declarVarGen(AstPtr, std::list<Environment>&);
    llvm::Value* ifBlockGen(AstPtr, std::list<Environment>&);
    llvm::Value* returnGen(AstPtr, std::list<Environment>&);
    llvm::Value* whileBlockGen(AstPtr, std::list<Environment>&);
    llvm::Value* exprGen(AstPtr, std::list<Environment>&);
    llvm::Value* opExprGen(AstPtr, std::list<Environment>&);
    llvm::Value* addExprGen(ExpressionPtr, ExpressionPtr, std::list<Environment>&);
    llvm::Value* numberExprGen(AstPtr, std::list<Environment>&);
    llvm::Value* blockGen(AstPtr, std::list<Environment>);
    llvm::Value* IdentifierExprGen(AstPtr, std::list<Environment>&);

    llvm::IRBuilder<> getBuilder(std::list<Environment>& env);
};

} //begonia