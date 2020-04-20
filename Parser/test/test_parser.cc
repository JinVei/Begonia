#include "lexer.h"
#include "Parser.h"

#include <iostream>
void RraverseStatement(begonia::StatementBlock statment_block);

int main()
{
    begonia::Parser parser("./test_parser.bga");
    parser.Parse();
    auto statment_block = parser._ast;

    RraverseStatement(parser._ast);

    return 0;
}

void RraverseStatement(begonia::StatementBlock statment_block) {
    for(auto statment : statment_block) {
        //dynamic_cast<begonia::IfStatementPtr>(statment.get()
        if (statment->GetType() == begonia::StatementType::ASSIGN_STATEMENT) {
            std::cout<<"ASSIGN_STATEMENT\n";
            auto ptr = std::dynamic_pointer_cast<begonia::AssignStatement>(statment);
            std::cout<<"name:" << ptr->_identifier << std::endl;

        } else if (statment->GetType() == begonia::StatementType::CALL_FUNC_STATEMENT) {
            std::cout<<"CALL_FUNC_STATEMENT\n";
            auto ptr = std::dynamic_pointer_cast<begonia::FuncCallStatement>(statment);
            std::cout<<"name:" << ptr->_identifier << std::endl;

        } else if (statment->GetType() == begonia::StatementType::DECL_FUNC_STATEMENT) {
            std::cout<<"DECL_FUNC_STATEMENT\n";
            auto ptr = std::dynamic_pointer_cast<begonia::DeclarFuncStatement>(statment);
            RraverseStatement(ptr->_block);
        } else if (statment->GetType() == begonia::StatementType::DECL_VAR_STATEMENT) {
            std::cout<<"DECL_VAR_STATEMENT\n";
        } else if (statment->GetType() == begonia::StatementType::IF_STATEMENT) {
            std::cout<<"IF_STATEMENT\n";
        } else if (statment->GetType() == begonia::StatementType::RETURN_STATEMENT) {
            std::cout<<"RETURN_STATEMENT\n";
        } else if (statment->GetType() == begonia::StatementType::UNKNOWN_STATEMENT) {
            std::cout<<"UNKNOWN_STATEMENT\n";
        } else if (statment->GetType() == begonia::StatementType::WHILE_STATEMENT) {
            std::cout<<"WHILE_STATEMENT\n";
        }
    }
}
