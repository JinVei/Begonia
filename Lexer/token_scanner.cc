#include "lexer.h"

#include <iostream>
#include <regex>

namespace begonia {
    void Lexer::InitAcceptableCharacterTable()
    {
        // 0 means unacceptable characters
        // 1 or 2 means acceptable characters
        // 2 means separated characters
        for (int i=33; i<=126; i++)
            acceptable_Chars_[i] = 1;
        
        for (int i=32; i<=47; i++)
            acceptable_Chars_[i] = 2;

        acceptable_Chars_['\t'] = 2;
        acceptable_Chars_['\r'] = 2;
        acceptable_Chars_['\n'] = 2;

        for (int i=58; i<=64; i++)
            acceptable_Chars_[i] = 2;
        
        for (int i=91; i<=96; i++)
            acceptable_Chars_[i] = 2;
        
        for (int i=123; i<=126; i++)
            acceptable_Chars_[i] = 2;

        acceptable_Chars_['_'] = 1;
    }

    Token Lexer::ScanSeparationToken()
    {
        char ch = source_.get();
        switch(ch)
        {
        case '=':
            if (source_.peek() == '=')
            {
                source_.ignore();
                return Token{TokenType::TOKEN_OP_EQ, current_line_, "==", src_file_name_};
            }
            else
                return Token{TokenType::TOKEN_OP_ASSIGN, current_line_, "=", src_file_name_};

        case '+':
            return Token{TokenType::TOKEN_OP_ADD, current_line_, "+", src_file_name_};
        case '-':
            return Token{TokenType::TOKEN_OP_SUB, current_line_, "-", src_file_name_};
        case '*':
            return Token{TokenType::TOKEN_OP_MUL, current_line_, "*", src_file_name_};
        case '/':
            return Token{TokenType::TOKEN_OP_DIV, current_line_, "/", src_file_name_};
        case '%':
            return Token{TokenType::TOKEN_OP_MOD, current_line_, "%", src_file_name_};
        case '^':
            return Token{TokenType::TOKEN_OP_XOR, current_line_, "^", src_file_name_};
        case '!':
            if (source_.peek() == '=')
            {
                source_.ignore();
                return Token{TokenType::TOKEN_OP_NEQ, current_line_, "!=", src_file_name_};
            }
            else
                return Token{TokenType::TOKEN_OP_NEG, current_line_, "!", src_file_name_};
            
        case '|':
            if (source_.peek() == '|')
            {
                source_.ignore();
                return Token{TokenType::TOKEN_OP_OR, current_line_, "||", src_file_name_};
            }
            else
                return Token{TokenType::TOKEN_OP_BOR, current_line_, "|", src_file_name_};

        case '&':
            if (source_.peek() == '&')
            {
                source_.ignore();
                return Token{TokenType::TOKEN_OP_AND, current_line_, "&&", src_file_name_};
            }
            else
                return Token{TokenType::TOKEN_OP_BAND, current_line_, "&", src_file_name_};

        case '<':
            if (source_.peek() == '=')
            {
                source_.ignore();
                return Token{TokenType::TOKEN_OP_LE, current_line_, "<=", src_file_name_};
            }
            else
                return Token{TokenType::TOKEN_OP_LT, current_line_, "<", src_file_name_};

        case '>':
            if (source_.peek() == '=')
            {
                source_.ignore();
                return Token{TokenType::TOKEN_OP_GE, current_line_, ">=", src_file_name_};
            }
            else
                return Token{TokenType::TOKEN_OP_GT, current_line_, ">", src_file_name_};

        case ';':
            return Token{TokenType::TOKEN_SEP_SEMICOLON, current_line_, ";", src_file_name_};
        case ',':
            return Token{TokenType::TOKEN_SEP_COMMA, current_line_, ",", src_file_name_};
        case '.':
            return Token{TokenType::TOKEN_SEP_DOT, current_line_, ".", src_file_name_};
        case ':':
            return Token{TokenType::TOKEN_SEP_COLON, current_line_, ":", src_file_name_};
        case '(':
            return Token{TokenType::TOKEN_SEP_LPAREN, current_line_, "(", src_file_name_};
        case ')':
            return Token{TokenType::TOKEN_SEP_RPAREN, current_line_, ")", src_file_name_};
        case '[':
            return Token{TokenType::TOKEN_SEP_LBRACKET, current_line_, "[", src_file_name_};
        case ']':
            return Token{TokenType::TOKEN_SEP_RBRACKET, current_line_, "]", src_file_name_};
        case '{':
            return Token{TokenType::TOKEN_SEP_LCURLY, current_line_, "{", src_file_name_};
        case '}':
            return Token{TokenType::TOKEN_SEP_RCURLY, current_line_, "}", src_file_name_};
        default:
            Interrupt(std::string("Can not accept '") + ch + std::string("'"));
            return Token{TokenType::TOKEN_SEP_EOF, current_line_, "ScanSeparationToken", src_file_name_};
        }
        // source_.unget();
    }

    bool Lexer::IsSeparationCharacter(char ch)
    {
        return acceptable_Chars_[ch] == 2 ? true : false;
    }

    bool Lexer::IsAcceptabCharacter(char ch)
    {
        if (ch == EOF) return true;
        return acceptable_Chars_[ch] == 0? false : true;
    }

    std::string Lexer::GetWord()
    {
        std::string word;
        char ch;
        while(1)
        {
            ch = source_.get();
            if( !IsAcceptabCharacter(ch)){
                printf("%d\n", ch);
                Interrupt(std::string("Can not accept '") + ch + std::string("'"));
            }

            if (IsSeparationCharacter(ch))
                break;

            word.push_back(ch);
        }
        source_.unget();

        if(word.empty())
            Interrupt("Get empty word");

        return word;
    }

    Token Lexer::ScanKeywordToken(std::string word)
    {
        auto key = key_word_type_.find(word);
        if(key == key_word_type_.end()) 
        {
            return Token{TokenType::TOKEN_SEP_EOF, current_line_, "ScanKeywordToekn", src_file_name_};
        }
        return Token{key->second, current_line_, key->first, src_file_name_};
    }

    Token Lexer::ScanNumberToken(std::string word)
    {
        std::regex integer("[[:digit:]]+");
        if (!regex_match(word, integer))
            return Token{TokenType::TOKEN_SEP_EOF, current_line_, "ScanNumberToken", src_file_name_};
        else
            return Token{TokenType::TOKEN_NUMBER, current_line_, word, src_file_name_};
    }

    Token Lexer::ScanQuoteToken()
    {
        if (source_.peek() != '\'' && source_.peek() != '\"')
            return Token{TokenType::TOKEN_SEP_EOF, current_line_, "ScanStringToken", src_file_name_};

        std::string str;
        char breakSymbol = source_.get();

        while(1)
        {
            char ch = source_.get();
            if (ch == breakSymbol)
                return Token{TokenType::TOKEN_STRING, current_line_, str, src_file_name_};
            else if(ch == EOF)
                Interrupt("miss quotation token");
            else
                str.push_back(ch);
        }
    }

    Token Lexer::ScanIdentifierToken(std::string word)
    {
        std::regex identifier("[a-zA-Z_][a-zA-Z0-9_]*");
        if (!regex_match(word, identifier))
            return Token{TokenType::TOKEN_SEP_EOF, current_line_, "ScanIdentifierToken", src_file_name_};
        else
            return Token{TokenType::TOKEN_IDENTIFIER, current_line_, word, src_file_name_};
    }
}
