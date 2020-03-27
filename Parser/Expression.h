#ifndef BEGONIA_EXPRESSION_H
#define BEGONIA_EXPRESSION_H
#include <memory>

namespace begonia
{
    struct Expression {
	};
	using ExpressionPtr = std::shared_ptr<Expression>;

	struct OperationExpresson: public Expression {
		std::string 				_operator;
		ExpressionPtr				_LExp;
		ExpressionPtr				_RExp;
	};
	using OperationExpressonPtr = std::shared_ptr<OperationExpresson>;

	struct BoolExpression: public Expression {
		bool						_value;
	};
	using BoolExpressionPtr = std::shared_ptr<BoolExpression>;

    struct NilExpression: public Expression {
	};
	using NilExpressionPtr = std::shared_ptr<NilExpression>;

	struct NumberExpression: public Expression {
		double						_number;
	};
	using NumberExpressionPtr = std::shared_ptr<NumberExpression>;

	struct StringExpression: public Expression {
		std::string					_string;
	};
	using StringExpressionPtr = std::shared_ptr<StringExpression>;

	struct IdentifierExpression: public Expression {
		std::string					_identifier;
	};
	using IdentifierExpressionPtr = std::shared_ptr<IdentifierExpression>;

    struct FuncCallExpression: public Expression {
		std::string					_identifier;
		std::vector<ExpressionPtr>	_parameters;
	};
	using FuncCallExpressionPtr = std::shared_ptr<FuncCallExpression>;

}
#endif
