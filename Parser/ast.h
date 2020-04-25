#ifndef BEGONIA_AST_H
#define BEGONIA_AST_H
#include <cstdint>
#include <memory>

namespace begonia {

#define GENERATE_ENUM(X) X,

#define ALL_AST_TYPES(FUNCTION)     \
    FUNCTION(Unknown)               \
    FUNCTION(Block)                 \
    FUNCTION(IfStatement)           \
    FUNCTION(AssignStatement)       \
    FUNCTION(DeclarVarStatement)    \
    FUNCTION(DeclarFuncStatement)   \
    FUNCTION(WhileStatement)        \
    FUNCTION(RetStatement)          \
    FUNCTION(Expr)                  \
    FUNCTION(FuncCallExpr)          \
    FUNCTION(OpExpr)                \
    FUNCTION(BoolExpr)              \
    FUNCTION(NilExp)                \
    FUNCTION(NumberExpr)            \
    FUNCTION(StringExpr)            \
    FUNCTION(IdentifierExpr)        \
    FUNCTION(Semicolon)             \

enum class AstType: uint8_t{
    ALL_AST_TYPES(GENERATE_ENUM)
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