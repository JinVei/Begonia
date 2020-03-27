#ifndef BEGONIA_STATEMENT_H
#define BEGONIA_STATEMENT_H
#include <vector>
#include <memory>
#include <list>
#include "Expression.h"

namespace begonia
{
    struct Statement {
    };
    using StatementPtr = std::shared_ptr<Statement>;

    using StatementBlock = std::vector<StatementPtr>;
    // struct StatementBlock {
    // 	std::vector<StatementPtr> 	_vStatements;
    // };
    struct IfBlock {
        StatementBlock 				_block;
        ExpressionPtr               _cond;
    };
    struct IfStatement: public  Statement {
        std::list<IfBlock>       	_ifBlocks;
        StatementBlock 				_elseBlock;
    };
    using IfStatementPtr = std::shared_ptr<IfStatement>;

    struct DefVarStatement: public  Statement {
        std::string					_name;
        ExpressionPtr				_assignValue;
    };
    using DefVarStatementPtr = std::shared_ptr<DefVarStatement>;

    struct DefFuncStatement: public Statement {
        std::string					_name;
        std::list<DefVarStatementPtr> _declaredVars;
        StatementBlock				_codeBlock;
    };
    using DefFuncStatementPtr = std::shared_ptr<DefFuncStatement>;

    struct AssignStatement: public Statement {
        std::string					_identifier;
        ExpressionPtr				_assignValue;
    };
    using AssignStatementPtr = std::shared_ptr<AssignStatement>;

    struct WhileStatement: public Statement {
        ExpressionPtr				_condition;
        StatementBlock				_block;
    };
    using WhileStatementPtr = std::shared_ptr<WhileStatement>;

    struct ReturnStatement: public Statement {
        std::vector<ExpressionPtr>	_retValues;
    };
    using ReturnStatementPtr = std::shared_ptr<ReturnStatement>;

    struct FuncCallStatement: public Statement, FuncCallExpression {
    };
    using FuncCallStatementPtr = std::shared_ptr<FuncCallStatement>;
}
#endif
