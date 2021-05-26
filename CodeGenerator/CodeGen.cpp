
#include "Parser.h"
#include "Expression.h"
#include "CodeGen.h"

#include <memory>

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
        {AstType::IfStatement, std::bind(&CodeGen::ifStatementGen, this, std::placeholders::_1, std::placeholders::_2)},
        {AstType::RetStatement, std::bind(&CodeGen::returnGen, this, std::placeholders::_1, std::placeholders::_2)},
        {AstType::Expr, std::bind(&CodeGen::exprGen, this, std::placeholders::_1, std::placeholders::_2)},
        {AstType::FuncallExpr, std::bind(&CodeGen::exprGen, this, std::placeholders::_1, std::placeholders::_2)},
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

    _module =  std::make_unique<llvm::Module>(_module_name.c_str(), _context);

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

  return 0;

}

int CodeGen::generate(AstPtr ast ) {
    std::error_code EC;
    llvm::raw_fd_ostream out_dest(_out_filename + ".o", EC, llvm::sys::fs::OF_None);
    if (EC) {
        llvm::errs() << "Could not open file: " << EC.message();
        return 1;
    }

    auto FileType = llvm::CGFT_ObjectFile;

    llvm::legacy::PassManager           pass;
    if (_target_machine->addPassesToEmitFile(pass, out_dest, nullptr, FileType)) {
        llvm::errs() << "TheTargetMachine can't emit a file of this type";
        return 1;
    }

    std::list<Environment> env;
    Environment e;
    llvm::BasicBlock *block = llvm::BasicBlock::Create(_context, "_main");
    e.block = block;;
    env.push_back(e);

    blockGen(ast, env);

    _module->print(llvm::errs(), nullptr);

    llvm::raw_ostream &output = llvm::errs();

    if (llvm::verifyModule(*_module.get(), &output)) {
        assert(false && "verifyModule failed");
    }
    pass.run(*_module);
    out_dest.flush();

    std::string ld_cmd = "ld -o " + _out_filename + " ./" + _out_filename + ".o " + " -lSystem -macosx_version_min 10.14";

    int retcode = system(ld_cmd.c_str());
    if (retcode != 0) 
        return 1;

    return 0;
}

}