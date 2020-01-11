#include "Lexer.h"

#include <map>
#include <string>
#include <iostream>

namespace begonia {

	std::map<std::string, TOKEN_VAL> KEY_WORD =  {
		{"if", 		TOKEN_VAL::TOKEN_KW_IF,},
		{"elif", 	TOKEN_VAL::TOKEN_KW_ELSEIF},
		{"else", 	TOKEN_VAL::TOKEN_KW_ELSE},
		{"for", 	TOKEN_VAL::TOKEN_KW_FOR},
		{"while", 	TOKEN_VAL::TOKEN_KW_WHILE},
		{"in", 		TOKEN_VAL::TOKEN_KW_IN},
		{"func", 	TOKEN_VAL::TOKEN_KW_FUNC},
		{"var", 	TOKEN_VAL::TOKEN_KW_VAR},
		{"false", 	TOKEN_VAL::TOKEN_KW_FALSE},
		{"true", 	TOKEN_VAL::TOKEN_KW_TRUE},
		{"or", 		TOKEN_VAL::TOKEN_OP_OR},
		{"and", 	TOKEN_VAL::TOKEN_OP_AND},
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

		initAcceptableCharacterTable();
		_nextToken = NextToken();
	}

	Lexer::~Lexer()
	{
		if (_sourceStream.is_open())
			_sourceStream.close();
	}
	
	void Lexer::Interrupt(std::string errlog)
	{
		char line[100]={0};
			_sourceStream.getline(line, 99);
			std::cout << "[ERROR] at line " << _currentLine << ":" << errlog << std::endl;
			std::cout <<  line << std::endl;
			std::terminate();
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
		Token nextToken = _nextToken;
		_nextToken = NextToken();
		return nextToken;
	}

	Token Lexer::NextToken()
	{
		SkipWhitespaceAndEmptyline();

		Token quote = ParseQuoteToken();
		if(quote.val != TOKEN_VAL::TOKEN_SEP_EOF)
			return quote;

		if (IsSeparationCharacter(_sourceStream.peek()))
			return ParseSeparationToken();

		std::string word = GetWord();

		Token keyword = ParseKeywordToken(word);
		if(keyword.val != TOKEN_VAL::TOKEN_SEP_EOF)
			return keyword;
		
		Token number = ParseNumberToken(word);
		if(number.val != TOKEN_VAL::TOKEN_SEP_EOF)
			return number;

		Token identifier = ParseIdentifierToken(word);
		if(identifier.val != TOKEN_VAL::TOKEN_SEP_EOF)
			return identifier;
		
		Interrupt("Can not parse Token: " + word);
	}

}
