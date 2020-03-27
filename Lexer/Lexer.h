#ifndef BEGONIA_LEXICAL_H
#define BEGONIA_LEXICAL_H
#include <string>
#include <fstream>
#include <map>
#include <vector>

namespace begonia
{
    using token_t = uint16_t;

    // TODO: naming notations.TokenType
    //enum class TokenType: token_t
    enum class TokenType: token_t
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

        TOKEN_NUMBER,
        TOKEN_STRING,
        TOKEN_IDENTIFIER,

    };

    struct Token
    {
        TokenType	val;
        long 		line;
        std::string	word;
        // TODO
        std::string	file_name_;
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
    public:
        ~Lexer();
        Lexer(std::string fileName);
        Token GetNextToken();
        Token LookAhead(size_t step);

    private:
        std::ifstream		_sourceStream;
        std::string			_sourceFileName;
        std::size_t			_currentLineOffset = 0;
        long				_currentLine = 0;
        bool				_isReady;
        Token				_nextToken;
        uint8_t				_acceptableCharacterTable[256] = {0};
        std::vector<Token>	_tokens;
        size_t				_tokenIndex = 0;
    };

    extern const std::map<std::string, TokenType>	KEY_WORD;
    extern std::map<TokenType, std::string>			KEY_WORD_TYPE;
}
#endif
