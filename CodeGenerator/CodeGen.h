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
#include "Expression.h"

#include <list>

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
        Void,
        Unkown,
    };
    struct Environment {
        std::map<std::string, llvm::Value*>         declared_variable;
        std::map<std::string, llvm::Function *>     declared_prototype;
        llvm::BasicBlock*                           block;
        uint64_t                                    if_block_num = 0;
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
    std::string                         _out_filename = "out";
    std::string                         _module_name = "module";
    llvm::TargetMachine*                _target_machine = nullptr;


    llvm::Type* getValueType(std::string type_name);
    llvm::Type* getPointerOriginType(llvm::Value* pointer_type);
    bool isDoubleType(llvm::Value* v);

    llvm::Value* declareProtoGen(AstPtr, std::list<Environment>&);
    llvm::Value* assignGen(AstPtr, std::list<Environment>&);
    llvm::Value* funcCallGen(AstPtr, std::list<Environment>&);
    llvm::Value* declareVarGen(AstPtr, std::list<Environment>&);
    llvm::Value* ifStatementGen(AstPtr, std::list<Environment>&);
    llvm::Value* returnGen(AstPtr, std::list<Environment>&);
    llvm::Value* whileStatementGen(AstPtr, std::list<Environment>&);
    llvm::Value* ifBlockGen(std::list<Environment>& env, IfBlock ast, llvm::BasicBlock* block, llvm::BasicBlock* branch, llvm::BasicBlock* merge);
    llvm::Value* elseBlockGen(std::list<Environment>& env, AstBlockPtr ast, llvm::BasicBlock* merge);

    llvm::Value* exprGen(AstPtr, std::list<Environment>&);
    llvm::Value* opExprGen(AstPtr, std::list<Environment>&);
    llvm::Value* addExprGen(ExpressionPtr, ExpressionPtr, std::list<Environment>&);
    llvm::Value* subExprGen(ExpressionPtr, ExpressionPtr, std::list<Environment>&);
    llvm::Value* mulExprGen(ExpressionPtr, ExpressionPtr, std::list<Environment>&);
    llvm::Value* divExprGen(ExpressionPtr, ExpressionPtr, std::list<Environment>&);
    llvm::Value* numberExprGen(AstPtr, std::list<Environment>&);
    llvm::Value* blockGen(AstPtr, std::list<Environment>);
    llvm::Value* identifierExprGen(AstPtr, std::list<Environment>&);
    llvm::Value* BoolExprGen(AstPtr, std::list<Environment>&);
    llvm::Value* stringExprGen(AstPtr, std::list<Environment>&);

    llvm::IRBuilder<> getBuilder(std::list<Environment>& env);
    void MainFuncCodegen();
};

} //begonia
