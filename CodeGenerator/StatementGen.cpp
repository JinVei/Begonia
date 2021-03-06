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
#include <list>

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
            printf("Unkown Type:%s\n",var->_type.c_str());
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

    Environment current_env;

    auto decl_args = (funcAst->_decl_vars).begin();
    for (auto &arg : func->args()) {
        arg.setName((*decl_args)->_name);
        decl_args++;
        current_env.declared_variable[arg.getName().str()] = &arg;
    }

    if (funcAst->_block->size() != 0) {
        llvm::BasicBlock *block = llvm::BasicBlock::Create(_context, "entry", func);
        current_env.block = block;
        _builder.SetInsertPoint(current_env.block);
 
        env.push_front(current_env);
        blockGen(funcAst->_block, env);
        env.pop_front();
    }
    // llvm::raw_ostream &output = llvm::errs();
    // if (llvm::verifyFunction(*func, &output)) {
    //     assert(false && "verifyFunction false");
    // }
    return nullptr;
}

llvm::Value* CodeGen::blockGen(AstPtr ast, std::list<Environment>& env) {
    auto ast_block = std::dynamic_pointer_cast<AstBlock>(ast);
    assert(ast_block != nullptr);
    auto block = _builder.GetInsertBlock();

    for(auto statement : *ast_block) {
        auto found = _generator.find(statement->GetType());
        assert(found != _generator.end());
        auto handler = found->second;
        handler(statement, env);

        if (statement->GetType() == AstType::RetStatement){
            break;
        }
    }
    return nullptr;
}

llvm::Value* CodeGen::assignGen(AstPtr ast, std::list<Environment>& env) {
    auto assignAst = std::dynamic_pointer_cast<AssignStatement>(ast);
    assert(assignAst != nullptr);
    auto& builder = _builder;
    builder.SetInsertPoint(env.front().block);
    

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

// llvm::IRBuilder<> CodeGen::getBuilder(std::list<Environment>& env){
//     auto block = env.front().block;
//     if ( block == nullptr) {
//         return _builder;
//     } else {
//         return llvm::IRBuilder<>(block);
//     }

// }

llvm::Value* CodeGen::declareVarGen(AstPtr ast, std::list<Environment>& env) {
    auto& builder = _builder;
    builder.SetInsertPoint(env.front().block);
    auto var_stat = std::dynamic_pointer_cast<DeclareVarStatement>(ast);
    assert(var_stat != nullptr);

    llvm::Value* var_addr = nullptr;
    if (var_stat->_assign_value != nullptr) {
        llvm::Value* assign_value = exprGen(var_stat->_assign_value, env);
        if (var_stat->_type != "" && (assign_value->getType() != getValueType(var_stat->_type))){
            assert(false && "var type no matched");
        }
        
        assert(assign_value != nullptr);
        if (assign_value->getType()->isPointerTy()
         && assign_value->getType() != llvm::Type::getInt8PtrTy(_context)) {
            var_addr = builder.CreateAlloca(assign_value->getType()->getPointerElementType());
            assign_value->getType()->getContainedType(0)->print(llvm::errs());
        } else {
            var_addr = builder.CreateAlloca(assign_value->getType());
        }

        builder.CreateStore(assign_value, var_addr);

    } else if (var_stat->_type != "") {
        var_addr = builder.CreateAlloca(getValueType(var_stat->_type));
    }else {
        assert(false&&"Unkown type for define variable");
    }

    env.front().declared_variable[var_stat->_name] = var_addr;
    
    return nullptr;
}

llvm::Value* CodeGen::returnGen(AstPtr ast, std::list<Environment>& env) {
    auto& builder = _builder;
    builder.SetInsertPoint(env.front().block);
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

llvm::Value* CodeGen::whileStatementGen(AstPtr ast, std::list<Environment>& env) {
    return nullptr;
}

llvm::Value* CodeGen::ifBlockGen(std::list<Environment>& env, IfBlock ast, llvm::BasicBlock* block, llvm::BasicBlock* then_block, llvm::BasicBlock* branch, llvm::BasicBlock* merge) {
    auto& builder = _builder;
    Environment frame;
    builder.SetInsertPoint(block);

    auto cond_expr = ast._cond;
    auto cond_val = exprGen(cond_expr, env);
    CondBranchGen(env,cond_val, then_block, branch);

    frame.block = then_block;
    env.push_front(frame);
    builder.SetInsertPoint(frame.block);
    blockGen(ast._block, env);

    if (then_block->getTerminator() == nullptr) {
        builder.CreateBr(merge);
    }
    env.pop_front();
    return nullptr;
}

llvm::Value* CodeGen::elseBlockGen(std::list<Environment>& env, AstBlockPtr ast, llvm::BasicBlock* block, llvm::BasicBlock* merge) {
    auto& builder = _builder;
    builder.SetInsertPoint(env.front().block);

    Environment frame;
    frame.block = block;
    env.push_front(frame);
    builder.SetInsertPoint(block);
    blockGen(ast, env);
    if (block->getTerminator() == nullptr) {
        builder.CreateBr(merge);
    }
    env.pop_front();
    return nullptr;
}

llvm::Value* CodeGen::ifStatementGen(AstPtr ast, std::list<Environment>& env) {
    auto& builder = _builder;
    builder.SetInsertPoint(env.front().block);

    auto if_stat = std::dynamic_pointer_cast<IfStatement>(ast);
    assert(if_stat != nullptr);

    auto paren_func = builder.GetInsertBlock()->getParent();

    std::vector<llvm::BasicBlock*> if_blocks;
    std::vector<llvm::BasicBlock*> then_blocks;
    for(size_t i=0; i < if_stat->_if_blocks.size(); i++) {
        auto block = llvm::BasicBlock::Create(_context, std::to_string(env.front().GetIncID()) + ".if", paren_func);
        if_blocks.push_back(block);

        auto tblock = llvm::BasicBlock::Create(_context, std::to_string(env.front().GetIncID()) + ".then", paren_func);
        then_blocks.push_back(tblock);
    }

    builder.CreateBr(if_blocks[0]);

    llvm::BasicBlock *else_block = nullptr;
    if (if_stat->_else_block != nullptr) {
        else_block = llvm::BasicBlock::Create(_context, std::to_string(env.front().GetIncID()) + ".else", paren_func);
    }

    auto merge_block = llvm::BasicBlock::Create(_context, std::to_string(env.front().GetIncID()) + ".ifend", paren_func);

    size_t if_block_num = if_stat->_if_blocks.size();
    auto block_ast = if_stat->_if_blocks;
    assert(block_ast.size() > 0);
    for(size_t i=0; i<if_block_num-1; i++) {
        ifBlockGen(env, block_ast[i], if_blocks[i], then_blocks[i], if_blocks[i+1], merge_block);
    }
    if (else_block != nullptr) {
        ifBlockGen(env, block_ast[if_block_num-1], if_blocks[if_block_num-1], then_blocks[if_block_num-1], else_block, merge_block);
        elseBlockGen(env, if_stat->_else_block, else_block, merge_block);
    } else {
        ifBlockGen(env, block_ast[if_block_num-1], if_blocks[if_block_num-1], then_blocks[if_block_num-1], merge_block, merge_block);
    }

    builder.SetInsertPoint(merge_block);
    env.front().block = merge_block;

    return nullptr;
}

void CodeGen::CondBranchGen(std::list<Environment>& env,llvm::Value* val, llvm::BasicBlock* true_br, llvm::BasicBlock* false_br) {
    auto& e = env.front();
    auto ltype = val->getType();
    llvm::Value* cond_val;
    if (ltype->isIntegerTy()) {
        llvm::Constant* nul_val;
        if (ltype == llvm::Type::getInt64Ty(_context)) {
            nul_val = llvm::Constant::getNullValue(llvm::Type::getInt64Ty(_context));
        } else if (ltype == llvm::Type::getInt1Ty(_context)) {
             nul_val = llvm::Constant::getNullValue(llvm::Type::getInt1Ty(_context));
        } else {
            ltype->print(llvm::errs());
            assert(false && "\nKnow Type");
        }
        cond_val = _builder.CreateICmpEQ(val, nul_val, std::to_string(e.GetIncID()));

    } else if (ltype->isDoubleTy()) {
        auto nul_val = llvm::Constant::getNullValue(llvm::Type::getDoubleTy(_context));
         cond_val = _builder.CreateFCmpOEQ(val, nul_val, std::to_string(e.GetIncID()));
    } else if (ltype->isVoidTy()) {
        cond_val = llvm::Constant::getNullValue(llvm::Type::getInt1Ty(_context));
    } else {
        ltype->print(llvm::errs());
        assert(false && "\nKnow Type");
    }

    _builder.CreateCondBr(cond_val,false_br, true_br);
}

} //begonia
