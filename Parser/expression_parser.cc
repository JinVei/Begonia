#include "parser.h"

namespace begonia {
    auto Parser::ParseExpression() -> ExpressionPtr {

        ExpressionPtr exp = ParseOpExpression({TokenType::TOKEN_OP_OR}, std::bind(&Parser::ParseExpressionL7, this));

        return exp;
    }


    auto Parser::ParseOpExpression (
        std::vector<TokenType> accepted_token_types, 
        OpExpPaser subExpPaeser) 
        -> ExpressionPtr
    {
        ExpressionPtr lexp = subExpPaeser();

        ExpressionPtr exp = lexp;
        while (1) {
            Token try_token = _lexer.LookAhead(0);
            bool is_accepted = false;
            for (auto accepted_token : accepted_token_types) {
                if (try_token.val == accepted_token) {
                    is_accepted = true;
                    break;
                }
            }

            if (is_accepted == false) {
                break;
            }

            Token operator_token = _lexer.GetNextToken();
            ExpressionPtr rexp = subExpPaeser();
            // std::cout << opToken.word;
            auto operation_exp = new OperationExpresson {
                ._operator = operator_token.word,
                ._lexp = exp,
                ._rexp = rexp
            };
            exp = ExpressionPtr(operation_exp);
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
                TokenType::TOKEN_OP_NEQ,// !=
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
                TokenType::TOKEN_OP_BOR,  // |
                TokenType::TOKEN_OP_BAND, // &
                TokenType::TOKEN_OP_XOR,  // ^
            },
            std::bind(&Parser::ParseExpressionL2, this)
        );
    }

    auto Parser::ParseExpressionL2() -> ExpressionPtr {
        Token try_token = _lexer.LookAhead(0);
        if (try_token.val == TokenType::TOKEN_OP_NEG) { // !
            Token operator_token = _lexer.GetNextToken();

            ExpressionPtr rExp = ParseExpressionL1();

            auto opExp = new OperationExpresson {
                ._operator = operator_token.word,
                ._lexp = nullptr,
                ._rexp = rExp
            };
            return OperationExpressonPtr(opExp);
        }

        return ParseExpressionL1();
    }

    auto Parser::ParseExpressionL1() -> ExpressionPtr {
        Token try_token = _lexer.LookAhead(0);
        Token try_token1;
        Token token;
        switch (try_token.val)
        {
        case TokenType::TOKEN_SEP_LPAREN: // ()
            _lexer.GetNextToken();
            ParseExpression();
            token = _lexer.GetNextToken();
            if (token.val != TokenType::TOKEN_SEP_RPAREN) {
                ParseError(try_token,")");
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
            try_token1 = _lexer.LookAhead(1);
            if (try_token1.val == TokenType::TOKEN_SEP_LPAREN) {
                return ParseFuncCallExpression();
            } else {
                token = _lexer.GetNextToken();
                return IdentifierExpressionPtr(new IdentifierExpression{._identifier = token.word});
            }
            break;

        default:
            ParseError(try_token,"(, false, true, nil, number, string, identifier");
            return ExpressionPtr(nullptr);
            break;
        }
        return ExpressionPtr(nullptr);
    }
    
    auto Parser::ParseMultipleExpression() -> std::vector<ExpressionPtr>{
        std::vector<ExpressionPtr> parameters;
        bool is_continue_parse = true;
        Token try_token;
        try_token = _lexer.LookAhead(0);
        if (try_token.val == TokenType::TOKEN_SEP_RPAREN) { // )
            return parameters;
        }

        do {
            ExpressionPtr exp = ParseExpression();
            parameters.push_back(exp);

            try_token = _lexer.LookAhead(0);
            if (try_token.val != TokenType::TOKEN_SEP_COMMA) {// ,
                is_continue_parse = false;
            } else {
                _lexer.GetNextToken();
            }
        } while (is_continue_parse);

        return parameters;
    }

    auto Parser::ParseFuncCallExpression() -> FuncCallExpressionPtr {
        Token id_token = _lexer.GetNextToken();
        Token lparen_token = _lexer.GetNextToken();
        if (id_token.val != TokenType::TOKEN_IDENTIFIER) {
            ParseError(id_token, "identifier");
            return FuncCallExpressionPtr(nullptr);
        }
        if (lparen_token.val != TokenType::TOKEN_SEP_LPAREN) {
            ParseError(lparen_token, "(");
            return FuncCallExpressionPtr(nullptr);
        }

        std::vector<ExpressionPtr> parameters;
        parameters = ParseMultipleExpression();

        Token rparen = _lexer.GetNextToken();
        if (rparen.val != TokenType::TOKEN_SEP_RPAREN) { // )
            ParseError(rparen, ")");
            return FuncCallExpressionPtr(nullptr);
        }
        auto funcallExp = new FuncCallExpression {
            ._identifier = id_token.word,
            ._parameters = parameters
        };

        return FuncCallExpressionPtr(funcallExp);
        
    }
}
