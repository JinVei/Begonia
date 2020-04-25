#include "Parser.h"
#include "iostream"
// TODO: KW_DOUBLE KW_INT KW_FALSE KW_TRUE KW_STRING
namespace begonia
{
    void Parser::initStatementParser() {
        _statement_parsers[AstType::IfStatement]        = std::bind(&Parser::ParseIfStatement,this);
        _statement_parsers[AstType::AssignStatement]    = std::bind(&Parser::ParseAssignStatement,this);
        _statement_parsers[AstType::DeclarFuncStatement] = std::bind(&Parser::ParseDeclarFuncStatement,this);
        _statement_parsers[AstType::DeclarVarStatement]  = std::bind(&Parser::ParseDeclarVarStatement,this);
        _statement_parsers[AstType::RetStatement]       = std::bind(&Parser::ParseReturnStatement,this);
        _statement_parsers[AstType::WhileStatement]     = std::bind(&Parser::ParseWhileStatement,this);
        _statement_parsers[AstType::Expr]               = std::bind(&Parser::ParseExpressionStatement,this);
        _statement_parsers[AstType::Semicolon]          = std::bind(&Parser::ParseSemicolon,this);
    }

    Parser::Parser(std::string sourcePath): _lexer(sourcePath) {
        initStatementParser();
    }

    bool isExprToken(TokenType type){
        return type == TokenType::TOKEN_KW_FALSE
            || type == TokenType::TOKEN_KW_NIL
            || type == TokenType::TOKEN_KW_TRUE
            || type == TokenType::TOKEN_NUMBER
            || type == TokenType::TOKEN_OP_ADD
            || type == TokenType::TOKEN_OP_AND
            || type == TokenType::TOKEN_OP_BAND
            || type == TokenType::TOKEN_OP_BOR
            || type == TokenType::TOKEN_OP_DIV
            || type == TokenType::TOKEN_OP_EQ
            || type == TokenType::TOKEN_OP_GE
            || type == TokenType::TOKEN_OP_GT
            || type == TokenType::TOKEN_OP_LE
            || type == TokenType::TOKEN_OP_LT
            || type == TokenType::TOKEN_OP_MOD
            || type == TokenType::TOKEN_OP_MUL
            || type == TokenType::TOKEN_OP_NEG
            || type == TokenType::TOKEN_OP_NEQ
            || type == TokenType::TOKEN_OP_OR
            || type == TokenType::TOKEN_OP_SUB
            || type == TokenType::TOKEN_OP_XOR
            || type == TokenType::TOKEN_STRING;
    }

    auto Parser::TryNextStatementType() -> AstType {
        Token token = _lexer.LookAhead(0);
        Token token2 = _lexer.LookAhead(1);

        switch (token.val) {
        case TokenType::TOKEN_KW_IF:
            return AstType::IfStatement;

        case TokenType::TOKEN_KW_VAR:
            return AstType::DeclarVarStatement;

        case TokenType::TOKEN_KW_FUNC:
            return AstType::DeclarFuncStatement;

        case TokenType::TOKEN_IDENTIFIER:
            if (token2.val == TokenType::TOKEN_OP_ASSIGN) {
                return AstType::AssignStatement;
            }
            else if (token2.val == TokenType::TOKEN_SEP_LPAREN) {
                //return AstType::FuncCallExpr;
                return AstType::Expr;
            }
            else if(isExprToken(token2.val)){
                return AstType::Expr;
            }
            else {
                return AstType::Unknown;
            }

        case TokenType::TOKEN_KW_WHILE:
            return AstType::WhileStatement;

        case TokenType::TOKEN_KW_RETURN:
            return AstType::RetStatement;
            
        case TokenType::TOKEN_SEP_LPAREN:
            return AstType::Expr;

        case TokenType::TOKEN_SEP_SEMICOLON:
            return AstType::Semicolon;

        default:
            if(isExprToken(token2.val)){
                return AstType::Expr;
            }
            ParseError(token, "UNKNOWN_STATEMENT");
            return AstType::Unknown;

        }
    }

    auto Parser::ParseStatement() -> AstPtr {
        AstType statement_type = TryNextStatementType();
        if (statement_type == AstType::Unknown) {
            exit(1);
        }
        auto statement_parser = _statement_parsers[statement_type];
        AstPtr statement = statement_parser();
        return statement;
    }

    void Parser::Parse(){
        auto block = AstBlockPtr(new AstBlock());
        do {
            Token try_token = _lexer.LookAhead(0);
            if (try_token.val != TokenType::TOKEN_SEP_EOF){
                AstPtr statement = ParseStatement();
                if (statement != nullptr) {
                    block->push_back(statement);
                }
            } else {
                break;
            }
        } while(1);
        _ast = block;
    }

    void Parser::ParseError(Token token, std::string expectedWord) {
        printf("[ParseError]:\nParse error at %s, line=%ld\n", token.file_name.c_str(), token.line);
        printf("want '%s', but have '%s'\n", expectedWord.c_str(), token.word.c_str());
        exit(1);
    }

    auto Parser::ParseCurlyBlock() -> AstBlockPtr {
        Token lcurly_token = _lexer.GetNextToken();
        if (lcurly_token.val != TokenType::TOKEN_SEP_LCURLY) {
            ParseError(lcurly_token, "{");
            return AstBlockPtr(new AstBlock{});
        }
        AstBlockPtr block(new AstBlock());
        do {
            Token try_token = _lexer.LookAhead(0);
            if (try_token.val == TokenType::TOKEN_SEP_RCURLY) {
                break;
            }
            AstPtr statement = ParseStatement();
            if (statement != nullptr) {
                block->push_back(statement);
            }
        } while(1);

        Token rcurly_token = _lexer.GetNextToken();
        if (rcurly_token.val != TokenType::TOKEN_SEP_RCURLY) {
            ParseError(rcurly_token, "}");
            return AstBlockPtr(new AstBlock{});
        }
        
        return block;
    }

    AstPtr Parser::ParseSemicolon() {
        Token semi_token = _lexer.GetNextToken();

        if (semi_token.val != TokenType::TOKEN_SEP_SEMICOLON) {
            ParseError(semi_token, ";");
        }
        return nullptr;
    }

    auto Parser::ParseWhileStatement() -> WhileStatementPtr {
        Token while_token = _lexer.GetNextToken();
        if (while_token.val != TokenType::TOKEN_KW_WHILE) {
            ParseError(while_token, "return");
        }

        ExpressionPtr cond_exp = ParseExpression();
        AstBlockPtr block = ParseCurlyBlock();

        auto statement = new WhileStatement(cond_exp, block);
        return WhileStatementPtr(statement);
    }

    auto Parser::ParseReturnStatement() -> ReturnStatementPtr {
        Token return_token = _lexer.GetNextToken();
        if (return_token.val != TokenType::TOKEN_KW_RETURN) {
            ParseError(return_token, "return");
        }

        std::vector<ExpressionPtr> return_val;
        return_val = ParseMultipleExpression();
        ParseSemicolon();

        return ReturnStatementPtr(new ReturnStatement(return_val));
    }

    auto Parser::ParseDeclarFuncStatement() -> DeclarFuncStatementPtr {
        Token func_kw_token = _lexer.GetNextToken(); // func
        if (func_kw_token.val != TokenType::TOKEN_KW_FUNC) {
            ParseError(func_kw_token, "func");
            return DeclarFuncStatementPtr(nullptr);
        }

        Token identifier_token = _lexer.GetNextToken();
        if (identifier_token.val != TokenType::TOKEN_IDENTIFIER) {
            ParseError(identifier_token, "identifier");
            return DeclarFuncStatementPtr(nullptr);
        }

        Token lparen_token = _lexer.GetNextToken();
        if (lparen_token.val != TokenType::TOKEN_SEP_LPAREN) {
            ParseError(lparen_token, "(");
            return DeclarFuncStatementPtr(nullptr);
        }

        std::list<DeclarVarStatementPtr> decl_vars;
        bool continue_parse_paremeter = true;

        Token try_token0 = _lexer.LookAhead(0);
        if (try_token0.val == TokenType::TOKEN_SEP_RPAREN) {
            _lexer.GetNextToken();
            continue_parse_paremeter = false;
        }

        while (continue_parse_paremeter){
            DeclarVarStatementPtr defVar = ParseDeclarVar();
            decl_vars.push_back(defVar);

            Token next_token = _lexer.GetNextToken();
            if (next_token.val == TokenType::TOKEN_SEP_COMMA) {// ,
                continue_parse_paremeter = true;
            } else if (next_token.val == TokenType::TOKEN_SEP_RPAREN) { // )
                continue_parse_paremeter = false;
            } else {
                ParseError(next_token, ", )");
                return DeclarFuncStatementPtr(nullptr);
            }
        } 

        //return type
        Token ret_type = _lexer.GetNextToken();
        if (ret_type.val != TokenType::TOKEN_IDENTIFIER) {
            ParseError(ret_type, "Need return type ");
            return DeclarFuncStatementPtr(nullptr);
        }

        AstBlockPtr block(new AstBlock);
        Token try_token = _lexer.LookAhead(0);
        if (try_token.val == TokenType::TOKEN_SEP_SEMICOLON) {
            _lexer.GetNextToken();
        } else {
            block = ParseCurlyBlock();
        }

        auto defFuncStat = new DeclarFuncStatement(
            identifier_token.word,
            decl_vars,
            ret_type.word,
            block
        );

        return DeclarFuncStatementPtr(defFuncStat);
        
    }

    auto Parser::ParseDeclarVar() -> DeclarVarStatementPtr {
        Token var_name = _lexer.GetNextToken();

        if (var_name.val != TokenType::TOKEN_IDENTIFIER) {
            ParseError(var_name, "identifier");
            return DeclarVarStatementPtr(nullptr);
        }
        // var type
        std::string type = "";
        Token try_token = _lexer.LookAhead(0);
        if (try_token.val == TokenType::TOKEN_IDENTIFIER
            || try_token.val == TokenType::TOKEN_KW_STRING
            || try_token.val == TokenType::TOKEN_KW_DOUBLE) {
            type = try_token.word;
            _lexer.GetNextToken(); // pass type
        }
        // =
        try_token = _lexer.LookAhead(0);
        if (try_token.val != TokenType::TOKEN_OP_ASSIGN) {
            if (type != "") {
                auto decl_var = new DeclarVarStatement (
                    var_name.word,
                    type,
                    nullptr
                );
                return DeclarVarStatementPtr(decl_var);
            } else {
                ParseError(var_name, std::string("Can't not infer type of the variable:") + var_name.word);
                return DeclarVarStatementPtr(nullptr);
            }
        }
        _lexer.GetNextToken(); // =
        ExpressionPtr exp = ParseExpression();

        auto decl_var = new DeclarVarStatement (
            var_name.word,
            type,
            exp
        );
        return DeclarVarStatementPtr(decl_var);
    }

    auto Parser::ParseDeclarVarStatement() -> DeclarVarStatementPtr {
        Token var_kw = _lexer.GetNextToken();
        if (var_kw.val != TokenType::TOKEN_KW_VAR) {
            ParseError(var_kw, "var");
            return DeclarVarStatementPtr(nullptr);
        }

        DeclarVarStatementPtr statement = ParseDeclarVar();
        ParseSemicolon();

        return statement;
    }

    auto Parser::ParseExpressionStatement() -> ExpressionPtr {
        ExpressionPtr expr = ParseExpression();
        ParseSemicolon();
        return expr;
            
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

        auto statement = new AssignStatement(token0.word, exp);
        ParseSemicolon();

        return AssignStatementPtr(statement);
    }

    auto Parser::ParseIfStatement() -> IfStatementPtr {
        Token if_token = _lexer.GetNextToken();
        if (if_token.val != TokenType::TOKEN_KW_IF) {
            ParseError(if_token, "if");
            return IfStatementPtr(nullptr);
        }

        std::list<IfBlock> if_blocks;
        AstBlockPtr else_block(new AstBlock{});

        ExpressionPtr if_cond_exp = ParseExpression();

        AstBlockPtr block = ParseCurlyBlock();
        if_blocks.push_back(IfBlock{block, if_cond_exp});

        bool continue_parse_elif_block = true;
        do {
            Token try_token = _lexer.LookAhead(0);
            if (try_token.val == TokenType::TOKEN_KW_ELSEIF) {
                Token elif_token = _lexer.GetNextToken();
                ExpressionPtr cond_exp = ParseExpression();
                AstBlockPtr block = ParseCurlyBlock();
                if_blocks.push_back(IfBlock{block, cond_exp});

            } else if (try_token.val == TokenType::TOKEN_KW_ELSE) {
                Token else_token = _lexer.GetNextToken();
                else_block = ParseCurlyBlock();
                continue_parse_elif_block = false;

            } else {
                continue_parse_elif_block = false;
            }
        } while (continue_parse_elif_block);

        auto if_statement = new IfStatement(if_blocks, else_block);

        return IfStatementPtr(if_statement);
    }
}
