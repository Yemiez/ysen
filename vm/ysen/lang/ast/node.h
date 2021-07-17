#pragma once
#include <ysen/lang/Lexer.h>
#include <ysen/lang/astvm/Value.h>
#include "ysen/core/Optional.h"

namespace ysen::lang::astvm {
	class Interpreter;
}

namespace ysen::lang::ast {

	enum class BinOp
	{
		Addition,
		Subtraction,
		Division,
		Multiplication,
		Greater,
		GreaterEqual,
		Less,
		LessEqual,
	};

	class AstNode;
	class Program;
	class Statement;
	class ScopeStatement;
	class VarDeclaration;
	class Expression;
	class BinOpExpression;
	class ConstantExpression;
	class NumericExpression;
	class IntegerExpression;
	class FloatExpression;
	class StringExpression;
	class IdentifierExpression;
	class FunctionDeclarationStatement;
	class FunctionCallExpression;
	class ReturnExpression;
	class FunctionExpression;
	class FunctionParameterExpression;
	class ArrayExpression;
	class AccessExpression;
	class ObjectExpression;
	class KeyValueExpression;
	class RangedLoopExpression;
	class NumericRangeExpression;
	class AssignmentExpression;
	
	using AstNodePtr = core::SharedPtr<AstNode>;
	using ProgramPtr = core::SharedPtr<Program>;
	using StatementPtr = core::SharedPtr<Statement>;
	using ScopeStatementPtr = core::SharedPtr<ScopeStatement>;
	using VarDeclarationPtr = core::SharedPtr<VarDeclaration>;
	using ExpressionPtr = core::SharedPtr<Expression>;
	using BinOpExpressionPtr = core::SharedPtr<BinOpExpression>;
	using ConstantExpressionPtr = core::SharedPtr<ConstantExpression>;
	using NumericExpressionPtr = core::SharedPtr<NumericExpression>;
	using IntegerExpressionPtr = core::SharedPtr<IntegerExpression>;
	using FloatExpressionPtr = core::SharedPtr<FloatExpression>;
	using StringExpressionPtr = core::SharedPtr<StringExpression>;
	using IdentifierExpressionPtr = core::SharedPtr<IdentifierExpression>;
	using FunctionDeclarationStatementPtr = core::SharedPtr<FunctionDeclarationStatement>;
	using FunctionCallExpressionPtr = core::SharedPtr<FunctionCallExpression>;
	using ReturnExpressionPtr = core::SharedPtr<ReturnExpression>;
	using FunctionExpressionPtr = core::SharedPtr<FunctionExpression>;
	using FunctionParameterExpressionPtr = core::SharedPtr<FunctionParameterExpression>;
	using ArrayExpressionPtr = core::SharedPtr<ArrayExpression>;
	using AccessExpressionPtr = core::SharedPtr<AccessExpression>;
	using ObjectExpressionPtr = core::SharedPtr<ObjectExpression>;
	using KeyValueExpressionPtr = core::SharedPtr<KeyValueExpression>;
	using RangedLoopExpressionPtr = core::SharedPtr<RangedLoopExpression>;
	using NumericRangeExpressionPtr = core::SharedPtr<NumericRangeExpression>;
	using AssignmentExpressionPtr = core::SharedPtr<AssignmentExpression>;
	
	class AstNode
	{
	public:
		AstNode(SourceRange source_range);
		virtual ~AstNode() = default;
		SourceRange source_range() const { return m_source_range; }

		virtual bool is_program() const { return false; }
		virtual bool is_statement() const { return false; }
		virtual bool is_scope_statement() const { return false; }
		virtual bool is_expression() const { return false; }
		virtual bool is_constant_expression() const { return false; }
		virtual bool is_numeric_expression() const { return false; }
		virtual bool is_integer_expression() const { return false; }
		virtual bool is_string_expression() const { return false; }
		virtual bool is_float_expression() const { return false; }
		virtual bool is_var_declaration() const { return false; }
		virtual bool is_bin_op_expression() const { return false; }
		virtual bool is_identifier_expression() const { return false; }
		virtual bool is_function_declaration() const { return false; }
		virtual bool is_function_call() const { return false; }
		virtual bool is_return_expression() const { return false; }
		virtual bool is_function_expression() const { return false; }
		virtual bool is_function_parameter() const { return false; }
		virtual bool is_array_expression() const { return false; }
		virtual bool is_access_expression() const { return false; }
		virtual bool is_object_expression() const { return false; }
		virtual bool is_ranged_loop_expression() const { return false; }

		
		virtual astvm::Value visit(astvm::Interpreter&) const { return {}; }
	protected:
		SourceRange m_source_range{};
	};

	class Program : public AstNode
	{
	public:
		Program(SourceRange);
		bool is_program() const override { return true; }

		const auto& children() const { return m_children; }
		void emit(core::SharedPtr<AstNode>);
		
		astvm::Value visit(astvm::Interpreter&) const override;
	private:
		std::vector<core::SharedPtr<AstNode>> m_children{};
	};
	
	class Expression : public AstNode
	{
	public:
		Expression(SourceRange);
		bool is_expression() const override { return true; }
	};

	class Statement : public Expression
	{
	public:
		Statement(SourceRange);
		bool is_statement() const override { return true; }
	};
	
	class ScopeStatement : public Statement
	{
	public:
		ScopeStatement(SourceRange);
		bool is_scope_statement() const override { return true; }

		const core::String& name() const { return m_name; }
		void set_name(core::String);
		
		const auto& statements() const { return m_statements; }
		void emit(core::SharedPtr<Statement>);

		astvm::Value visit(astvm::Interpreter&) const override;
	private:
		std::vector<core::SharedPtr<Statement>> m_statements{};
		core::String m_name{};
	};

	class FunctionParameterExpression : public Expression
	{
	public:
		FunctionParameterExpression(SourceRange, core::String name, core::String type_name, bool variadic = false);
		bool is_function_parameter() const override { return true; }

		const auto& name() const { return m_name; }
		const auto& type_name() const { return m_type_name; }
		const auto& variadic() const { return m_variadic; }
	private:
		core::String m_name{};
		core::String m_type_name{};
		bool m_variadic{false};
	};
	
	class FunctionExpression : public Expression
	{
	public:
		FunctionExpression(SourceRange, std::vector<FunctionParameterExpressionPtr>, ExpressionPtr body);
		bool is_function_expression() const override { return true; }

		const auto& parameters() const { return m_parameters; }
		const auto& body() const { return m_body; }

		astvm::Value visit(astvm::Interpreter&) const override;
	private:
		std::vector<FunctionParameterExpressionPtr> m_parameters{};
		ExpressionPtr m_body{};
	};
	
	class FunctionDeclarationStatement : public Expression
	{
	public:
		FunctionDeclarationStatement(SourceRange, core::String, std::vector<FunctionParameterExpressionPtr>, ExpressionPtr body);
		bool is_function_declaration() const override { return true; }

		const auto& name() const { return m_name; }
		const auto& parameters() const { return m_parameters; }
		const auto& body() const { return m_body; }
		astvm::Value visit(astvm::Interpreter&) const override;
	private:
		core::String m_name{};
		std::vector<FunctionParameterExpressionPtr> m_parameters{};
		ExpressionPtr m_body{};
	};

	class VarDeclaration : public Statement
	{
	public:
		VarDeclaration(SourceRange, core::String name, core::SharedPtr<Expression> init = {});
		bool is_var_declaration() const override { return true; }

		const auto& name() const { return m_name; }
		const auto& expression() const { return m_expression; }

		astvm::Value visit(astvm::Interpreter&) const override;
	private:
		core::String m_name{};
		core::SharedPtr<Expression> m_expression{};
	};

	class FunctionCallExpression : public Expression
	{
	public:
		FunctionCallExpression(SourceRange, core::String name, std::vector<ExpressionPtr> arguments);
		bool is_function_call() const override { return true; }

		const auto& name() const { return m_name; }
		const auto& arguments() const { return m_arguments; }

		astvm::Value visit(astvm::Interpreter&) const override;
	private:
		core::String m_name{};
		std::vector<ExpressionPtr> m_arguments{};
	};

	class ReturnExpression : public Expression
	{
	public:
		ReturnExpression(SourceRange, ExpressionPtr);
		bool is_return_expression() const override { return true; }
		
		const auto& expression() const { return m_expression; }

		astvm::Value visit(astvm::Interpreter&) const override;
	private:
		ExpressionPtr m_expression{};
	};
	
	class BinOpExpression : public Expression
	{
	public:
		BinOpExpression(SourceRange, ExpressionPtr left, ExpressionPtr right, BinOp op);

		const auto& left() const { return m_left; }
		auto& left() { return m_left; }
		const auto& right() const { return m_right; }
		auto& right() { return m_right; }
		const auto& op() const { return m_op; }
		
		bool is_bin_op_expression() const override { return true; }

		astvm::Value visit(astvm::Interpreter&) const override;
	private:
		ExpressionPtr m_left{};
		ExpressionPtr m_right{};
		BinOp m_op{};
	};

	class ConstantExpression : public Expression
	{
	public:
		ConstantExpression(SourceRange);
		bool is_constant_expression() const override { return true; }
	};

	class NumericExpression : public ConstantExpression
	{
	public:
		NumericExpression(SourceRange);
		bool is_numeric_expression() const override { return true; }
	};

	class StringExpression : public ConstantExpression
	{
	public:
		StringExpression(SourceRange, core::String);
		bool is_string_expression() const override { return true; }

		const core::String& value() const { return m_value; }
		void set_value(core::String value) { m_value = std::move(value); }

		astvm::Value visit(astvm::Interpreter&) const override;
	private:
		core::String m_value{};
	};

	class IntegerExpression : public NumericExpression
	{
	public:
		IntegerExpression(SourceRange, int);
		bool is_integer_expression() const override { return true; }
		
		int value() const { return m_value; }
		void set_value(int value) { m_value = value; }

		astvm::Value visit(astvm::Interpreter&) const override;
	private:
		int m_value{};
	};

	class FloatExpression : public NumericExpression
	{
	public:
		FloatExpression(SourceRange, float);
		bool is_float_expression() const override { return true; }

		float value() const { return m_value; }
		void set_value(float value) { m_value = value; }

		astvm::Value visit(astvm::Interpreter&) const override;
	private:
		float m_value{};
	};

	class IdentifierExpression : public Expression
	{
	public:
		IdentifierExpression(SourceRange, core::String);
		bool is_identifier_expression() const override { return true; }

		const core::String& name() const { return m_name; }

		astvm::Value visit(astvm::Interpreter&) const override;
	private:
		core::String m_name{};
	};

	class ArrayExpression : public Expression
	{
	public:
		ArrayExpression(SourceRange, std::vector<ExpressionPtr> expressions);
		bool is_array_expression() const override { return true; }

		const auto& expressions() const { return m_expressions; }
		
		astvm::Value visit(astvm::Interpreter&) const override;
	private:
		std::vector<ExpressionPtr> m_expressions{};
	};

	class AccessExpression : public Expression
	{
	public:
		AccessExpression(SourceRange, core::String object, core::String field);
		bool is_access_expression() const override { return true; }

		const auto& object() const { return m_object; }
		const auto& field() const { return m_field; }

		astvm::Value visit(astvm::Interpreter&) const override;
	private:
		core::String m_object{};
		core::String m_field{};
	};

	class ObjectExpression : public Expression
	{
	public:
		ObjectExpression(SourceRange, std::vector<KeyValueExpressionPtr> key_value_expressions);
		bool is_object_expression() const override { return true; }

		const auto& key_value_expressions() const { return m_key_value_expressions; }

		astvm::Value visit(astvm::Interpreter&) const override;
	private:
		std::vector<KeyValueExpressionPtr> m_key_value_expressions{};
	};

	class KeyValueExpression : public Expression
	{
	public:
		KeyValueExpression(SourceRange, ExpressionPtr key, ExpressionPtr value);

		const auto& key() const { return m_key; }
		const auto& value() const { return m_value; }
	private:
		ExpressionPtr m_key{};
		ExpressionPtr m_value{};
	};

	class NumericRangeExpression : public Expression
	{
	public:
		NumericRangeExpression(SourceRange, int min, int max);

		const auto& min() const { return m_min; }
		const auto& max() const { return m_max; }

		astvm::Value visit(astvm::Interpreter&) const override;
	private:
		int m_min{};
		int m_max{};
	};

	class RangedLoopExpression : public Expression
	{
	public:
		RangedLoopExpression(SourceRange, ExpressionPtr declaration, ExpressionPtr range_expression, ExpressionPtr body);
		bool is_ranged_loop_expression() const override { return true; }

		const auto& declaration() const { return m_declaration; }
		const auto& range_expression() const { return m_range_expression; }
		const auto& body() const { return m_body; }

		astvm::Value visit(astvm::Interpreter&) const override;
	private:
		ExpressionPtr m_declaration{};
		ExpressionPtr m_range_expression{};
		ExpressionPtr m_body{};
	};

	class AssignmentExpression : public Expression
	{
	public:
		AssignmentExpression(SourceRange, core::String name, ExpressionPtr body);

		const auto& name() const { return m_name; }
		const auto& body() const { return m_body; }

		astvm::Value visit(astvm::Interpreter&) const override;
	private:
		core::String m_name{};
		ExpressionPtr m_body{};
	};
}
