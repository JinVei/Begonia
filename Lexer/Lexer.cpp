#include "Lexer.h"

#include <map>
#include <string>
#include <iostream>

#include <execinfo.h>

//int backtrace(void **buffer, int size);
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

		InitAcceptableCharacterTable();
		_nextToken = NextToken();
	}

	Lexer::~Lexer()
	{
		if (_sourceStream.is_open())
			_sourceStream.close();
	}
	
	void Lexer::Interrupt(std::string errmsg)
	{
		
		char line[100]={0};
		_sourceStream.seekg(_currentLineOffset, std::ios::beg);
		_sourceStream.getline(line, 99);
		std::cout << "[ERROR] at line " << _currentLine << ": " << errmsg << std::endl;
		std::cout << "Current line: \n" <<  line << std::endl;

		void* addr[10];
		int nptrs = backtrace(addr, 10);

		char** bt = backtrace_symbols(addr, nptrs);

		std::cout << std::endl << "backtrace: " << std::endl;
		//std::cout << bt << std::endl;

		for (int i = 0; i < nptrs; i++)
			std::cout << bt[i] << std::endl;

		free(bt);

		std::terminate();
	}

	bool Lexer::SkipWhitespaceAndEmptyline()
	{
		while(1){
			char c = _sourceStream.get();
			if (c == ' ' || c == '\r' || c == '\t')
				continue;
			else if ( c == '\n')
			{
				_currentLine++;
				_currentLineOffset = _sourceStream.tellg();
			}
			else if (c == EOF)
				return false;
			else
				break;
		}
		_sourceStream.unget();
		return true;
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
		if (!SkipWhitespaceAndEmptyline())
			return Token{TOKEN_VAL::TOKEN_SEP_EOF, _currentLine, "EOF"};

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
		return Token{TOKEN_VAL::TOKEN_SEP_EOF, _currentLine, "Interrupt"};
	}

}
