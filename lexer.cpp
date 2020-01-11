#include <map>
#include <string>

#include "lexer.h"

namespace begonia {

	std::map<TOKEN_VAL, std::string> KEY_WORD =  {
		{TOKEN_VAL::TOKEN_KW_IF, 		"if"},
		{TOKEN_VAL::TOKEN_KW_ELSEIF, 	"elif"},
		{TOKEN_VAL::TOKEN_KW_ELSE,	 	"else"},
		{TOKEN_VAL::TOKEN_KW_FOR,	 	"for"},
		{TOKEN_VAL::TOKEN_KW_WHILE,	 	"while"},
		{TOKEN_VAL::TOKEN_KW_IN,	 	"in"},
		{TOKEN_VAL::TOKEN_KW_FUNC,	 	"func"},
		{TOKEN_VAL::TOKEN_KW_VAR,	 	"var"},
		{TOKEN_VAL::TOKEN_KW_FALSE,	 	"false"},
		{TOKEN_VAL::TOKEN_KW_TRUE,	 	"true"},
		{TOKEN_VAL::TOKEN_OP_OR,	 	"or"},
		{TOKEN_VAL::TOKEN_OP_AND,	 	"and"},
	};

	Lexer::Lexer(std::string fileName)
	{
		_sourceStream 		= std::ifstream(fileName, std::ifstream::in);
		if (!_sourceStream.is_open()){
			_isReady = false;
			_nextToken = Token{TOKEN_VAL::TOKEN_SEP_EOF, _currentLine, "Failed to open file:" + fileName};
			return;
		}
		_sourceFileName = fileName;
		_currentLine 	= 1;
		_isReady = true;

		_nextToken = NextToken();
	}

	Token Lexer::AssignToken()
	{
		return Token{TOKEN_VAL::TOKEN_OP_ASSIGN, _currentLine, "="};
	}

	Token Lexer::EqualToken()
	{
		return Token{TOKEN_VAL::TOKEN_OP_EQ, _currentLine, "=="};
	}

	Token Lexer::AdditionToken()
	{
		return Token{TOKEN_VAL::TOKEN_OP_ADD, _currentLine, "+"};
	}

	Token Lexer::SubtractionToken()
	{
		return Token{TOKEN_VAL::TOKEN_OP_SUB, _currentLine, "-"};
	}

	Token Lexer::MultiplicationToken()
	{
		return Token{TOKEN_VAL::TOKEN_OP_MUL, _currentLine, "*"};
	}

	Token Lexer::DivisionToken()
	{
		return Token{TOKEN_VAL::TOKEN_OP_DIV, _currentLine, "/"};
	}

	void Lexer::SkipWhitespaceAndEmptyline()
	{
		while(1){
			char c = _sourceStream.get();
			if (c == ' ' || c == '\r')
				continue;
			else if ( c == '\n')
				_currentLine++;
			else
				break;
		}
		_sourceStream.unget();
	}

	Token Lexer::GetNextToken()
	{
		if (!_isReady || _nextToken.val == TOKEN_VAL::TOKEN_SEP_EOF)
			return _nextToken;
		Token ret = _nextToken;
		_nextToken = NextToken();
		return ret;
	}

	Token Lexer::NextToken()
	{
		SkipWhitespaceAndEmptyline();

		std::string word;
		char ch = _sourceStream.get();
		switch(ch)
		{
		case '=':
			if (_sourceStream.peek() != '=')
				return AssignToken();
			else
			{
				_sourceStream.ignore();
				return EqualToken();
			}
		case '+':
			return AdditionToken();
		case '-':
			return SubtractionToken();
		case '*':
			return MultiplicationToken();
		case '/':
			return DivisionToken();
		case '%':
			break;
		case '^':
			break;
		case '!':
			// !=
			break;
		case '|':
		// 	// ||
		// 	break;
		 case '&':
		// 	// &&
		// 	break;
		case '<':
			// <=
			break;
		case '>':
			// >=
			break;
		case '{':
			break;
		case '}':
			break;
		}
	}

}
