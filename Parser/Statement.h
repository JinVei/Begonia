#ifndef BEGONIA_STATEMENT_H
#define BEGONIA_STATEMENT_H
#include "expression.h"
#include "ast.h"

#include <vector>
#include <memory>
#include <list>

namespace begonia
{
    struct Statement: virtual public AST {
    };

    using StatementPtr = std::shared_ptr<Statement>;

    struct AstBlock: virtual public AST, public std::vector<AstPtr> {
        AstType GetType() override {
            return AstType::Block;
        }
    };
    using AstBlockPtr = std::shared_ptr<AstBlock>;

    struct IfBlock {
        AstBlockPtr         _block;
        ExpressionPtr       _cond;
    };
    struct IfStatement: public  Statement {
        std::list<IfBlock>  _if_blocks;
        AstBlockPtr            _else_block;

        IfStatement(std::list<IfBlock> if_block, AstBlockPtr else_block) {
            _if_blocks = if_block;
            _else_block = else_block;
        }

        AstType GetType() override {
            return AstType::IfStatement;
        }
    };
    using IfStatementPtr = std::shared_ptr<IfStatement>;

    struct DeclarVarStatement: public  Statement {
        std::string         _name;
        std::string         _type;
        ExpressionPtr       _assign_value;

        DeclarVarStatement(std::string name, std::string type, ExpressionPtr assign_value) {
            _name = name;
            _type = type;
            _assign_value = assign_value;
        }
    
        AstType GetType() override {
            return AstType::DeclarVarStatement;
        }
    };
    using DeclarVarStatementPtr = std::shared_ptr<DeclarVarStatement>;

    struct DeclarFuncStatement: public Statement {
        std::string	                        _name;
        std::list<DeclarVarStatementPtr>    _decl_vars;
        std::string	                        _ret_type;
        AstBlockPtr                         _block;

        DeclarFuncStatement(std::string name, std::list<DeclarVarStatementPtr> decl_vars, std::string ret_type, AstBlockPtr  block) {
            _name = name;
            _decl_vars = decl_vars;
            _ret_type = ret_type;
            _block = block;
        }

        AstType GetType() override {
            return AstType::DeclarFuncStatement;
        }
    };
    using DeclarFuncStatementPtr = std::shared_ptr<DeclarFuncStatement>;

    struct AssignStatement: public Statement {
        std::string        _identifier;
        ExpressionPtr      _assign_value;

        AssignStatement(std::string identifier, ExpressionPtr assign_value) {
            _identifier = identifier;
            _assign_value = assign_value;
        }

        AstType GetType() override {
            return AstType::AssignStatement;
        }
    };
    using AssignStatementPtr = std::shared_ptr<AssignStatement>;

    struct WhileStatement: public Statement {
        ExpressionPtr      _condition;
        AstBlockPtr        _block;

        WhileStatement(ExpressionPtr condition, AstBlockPtr block) {
            _condition = condition;
            _block = block;
        }

        AstType GetType() override {
            return AstType::WhileStatement;
        }
    };
    using WhileStatementPtr = std::shared_ptr<WhileStatement>;

    struct ReturnStatement: public Statement {
        std::vector<ExpressionPtr>  _ret_values;

        ReturnStatement(std::vector<ExpressionPtr>  ret_values) {
            _ret_values = ret_values;
        }

        AstType GetType() override {
            return AstType::RetStatement;
        }
    };
    using ReturnStatementPtr = std::shared_ptr<ReturnStatement>;
}
#endif
