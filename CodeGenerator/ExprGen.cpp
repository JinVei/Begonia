
#include "Parser.h"
#include "Expression.h"
#include "CodeGen.h"

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace begonia {

llvm::Value* CodeGen::exprGen(AstPtr ast, std::list<Environment>& env) {
    assert(ast != nullptr);
    auto expr = std::dynamic_pointer_cast<Expression>(ast);
    assert(expr != nullptr);
    switch(expr->GetType()) {
        case AstType::OpExpr:
            return opExprGen(expr, env);
        case AstType::NumberExpr:
            return numberExprGen(ast, env);
        case AstType::IdentifierExpr:
            return identifierExprGen(ast, env);
        case AstType::FuncCallExpr:
            return funcCallGen(ast, env);
        case AstType::BoolExpr:
            return BoolExprGen(ast, env);
        case AstType::StringExpr:
            return StringExprGen(ast, env);
        case AstType::NilExp:
            //TODO:
        default:
            printf("[exprGen] unknown expr type:%hhu\n", expr->GetType());
            assert(false);
            return nullptr;
    }
}

llvm::Value* CodeGen::opExprGen(AstPtr ast, std::list<Environment>& env) {
    auto opexpr = std::dynamic_pointer_cast<OperationExpresson>(ast);
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
            return addExprGen(lexpr,rexpr, env);
        case TokenType::TOKEN_OP_SUB:
            return addExprGen(lexpr,rexpr, env);
        case TokenType::TOKEN_OP_MUL:
            return addExprGen(lexpr,rexpr, env);
        case TokenType::TOKEN_OP_DIV:
            return addExprGen(lexpr,rexpr, env);
        case TokenType::TOKEN_OP_AND:
            //TODO:
        case TokenType::TOKEN_OP_OR:
            //TODO:
        case TokenType::TOKEN_OP_EQ:
            //TODO:
        case TokenType::TOKEN_OP_GE:
            //TODO:
        case TokenType::TOKEN_OP_GT:
            //TODO:
        case TokenType::TOKEN_OP_LE:
            //TODO:
        case TokenType::TOKEN_OP_LT:
            //TODO:
        default:
            printf("[opExprGen] unkown op");
            assert(false);
            return nullptr;
    }
}

llvm::Value* CodeGen::addExprGen(ExpressionPtr lexpr, ExpressionPtr rexpr, std::list<Environment>& env){
    auto builder = getBuilder(env);
    auto lval = exprGen(lexpr, env);
    auto rval = exprGen(rexpr, env);
    if (!isDoubleType(lval) || !isDoubleType(rval)){
        assert(false&&"expr type no match double type");
        return nullptr;
    }
    if(rval->getType()->isPointerTy()){
        rval = builder.CreateLoad(llvm::Type::getDoubleTy(_context), rval);
    }
    if(lval->getType()->isPointerTy()){
        lval = builder.CreateLoad(llvm::Type::getDoubleTy(_context), lval);
    }
    auto val = builder.CreateFAdd(lval, rval);
    return val;
}

llvm::Value* CodeGen::numberExprGen(AstPtr expr, std::list<Environment>& env){
    NumberExpressionPtr numberExpr = std::dynamic_pointer_cast<NumberExpression>(expr);
    assert(numberExpr != nullptr);
    llvm::Value* value;
    if (numberExpr->_is_float) {
        value = llvm::ConstantFP::get(_context, llvm::APFloat(numberExpr->_number));
    } else {
        value = llvm::ConstantInt::get(llvm::Type::getInt64Ty(_context), long(numberExpr->_number));
    }
    return value;
}

llvm::Value* CodeGen::StringExprGen(AstPtr ast , std::list<Environment>& env) {
    auto builder = getBuilder(env);
    auto str_expr = std::dynamic_pointer_cast<StringExpression>(ast);
    assert(str_expr != nullptr);
    auto value = builder.CreateGlobalStringPtr(str_expr->_string);
    return value;
}


llvm::Value* CodeGen::identifierExprGen(AstPtr ast, std::list<Environment>& env) {
    auto id_expr = std::dynamic_pointer_cast<IdentifierExpression>(ast);
    assert(id_expr != nullptr);
    std::string id = id_expr->_identifier;
    auto found = env.front().declared_variable.find(id);
    if (found == env.front().declared_variable.end()) {
        printf("Can't find identifier:%s\n", id.c_str());
        exit(1);
        return nullptr;
    }
    return found->second;
}

llvm::Value* CodeGen::BoolExprGen(AstPtr ast, std::list<Environment>& env) {
    auto bool_expr = std::dynamic_pointer_cast<BoolExpression>(ast);
    assert(bool_expr != nullptr);
    auto value = llvm::ConstantInt::get(llvm::Type::getInt1Ty(_context), bool_expr->_value);
    return value;
}

llvm::Value* CodeGen::funcCallGen(AstPtr ast, std::list<Environment>& env) {
    auto builder = getBuilder(env);
    auto funcall_ast = std::dynamic_pointer_cast<FuncCallExpression>(ast);
    assert(funcall_ast);
    auto found = env.front().declared_prototype.end();
    for (auto& env_frame : env) {
        found = env_frame.declared_prototype.find(funcall_ast->_identifier);
        if (found != env_frame.declared_prototype.end()) {
            break;
        }
    }
    if (found == env.back().declared_prototype.end()) {
        printf("Can't find func:%s\n", funcall_ast->_identifier.c_str());
        exit(1);
    }

    auto func_proto = found->second;

    std::vector<llvm::Value*> args;
    for (auto arg_ast : funcall_ast->_parameters) {
        auto arg =  exprGen(arg_ast, env);
        if(arg->getType()->isPointerTy() && arg->getType() != llvm::Type::getInt8PtrTy(_context)){
            arg = builder.CreateLoad(arg->getType()->getPointerElementType(), arg);
        }
        assert(arg != nullptr);
        args.push_back(arg);
    }

    return builder.CreateCall(func_proto, llvm::makeArrayRef(args));
}

bool CodeGen::isDoubleType(llvm::Value* v){
    bool is_double = v->getType()->isDoubleTy();
    bool is_double_pt = v->getType()->isPointerTy() && v->getType()->getPointerElementType() == llvm::Type::getDoubleTy(_context);
    return is_double || is_double_pt;
}

}