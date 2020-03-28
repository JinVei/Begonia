#ifndef BEGONIA_STATEMENT_H
#define BEGONIA_STATEMENT_H
#include <vector>
#include <memory>
#include <list>
#include "expression.h"

namespace begonia
{
    enum class StatementType: uint8_t {
        IF_STATEMENT = 0,
        DECL_VAR_STATEMENT,
        DECL_FUNC_STATEMENT,
        ASSIGN_STATEMENT,
        WHILE_STATEMENT,
        RETURN_STATEMENT,
        CALL_FUNC_STATEMENT,
        UNKNOWN_STATEMENT,
    };

    struct Statement {
        virtual StatementType GetType() {
            return StatementType::UNKNOWN_STATEMENT;
        }
    };

    using StatementPtr = std::shared_ptr<Statement>;

    using StatementBlock = std::vector<StatementPtr>;

    struct IfBlock {
        StatementBlock      _block;
        ExpressionPtr       _cond;
    };
    struct IfStatement: public  Statement {
        std::list<IfBlock>  _if_blocks;
        StatementBlock      _else_block;

        IfStatement(std::list<IfBlock> if_block, StatementBlock else_block) {
            _if_blocks = if_block;
            _else_block = else_block;
        }

        StatementType GetType() override {
            return StatementType::IF_STATEMENT;
        }
    };
    using IfStatementPtr = std::shared_ptr<IfStatement>;

    struct DeclarVarStatement: public  Statement {
        std::string         _name;
        ExpressionPtr       _assign_value;

        DeclarVarStatement(std::string name, ExpressionPtr assign_value) {
            _name = name;
            _assign_value = assign_value;
        }
    
        StatementType GetType() override {
            return StatementType::DECL_VAR_STATEMENT;
        }
    };
    using DeclarVarStatementPtr = std::shared_ptr<DeclarVarStatement>;

    struct DeclarFuncStatement: public Statement {
        std::string	                        _name;
        std::list<DeclarVarStatementPtr>    _decl_vars;
        StatementBlock                      _block;

        DeclarFuncStatement(std::string name, std::list<DeclarVarStatementPtr> decl_vars, StatementBlock block) {
            _name = name;
            _decl_vars = decl_vars;
            _block = block;
        }

        StatementType GetType() override {
            return StatementType::DECL_FUNC_STATEMENT;
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

        StatementType GetType() override {
            return StatementType::ASSIGN_STATEMENT;
        }
    };
    using AssignStatementPtr = std::shared_ptr<AssignStatement>;

    struct WhileStatement: public Statement {
        ExpressionPtr      _condition;
        StatementBlock     _block;

        WhileStatement(ExpressionPtr condition, StatementBlock block) {
            _condition = condition;
            _block = block;
        }

        StatementType GetType() override {
            return StatementType::WHILE_STATEMENT;
        }
    };
    using WhileStatementPtr = std::shared_ptr<WhileStatement>;

    struct ReturnStatement: public Statement {
        std::vector<ExpressionPtr>  _ret_values;

        ReturnStatement(std::vector<ExpressionPtr>  ret_values) {
            _ret_values = ret_values;
        }

        StatementType GetType() override {
            return StatementType::RETURN_STATEMENT;
        }
    };
    using ReturnStatementPtr = std::shared_ptr<ReturnStatement>;

    struct FuncCallStatement: public Statement/* , FuncCallExpression*/ {
        std::string                 _identifier;
        std::vector<ExpressionPtr>  _parameters;

        FuncCallStatement(std::string identifier, std::vector<ExpressionPtr>  parameters) {
            _identifier = identifier;
            _parameters = parameters;
        }

        StatementType GetType() override {
            return StatementType::CALL_FUNC_STATEMENT;
        }
    };
    using FuncCallStatementPtr = std::shared_ptr<FuncCallStatement>;
}
#endif
