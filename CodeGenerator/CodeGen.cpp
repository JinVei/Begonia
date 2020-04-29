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

using namespace llvm;

namespace begonia {

CodeGen::CodeGen(): _builder(_context) {
    _basic_variable_type = {
        {"string",   ValueType::String},
        {"int",      ValueType::Int},
        {"double",   ValueType::Double},
        {"bool",     ValueType::Bool},
        {"void",     ValueType::Void},
    };
    _generator = {
        {AstType::AssignStatement, std::bind(&CodeGen::assignGen, this, std::placeholders::_1, std::placeholders::_2)},
        {AstType::DeclareFuncStatement, std::bind(&CodeGen::declareProtoGen, this, std::placeholders::_1, std::placeholders::_2)},
        {AstType::DeclareVarStatement, std::bind(&CodeGen::declareVarGen, this, std::placeholders::_1, std::placeholders::_2)},
        {AstType::IfStatement, std::bind(&CodeGen::ifBlockGen, this, std::placeholders::_1, std::placeholders::_2)},
        {AstType::RetStatement, std::bind(&CodeGen::returnGen, this, std::placeholders::_1, std::placeholders::_2)},
        {AstType::Expr, std::bind(&CodeGen::exprGen, this, std::placeholders::_1, std::placeholders::_2)},
        {AstType::FuncCallExpr, std::bind(&CodeGen::exprGen, this, std::placeholders::_1, std::placeholders::_2)},
        {AstType::OpExpr, std::bind(&CodeGen::exprGen, this, std::placeholders::_1, std::placeholders::_2)},
        {AstType::BoolExpr, std::bind(&CodeGen::exprGen, this, std::placeholders::_1, std::placeholders::_2)},
        {AstType::NilExp, std::bind(&CodeGen::exprGen, this, std::placeholders::_1, std::placeholders::_2)},
        {AstType::NumberExpr, std::bind(&CodeGen::exprGen, this, std::placeholders::_1, std::placeholders::_2)},
        {AstType::StringExpr, std::bind(&CodeGen::exprGen, this, std::placeholders::_1, std::placeholders::_2)},
        {AstType::IdentifierExpr, std::bind(&CodeGen::exprGen, this, std::placeholders::_1, std::placeholders::_2)},
    };
}

int CodeGen::initialize(){
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    _module =  make_unique<llvm::Module>(_module_name.c_str(), _context);

    auto TargetTriple = llvm::sys::getDefaultTargetTriple();
    std::string Error;
    auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple, Error);

    if (!Target) {
        llvm::errs() << Error;
        return 1;
    }
    auto CPU = "generic";
    auto Features = "";

    llvm::TargetOptions opt;
    auto RM = llvm::Optional<llvm::Reloc::Model>();
    _target_machine = Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM);
    auto layout = _target_machine->createDataLayout();
    _module->setDataLayout(layout);
    _module->setTargetTriple(TargetTriple);


    //llvm::legacy::PassManager pass;
  
  return 0;

}

int CodeGen::generate(AstPtr ast ) {
    std::error_code EC;
    llvm::raw_fd_ostream out_dest(_out_filename + ".o", EC, llvm::sys::fs::OF_None);
    if (EC) {
        llvm::errs() << "Could not open file: " << EC.message();
        return 1;
    }

    auto FileType = llvm::TargetMachine::CGFT_ObjectFile;

    llvm::legacy::PassManager           pass;
    if (_target_machine->addPassesToEmitFile(pass, out_dest, nullptr, FileType)) {
        llvm::errs() << "TheTargetMachine can't emit a file of this type";
        return 1;
    }

    Environment env;
    env.block = nullptr;
    blockGen(ast, {env});

    _module->print(llvm::errs(), nullptr);

    pass.run(*_module);
    out_dest.flush();

    std::string ld_cmd = "ld -o " + _out_filename + " ./" + _out_filename + ".o " + " -lSystem -macosx_version_min 10.14";

    int retcode = system(ld_cmd.c_str());
    if (retcode != 0) 
        return 1;

    return 0;
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
            return funcallGen(ast, env);
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
            break;
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
llvm::IRBuilder<> CodeGen::getBuilder(std::list<Environment>& env){
    auto block = env.front().block;
    if ( block == nullptr) {
        return _builder;
    } else {
        return llvm::IRBuilder<>(block);
    }

}

bool CodeGen::isDoubleType(llvm::Value* v){
    bool is_double = v->getType()->isDoubleTy();
    bool is_double_pt = v->getType()->isPointerTy() && v->getType()->getPointerElementType() == llvm::Type::getDoubleTy(_context);
    return is_double || is_double_pt;
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

llvm::Value* CodeGen::funcallGen(AstPtr ast, std::list<Environment>& env) {
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

llvm::Value* CodeGen::StringExprGen(AstPtr ast , std::list<Environment>& env) {
    auto builder = getBuilder(env);
    auto str_expr = std::dynamic_pointer_cast<StringExpression>(ast);
    assert(str_expr != nullptr);
    auto value = builder.CreateGlobalStringPtr(str_expr->_string);
    return value;
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

llvm::Value* CodeGen::ifBlockGen(AstPtr ast, std::list<Environment>& env) {
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

} //begonia

//clang++ CodeGen.cpp -o CodeGen.o -L/usr/local/opt/llvm/lib -Wl,-rpath,/usr/local/opt/llvm/lib `llvm-config --cxxflags --ldflags --system-libs --libs all` -std=c++2a -I../parser/ -I../lexer/