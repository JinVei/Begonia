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

IfStat := if exp '{' block '}' [elif exp '{' block '}'] [else '{' block '}'] 

DeclarVarStat := 'var' identifier ['=' exp] ;

DeclarFuncStat := func identifier '(' [exp ','] exp  ')' '{' block '}' ;

AssignStat := identifier '=' exp ;

WhileStat := while exp '{' block '}' 

RetStat := return [exp [,]] ;

exp0:
nil false true number string identifier
exp1:
()
exp2:
!
exp3:
|
&
^
exp4:
*
/
%
exp5:
+
-
exp6:
<
>
>=
<=
==
exp7:
&&
exp8:
||


exp8 := exp7 {('||') exp7}
exp7 := exp6 {(&&) exp6}
exp6 := exp5 { ('<='|'>='|'=='|'<'|'>') exp5}
exp5 := exp4 { ('+'|'-') exp4}
exp4 := exp3 {('*'|'/'|'%') exp3}
exp3 := exp2 {('|' | '&' | '^') exp2}
exp2 := !exp1 | exp1
exp1 := '(' exp8 ')' | exp0
exp0 := nil | false | true | number | string | identifier | funcallStat
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
	private:
		Lexer _lexer;

		StatementBlock _ast;

		auto ParsingStatement() -> StatementPtr;
		auto TryNextStatementType() -> StatementType;
		auto ParsingAssignStatement() -> AssignStatementPtr;
		auto ParsingExpression() -> ExpressionPtr;
		auto ParsingExpressionL8() -> ExpressionPtr;
		auto ParsingExpressionL7() -> ExpressionPtr;
		auto ParsingExpressionL6() -> ExpressionPtr;
		auto ParsingExpressionL5() -> ExpressionPtr;
		auto ParsingExpressionL4() -> ExpressionPtr;
		auto ParsingExpressionL3() -> ExpressionPtr;
		auto ParsingExpressionL2() -> ExpressionPtr;
		auto ParsingExpressionL1() -> ExpressionPtr;
		// auto ParsingExpressionL0() -> ExpressionPtr;
		auto ParsingOpExpression (std::vector<TokenType> acceptedTokenType,  OpExpPaser subExpPaeser) -> ExpressionPtr;
		auto ParsingFuncCallExpression() -> FuncCallExpressionPtr;
		auto ParsingSemicolon();
		auto ParsingIfStatement() -> IfStatementPtr;
		auto ParsingCurlyBlock() -> StatementBlock;
		auto ParsingCallFuncStatement() -> FuncCallStatementPtr;
		auto ParsingDefineVarStatement() -> DefVarStatementPtr;
		auto ParsingDefineVar() -> DefVarStatementPtr;
		auto ParsingDefineFuncStatement() -> DefFuncStatementPtr;
		auto ParsingMultipleExpression() -> std::vector<ExpressionPtr>;
		auto ParsingReturnStatement() -> ReturnStatementPtr;
		auto ParsingWhileStatement() -> WhileStatementPtr;
	public:
		void Parsing();
	};

}
#endif
