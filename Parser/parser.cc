#include "parser.h"
#include "iostream"

namespace begonia
{
    void Parser::initStatementParser() {
        _statement_parsers[StatementType::IF_STATEMENT]        = std::bind(&Parser::ParseIfStatement,this);
        _statement_parsers[StatementType::ASSIGN_STATEMENT]    = std::bind(&Parser::ParseAssignStatement,this);
        _statement_parsers[StatementType::CALL_FUNC_STATEMENT] = std::bind(&Parser::ParseCallFuncStatement,this);
        _statement_parsers[StatementType::DECL_FUNC_STATEMENT] = std::bind(&Parser::ParseDeclarFuncStatement,this);
        _statement_parsers[StatementType::DECL_VAR_STATEMENT]  = std::bind(&Parser::ParseDeclarVarStatement,this);
        _statement_parsers[StatementType::RETURN_STATEMENT]    = std::bind(&Parser::ParseReturnStatement,this);
        _statement_parsers[StatementType::WHILE_STATEMENT]     = std::bind(&Parser::ParseWhileStatement,this);
    }

    Parser::Parser(std::string sourcePath): _lexer(sourcePath) {
        initStatementParser();
    }

    auto Parser::TryNextStatementType() -> StatementType {
        Token token = _lexer.LookAhead(0);
        Token token2 = _lexer.LookAhead(1);

        switch (token.val) {
        case TokenType::TOKEN_KW_IF:
            return StatementType::IF_STATEMENT;

        case TokenType::TOKEN_KW_VAR:
            return StatementType::DECL_VAR_STATEMENT;

        case TokenType::TOKEN_KW_FUNC:
            return StatementType::DECL_FUNC_STATEMENT;

        case TokenType::TOKEN_IDENTIFIER:
            if (token2.val == TokenType::TOKEN_OP_ASSIGN) {
                return StatementType::ASSIGN_STATEMENT;
            }
            else if (token2.val == TokenType::TOKEN_SEP_LPAREN) {
                return StatementType::CALL_FUNC_STATEMENT;
            }
            else {
                return StatementType::UNKNOWN_STATEMENT;
            }

        case TokenType::TOKEN_KW_WHILE:
            return StatementType::WHILE_STATEMENT;

        case TokenType::TOKEN_KW_RETURN:
            return StatementType::RETURN_STATEMENT;

        default:
            ParseError(token, "UNKNOWN_STATEMENT");
            return StatementType::UNKNOWN_STATEMENT;

        }
    }

    auto Parser::ParseStatement() -> StatementPtr {
        StatementType statement_type = TryNextStatementType();
        if (statement_type == StatementType::UNKNOWN_STATEMENT) {
            exit(1);
        }
        auto statement_parser = _statement_parsers[statement_type];
        StatementPtr statement = statement_parser();
        return statement;
    }

    void Parser::Parse(){
        do {
            Token try_token = _lexer.LookAhead(0);
            if (try_token.val != TokenType::TOKEN_SEP_EOF){
                StatementPtr statement = ParseStatement();
                _ast.push_back(statement);
            } else {
                break;
            }
        } while(1);
    }

    void Parser::ParseError(Token token, std::string expectedWord) {
        printf("[ParseError]:\nParse error at %s, line=%ld\n", token.file_name.c_str(), token.line);
        printf("want '%s', but have '%s'\n", expectedWord.c_str(), token.word.c_str());
        exit(1);
    }

    auto Parser::ParseCurlyBlock() -> StatementBlock {
        Token lcurly_token = _lexer.GetNextToken();
        if (lcurly_token.val != TokenType::TOKEN_SEP_LCURLY) {
            ParseError(lcurly_token, "{");
            return StatementBlock();
        }
        StatementBlock block;
        Statement statement;
        do {
            Token try_token = _lexer.LookAhead(0);
            if (try_token.val == TokenType::TOKEN_SEP_RCURLY) {
                break;
            }
            StatementPtr statement = ParseStatement();
            block.push_back(statement);
        } while(1);

        Token rcurly_token = _lexer.GetNextToken();
        if (rcurly_token.val != TokenType::TOKEN_SEP_RCURLY) {
            ParseError(rcurly_token, "}");
            return StatementBlock();
        }
        
        return block;
    }

    auto Parser::ParseSemicolon() {
        Token semi_token = _lexer.GetNextToken();

        if (semi_token.val != TokenType::TOKEN_SEP_SEMICOLON) {
            ParseError(semi_token, ";");
        }
    }

    auto Parser::ParseWhileStatement() -> WhileStatementPtr {
        Token while_token = _lexer.GetNextToken();
        if (while_token.val != TokenType::TOKEN_KW_WHILE) {
            ParseError(while_token, "return");
        }

        ExpressionPtr cond_exp = ParseExpression();
        StatementBlock block = ParseCurlyBlock();

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

            Token try_token = _lexer.GetNextToken();
            if (try_token.val == TokenType::TOKEN_SEP_COMMA) {// ,
                continue_parse_paremeter = true;
            } else if (try_token.val == TokenType::TOKEN_SEP_RPAREN) { // )
                continue_parse_paremeter = false;
            } else {
                ParseError(try_token, ", )");
                return DeclarFuncStatementPtr(nullptr);
            }
        } 

        StatementBlock block = ParseCurlyBlock();

        auto defFuncStat = new DeclarFuncStatement(
            identifier_token.word,
            decl_vars,
            block
        );

        return DeclarFuncStatementPtr(defFuncStat);
        
    }

    auto Parser::ParseDeclarVar() -> DeclarVarStatementPtr {
        Token var_kw = _lexer.GetNextToken();
        Token var_name = _lexer.GetNextToken();

        if (var_kw.val != TokenType::TOKEN_KW_VAR) {
            ParseError(var_kw, "var");
            return DeclarVarStatementPtr(nullptr);
        }

        if (var_name.val != TokenType::TOKEN_IDENTIFIER) {
            ParseError(var_name, "identifier");
            return DeclarVarStatementPtr(nullptr);
        }
        Token try_token = _lexer.LookAhead(0);
        if (try_token.val != TokenType::TOKEN_OP_ASSIGN) {
            auto decl_var = new DeclarVarStatement (
                var_name.word,
                nullptr
            );
            return DeclarVarStatementPtr(decl_var);
        }
        _lexer.GetNextToken(); // =
        ExpressionPtr exp = ParseExpression();

        auto decl_var = new DeclarVarStatement (
            var_name.word,
            exp
        );
        return DeclarVarStatementPtr(decl_var);
    }

    auto Parser::ParseDeclarVarStatement() -> DeclarVarStatementPtr {
        DeclarVarStatementPtr statement = ParseDeclarVar();
        ParseSemicolon();

        return statement;
    }

    auto Parser::ParseCallFuncStatement() -> FuncCallStatementPtr {
        FuncCallExpressionPtr func_call_exp = ParseFuncCallExpression();
        ParseSemicolon();
        
        auto funcall_statement = new FuncCallStatement (
            func_call_exp->_identifier,
            func_call_exp->_parameters
        );
        return FuncCallStatementPtr(funcall_statement);
            
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
        StatementBlock else_block;

        ExpressionPtr if_cond_exp = ParseExpression();

        StatementBlock block = ParseCurlyBlock();
        if_blocks.push_back(IfBlock{block, if_cond_exp});

        bool continue_parse_elif_block = true;
        do {
            Token try_token = _lexer.LookAhead(0);
            if (try_token.val == TokenType::TOKEN_KW_ELSEIF) {
                Token elif_token = _lexer.GetNextToken();
                ExpressionPtr cond_exp = ParseExpression();
                StatementBlock block = ParseCurlyBlock();
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
