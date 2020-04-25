#ifndef BEGONIA_LEXICAL_H
#define BEGONIA_LEXICAL_H
#include <string>
#include <fstream>
#include <map>
#include <vector>

namespace begonia
{
    using token_size = uint16_t;
    enum class TokenType: token_size
    {
        TOKEN_SEP_EOF			= 0,
        TOKEN_SEP_SEMICOLON,	// ;
        TOKEN_SEP_COMMA,		// ,
        TOKEN_SEP_DOT,			// .
        TOKEN_SEP_COLON,		// :
        //TOKEN_LABLE,
        TOKEN_SEP_LPAREN,		// (
        TOKEN_SEP_RPAREN,		// )
        TOKEN_SEP_LBRACKET,		// [
        TOKEN_SEP_RBRACKET,		// ]
        TOKEN_SEP_LCURLY,		// {
        TOKEN_SEP_RCURLY,		// }

        TOKEN_OP_ASSIGN,	// =
        TOKEN_OP_SUB,		// -
        TOKEN_OP_ADD,		// +
        TOKEN_OP_MUL,		// *
        TOKEN_OP_DIV,		// /
        TOKEN_OP_MOD,		// %
        TOKEN_OP_XOR,		// ^
        TOKEN_OP_BAND,		// &
        TOKEN_OP_BOR,		// |
        TOKEN_OP_AND,		// &&
        TOKEN_OP_OR,		// ||
        TOKEN_OP_NEG,		// !
        TOKEN_OP_LT,		// <
        TOKEN_OP_LE,		// <=
        TOKEN_OP_GT,		// >
        TOKEN_OP_GE,		// >=
        TOKEN_OP_EQ,		// ==
        TOKEN_OP_NEQ,		// !=

        TOKEN_KW_IF,
        TOKEN_KW_ELSEIF,
        TOKEN_KW_ELSE,
        TOKEN_KW_WHILE,
        TOKEN_KW_FOR,
        TOKEN_KW_IN,
        TOKEN_KW_FALSE,
        TOKEN_KW_TRUE,
        TOKEN_KW_VAR,
        TOKEN_KW_FUNC,
        TOKEN_KW_RETURN,
        TOKEN_KW_NIL,
        TOKEN_KW_DOUBLE,
        TOKEN_KW_STRING,

        TOKEN_NUMBER,
        TOKEN_STRING,
        TOKEN_IDENTIFIER,
    };

    struct Token
    {
        TokenType   val;
        long        line;
        std::string word;
        std::string file_name;
    };

    class Lexer
    {
    private:
        bool SkipWhitespaceAndEmptyline();

        Token ScanKeywordToken(std::string);
        Token ScanSeparationToken();
        Token ScanNumberToken(std::string);
        Token ScanQuoteToken();
        Token ScanIdentifierToken(std::string);

        void InitAcceptableCharacterTable();
        bool IsSeparationCharacter(char);
        bool IsAcceptabCharacter(char);
        auto GetWord() -> std::string;

        void Interrupt(std::string);

        Token NextToken();

        void initKeyWord();

    public:
        ~Lexer();
        Lexer(std::string file_name);
        Token GetNextToken();
        Token LookAhead(size_t step);

        std::map<std::string, TokenType>    key_word_type_;
        std::map<TokenType, std::string>    key_word_;

    private:
        std::ifstream       source_;
        std::string         src_file_name_;
        std::size_t         current_line_offset_ = 0;
        long                current_line_ = 0;
        bool                is_ready_;
        Token               next_token_;
        uint8_t             acceptable_Chars_[256] = {0};
        std::vector<Token>  tokens_;
        size_t              token_index_ = 0;
    };
}
#endif
