#include "Lexer.h"

#include <map>
#include <string>
#include <iostream>

#include <execinfo.h>

//int backtrace(void **buffer, int size);
namespace begonia {

    const std::map<std::string, TokenType> KEY_WORD =  {
        {"if", 		TokenType::TOKEN_KW_IF,},
        {"elif", 	TokenType::TOKEN_KW_ELSEIF},
        {"else", 	TokenType::TOKEN_KW_ELSE},
        {"for", 	TokenType::TOKEN_KW_FOR},
        {"while", 	TokenType::TOKEN_KW_WHILE},
        //{"in", 		TokenType::TOKEN_KW_IN},
        {"func", 	TokenType::TOKEN_KW_FUNC},
        {"var", 	TokenType::TOKEN_KW_VAR},
        {"false", 	TokenType::TOKEN_KW_FALSE},
        {"true", 	TokenType::TOKEN_KW_TRUE},
        {"or", 		TokenType::TOKEN_OP_OR},
        {"and", 	TokenType::TOKEN_OP_AND},
        {"return", 	TokenType::TOKEN_KW_RETURN},
        {"nil", 	TokenType::TOKEN_KW_NIL},
    };
    std::map<TokenType, std::string> KEY_WORD_TYPE;

    Lexer::Lexer(std::string fileName)
    {
        for(auto kv : KEY_WORD) {
            KEY_WORD_TYPE[kv.second] = kv.first;
        }

        _sourceStream 		= std::ifstream(fileName, std::ifstream::in);
        if (!_sourceStream.is_open()){
            _isReady = false;
            _nextToken = Token{TokenType::TOKEN_SEP_EOF, _currentLine, "Failed to open file:" + fileName};
            return;
        }
        _sourceFileName = fileName;
        _currentLine 	= 1;
        _isReady = true;

        InitAcceptableCharacterTable();
        _nextToken = NextToken();
        _tokens.push_back(_nextToken);
        _tokenIndex = 0;
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
        // TODO: windows backtrace_symbols
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
        if (_tokenIndex < _tokens.size()) 
        {
            return _tokens[_tokenIndex++];
        } 
        
        while (_tokens.size() <= _tokenIndex) 
        {
            if (!_isReady || _nextToken.val == TokenType::TOKEN_SEP_EOF)
                return _nextToken;

            _nextToken = NextToken();
            _tokens.push_back(_nextToken);
        }
        return _tokens[_tokenIndex++];
    }

    Token Lexer::NextToken()
    {
        if (!SkipWhitespaceAndEmptyline())
            return Token{TokenType::TOKEN_SEP_EOF, _currentLine, "EOF"};

        Token quote = ScanQuoteToken();
        if(quote.val != TokenType::TOKEN_SEP_EOF)
            return quote;

        if (IsSeparationCharacter(_sourceStream.peek()))
            return ScanSeparationToken();

        std::string word = GetWord();

        Token keyword = ScanKeywordToken(word);
        if(keyword.val != TokenType::TOKEN_SEP_EOF)
            return keyword;
        
        Token number = ScanNumberToken(word);
        if(number.val != TokenType::TOKEN_SEP_EOF)
            return number;

        Token identifier = ScanIdentifierToken(word);
        if(identifier.val != TokenType::TOKEN_SEP_EOF)
            return identifier;
        
        Interrupt("Can not parse Token: " + word);
        return Token{TokenType::TOKEN_SEP_EOF, _currentLine, "Interrupt"};
    }

    // step = 0 mean look ahead next token
    Token Lexer::LookAhead(size_t step) {
        if (_tokenIndex + step < _tokens.size()) 
        {
            return _tokens[_tokenIndex + step];
        }

        while (_tokens.size() <= _tokenIndex + step) 
        {
            if (!_isReady || _nextToken.val == TokenType::TOKEN_SEP_EOF)
                return _nextToken;

            _nextToken = NextToken();
            _tokens.push_back(_nextToken);
        }
        return _tokens[_tokenIndex + step];
    }

}
