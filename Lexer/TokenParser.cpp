#include "Lexer.h"

#include <iostream>
#include <regex>

namespace begonia {
	void Lexer::InitAcceptableCharacterTable()
	{
		// 0 means unacceptable characters
		// 1 or 2 means acceptable characters
		// 2 means separated characters
		for (int i=33; i<=126; i++)
			_acceptableCharacterTable[i] = 1;
		
		for (int i=32; i<=47; i++)
			_acceptableCharacterTable[i] = 2;

		_acceptableCharacterTable['\t'] = 2;
		_acceptableCharacterTable['\r'] = 2;
		_acceptableCharacterTable['\n'] = 2;

		for (int i=58; i<=64; i++)
			_acceptableCharacterTable[i] = 2;
		
		for (int i=91; i<=96; i++)
			_acceptableCharacterTable[i] = 2;
		
		for (int i=123; i<=126; i++)
			_acceptableCharacterTable[i] = 2;

	}

	Token Lexer::ParseSeparationToken()
	{
		char ch = _sourceStream.get();
		switch(ch)
		{
		case '=':
			if (_sourceStream.peek() == '=')
			{
				_sourceStream.ignore();
				return Token{TOKEN_VAL::TOKEN_OP_EQ, _currentLine, "=="};
			}
			else
				return Token{TOKEN_VAL::TOKEN_OP_ASSIGN, _currentLine, "="};

		case '+':
			return Token{TOKEN_VAL::TOKEN_OP_ADD, _currentLine, "+"};
		case '-':
			return Token{TOKEN_VAL::TOKEN_OP_SUB, _currentLine, "-"};
		case '*':
			return Token{TOKEN_VAL::TOKEN_OP_MUL, _currentLine, "*"};
		case '/':
			return Token{TOKEN_VAL::TOKEN_OP_DIV, _currentLine, "/"};
		case '%':
			return Token{TOKEN_VAL::TOKEN_OP_MOD, _currentLine, "%"};
		case '^':
			return Token{TOKEN_VAL::TOKEN_OP_XOR, _currentLine, "^"};
		case '!':
			if (_sourceStream.peek() == '=')
			{
				_sourceStream.ignore();
				return Token{TOKEN_VAL::TOKEN_OP_NEQ, _currentLine, "!="};
			}
			else
				return Token{TOKEN_VAL::TOKEN_OP_NEG, _currentLine, "!"};
			
		case '|':
			if (_sourceStream.peek() == '|')
			{
				_sourceStream.ignore();
				return Token{TOKEN_VAL::TOKEN_OP_OR, _currentLine, "||"};
			}
			else
				return Token{TOKEN_VAL::TOKEN_OP_BOR, _currentLine, "|"};

		 case '&':
		 	if (_sourceStream.peek() == '&')
			 {
				_sourceStream.ignore();
				return Token{TOKEN_VAL::TOKEN_OP_AND, _currentLine, "&&"};
			}
			else
				return Token{TOKEN_VAL::TOKEN_OP_BAND, _currentLine, "&"};

		case '<':
			if (_sourceStream.peek() == '=')
			 {
				_sourceStream.ignore();
				return Token{TOKEN_VAL::TOKEN_OP_LE, _currentLine, "<="};
			}
			else
				return Token{TOKEN_VAL::TOKEN_OP_LT, _currentLine, "<"};

		case '>':
			if (_sourceStream.peek() == '=')
			 {
				_sourceStream.ignore();
				return Token{TOKEN_VAL::TOKEN_OP_GE, _currentLine, ">="};
			}
			else
				return Token{TOKEN_VAL::TOKEN_OP_GT, _currentLine, ">"};

		case ';':
			return Token{TOKEN_VAL::TOKEN_SEP_SEMICOLON, _currentLine, ";"};
		case ',':
			return Token{TOKEN_VAL::TOKEN_SEP_COMMA, _currentLine, ","};
		case '.':
			return Token{TOKEN_VAL::TOKEN_SEP_DOT, _currentLine, "."};
		case ':':
			return Token{TOKEN_VAL::TOKEN_SEP_COLON, _currentLine, ":"};
		case '(':
			return Token{TOKEN_VAL::TOKEN_SEP_LPAREN, _currentLine, "("};
		case ')':
			return Token{TOKEN_VAL::TOKEN_SEP_RPAREN, _currentLine, ")"};
		case '[':
			return Token{TOKEN_VAL::TOKEN_SEP_LBRACKET, _currentLine, "["};
		case ']':
			return Token{TOKEN_VAL::TOKEN_SEP_RBRACKET, _currentLine, "]"};
		case '{':
			return Token{TOKEN_VAL::TOKEN_SEP_LCURLY, _currentLine, "{"};
		case '}':
			return Token{TOKEN_VAL::TOKEN_SEP_RPAREN, _currentLine, "}"};
		default:
			Interrupt(std::string("Can not accept '") + ch + std::string("'"));
			return Token{TOKEN_VAL::TOKEN_SEP_EOF, _currentLine, "ParseSeparationToken"};
		}
		// _sourceStream.unget();
	}

	bool Lexer::IsSeparationCharacter(char ch)
	{
		return _acceptableCharacterTable[ch] == 2 ? true : false;
	}

	bool Lexer::IsAcceptabCharacter(char ch)
	{
		if (ch == EOF) return true;
		return _acceptableCharacterTable[ch] == 0? false : true;
	}

	std::string Lexer::GetWord()
	{
		std::string word;
		char ch;
		while(1)
		{
			ch = _sourceStream.get();
			if( !IsAcceptabCharacter(ch)){
				printf("%d\n", ch);
				Interrupt(std::string("Can not accept '") + ch + std::string("'"));
			}

			if (IsSeparationCharacter(ch))
				break;

			word.push_back(ch);
		}
		_sourceStream.unget();

		if(word.empty())
			Interrupt("Get empty word");

		return word;
	}

	Token Lexer::ParseKeywordToken(std::string word)
	{
		auto key = KEY_WORD.find(word);
		if(key == KEY_WORD.end())
			return Token{TOKEN_VAL::TOKEN_SEP_EOF, _currentLine, "ParseKeywordToekn"};

		switch (key->second)
		{
		case TOKEN_VAL::TOKEN_KW_IF:
			return Token{TOKEN_VAL::TOKEN_KW_IF, _currentLine, key->first};
		case TOKEN_VAL::TOKEN_KW_ELSE:
			return Token{TOKEN_VAL::TOKEN_KW_ELSE, _currentLine, key->first};
		case TOKEN_VAL::TOKEN_KW_ELSEIF:
			return Token{TOKEN_VAL::TOKEN_KW_ELSEIF, _currentLine, key->first};
		case TOKEN_VAL::TOKEN_KW_FALSE:
			return Token{TOKEN_VAL::TOKEN_KW_FALSE, _currentLine, key->first};
		case TOKEN_VAL::TOKEN_KW_TRUE:
			return Token{TOKEN_VAL::TOKEN_KW_TRUE, _currentLine, key->first};
		case TOKEN_VAL::TOKEN_KW_FOR:
			return Token{TOKEN_VAL::TOKEN_KW_FOR, _currentLine, key->first};
		case TOKEN_VAL::TOKEN_KW_FUNC:
			return Token{TOKEN_VAL::TOKEN_KW_FUNC, _currentLine, key->first};
		case TOKEN_VAL::TOKEN_KW_IN:
			return Token{TOKEN_VAL::TOKEN_KW_IN, _currentLine, key->first};
		case TOKEN_VAL::TOKEN_KW_VAR:
			return Token{TOKEN_VAL::TOKEN_KW_VAR, _currentLine, key->first};
		case TOKEN_VAL::TOKEN_KW_WHILE:
			return Token{TOKEN_VAL::TOKEN_KW_WHILE, _currentLine, key->first};
		default:
			Interrupt("Trap in default case");
			return Token{TOKEN_VAL::TOKEN_SEP_EOF, _currentLine, "ParseKeywordToken"};
		}
	}

	Token Lexer::ParseNumberToken(std::string word)
	{
		std::regex integer("[[:digit:]]+");
		if (!regex_match(word, integer))
			return Token{TOKEN_VAL::TOKEN_SEP_EOF, _currentLine, "ParseNumberToken"};
		else
			return Token{TOKEN_VAL::TOKEN_NUMBER, _currentLine, word};
	}

	Token Lexer::ParseQuoteToken()
	{
		if (_sourceStream.peek() != '\'' && _sourceStream.peek() != '\"')
			return Token{TOKEN_VAL::TOKEN_SEP_EOF, _currentLine, "ParseStringToken"};

		std::string str;
		char breakSymbol = _sourceStream.get();

		while(1)
		{
			char ch = _sourceStream.get();
			if (ch == breakSymbol)
				return Token{TOKEN_VAL::TOKEN_STRING, _currentLine, str};
			else if(ch == EOF)
				Interrupt("miss quotation token");
			else
				str.push_back(ch);
		}
	}

	Token Lexer::ParseIdentifierToken(std::string word)
	{
		std::regex identifier("[a-zA-Z][a-zA-Z0-9]*");
		if (!regex_match(word, identifier))
			return Token{TOKEN_VAL::TOKEN_SEP_EOF, _currentLine, "ParseIdentifierToken"};
		else
			return Token{TOKEN_VAL::TOKEN_IDENTIFIER, _currentLine, word};
	}
}
