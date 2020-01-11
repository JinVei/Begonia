#ifndef BEGONIA_LEXICAL_H
#define BEGONIA_LEXICAL_H
#include <string>
#include <fstream>
#include <map>

namespace begonia
{
	using token_t = uint16_t;

	enum class TOKEN_VAL: token_t
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

		TOKEN_NUMBER,
		TOKEN_STRING,
		TOKEN_IDENTIFIER,

	};

	struct Token
	{
		TOKEN_VAL	val;
		long 		line;
		std::string	word;
	};

	class Lexer
	{
	private:
		void SkipWhitespaceAndEmptyline();

		Token ParseKeywordToken(std::string);
		Token ParseSeparationToken();
		Token ParseNumberToken(std::string);
		Token ParseQuoteToken();
		Token ParseIdentifierToken(std::string);

		void initAcceptableCharacterTable();
		bool IsSeparationCharacter(char);
		bool IsAcceptabCharacter(char);
		auto GetWord() -> std::string;

		void Interrupt(std::string);
	public:
		~Lexer();
		Lexer(std::string fileName);
		Token NextToken();
		Token GetNextToken();
	private:
		std::ifstream 	_sourceStream;
		std::string 	_sourceFileName;
		long		 	_currentLine;
		bool			_isReady;
		Token			_nextToken;
		uint8_t			_acceptableCharacterTable[256] = {0};
	};

	extern std::map<std::string, TOKEN_VAL> KEY_WORD;
}
#endif
