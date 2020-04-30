#include "Parser.h"
#include "Expression.h"
#include "CodeGen.h"

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
#include <iostream>

namespace begonia {

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
        case ValueType::Void:
            return llvm::Type::getVoidTy(_context);
        default:
            //TODO:
            //return llvm::StructType::get(_context);
            printf("Unknown type:%s\n", type_name.c_str());
            assert(false);
            return nullptr;
        }
    } else {
        printf("Unknown type:%s\n", type_name.c_str());
        assert(false);
    }
    
    // TODO: struct type
    return nullptr;

}

llvm::Value* CodeGen::declareProtoGen(AstPtr ast, std::list<Environment>& env) {
    auto funcAst = std::dynamic_pointer_cast<DeclareFuncStatement>(ast);
    assert(funcAst != nullptr);

    auto has_declared = env.front().declared_prototype.find(funcAst->_name);
    if (has_declared != env.front().declared_prototype.end()) {
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
    
    env.front().declared_prototype[funcAst->_name] = func;

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

        env.push_front(current_env);
        blockGen(funcAst->_block, env);
        env.pop_front();

    }
    llvm::verifyFunction(*func);
    return nullptr;
}

llvm::Value* CodeGen::blockGen(AstPtr ast, std::list<Environment> env) {
    auto block = std::dynamic_pointer_cast<AstBlock>(ast);
    assert(block != nullptr);

    for(auto statement : *block) {
        auto found = _generator.find(statement->GetType());
        assert(found != _generator.end());
        auto handler = found->second;
        handler(statement, env);
    }
    return nullptr;
}

llvm::Value* CodeGen::assignGen(AstPtr ast, std::list<Environment>& env) {
    auto assignAst = std::dynamic_pointer_cast<AssignStatement>(ast);
    assert(assignAst != nullptr);
    auto builder = getBuilder(env);

    auto var_name = assignAst->_identifier;
    llvm::Value* var_addr;
    auto& declared_variable = env.front().declared_variable;

    auto found = declared_variable.find(var_name);
    if (found == declared_variable.end()) {
        printf("undefined var:%s\n", var_name.c_str());
        assert(false);
    }
    var_addr = found->second;

    auto val_expr = assignAst->_assign_value;
    auto val = exprGen(val_expr, env);
    if(val->getType()->isPointerTy() && val->getType() != llvm::Type::getInt8PtrTy(_context)){
        val = builder.CreateLoad(val->getType()->getPointerElementType(), val);
    }
    builder.CreateStore(val, var_addr);
    return nullptr;
}

llvm::IRBuilder<> CodeGen::getBuilder(std::list<Environment>& env){
    auto block = env.front().block;
    if ( block == nullptr) {
        return _builder;
    } else {
        return llvm::IRBuilder<>(block);
    }

}

llvm::Value* CodeGen::declareVarGen(AstPtr ast, std::list<Environment>& env) {
    auto builder = getBuilder(env);
    auto var_stat = std::dynamic_pointer_cast<DeclareVarStatement>(ast);
    assert(var_stat != nullptr);

    llvm::Value* var_addr = nullptr;
    if (var_stat->_type != "") {
        var_addr = builder.CreateAlloca(getValueType(var_stat->_type));
    
    }else if (var_stat->_assign_value != nullptr) {
        llvm::Value* assign_value = exprGen(var_stat->_assign_value, env);
        assert(assign_value != nullptr);
        if (assign_value->getType()->isPointerTy()
         && assign_value->getType() != llvm::Type::getInt8PtrTy(_context)) {
            var_addr = builder.CreateAlloca(assign_value->getType()->getPointerElementType());
            assign_value->getType()->getContainedType(0)->print(llvm::errs());
        } else {
            var_addr = builder.CreateAlloca(assign_value->getType());
        }

        builder.CreateStore(assign_value, var_addr);
    } else{
        assert(false&&"Unkown type for define variable");
    }

    env.front().declared_variable[var_stat->_name] = var_addr;
    
    return nullptr;
}

llvm::Value* CodeGen::returnGen(AstPtr ast, std::list<Environment>& env) {
    auto builder = getBuilder(env);
    auto ret_stat = std::dynamic_pointer_cast<ReturnStatement>(ast);
    assert(ret_stat != nullptr);
    if (ret_stat->_ret_values.size() == 0) {
        builder.CreateRetVoid();
    } else {
        auto ret_val = exprGen(ret_stat->_ret_values[0], env);
        builder.CreateRet(ret_val);
    }
    return nullptr;
}

llvm::Value* CodeGen::whileBlockGen(AstPtr ast, std::list<Environment>& env) {
    return nullptr;
}

llvm::Value* CodeGen::ifBlockGen(AstPtr ast, std::list<Environment>& env) {
    return nullptr;
}

} //begonia
