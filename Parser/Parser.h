#ifndef BEGONIA_PARSER_H
#define BEGONIA_PARSER_H

/*
block := {Stat | CallFuncExp}
stat :=   [IfStat]
        | [DeclarVarStat]
        | [DeclarFuncStat]
        | [AssignStat]
        | [ForStat]
        | [WhileStat]
        | [RetStat]
        | CallFuncStat

IfStat          := if exp '{' block '}' [elif exp '{' block '}'] [else '{' block '}'] 
DeclarVarStat   := var identifier ['=' exp] ;
DeclarFuncStat  := func identifier '(' () | (exp [',' exp])  ')' '{' block '}' ;
AssignStat      := identifier '=' exp ;
WhileStat       := while exp '{' block '}' 
RetStat         := return | return exp ["," exp];

exp  := exp7 {('||') exp7}
exp7 := exp6 {(&&) exp6}
exp6 := exp5 { ('<='|'>='|'=='|'<'|'>'|'!=') exp5}
exp5 := exp4 { ('+'|'-') exp4}
exp4 := exp3 {('*'|'/'|'%') exp3}
exp3 := exp2 {('|' | '&' | '^') exp2}
exp2 := !exp1 | exp1
exp1 :=  '(' exp8 ')' | nil | false | true | number | string | identifier | funcallStat
*/

#include "Lexer.h"
#include "Statement.h"
#include "Expression.h"

namespace begonia
{
    enum class StatementType: uint8_t {
        IF_STAT = 0,
        DEF_VAR_STAT,
        DEF_FUNC_STAT,
        ASSIGN_STAT,
        WHILE_STAT,
        RETURN_STAT,
        CALL_FUNC_STAT,
        UNKNOWN_STAT,
    };

    using OpExpPaser = std::function<ExpressionPtr ()>;
    class Parser {
    public:
        Parser(std::string sourcePath);
        void Parse();
    private:
        Lexer _lexer;

        StatementBlock _ast;

        auto ParseStatement()       -> StatementPtr;
        auto TryNextStatementType() -> StatementType;
        auto ParseAssignStatement() -> AssignStatementPtr;
        auto ParseExpression()      -> ExpressionPtr;
        auto ParseExpressionL8()    -> ExpressionPtr;
        auto ParseExpressionL7()    -> ExpressionPtr;
        auto ParseExpressionL6()    -> ExpressionPtr;
        auto ParseExpressionL5()    -> ExpressionPtr;
        auto ParseExpressionL4()    -> ExpressionPtr;
        auto ParseExpressionL3()    -> ExpressionPtr;
        auto ParseExpressionL2()    -> ExpressionPtr;
        auto ParseExpressionL1()    -> ExpressionPtr;
        // auto ParseExpressionL0() -> ExpressionPtr;
        ExpressionPtr
        ParseOpExpression ( std::vector<TokenType>  acceptedTokenType,
                            OpExpPaser              subExpPaeser);

        auto ParseFuncCallExpression()  -> FuncCallExpressionPtr;
        auto ParseSemicolon();
        auto ParseIfStatement()         -> IfStatementPtr;
        auto ParseCurlyBlock()          -> StatementBlock;
        auto ParseCallFuncStatement()   -> FuncCallStatementPtr;
        auto ParseDefineVarStatement()  -> DefVarStatementPtr;
        auto ParseDefineVar()           -> DefVarStatementPtr;
        auto ParseDefineFuncStatement() -> DefFuncStatementPtr;
        auto ParseMultipleExpression()  -> std::vector<ExpressionPtr>;
        auto ParseReturnStatement()     -> ReturnStatementPtr;
        auto ParseWhileStatement()      -> WhileStatementPtr;
    };

}
#endif
