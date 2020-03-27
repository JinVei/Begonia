#include "Parser.h"
#include "iostream"

namespace begonia
{
	Parser::Parser(std::string sourcePath): _lexer(sourcePath) {
	}

	void Parser::Parsing(){
		do {
			Token tryToken = _lexer.LookAhead(0);
			if (tryToken.val != TOKEN_VAL::TOKEN_SEP_EOF){
				StatementPtr stat = ParsingStatement();
				_ast.push_back(stat);
			} else {
				break;
			}
		} while(1);
	}

	void ParsingError(Token token, std::string expectedWord) {
		printf("[ParsingError]:\nParsing error at line:%ld\n",token.line);
		printf("want '%s', but have '%s'\n", expectedWord.c_str(), token.word.c_str());
		exit(1);
	}

	auto Parser::TryNextStatementType() -> StatementType {
		Token token = _lexer.LookAhead(0);
		Token token2 = _lexer.LookAhead(1);

		switch (token.val) {
		case TOKEN_VAL::TOKEN_KW_IF:
			return StatementType::IF_STAT;

		case TOKEN_VAL::TOKEN_KW_VAR:
			return StatementType::DEF_VAR_STAT;

		case TOKEN_VAL::TOKEN_KW_FUNC:
			return StatementType::DEF_FUNC_STAT;

		case TOKEN_VAL::TOKEN_IDENTIFIER:
			if (token2.val == TOKEN_VAL::TOKEN_OP_ASSIGN) {
				return StatementType::ASSIGN_STAT;
			}
			else if (token2.val == TOKEN_VAL::TOKEN_SEP_LPAREN) {
				return StatementType::CALL_FUNC_STAT;
			}
			else {
				return StatementType::UNKNOWN_STAT;
			}

		case TOKEN_VAL::TOKEN_KW_WHILE:
			return StatementType::WHILE_STAT;

		case TOKEN_VAL::TOKEN_KW_RETURN:
			return StatementType::RETURN_STAT;

		default:
			ParsingError(token, "UNKNOWN_STAT");
			return StatementType::UNKNOWN_STAT;

		}
	}

	auto Parser::ParsingStatement() -> StatementPtr {
		StatementPtr statement;
		switch (TryNextStatementType()) {
		case StatementType::IF_STAT :
			statement = ParsingIfStatement();
			_ast.push_back(statement);
			break;

		case StatementType::ASSIGN_STAT :
			statement = ParsingAssignStatement();
			_ast.push_back(statement);
			break;

		case StatementType::CALL_FUNC_STAT :
			statement = ParsingCallFuncStatement();
			_ast.push_back(statement);
			break;

		case StatementType::DEF_FUNC_STAT :
			statement = ParsingDefineFuncStatement();
			_ast.push_back(statement);
			break;

		case StatementType::DEF_VAR_STAT :
			statement = ParsingDefineVarStatement();
			_ast.push_back(statement);
			break;

		case StatementType::RETURN_STAT :
			statement = ParsingReturnStatement();
			_ast.push_back(statement);
			break;

		case StatementType::WHILE_STAT :
			statement = ParsingWhileStatement();
			_ast.push_back(statement);
			break;

		case StatementType::UNKNOWN_STAT :
			exit(1);
			break;
		}
		return statement;
	}

	auto Parser::ParsingCurlyBlock() -> StatementBlock {
		Token lCurlyToken = _lexer.GetNextToken();
		if (lCurlyToken.val != TOKEN_VAL::TOKEN_SEP_LCURLY) {
			ParsingError(lCurlyToken, "{");
			return StatementBlock();
		}
		StatementBlock block;
		Statement stat;
		do {
			Token tryToken = _lexer.LookAhead(0);
			if (tryToken.val == TOKEN_VAL::TOKEN_SEP_RCURLY) {
				break;
			}
			StatementPtr stat = ParsingStatement();
			block.push_back(stat);
		} while(1);

		Token rCurlyToken = _lexer.GetNextToken();
		if (rCurlyToken.val != TOKEN_VAL::TOKEN_SEP_RCURLY) {
			ParsingError(rCurlyToken, "}");
			return StatementBlock();
		}
		
		return block;
	}

	auto Parser::ParsingSemicolon() {
		Token SemiToken = _lexer.GetNextToken();

		if (SemiToken.val != TOKEN_VAL::TOKEN_SEP_SEMICOLON) {
			ParsingError(SemiToken, ";");
		}
	}

	auto Parser::ParsingWhileStatement() -> WhileStatementPtr {
		Token whileToken = _lexer.GetNextToken();
		if (whileToken.val != TOKEN_VAL::TOKEN_KW_WHILE) {
			ParsingError(whileToken, "return");
		}

		ExpressionPtr condExp = ParsingExpression();
		StatementBlock block = ParsingCurlyBlock();

		auto stat = new WhileStatement{
			._condition	= condExp,
			._block		= block,
		};
		return WhileStatementPtr(stat);
	}

	auto Parser::ParsingReturnStatement() -> ReturnStatementPtr {
		Token returnToken = _lexer.GetNextToken();
		if (returnToken.val != TOKEN_VAL::TOKEN_KW_RETURN) {
			ParsingError(returnToken, "return");
		}

		std::vector<ExpressionPtr> returnVal;
		returnVal = ParsingMultipleExpression();
		ParsingSemicolon();

		auto Stat = new ReturnStatement{._retValues = returnVal};
		return ReturnStatementPtr(Stat);
	}

	auto Parser::ParsingDefineFuncStatement() -> DefFuncStatementPtr {
		Token funcKWToken = _lexer.GetNextToken(); // func
		if (funcKWToken.val != TOKEN_VAL::TOKEN_KW_FUNC) {
			ParsingError(funcKWToken, "func");
			return DefFuncStatementPtr(nullptr);
		}

		Token identifierToken = _lexer.GetNextToken();
		if (identifierToken.val != TOKEN_VAL::TOKEN_IDENTIFIER) {
			ParsingError(identifierToken, "identifier");
			return DefFuncStatementPtr(nullptr);
		}

		Token lParenToken = _lexer.GetNextToken();
		if (lParenToken.val != TOKEN_VAL::TOKEN_SEP_LPAREN) {
			ParsingError(lParenToken, "(");
			return DefFuncStatementPtr(nullptr);
		}

		std::list<DefVarStatementPtr> defVars;
		bool continueParseDefVar = true;

		Token tryToken0 = _lexer.LookAhead(0);
		if (tryToken0.val == TOKEN_VAL::TOKEN_SEP_RPAREN) {
			_lexer.GetNextToken();
			continueParseDefVar = false;
		}

		while (continueParseDefVar){
			DefVarStatementPtr defVar = ParsingDefineVar();
			defVars.push_back(defVar);

			Token tryToken = _lexer.GetNextToken();
			if (tryToken.val == TOKEN_VAL::TOKEN_SEP_COMMA) {// ,
				continueParseDefVar = true;
			} else if (tryToken.val == TOKEN_VAL::TOKEN_SEP_RPAREN) { // )
				continueParseDefVar = false;
			} else {
				ParsingError(tryToken, ", )");
				return DefFuncStatementPtr(nullptr);
			}
		} 

		StatementBlock block = ParsingCurlyBlock();

		auto defFuncStat = new DefFuncStatement {
			._name 			= identifierToken.word,
			._declaredVars	= defVars,
			._codeBlock		= block,
		};

		return DefFuncStatementPtr(defFuncStat);
		
	}

	auto Parser::ParsingDefineVar() -> DefVarStatementPtr {
		Token varKw = _lexer.GetNextToken();
		Token varName = _lexer.GetNextToken();

		if (varKw.val != TOKEN_VAL::TOKEN_KW_VAR) {
			ParsingError(varKw, "var");
			return DefVarStatementPtr(nullptr);
		}

		if (varName.val != TOKEN_VAL::TOKEN_IDENTIFIER) {
			ParsingError(varName, "identifier");
			return DefVarStatementPtr(nullptr);
		}
		Token tryToken = _lexer.LookAhead(0);
		if (tryToken.val != TOKEN_VAL::TOKEN_OP_ASSIGN) {
			auto varDef = new DefVarStatement {
				._name = varName.word,
				._assignValue = nullptr,
			};
			return DefVarStatementPtr(varDef);
		}
		_lexer.GetNextToken(); // =
		ExpressionPtr exp = ParsingExpression();

		auto varDef = new DefVarStatement {
			//._type = varType.word,
			._name = varName.word,
			._assignValue = exp,
		};
		return DefVarStatementPtr(varDef);
	}

	auto Parser::ParsingDefineVarStatement() -> DefVarStatementPtr {
		DefVarStatementPtr stat = ParsingDefineVar();
		ParsingSemicolon();

		return stat;
	}

	auto Parser::ParsingCallFuncStatement() -> FuncCallStatementPtr {
		FuncCallExpressionPtr funcallExp = ParsingFuncCallExpression();
		ParsingSemicolon();
		return FuncCallStatementPtr(new FuncCallStatement{Statement{},*funcallExp});
	}
	
	auto Parser::ParsingAssignStatement() -> AssignStatementPtr {
		Token token0 = _lexer.GetNextToken();
		Token token1 = _lexer.GetNextToken();

		if (token0.val != TOKEN_VAL::TOKEN_IDENTIFIER) {
			ParsingError(token0, "identifier");
		}

		if (token1.val != TOKEN_VAL::TOKEN_OP_ASSIGN) {
			ParsingError(token1, "=");
		}

		ExpressionPtr exp = ParsingExpression();

		auto stat = new AssignStatement {
			._identifier = token0.word,
			._assignValue = exp
		};

		ParsingSemicolon();

		return AssignStatementPtr(stat);
	}

	auto Parser::ParsingIfStatement() -> IfStatementPtr {
		Token IfToken = _lexer.GetNextToken();
		if (IfToken.val != TOKEN_VAL::TOKEN_KW_IF) {
			ParsingError(IfToken, "if");
			return IfStatementPtr(nullptr);
		}

		std::list<IfBlock> ifBlocks;
		StatementBlock elseBlock;

		ExpressionPtr ifCondExp = ParsingExpression();

		StatementBlock block = ParsingCurlyBlock();
		ifBlocks.push_back(IfBlock{block, ifCondExp});

		bool continueParseElifBlock = true;
		do {
			Token tryToken = _lexer.LookAhead(0);
			if (tryToken.val == TOKEN_VAL::TOKEN_KW_ELSEIF) {
				Token elifToken = _lexer.GetNextToken();
				ExpressionPtr condExp = ParsingExpression();
				StatementBlock block = ParsingCurlyBlock();
				ifBlocks.push_back(IfBlock{block, condExp});

			} else if (tryToken.val == TOKEN_VAL::TOKEN_KW_ELSE) {
				Token elseToken = _lexer.GetNextToken();
				elseBlock = ParsingCurlyBlock();
				continueParseElifBlock = false;

			} else {
				continueParseElifBlock = false;
			}
		} while(continueParseElifBlock);

		auto ifStat = new IfStatement {
			._ifBlocks = ifBlocks,
			._elseBlock = elseBlock,
		};

		return IfStatementPtr(ifStat);
	}

	auto Parser::ParsingExpression() -> ExpressionPtr {

		ExpressionPtr exp = ParsingOpExpression({TOKEN_VAL::TOKEN_OP_OR, TOKEN_VAL::TOKEN_OP_AND}, std::bind(&Parser::ParsingExpressionL7, this));

		return exp;
	}


	auto Parser::ParsingOpExpression (
		std::vector<TOKEN_VAL> vAcceptedTokenType, 
		OpExpPaser subExpPaeser) 
		-> ExpressionPtr
	{
		ExpressionPtr lExp = subExpPaeser();

		ExpressionPtr exp = lExp;
		while (1) {
			Token tryToken = _lexer.LookAhead(0);
			bool isAccepted = false;
			for (auto acceptedTokenType : vAcceptedTokenType) {
				if (tryToken.val == acceptedTokenType) {
					isAccepted = true;
					break;
				}
			}

			if (isAccepted == false) {
				//return lExp;
				break;
			}

			Token opToken = _lexer.GetNextToken();
			ExpressionPtr rExp = subExpPaeser();
			// std::cout << opToken.word;
			auto opExp = new OperationExpresson {
				._operator = opToken.word,
				._LExp = exp,
				._RExp = rExp
			};
			exp = ExpressionPtr(opExp);
		}

		return exp;
	}

	auto Parser::ParsingExpressionL7() -> ExpressionPtr {
		return ParsingOpExpression({TOKEN_VAL::TOKEN_OP_AND}, std::bind(&Parser::ParsingExpressionL6, this));
	}

	auto Parser::ParsingExpressionL6() -> ExpressionPtr {
		return ParsingOpExpression({
				TOKEN_VAL::TOKEN_OP_LT, // <
				TOKEN_VAL::TOKEN_OP_LE, // <=
				TOKEN_VAL::TOKEN_OP_GT, // >
				TOKEN_VAL::TOKEN_OP_GE, // >=
				TOKEN_VAL::TOKEN_OP_EQ, // ==
				TOKEN_VAL::TOKEN_OP_NEQ, // !=
			},
			std::bind(&Parser::ParsingExpressionL5, this)
		);
	}

	auto Parser::ParsingExpressionL5() -> ExpressionPtr {
		return ParsingOpExpression({
				TOKEN_VAL::TOKEN_OP_ADD, // +
				TOKEN_VAL::TOKEN_OP_SUB, // -
			},
			std::bind(&Parser::ParsingExpressionL4, this)
		);
	}

	auto Parser::ParsingExpressionL4() -> ExpressionPtr {
		return ParsingOpExpression({
				TOKEN_VAL::TOKEN_OP_MUL, // *
				TOKEN_VAL::TOKEN_OP_DIV, // /
				TOKEN_VAL::TOKEN_OP_MOD, // %
			},
			std::bind(&Parser::ParsingExpressionL3, this)
		);
	}

	auto Parser::ParsingExpressionL3() -> ExpressionPtr {
		return ParsingOpExpression({
				TOKEN_VAL::TOKEN_OP_BOR, // |
				TOKEN_VAL::TOKEN_OP_BAND, // &
				TOKEN_VAL::TOKEN_OP_XOR, // ^
			},
			std::bind(&Parser::ParsingExpressionL2, this)
		);
	}

	auto Parser::ParsingExpressionL2() -> ExpressionPtr {
		Token tryToken = _lexer.LookAhead(0);
		// !
		if (tryToken.val == TOKEN_VAL::TOKEN_OP_NEG) {
			Token opToken = _lexer.GetNextToken();

			ExpressionPtr rExp = ParsingExpressionL1();

			auto opExp = new OperationExpresson {
				._operator = opToken.word,
				._LExp = nullptr,
				._RExp = rExp
			};
			return OperationExpressonPtr(opExp);
		}

		return ParsingExpressionL1();
	}

	auto Parser::ParsingExpressionL1() -> ExpressionPtr {
		Token tryToken = _lexer.LookAhead(0);
		Token tryToken1;
		Token token;
		switch (tryToken.val)
		{
		case TOKEN_VAL::TOKEN_SEP_LPAREN: // ()
			_lexer.GetNextToken();
			ParsingExpression();
			token = _lexer.GetNextToken();
			if (token.val != TOKEN_VAL::TOKEN_SEP_RPAREN) {
				ParsingError(tryToken,")");
			}
			break;

		case TOKEN_VAL::TOKEN_KW_FALSE:
			token = _lexer.GetNextToken();
			return BoolExpressionPtr(new BoolExpression{._value = false});
			break;

		case TOKEN_VAL::TOKEN_KW_TRUE:
			token = _lexer.GetNextToken();
			return BoolExpressionPtr(new BoolExpression{._value = true});
			break;
		
		case TOKEN_VAL::TOKEN_KW_NIL:
			token = _lexer.GetNextToken();
			return NilExpressionPtr(new NilExpression());
			break;

		case TOKEN_VAL::TOKEN_NUMBER:
			token = _lexer.GetNextToken();
			return NumberExpressionPtr(new NumberExpression{._number = std::stod(token.word)});
			break;

		case TOKEN_VAL::TOKEN_STRING:
			token = _lexer.GetNextToken();
			return StringExpressionPtr(new StringExpression{._string = token.word});
			break;

		case TOKEN_VAL::TOKEN_IDENTIFIER:
			tryToken1 = _lexer.LookAhead(1);
			if (tryToken1.val == TOKEN_VAL::TOKEN_SEP_LPAREN) {
				return ParsingFuncCallExpression();
			} else {
				token = _lexer.GetNextToken();
				return IdentifierExpressionPtr(new IdentifierExpression{._identifier = token.word});
			}
			break;

		default:
			ParsingError(tryToken,"(, false, true, nil, number, string, identifier");
			return ExpressionPtr(nullptr);
			break;
		}
		return ExpressionPtr(nullptr);
	}
	
	auto Parser::ParsingMultipleExpression() -> std::vector<ExpressionPtr>{
		std::vector<ExpressionPtr> parameters;
		bool continueParseExp = true;
		Token tryToken;
		tryToken = _lexer.LookAhead(0);
		if (tryToken.val == TOKEN_VAL::TOKEN_SEP_RPAREN) { // )
			return parameters;
		}

		do {
			ExpressionPtr exp = ParsingExpression();
			parameters.push_back(exp);

			tryToken = _lexer.LookAhead(0);
			if (tryToken.val != TOKEN_VAL::TOKEN_SEP_COMMA) {// ,
				continueParseExp = false;
			} else {
				_lexer.GetNextToken();
			}
		} while (continueParseExp);

		return parameters;
	}

	auto Parser::ParsingFuncCallExpression() -> FuncCallExpressionPtr {
		Token identifierToken = _lexer.GetNextToken();
		Token lParenToken = _lexer.GetNextToken();
		if (identifierToken.val != TOKEN_VAL::TOKEN_IDENTIFIER) {
			ParsingError(identifierToken, "identifier");
			return FuncCallExpressionPtr(nullptr);
		}
		if (lParenToken.val != TOKEN_VAL::TOKEN_SEP_LPAREN) {
			ParsingError(lParenToken, "(");
			return FuncCallExpressionPtr(nullptr);
		}

		std::vector<ExpressionPtr> parameters;
		parameters = ParsingMultipleExpression();

		Token rParen = _lexer.GetNextToken();
 		if (rParen.val != TOKEN_VAL::TOKEN_SEP_RPAREN) { // )
			ParsingError(rParen, ")");
			return FuncCallExpressionPtr(nullptr);
		}
		auto funcallExp = new FuncCallExpression {
			._identifier = identifierToken.word,
			._parameters = parameters
		};

		return FuncCallExpressionPtr(funcallExp);
		
	}
}
