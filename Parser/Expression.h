#ifndef BEGONIA_EXPRESSION_H
#define BEGONIA_EXPRESSION_H
#include "AST.h"

#include <memory>
#include <vector>

namespace begonia
{
    struct Expression: virtual public AST {
    };
    using ExpressionPtr = std::shared_ptr<Expression>;

    struct OperationExpresson: public Expression {
        //std::string     _operator;
        TokenType       _op;
        ExpressionPtr   _lexp;
        ExpressionPtr   _rexp;
        OperationExpresson(TokenType op, ExpressionPtr lexp, ExpressionPtr rexp) {
            _op = op;
            _lexp = lexp;
            _rexp = rexp;
            _type = AstType::OpExpr;
        }
    };
    using OperationExpressonPtr = std::shared_ptr<OperationExpresson>;

    struct BoolExpression: public Expression {
        bool    _value;
        BoolExpression(bool value){
            _value = value;
            _type = AstType::BoolExpr;
        }
    };
    using BoolExpressionPtr = std::shared_ptr<BoolExpression>;

    struct NilExpression: public Expression {
        NilExpression(){
            _type = AstType::NilExp;
        }
    };
    using NilExpressionPtr = std::shared_ptr<NilExpression>;

    struct NumberExpression: public Expression {
        double  _number;
        bool    _is_float;
        NumberExpression(double number, bool is_float){
            _number = number;
            _is_float = is_float;
            _type = AstType::NumberExpr;
        }
    };
    using NumberExpressionPtr = std::shared_ptr<NumberExpression>;

    struct StringExpression: public Expression {
        std::string _string;
        StringExpression(std::string string){
            _string = string;
            _type = AstType::StringExpr;
        }
    };
    using StringExpressionPtr = std::shared_ptr<StringExpression>;

    struct IdentifierExpression: public Expression {
        std::string _identifier;
        IdentifierExpression(std::string identifier){
            _identifier = identifier;
            _type = AstType::IdentifierExpr;
        }
    };
    using IdentifierExpressionPtr = std::shared_ptr<IdentifierExpression>;

    struct FuncCallExpression: public Expression {
        std::string                 _identifier;
        std::vector<ExpressionPtr>  _parameters;
        FuncCallExpression(std::string identifier, std::vector<ExpressionPtr>  parameters){
            _identifier = identifier;
            _parameters = parameters;
            _type = AstType::FuncallExpr;
        }
    };
    using FuncCallExpressionPtr = std::shared_ptr<FuncCallExpression>;

}
#endif
