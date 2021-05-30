#ifndef BEGONIA_PARSER_H
#define BEGONIA_PARSER_H
// TODO: import, table, for
/*
block := {Statement}
Statement := IfStat
        | DeclarVarStat
        | DeclarFuncStat
        | AssignStat
        | ForStat
        | WhileStat
        | RetStat
        | exp
        | ;

IfStat          := if exp '{' block '}' [elif exp '{' block '}'] [else '{' block '}'] 
DeclarVarStat   := var identifier ['=' exp] ;
DeclarFuncStat  := func identifier '(' () | (exp [',' exp])  ')' '{' block '}' ;
AssignStat      := identifier '=' exp ;
WhileStat       := while exp '{' block '}' 
RetStat         := return | return exp ["," exp];
ExprStat        := exp;

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
#include <functional>

namespace begonia
{
    using OpExpPaser = std::function<ExpressionPtr ()>;
    using StatementParser = std::map<AstType, std::function<AstPtr (void)> >;

    class Parser {
    public:
        Parser(std::string source_file);
        void Parse();
        AstPtr      _ast;

    private:
        Lexer               _lexer;
        StatementParser     _statement_parsers;

    private:
        auto ParseStatement()       -> AstPtr;
        auto TryNextStatementType() -> AstType;
        auto ParseAssignStatement() -> AssignStatementPtr;

        auto ParseFuncallExpression()  -> FuncallExpressionPtr;
        auto ParseSemicolon()           -> AstPtr;
        auto ParseIfStatement()         -> IfStatementPtr;
        auto ParseCurlyBlock()          -> AstBlockPtr;
        auto ParseExpressionStatement() -> ExpressionPtr;
        auto ParseDeclareVarStatement()  -> DeclareVarStatementPtr;
        auto ParseDeclarVar()           -> DeclareVarStatementPtr;
        auto ParseDeclareFuncStatement() -> DeclareFuncStatementPtr;
        auto ParseMultipleExpression()  -> std::vector<ExpressionPtr>;
        auto ParseReturnStatement()     -> ReturnStatementPtr;
        auto ParseWhileStatement()      -> WhileStatementPtr;

        static void ParseError(Token token, std::string expected_word);
        void initStatementParser();

        auto ParseExpression()      -> ExpressionPtr;
        auto ParseExpressionL8()    -> ExpressionPtr;
        auto ParseExpressionL7()    -> ExpressionPtr;
        auto ParseExpressionL6()    -> ExpressionPtr;
        auto ParseExpressionL5()    -> ExpressionPtr;
        auto ParseExpressionL4()    -> ExpressionPtr;
        auto ParseExpressionL3()    -> ExpressionPtr;
        auto ParseExpressionL2()    -> ExpressionPtr;
        auto ParseExpressionL1()    -> ExpressionPtr;

        ExpressionPtr
        ParseOpExpression ( std::vector<TokenType>  accepted_token_type,
                            OpExpPaser              sub_exp_paeser);

    };
}
#endif
