#include "../Lexer.h"
#include <iostream>

using namespace begonia;

int main()
{
	Lexer lexer("./source_code.begonia");
	Token token;
	do {
		token = lexer.GetNextToken();
		std::cout << "val:" << std::to_string(int(token.val)) << ", " << "line:" << token.line << ", " << "word:" << token.word << std::endl;
	} while(token.val != TOKEN_VAL::TOKEN_SEP_EOF);

}
