#ifndef BEGONIA_AST_H
#define BEGONIA_AST_H
#include <cstdint>
#include <memory>

namespace begonia {

enum class AstType: uint8_t {
    Unknown,
    Block,   
    IfStatement,
    AssignStatement,
    DeclareVarStatement,
    DeclareFuncStatement,
    WhileStatement,
    RetStatement,
    Expr,
    FuncallExpr,
    OpExpr,
    BoolExpr,
    NilExp,
    NumberExpr,
    StringExpr,
    IdentifierExpr,
    Semicolon
};
struct AST {
    AstType _type;
    AST(){
        _type = AstType::Unknown;
    }
    virtual AstType GetType(){
        return _type;
    }
};
using AstPtr = std::shared_ptr<AST>;

} // begonia
#endif
