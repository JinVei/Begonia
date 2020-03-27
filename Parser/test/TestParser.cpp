#include "Lexer.h"
#include "Parser.h"
int main()
{
    begonia::Parser parser("./test_parser.bga");
    parser.Parsing();
    return 0;
}
