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
//using namespace llvm;

static llvm::LLVMContext TheContext;
static llvm::IRBuilder<> Builder(TheContext);
static std::unique_ptr<llvm::Module> TheModule;
static std::map<std::string, llvm::AllocaInst *> NamedValues;
//static std::map<std::string, std::unique_ptr<llvm::PrototypeAST>> FunctionProtos;

static void InitializeModuleAndPassManager() {
  // Open a new module.
  TheModule = std::make_unique<llvm::Module>("my cool jit", TheContext);
}
void MainFuncCodegen();
int demo(){
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    InitializeModuleAndPassManager();

    auto TargetTriple = llvm::sys::getDefaultTargetTriple();
    std::string Error;
    auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple, Error);

    // Print an error and exit if we couldn't find the requested target.
    // This generally occurs if we've forgotten to initialise the
    // TargetRegistry or we have a bogus target triple.
    if (!Target) {
        llvm::errs() << Error;
        return 1;
    }
    auto CPU = "generic";
    auto Features = "";

    llvm::TargetOptions opt;
    auto RM = llvm::Optional<llvm::Reloc::Model>();
    auto TargetMachine = Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM);
    auto layout = TargetMachine->createDataLayout();
    TheModule->setDataLayout(layout);
    TheModule->setTargetTriple(TargetTriple);

    auto Filename = "output.o";
  std::error_code EC;
  llvm::raw_fd_ostream dest(Filename, EC, llvm::sys::fs::OF_None);
  
  if (EC) {
    llvm::errs() << "Could not open file: " << EC.message();
    return 1;
  }

  llvm::legacy::PassManager pass;
  auto FileType = llvm::TargetMachine::CGFT_ObjectFile;

  if (TargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType)) {
    llvm::errs() << "TheTargetMachine can't emit a file of this type";
    return 1;
  }
  
  MainFuncCodegen();

  pass.run(*TheModule);
  dest.flush();

  llvm::outs() << "Wrote " << Filename << "\n";

    int retcode = system("llc file.ll -filetype=obj -o file.o");
    if (retcode != 0) 
        return 1;
    retcode = system("ld -o test ./file.o ./output.o  -lSystem -macosx_version_min 10.14");
    if (retcode != 0) 
        return 1;

    TheModule->print(llvm::errs(), nullptr);

    return 0;
}

void buildPrint(){
    // llvm::Type* voidType[1];
    // voidType[0] = llvm::Type::getVoidTy(TheContext);
    // llvm::ArrayRef<llvm::Type*> voidTypeARef (voidType, 1);

    // llvm::FunctionType* signature = llvm::FunctionType::get(voidTypeARef, false);
    // llvm::Function* func = llvm::Function::Create(signature, llvm::Function::ExternalLinkage, "fun", TheModule);
    // TheExecutionEngine->addGlobalMapping(func, const_cast<void*>(fnPtr));
    // llvm::Function *FuncToCall= TheModule->getFunction("fun");
    // std::vector<llvm::Value*> Args; // This is empty since void parameters of function
    // llvm::Value *Result = Builder.CreateCall(FuncToCall, Args, "calltmp"); // Result is void
}

int main() {
    demo();
}

llvm::Function* PrototypeCodegen(){
    std::vector<llvm::Type *> Doubles(2/* Args.size()*/, llvm::Type::getDoubleTy(TheContext));
    llvm::FunctionType *FT =
        llvm::FunctionType::get(llvm::Type::getDoubleTy(TheContext), Doubles, false);

    llvm::Function *F =
        llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "Name", TheModule.get());

    // Set names for all arguments.
    unsigned Idx = 0;
    for (auto &Arg : F->args())
        Arg.setName(/* Args[Idx++]*/"arg name idx");

    return F;
}

/// CreateEntryBlockAlloca - Create an alloca instruction in the entry block of
/// the function.  This is used for mutable variables etc.
static llvm::AllocaInst *CreateEntryBlockAlloca(llvm::Function *TheFunction,
                                          llvm::StringRef VarName) {
  llvm::IRBuilder<> TmpB(&TheFunction->getEntryBlock(),
                   TheFunction->getEntryBlock().begin());
  return TmpB.CreateAlloca(llvm::Type::getDoubleTy(TheContext), nullptr, VarName);
}

llvm::Function* FunctionCodegen(){
    auto TheFunction = PrototypeCodegen();
    llvm::BasicBlock *BB = llvm::BasicBlock::Create(TheContext, "entry", TheFunction);
    Builder.SetInsertPoint(BB);

      NamedValues.clear();
  for (auto &Arg : TheFunction->args()) {
    // Create an alloca for this variable.
    llvm::AllocaInst *Alloca = CreateEntryBlockAlloca(TheFunction, Arg.getName());

    // Store the initial value into the alloca.
    Builder.CreateStore(&Arg, Alloca);

    // Add arguments to variable symbol table.
    NamedValues[std::string(Arg.getName())] = Alloca;
  }
    //Body = statementCodegen or expressionCodegen
  //if (llvm::Value *RetVal = Body->codegen()) {
    if (llvm::Value *RetVal = nullptr) {
    // Finish off the function.
    Builder.CreateRet(RetVal);

    // Validate the generated code, checking for consistency.
    verifyFunction(*TheFunction);

    return TheFunction;
  }
    // Error reading body, remove function.
  TheFunction->eraseFromParent();
    return nullptr;

}
void MainFuncCodegen(){
    // declar printf
    // define void main (void)
    //  call print(len, str)
    //   return 

    std::vector<llvm::Type *> void_param(0, llvm::Type::getVoidTy(TheContext));
    llvm::FunctionType *mockPrintFT =
        llvm::FunctionType::get(llvm::Type::getVoidTy(TheContext), void_param, false);

    llvm::Function *mockPrintF =
        llvm::Function::Create(mockPrintFT, llvm::Function::ExternalLinkage, "mock_print", TheModule.get());

    //std::vector<llvm::Type *> void_param(0, llvm::Type::getVoidTy(TheContext));
    llvm::FunctionType *FT =
        llvm::FunctionType::get(llvm::Type::getVoidTy(TheContext), void_param, false);

    llvm::Function *F =
        llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "main", TheModule.get());

    llvm::BasicBlock *BB = llvm::BasicBlock::Create(TheContext, "entry", F);
    Builder.SetInsertPoint(BB);
    llvm::IRBuilder<> blockbBuilder(BB);
    std::vector<llvm::Value*> emptyArgs;
    blockbBuilder.CreateCall(mockPrintF, llvm::makeArrayRef(emptyArgs));

    //auto RetVal = llvm::UndefValue::get(llvm::Type::getVoidTy(TheContext));
    auto RetVal = Builder.CreateRetVoid();

    //Builder.CreateRet(RetVal);

    // Validate the generated code, checking for consistency.
    llvm::verifyFunction(*F);

}
//Builder.CreateCall

// llvm::SMDiagnostic Err;
// unique_ptr<Module> Mod = getLazyIRFileModule("f.ll", Err, TheContext);
// llvm::EngineBuilder engineBuilder(std::move(Mod));

// clang++ CodeGenerator.cpp -o CodeGenerator -L/usr/local/opt/llvm/lib -Wl,-rpath,/usr/local/opt/llvm/lib `llvm-config --cxxflags --ldflags --system-libs --libs all` -std=c++17