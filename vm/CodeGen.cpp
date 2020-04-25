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
#include "parser.h"
#include "expression.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

using namespace llvm;
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
    };
    struct Environment {
        std::map<std::string, llvm::Value*>         declared_variable;
        std::map<std::string, llvm::FunctionType *> declared_prototype;
        llvm::BasicBlock*                           block;
    };
    using GeneratorHandler = std::function<llvm::Value*(AstPtr,std::list<Environment>&)>;

    CodeGen();
    llvm::Value* blockGen(AstPtr, std::list<Environment>);

private:
    llvm::LLVMContext                   _context;
    llvm::IRBuilder<>                   _builder;
    std::unique_ptr<llvm::Module>       _module;
    std::map<std::string, ValueType>    _basic_variable_type;
    std::map<AstType, GeneratorHandler> _generator;
    Environment                         _global_env;


    llvm::Type* getValueType(std::string type_name);

    llvm::Value* declareProtoGen(AstPtr, std::list<Environment>&);
    llvm::Value* assignGen(AstPtr, std::list<Environment>&);
    llvm::Value* funcallGen(AstPtr, std::list<Environment>&);
    llvm::Value* declarVarGen(AstPtr, std::list<Environment>&);
    llvm::Value* ifBlockGen(AstPtr, std::list<Environment>&);
    llvm::Value* returnGen(AstPtr, std::list<Environment>&);
    llvm::Value* whileBlockGen(AstPtr, std::list<Environment>&);
    llvm::Value* exprGen(AstPtr ast, std::list<Environment>& env);
    llvm::Value* opExprGen(AstPtr ast, std::list<Environment>& env);
    llvm::Value* addExprGen(ExpressionPtr lexpr, ExpressionPtr rexpr);
    llvm::Value* numberExprGen(ExpressionPtr expr);

};

CodeGen::CodeGen(): _builder(_context) {
    _basic_variable_type = {
        {"string",   ValueType::String},
        {"int",      ValueType::Int},
        {"double",   ValueType::Double},
        {"bool",     ValueType::Bool},
    };
    _generator = {
        {AstType::AssignStatement, std::bind(&CodeGen::assignGen, this, std::placeholders::_1, std::placeholders::_2)},
        {AstType::FuncCallExpr, std::bind(&CodeGen::funcallGen, this, std::placeholders::_1, std::placeholders::_2)},
        {AstType::DeclarFuncStatement, std::bind(&CodeGen::declareProtoGen, this, std::placeholders::_1, std::placeholders::_2)},
        {AstType::DeclarVarStatement, std::bind(&CodeGen::declarVarGen, this, std::placeholders::_1, std::placeholders::_2)},
        {AstType::IfStatement, std::bind(&CodeGen::ifBlockGen, this, std::placeholders::_1, std::placeholders::_2)},
        {AstType::RetStatement, std::bind(&CodeGen::returnGen, this, std::placeholders::_1, std::placeholders::_2)},
    };
}

llvm::Type* CodeGen::getValueType(std::string type_name) {
    auto type = _basic_variable_type.find(type_name);
    if (type != _basic_variable_type.end()) {
        switch (type->second) {
        case ValueType::Bool:
            return llvm::Type::getInt1Ty(_context);
        case ValueType::Double:
            return llvm::Type::getDoubleTy(_context);
        case ValueType::Int:
            return llvm::Type::getInt64Ty(_context);
        case ValueType::String:
            return llvm::Type::getInt8PtrTy(_context);
        default:
            //return llvm::StructType::get(_context);
            return nullptr;
        }
    }
    
    // TODO: struct type
    return nullptr;

}

llvm::Value* CodeGen::declareProtoGen(AstPtr ast, std::list<Environment>& env) {
    auto funcAst = dynamic_pointer_cast<DeclarFuncStatement>(ast);
    assert(funcAst != nullptr);

    auto has_declared = _global_env.declared_prototype.find(funcAst->_name);
    if (has_declared != _global_env.declared_prototype.end()) {
        printf("prototype:%s has declared before\n", funcAst->_name.c_str());
        exit(1);
    }
    std::vector<llvm::Type *> arg_type;
    llvm::Type* ret_type = nullptr;

    for(auto var : funcAst->_decl_vars){
        auto type = getValueType(var->_type);
        if (type == nullptr) {
            printf("got null type\n");
            exit(1);
        }
        arg_type.push_back(type);
    }
    ret_type = getValueType(funcAst->_ret_type);

    llvm::FunctionType *func_proto =
        llvm::FunctionType::get(ret_type, arg_type, false);
    
    llvm::Function *func =
        llvm::Function::Create(func_proto, llvm::Function::ExternalLinkage, funcAst->_name, _module.get());
    
    _global_env.declared_prototype[funcAst->_name] = func_proto;//Todo: local func

    auto decl_arg = (funcAst->_decl_vars).begin();
    for (auto &arg : func->args()) {
        arg.setName((*decl_arg)->_name);
    }

    Environment current_env;
    if (funcAst->_block->size() != 0) {
        llvm::BasicBlock *block = llvm::BasicBlock::Create(_context, "entry", func);
        for (auto &arg : func->args()) {
            llvm::IRBuilder<> block_builder(block);
            llvm::AllocaInst *Alloca = block_builder.CreateAlloca(arg.getType(), nullptr, arg.getName());
            block_builder.CreateStore(&arg, Alloca);
            current_env.declared_variable[arg.getName()] = Alloca;
        }
        current_env.block = block;
        // codeGen body
        env.push_front(current_env);
        blockGen(funcAst->_block, env);
        env.pop_front();
        
    }
    return nullptr;
}

llvm::Value* CodeGen::blockGen(AstPtr ast, std::list<Environment> env) {
    auto block = dynamic_pointer_cast<AstBlock>(ast);
    assert(block != nullptr);

    for(auto statement : *block) {
        auto it = _generator.find(statement->GetType());
        assert(it != _generator.end());
        auto handler = it->second;
        handler(statement, env);
    }
    return nullptr;
}

llvm::Value* CodeGen::assignGen(AstPtr ast, std::list<Environment>& env) {
    auto assignAst = dynamic_pointer_cast<AssignStatement>(ast);
    assert(assignAst != nullptr);
    llvm::IRBuilder<> builder(env.front().block);

    auto var_name = assignAst->_identifier;
    llvm::Value* var_addr;
    auto& declared_variable = env.front().declared_variable;

    auto found = declared_variable.find(var_name);
    if (found == declared_variable.end()) {
        printf("undefined var:%s\n", var_name.c_str());
        abort();
    }
    var_addr = found->second;

    auto val_expr = assignAst->_assign_value;
    auto val = exprGen(val_expr, env);

    builder.CreateStore(val, var_addr);
    return nullptr;
}

llvm::Value* CodeGen::exprGen(AstPtr ast, std::list<Environment>& env) {
    assert(ast != nullptr);
    auto expr = dynamic_pointer_cast<Expression>(ast);
    assert(expr != nullptr);
    switch(expr->GetType()) {
        case AstType::OpExpr:
            return opExprGen(expr, env);
        case AstType::FuncCallExpr:
            return funcallGen(ast, env);
        case AstType::IdentifierExpr:
        case AstType::NumberExpr:
        case AstType::StringExpr:
        case AstType::BoolExpr:
        case AstType::NilExp:
        default:
            printf("[exprGen] unknown expr type\n");
            abort();
            return nullptr;
    }
}

llvm::Value* CodeGen::opExprGen(AstPtr ast, std::list<Environment>& env) {
    auto opexpr = dynamic_pointer_cast<OperationExpresson>(ast);
    assert(opexpr != nullptr);

    auto lexpr = opexpr->_lexp;
    auto rexpr = opexpr->_rexp;
    llvm::Value* lval = nullptr;
    llvm::Value* rval = nullptr;
    if (lexpr != nullptr) {
        lval = exprGen(lexpr, env);
    }
    if (rexpr != nullptr) {
        rval = exprGen(rexpr, env);
    }
    switch(opexpr->_op){
        case TokenType::TOKEN_OP_ADD:
            return addExprGen(lexpr,rexpr);
        case TokenType::TOKEN_OP_SUB:
            return addExprGen(lexpr,rexpr);
        case TokenType::TOKEN_OP_MUL:
            return addExprGen(lexpr,rexpr);
            break;
        case TokenType::TOKEN_OP_DIV:
            return addExprGen(lexpr,rexpr);
        case TokenType::TOKEN_OP_AND:
        case TokenType::TOKEN_OP_OR:
        case TokenType::TOKEN_OP_EQ:
        case TokenType::TOKEN_OP_GE:
        case TokenType::TOKEN_OP_GT:
        case TokenType::TOKEN_OP_LE:
        case TokenType::TOKEN_OP_LT:
        default:
            printf("[opExprGen] unkown op");
            abort();
            return nullptr;
    }
}

llvm::Value* CodeGen::addExprGen(ExpressionPtr lexpr, ExpressionPtr rexpr){
    return nullptr;
}

llvm::Value* CodeGen::numberExprGen(ExpressionPtr expr){
    NumberExpressionPtr numberExpr = dynamic_pointer_cast<NumberExpression>(expr);
    assert(numberExpr != nullptr);
    auto value = llvm::ConstantFP::get(_context, llvm::APFloat(numberExpr->_number));
    return value;
    
}

llvm::Value* CodeGen::funcallGen(AstPtr ast, std::list<Environment>& env) {
    return nullptr;
}
llvm::Value* CodeGen::declarVarGen(AstPtr ast, std::list<Environment>& env) {
    return nullptr;
}
llvm::Value* CodeGen::ifBlockGen(AstPtr ast, std::list<Environment>& env) {
    return nullptr;
}
llvm::Value* CodeGen::returnGen(AstPtr ast, std::list<Environment>& env) {
    return nullptr;
}
llvm::Value* CodeGen::whileBlockGen(AstPtr ast, std::list<Environment>& env) {
    return nullptr;
}

} //begonia

//clang++ CodeGen.cpp -o CodeGen.o -L/usr/local/opt/llvm/lib -Wl,-rpath,/usr/local/opt/llvm/lib `llvm-config --cxxflags --ldflags --system-libs --libs all` -std=c++2a -I../parser/ -I../lexer/