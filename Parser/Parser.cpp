#include "Parser.h"
#include "iostream"

namespace begonia
{
    Parser::Parser(std::string sourcePath): _lexer(sourcePath) {
    }

    void Parser::Parse(){
        do {
            Token tryToken = _lexer.LookAhead(0);
            if (tryToken.val != TokenType::TOKEN_SEP_EOF){
                StatementPtr stat = ParseStatement();
                _ast.push_back(stat);
            } else {
                break;
            }
        } while(1);
    }

    void ParseError(Token token, std::string expectedWord) {
        printf("[ParseError]:\nParse error at line:%ld\n",token.line);
        printf("want '%s', but have '%s'\n", expectedWord.c_str(), token.word.c_str());
        exit(1);
    }

    auto Parser::TryNextStatementType() -> StatementType {
        Token token = _lexer.LookAhead(0);
        Token token2 = _lexer.LookAhead(1);

        switch (token.val) {
        case TokenType::TOKEN_KW_IF:
            return StatementType::IF_STAT;

        case TokenType::TOKEN_KW_VAR:
            return StatementType::DEF_VAR_STAT;

        case TokenType::TOKEN_KW_FUNC:
            return StatementType::DEF_FUNC_STAT;

        case TokenType::TOKEN_IDENTIFIER:
            if (token2.val == TokenType::TOKEN_OP_ASSIGN) {
                return StatementType::ASSIGN_STAT;
            }
            else if (token2.val == TokenType::TOKEN_SEP_LPAREN) {
                return StatementType::CALL_FUNC_STAT;
            }
            else {
                return StatementType::UNKNOWN_STAT;
            }

        case TokenType::TOKEN_KW_WHILE:
            return StatementType::WHILE_STAT;

        case TokenType::TOKEN_KW_RETURN:
            return StatementType::RETURN_STAT;

        default:
            ParseError(token, "UNKNOWN_STAT");
            return StatementType::UNKNOWN_STAT;

        }
    }

    auto Parser::ParseStatement() -> StatementPtr {
        StatementPtr statement;
        switch (TryNextStatementType()) {
        case StatementType::IF_STAT :
            statement = ParseIfStatement();
            _ast.push_back(statement);
            break;

        case StatementType::ASSIGN_STAT :
            statement = ParseAssignStatement();
            _ast.push_back(statement);
            break;

        case StatementType::CALL_FUNC_STAT :
            statement = ParseCallFuncStatement();
            _ast.push_back(statement);
            break;

        case StatementType::DEF_FUNC_STAT :
            statement = ParseDefineFuncStatement();
            _ast.push_back(statement);
            break;

        case StatementType::DEF_VAR_STAT :
            statement = ParseDefineVarStatement();
            _ast.push_back(statement);
            break;

        case StatementType::RETURN_STAT :
            statement = ParseReturnStatement();
            _ast.push_back(statement);
            break;

        case StatementType::WHILE_STAT :
            statement = ParseWhileStatement();
            _ast.push_back(statement);
            break;

        case StatementType::UNKNOWN_STAT :
            exit(1);
            break;
        }
        return statement;
    }

    auto Parser::ParseCurlyBlock() -> StatementBlock {
        Token lCurlyToken = _lexer.GetNextToken();
        if (lCurlyToken.val != TokenType::TOKEN_SEP_LCURLY) {
            ParseError(lCurlyToken, "{");
            return StatementBlock();
        }
        StatementBlock block;
        Statement stat;
        do {
            Token tryToken = _lexer.LookAhead(0);
            if (tryToken.val == TokenType::TOKEN_SEP_RCURLY) {
                break;
            }
            StatementPtr stat = ParseStatement();
            block.push_back(stat);
        } while(1);

        Token rCurlyToken = _lexer.GetNextToken();
        if (rCurlyToken.val != TokenType::TOKEN_SEP_RCURLY) {
            ParseError(rCurlyToken, "}");
            return StatementBlock();
        }
        
        return block;
    }

    auto Parser::ParseSemicolon() {
        Token SemiToken = _lexer.GetNextToken();

        if (SemiToken.val != TokenType::TOKEN_SEP_SEMICOLON) {
            ParseError(SemiToken, ";");
        }
    }

    auto Parser::ParseWhileStatement() -> WhileStatementPtr {
        Token whileToken = _lexer.GetNextToken();
        if (whileToken.val != TokenType::TOKEN_KW_WHILE) {
            ParseError(whileToken, "return");
        }

        ExpressionPtr condExp = ParseExpression();
        StatementBlock block = ParseCurlyBlock();

        auto stat = new WhileStatement{
            ._condition	= condExp,
            ._block		= block,
        };
        return WhileStatementPtr(stat);
    }

    auto Parser::ParseReturnStatement() -> ReturnStatementPtr {
        Token returnToken = _lexer.GetNextToken();
        if (returnToken.val != TokenType::TOKEN_KW_RETURN) {
            ParseError(returnToken, "return");
        }

        std::vector<ExpressionPtr> returnVal;
        returnVal = ParseMultipleExpression();
        ParseSemicolon();

        auto Stat = new ReturnStatement{._retValues = returnVal};
        return ReturnStatementPtr(Stat);
    }

    auto Parser::ParseDefineFuncStatement() -> DefFuncStatementPtr {
        Token funcKWToken = _lexer.GetNextToken(); // func
        if (funcKWToken.val != TokenType::TOKEN_KW_FUNC) {
            ParseError(funcKWToken, "func");
            return DefFuncStatementPtr(nullptr);
        }

        Token identifierToken = _lexer.GetNextToken();
        if (identifierToken.val != TokenType::TOKEN_IDENTIFIER) {
            ParseError(identifierToken, "identifier");
            return DefFuncStatementPtr(nullptr);
        }

        Token lParenToken = _lexer.GetNextToken();
        if (lParenToken.val != TokenType::TOKEN_SEP_LPAREN) {
            ParseError(lParenToken, "(");
            return DefFuncStatementPtr(nullptr);
        }

        std::list<DefVarStatementPtr> defVars;
        bool continueParseDefVar = true;

        Token tryToken0 = _lexer.LookAhead(0);
        if (tryToken0.val == TokenType::TOKEN_SEP_RPAREN) {
            _lexer.GetNextToken();
            continueParseDefVar = false;
        }

        while (continueParseDefVar){
            DefVarStatementPtr defVar = ParseDefineVar();
            defVars.push_back(defVar);

            Token tryToken = _lexer.GetNextToken();
            if (tryToken.val == TokenType::TOKEN_SEP_COMMA) {// ,
                continueParseDefVar = true;
            } else if (tryToken.val == TokenType::TOKEN_SEP_RPAREN) { // )
                continueParseDefVar = false;
            } else {
                ParseError(tryToken, ", )");
                return DefFuncStatementPtr(nullptr);
            }
        } 

        StatementBlock block = ParseCurlyBlock();

        auto defFuncStat = new DefFuncStatement {
            ._name 			= identifierToken.word,
            ._declaredVars	= defVars,
            ._codeBlock		= block,
        };

        return DefFuncStatementPtr(defFuncStat);
        
    }

    auto Parser::ParseDefineVar() -> DefVarStatementPtr {
        Token varKw = _lexer.GetNextToken();
        Token varName = _lexer.GetNextToken();

        if (varKw.val != TokenType::TOKEN_KW_VAR) {
            ParseError(varKw, "var");
            return DefVarStatementPtr(nullptr);
        }

        if (varName.val != TokenType::TOKEN_IDENTIFIER) {
            ParseError(varName, "identifier");
            return DefVarStatementPtr(nullptr);
        }
        Token tryToken = _lexer.LookAhead(0);
        if (tryToken.val != TokenType::TOKEN_OP_ASSIGN) {
            auto varDef = new DefVarStatement {
                ._name = varName.word,
                ._assignValue = nullptr,
            };
            return DefVarStatementPtr(varDef);
        }
        _lexer.GetNextToken(); // =
        ExpressionPtr exp = ParseExpression();

        auto varDef = new DefVarStatement {
            //._type = varType.word,
            ._name = varName.word,
            ._assignValue = exp,
        };
        return DefVarStatementPtr(varDef);
    }

    auto Parser::ParseDefineVarStatement() -> DefVarStatementPtr {
        DefVarStatementPtr stat = ParseDefineVar();
        ParseSemicolon();

        return stat;
    }

    auto Parser::ParseCallFuncStatement() -> FuncCallStatementPtr {
        FuncCallExpressionPtr funcallExp = ParseFuncCallExpression();
        ParseSemicolon();
        return FuncCallStatementPtr(new FuncCallStatement{Statement{},*funcallExp});
    }
    
    auto Parser::ParseAssignStatement() -> AssignStatementPtr {
        Token token0 = _lexer.GetNextToken();
        Token token1 = _lexer.GetNextToken();

        if (token0.val != TokenType::TOKEN_IDENTIFIER) {
            ParseError(token0, "identifier");
        }

        if (token1.val != TokenType::TOKEN_OP_ASSIGN) {
            ParseError(token1, "=");
        }

        ExpressionPtr exp = ParseExpression();

        auto stat = new AssignStatement {
            ._identifier = token0.word,
            ._assignValue = exp
        };

        ParseSemicolon();

        return AssignStatementPtr(stat);
    }

    auto Parser::ParseIfStatement() -> IfStatementPtr {
        Token IfToken = _lexer.GetNextToken();
        if (IfToken.val != TokenType::TOKEN_KW_IF) {
            ParseError(IfToken, "if");
            return IfStatementPtr(nullptr);
        }

        std::list<IfBlock> ifBlocks;
        StatementBlock elseBlock;

        ExpressionPtr ifCondExp = ParseExpression();

        StatementBlock block = ParseCurlyBlock();
        ifBlocks.push_back(IfBlock{block, ifCondExp});

        bool continueParseElifBlock = true;
        do {
            Token tryToken = _lexer.LookAhead(0);
            if (tryToken.val == TokenType::TOKEN_KW_ELSEIF) {
                Token elifToken = _lexer.GetNextToken();
                ExpressionPtr condExp = ParseExpression();
                StatementBlock block = ParseCurlyBlock();
                ifBlocks.push_back(IfBlock{block, condExp});

            } else if (tryToken.val == TokenType::TOKEN_KW_ELSE) {
                Token elseToken = _lexer.GetNextToken();
                elseBlock = ParseCurlyBlock();
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

    auto Parser::ParseExpression() -> ExpressionPtr {

        ExpressionPtr exp = ParseOpExpression({TokenType::TOKEN_OP_OR, TokenType::TOKEN_OP_AND}, std::bind(&Parser::ParseExpressionL7, this));

        return exp;
    }


    auto Parser::ParseOpExpression (
        std::vector<TokenType> vAcceptedTokenType, 
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

    auto Parser::ParseExpressionL7() -> ExpressionPtr {
        return ParseOpExpression({TokenType::TOKEN_OP_AND}, std::bind(&Parser::ParseExpressionL6, this));
    }

    auto Parser::ParseExpressionL6() -> ExpressionPtr {
        return ParseOpExpression({
                TokenType::TOKEN_OP_LT, // <
                TokenType::TOKEN_OP_LE, // <=
                TokenType::TOKEN_OP_GT, // >
                TokenType::TOKEN_OP_GE, // >=
                TokenType::TOKEN_OP_EQ, // ==
                TokenType::TOKEN_OP_NEQ, // !=
            },
            std::bind(&Parser::ParseExpressionL5, this)
        );
    }

    auto Parser::ParseExpressionL5() -> ExpressionPtr {
        return ParseOpExpression({
                TokenType::TOKEN_OP_ADD, // +
                TokenType::TOKEN_OP_SUB, // -
            },
            std::bind(&Parser::ParseExpressionL4, this)
        );
    }

    auto Parser::ParseExpressionL4() -> ExpressionPtr {
        return ParseOpExpression({
                TokenType::TOKEN_OP_MUL, // *
                TokenType::TOKEN_OP_DIV, // /
                TokenType::TOKEN_OP_MOD, // %
            },
            std::bind(&Parser::ParseExpressionL3, this)
        );
    }

    auto Parser::ParseExpressionL3() -> ExpressionPtr {
        return ParseOpExpression({
                TokenType::TOKEN_OP_BOR, // |
                TokenType::TOKEN_OP_BAND, // &
                TokenType::TOKEN_OP_XOR, // ^
            },
            std::bind(&Parser::ParseExpressionL2, this)
        );
    }

    auto Parser::ParseExpressionL2() -> ExpressionPtr {
        Token tryToken = _lexer.LookAhead(0);
        // !
        if (tryToken.val == TokenType::TOKEN_OP_NEG) {
            Token opToken = _lexer.GetNextToken();

            ExpressionPtr rExp = ParseExpressionL1();

            auto opExp = new OperationExpresson {
                ._operator = opToken.word,
                ._LExp = nullptr,
                ._RExp = rExp
            };
            return OperationExpressonPtr(opExp);
        }

        return ParseExpressionL1();
    }

    auto Parser::ParseExpressionL1() -> ExpressionPtr {
        Token tryToken = _lexer.LookAhead(0);
        Token tryToken1;
        Token token;
        switch (tryToken.val)
        {
        case TokenType::TOKEN_SEP_LPAREN: // ()
            _lexer.GetNextToken();
            ParseExpression();
            token = _lexer.GetNextToken();
            if (token.val != TokenType::TOKEN_SEP_RPAREN) {
                ParseError(tryToken,")");
            }
            break;

        case TokenType::TOKEN_KW_FALSE:
            token = _lexer.GetNextToken();
            return BoolExpressionPtr(new BoolExpression{._value = false});
            break;

        case TokenType::TOKEN_KW_TRUE:
            token = _lexer.GetNextToken();
            return BoolExpressionPtr(new BoolExpression{._value = true});
            break;
        
        case TokenType::TOKEN_KW_NIL:
            token = _lexer.GetNextToken();
            return NilExpressionPtr(new NilExpression());
            break;

        case TokenType::TOKEN_NUMBER:
            token = _lexer.GetNextToken();
            return NumberExpressionPtr(new NumberExpression{._number = std::stod(token.word)});
            break;

        case TokenType::TOKEN_STRING:
            token = _lexer.GetNextToken();
            return StringExpressionPtr(new StringExpression{._string = token.word});
            break;

        case TokenType::TOKEN_IDENTIFIER:
            tryToken1 = _lexer.LookAhead(1);
            if (tryToken1.val == TokenType::TOKEN_SEP_LPAREN) {
                return ParseFuncCallExpression();
            } else {
                token = _lexer.GetNextToken();
                return IdentifierExpressionPtr(new IdentifierExpression{._identifier = token.word});
            }
            break;

        default:
            ParseError(tryToken,"(, false, true, nil, number, string, identifier");
            return ExpressionPtr(nullptr);
            break;
        }
        return ExpressionPtr(nullptr);
    }
    
    auto Parser::ParseMultipleExpression() -> std::vector<ExpressionPtr>{
        std::vector<ExpressionPtr> parameters;
        bool continueParseExp = true;
        Token tryToken;
        tryToken = _lexer.LookAhead(0);
        if (tryToken.val == TokenType::TOKEN_SEP_RPAREN) { // )
            return parameters;
        }

        do {
            ExpressionPtr exp = ParseExpression();
            parameters.push_back(exp);

            tryToken = _lexer.LookAhead(0);
            if (tryToken.val != TokenType::TOKEN_SEP_COMMA) {// ,
                continueParseExp = false;
            } else {
                _lexer.GetNextToken();
            }
        } while (continueParseExp);

        return parameters;
    }

    auto Parser::ParseFuncCallExpression() -> FuncCallExpressionPtr {
        Token identifierToken = _lexer.GetNextToken();
        Token lParenToken = _lexer.GetNextToken();
        if (identifierToken.val != TokenType::TOKEN_IDENTIFIER) {
            ParseError(identifierToken, "identifier");
            return FuncCallExpressionPtr(nullptr);
        }
        if (lParenToken.val != TokenType::TOKEN_SEP_LPAREN) {
            ParseError(lParenToken, "(");
            return FuncCallExpressionPtr(nullptr);
        }

        std::vector<ExpressionPtr> parameters;
        parameters = ParseMultipleExpression();

        Token rParen = _lexer.GetNextToken();
        if (rParen.val != TokenType::TOKEN_SEP_RPAREN) { // )
            ParseError(rParen, ")");
            return FuncCallExpressionPtr(nullptr);
        }
        auto funcallExp = new FuncCallExpression {
            ._identifier = identifierToken.word,
            ._parameters = parameters
        };

        return FuncCallExpressionPtr(funcallExp);
        
    }
}
