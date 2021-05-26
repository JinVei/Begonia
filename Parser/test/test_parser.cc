#include "lexer.h"
#include "Parser.h"

#include <iostream>
void RraverseStatement(begonia::AstBlock statment_block);

int main()
{
    begonia::Parser parser("./test_parser.bga");
    parser.Parse();
    auto statment_block = parser._ast;

    RraverseStatement(*std::dynamic_pointer_cast<begonia::AstBlock>(parser._ast));

    return 0;
}

void RraverseStatement(begonia::AstBlock statment_block) {
    for(auto statment : statment_block) {
        //dynamic_cast<begonia::IfStatementPtr>(statment.get()
        if (statment->GetType() == begonia::AstType::AssignStatement) {
            std::cout<<"ASSIGN_STATEMENT\n";
            auto ptr = std::dynamic_pointer_cast<begonia::AssignStatement>(statment);
            std::cout<<"name:" << ptr->_identifier << std::endl;

        } else if (statment->GetType() == begonia::AstType::FuncallExpr) {
            std::cout<<"CALL_FUNC_STATEMENT\n";
            auto ptr = std::dynamic_pointer_cast<begonia::FuncCallExpression>(statment);
            std::cout<<"name:" << ptr->_identifier << std::endl;

        } else if (statment->GetType() == begonia::AstType::DeclareFuncStatement) {
            std::cout<<"DECL_FUNC_STATEMENT\n";
            auto ptr = std::dynamic_pointer_cast<begonia::DeclareFuncStatement>(statment);
            RraverseStatement(*ptr->_block);
        } else if (statment->GetType() == begonia::AstType::DeclareVarStatement) {
            std::cout<<"DECL_VAR_STATEMENT\n";
        } else if (statment->GetType() == begonia::AstType::IfStatement) {
            std::cout<<"IF_STATEMENT\n";
        } else if (statment->GetType() == begonia::AstType::RetStatement) {
            std::cout<<"RETURN_STATEMENT\n";
        } else if (statment->GetType() == begonia::AstType::Unknown) {
            std::cout<<"UNKNOWN_STATEMENT\n";
        } else if (statment->GetType() == begonia::AstType::WhileStatement) {
            std::cout<<"WHILE_STATEMENT\n";
        }
    }
}
