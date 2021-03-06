#include "Lexer.h"

#include <map>
#include <string>
#include <iostream>
#if defined(__APPLE__) || defined(__linux__)
#include <execinfo.h>
#endif

//int backtrace(void **buffer, int size);
namespace begonia {
    void Lexer::initKeyWord() {
        key_word_type_ =  {
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
            {"double", 	TokenType::TOKEN_KW_DOUBLE},
            {"string", 	TokenType::TOKEN_KW_STRING},
        };

        for(auto kv : key_word_type_) {
            key_word_[kv.second] = kv.first;
        }

    }

    std::map<TokenType, std::string> KEY_WORD_TYPE;

    Lexer::Lexer(std::string file_name)
    {
        initKeyWord();
        source_ = std::ifstream(file_name, std::ifstream::in);
        if (!source_.is_open()){
            is_ready_ = false;
            next_token_ = Token{TokenType::TOKEN_SEP_EOF, current_line_, "Failed to open file:" + file_name,file_name};
            return;
        }
        src_file_name_ = file_name;
        current_line_ 	= 1;
        is_ready_ = true;

        InitAcceptableCharacterTable();
        next_token_ = NextToken();
        tokens_.push_back(next_token_);
        token_index_ = 0;
    }

    Lexer::~Lexer()
    {
        if (source_.is_open())
            source_.close();
    }
    
    void Lexer::Interrupt(std::string errmsg)
    {
        
        char line[100]={0};
        source_.seekg(current_line_offset_, std::ios::beg);
        source_.getline(line, 99);
        std::cout << "[ERROR] at line " << current_line_ << ": " << errmsg << std::endl;
        std::cout << "Current line: \n" <<  line << std::endl;

#if defined(__APPLE__) || defined(__linux__)
        void* addr[10];
        int nptrs = backtrace(addr, 10);
        char** bt = backtrace_symbols(addr, nptrs);

        std::cout << std::endl << "backtrace: " << std::endl;
        for (int i = 0; i < nptrs; i++)
            std::cout << bt[i] << std::endl;

        free(bt);
#endif
        std::terminate();
    }

    bool Lexer::SkipWhitespaceAndEmptyline()
    {
        while(1){
            char c = source_.get();
            if (c == ' ' || c == '\r' || c == '\t')
                continue;
            else if ( c == '\n')
            {
                current_line_++;
                current_line_offset_ = source_.tellg();
            }
            else if (c == EOF)
                return false;
            else
                break;
        }
        source_.unget();
        return true;
    }

    Token Lexer::GetNextToken()
    {
        if (token_index_ < tokens_.size()) 
        {
            return tokens_[token_index_++];
        } 
        
        while (tokens_.size() <= token_index_) 
        {
            if (!is_ready_ || next_token_.val == TokenType::TOKEN_SEP_EOF)
                return next_token_;

            next_token_ = NextToken();
            tokens_.push_back(next_token_);
        }
        return tokens_[token_index_++];
    }

    Token Lexer::NextToken()
    {
        if (!SkipWhitespaceAndEmptyline())
            return Token{TokenType::TOKEN_SEP_EOF, current_line_, "EOF", src_file_name_};

        Token quote = ScanQuoteToken();
        if(quote.val != TokenType::TOKEN_SEP_EOF)
            return quote;

        if (IsSeparationCharacter(source_.peek()))
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
        return Token{TokenType::TOKEN_SEP_EOF, current_line_, "Interrupt",src_file_name_};
    }

    // step = 0 mean look ahead next token
    Token Lexer::LookAhead(size_t step) {
        if (token_index_ + step < tokens_.size()) 
        {
            return tokens_[token_index_ + step];
        }

        while (tokens_.size() <= token_index_ + step) 
        {
            if (!is_ready_ || next_token_.val == TokenType::TOKEN_SEP_EOF)
                return next_token_;

            next_token_ = NextToken();
            tokens_.push_back(next_token_);
        }
        return tokens_[token_index_ + step];
    }

}
