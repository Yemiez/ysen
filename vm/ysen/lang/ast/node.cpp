#include "node.h"


#include "ysen/core/format.h"
#include "ysen/core/ScopeExit.h"
#include "ysen/lang/astvm/Interpreter.h"

ysen::lang::ast::AstNode::AstNode(SourceRange source_range)
	: m_source_range(source_range)
{}

ysen::lang::ast::Statement::Statement(SourceRange source_range)
	: Expression(source_range)
{}

ysen::lang::ast::ScopeStatement::ScopeStatement(SourceRange source_range)
	: Statement(source_range)
{}

void ysen::lang::ast::ScopeStatement::set_name(core::String name)
{
	m_name = std::move(name);
}

void ysen::lang::ast::ScopeStatement::emit(core::SharedPtr<Statement> statement)
{
	m_statements.emplace_back(std::move(statement));
}

ysen::lang::astvm::Value ysen::lang::ast::ScopeStatement::visit(astvm::Interpreter& vm) const
{
	vm.enter_scope("anon");
	astvm::Value ret{};

	for (const auto &node : m_statements) {
		ret = node->visit(vm);
		if (vm.current_scope()->returning()) {
			break;
		}
	}

	vm.exit_scope();
	return ret;
}

void ysen::lang::ast::ScopeStatement::generate_bytecode(bytecode::Generator& generator) const
{
	for (const auto &node : m_statements) {
		node->generate_bytecode(generator);
	}
}

ysen::lang::ast::FunctionParameterExpression::FunctionParameterExpression(SourceRange source_range, core::String name,
																		core::String type_name, bool variadic
)
	: Expression(source_range), m_name(std::move(name)), m_type_name(std::move(type_name)), m_variadic(variadic)
{}

ysen::lang::ast::FunctionExpression::FunctionExpression(SourceRange source_range, std::vector<FunctionParameterExpressionPtr> params, ExpressionPtr body)
	: Expression(source_range), m_parameters(std::move(params)), m_body(std::move(body))
{}

ysen::lang::astvm::Value ysen::lang::ast::FunctionExpression::visit(astvm::Interpreter&) const
{
	astvm::FunctionParameterList parameters{};

	for (const auto& param : m_parameters) {
		parameters.emplace_back(
			core::adopt_shared(new astvm::FunctionParameter{
				param->name(),
				param->type_name(),
				param.ptr()
			})
		);
	}
	
	return astvm::function(core::format("lambda({})", source_range().to_string()), std::move(parameters), this);
}

ysen::lang::ast::FunctionDeclarationStatement::FunctionDeclarationStatement(
	SourceRange source_range, 
	core::String name, 
	std::vector<FunctionParameterExpressionPtr> params,
	ExpressionPtr body
)
	: Expression(source_range), m_name(std::move(name)), m_parameters(std::move(params)), m_body(std::move(body))
{}

ysen::lang::astvm::Value ysen::lang::ast::FunctionDeclarationStatement::visit(astvm::Interpreter& vm) const
{
	astvm::FunctionParameterList params{};

	for (const auto &param : parameters()) {
		params.emplace_back(core::adopt_shared(new astvm::FunctionParameter(
			param->name(), 
			param->type_name(), 
			param.ptr()
		)));
	}

	auto function = core::adopt_shared(new astvm::Function(m_name, std::move(params), this));
	vm.current_scope()->declare_function(std::move(function));
	return {};
}

void ysen::lang::ast::FunctionDeclarationStatement::generate_bytecode(bytecode::Generator& generator) const
{
	generator.emit_block(m_name);

	for (const auto& param : parameters()) {
		generator.emit<bytecode::Pop>();
		generator.emit<bytecode::StoreVariable>(param->name());
	}
	
	m_body->generate_bytecode(generator);
	generator.end_block();
}

ysen::lang::ast::VarDeclaration::VarDeclaration(SourceRange source_range, core::String name, core::SharedPtr<Expression> init)
	: Statement(source_range), m_name(std::move(name)), m_expression(std::move(init))
{}

ysen::lang::astvm::Value ysen::lang::ast::VarDeclaration::visit(astvm::Interpreter& vm) const
{
	astvm::Value value{};

	if (expression()) {
		value = expression()->visit(vm);
	}

	// Declare it
	vm.current_scope()->declare_variable(astvm::var(name(), astvm::value(value)));
	return value;
}

void ysen::lang::ast::VarDeclaration::generate_bytecode(bytecode::Generator& generator) const
{
	if (m_expression) {
		m_expression->generate_bytecode(generator);
	}
	else {
		generator.emit<bytecode::LoadImmediate>(astvm::undefined());
	}

	generator.emit<bytecode::StoreVariable>(m_name);
}

ysen::lang::ast::FunctionCallExpression::FunctionCallExpression(SourceRange source_range, core::String name,
																std::vector<ExpressionPtr> arguments
)
	: Expression(source_range), m_name(std::move(name)), m_arguments(std::move(arguments))
{}

ysen::lang::ast::Program::Program(SourceRange source_range)
	: AstNode(source_range)
{}

void ysen::lang::ast::Program::emit(core::SharedPtr<AstNode> child)
{
	m_children.emplace_back(std::move(child));
}

ysen::lang::astvm::Value ysen::lang::ast::Program::visit(astvm::Interpreter& vm) const
{
	astvm::Value ret{};
	for (const auto& child : m_children) {
		ret = child->visit(vm);
		if (vm.current_scope()->returning()) {
			break;
		}
	}
	return ret;
}

void ysen::lang::ast::Program::generate_bytecode(bytecode::Generator& generator) const
{
	generator.emit_block("main");
	for (const auto &node : m_children) {
		node->generate_bytecode(generator);
	}
	generator.end_block();
}

ysen::lang::ast::Expression::Expression(SourceRange source_range)
	: AstNode(source_range)
{}

ysen::lang::astvm::Value ysen::lang::ast::FunctionCallExpression::visit(astvm::Interpreter& vm) const
{
	auto function = vm.current_scope()->find_function(name());

	if (!function) {
		auto variable = vm.current_scope()->find_variable(name());

		if (!variable) {
			return {};
		}

		if (variable->value()->is_function()) {
			function = variable->value()->function();
		}
		else if (variable->value()->is_string()) {
			function = vm.current_scope()->find_function(variable->value()->string());
		}

		// If not found from string or variable, exit with undefined
		if (!function) {
			return {}; // TODO throw error
		}
	}

	std::vector<astvm::Value> values{};
	for (const auto& arg : m_arguments) {
		values.emplace_back(arg->visit(vm));
	}

	return function->invoke(vm, values);
}

void ysen::lang::ast::FunctionCallExpression::generate_bytecode(bytecode::Generator& generator) const
{
	for (const auto& arg : m_arguments) {
		arg->generate_bytecode(generator);
		generator.emit<bytecode::Push>();
	}
	
	generator.emit<bytecode::Call>(m_name);
}

ysen::lang::ast::ReturnExpression::ReturnExpression(SourceRange source_range, ExpressionPtr expression)
	: Expression(source_range), m_expression(std::move(expression))
{}

ysen::lang::astvm::Value ysen::lang::ast::ReturnExpression::visit(astvm::Interpreter& vm) const
{
	vm.current_scope()->mark_return();
	return m_expression->visit(vm);
}

void ysen::lang::ast::ReturnExpression::generate_bytecode(bytecode::Generator& generator) const
{
	m_expression->generate_bytecode(generator);
	generator.emit<bytecode::Ret>();
}

ysen::lang::ast::BinOpExpression::BinOpExpression(SourceRange source_range, ExpressionPtr left, ExpressionPtr right, BinOp op)
	: Expression(source_range), m_left(std::move(left)), m_right(std::move(right)), m_op(op)
{}

ysen::lang::astvm::Value ysen::lang::ast::BinOpExpression::visit(astvm::Interpreter& vm) const
{
	auto lhs = m_left->visit(vm);
	auto rhs = m_right->visit(vm);
	
	switch (m_op) {
	case BinOp::Addition: return lhs + rhs;
	case BinOp::Subtraction: return lhs - rhs;
	case BinOp::Division: return lhs / rhs;
	case BinOp::Multiplication: return lhs * rhs;
	case BinOp::Greater: return lhs > rhs;
	case BinOp::Less: return lhs < rhs;
	case BinOp::GreaterEqual: return lhs > rhs || lhs == rhs;
	case BinOp::LessEqual: return lhs < rhs || lhs == rhs;
	default: return {};
	}
}

void ysen::lang::ast::BinOpExpression::generate_bytecode(bytecode::Generator& generator) const
{
	m_left->generate_bytecode(generator);
	auto reg = generator.allocate_register();
	generator.emit(core::adopt_shared(static_cast<bytecode::Instruction*>(new bytecode::Store(reg))));
	m_right->generate_bytecode(generator);

	switch (m_op) {
	case BinOp::Addition: 
		generator.emit(core::adopt_shared(static_cast<bytecode::Instruction*>(new bytecode::Add(reg))));
		break;
	case BinOp::Subtraction: break;
	case BinOp::Division: break;
	case BinOp::Multiplication: break;
	default: ;
	}
}

ysen::lang::ast::ConstantExpression::ConstantExpression(SourceRange source_range)
	: Expression(source_range)
{}

ysen::lang::ast::NumericExpression::NumericExpression(SourceRange source_range)
	: ConstantExpression(source_range)
{}

ysen::lang::ast::StringExpression::StringExpression(SourceRange source_range, core::String value)
	: ConstantExpression(source_range), m_value(std::move(value))
{}

ysen::lang::astvm::Value ysen::lang::ast::StringExpression::visit(astvm::Interpreter&) const
{
	return { m_value };
}

ysen::lang::ast::IntegerExpression::IntegerExpression(SourceRange source_range, int value)
	: NumericExpression(source_range), m_value(value)
{}

ysen::lang::astvm::Value ysen::lang::ast::IntegerExpression::visit(astvm::Interpreter&) const
{
	return { m_value };
}

void ysen::lang::ast::IntegerExpression::generate_bytecode(bytecode::Generator& generator) const
{
	generator.emit(core::adopt_shared(static_cast<bytecode::Instruction*>(new bytecode::LoadImmediate(m_value))));
}

ysen::lang::ast::FloatExpression::FloatExpression(SourceRange source_range, float value)
	: NumericExpression(source_range), m_value(value)
{}

ysen::lang::astvm::Value ysen::lang::ast::FloatExpression::visit(astvm::Interpreter&) const
{
	return { m_value };
}

ysen::lang::ast::IdentifierExpression::IdentifierExpression(SourceRange source_range, core::String name)
	: Expression(source_range), m_name(std::move(name))
{}

ysen::lang::astvm::Value ysen::lang::ast::IdentifierExpression::visit(astvm::Interpreter& vm) const
{
	// Needs scoping
	auto variable = vm.current_scope()->find_variable(name());

	if (!variable) {
		auto function = vm.current_scope()->find_function(name());

		if (function) {
			return function;
		}
		
		return {};
	}
	
	return *variable->value();
}

void ysen::lang::ast::IdentifierExpression::generate_bytecode(bytecode::Generator& generator) const
{
	generator.emit<bytecode::LoadVariable>(m_name);
}

ysen::lang::ast::ArrayExpression::ArrayExpression(SourceRange source_range, std::vector<ExpressionPtr> expressions)
	: Expression(source_range), m_expressions(expressions)
{}

ysen::lang::astvm::Value ysen::lang::ast::ArrayExpression::visit(astvm::Interpreter& vm) const
{
	astvm::Value::Array array{};

	for (const auto& expr : m_expressions) {
		array.emplace_back(expr->visit(vm));
	}

	return array;
}

ysen::lang::ast::AccessExpression::AccessExpression(SourceRange source_range, core::String object, core::String field)
	: Expression(source_range), m_object(std::move(object)), m_field(std::move(field))
{}

ysen::lang::astvm::Value ysen::lang::ast::AccessExpression::visit(astvm::Interpreter& vm) const
{
	auto var = vm.current_scope()->find_variable(m_object);

	if (!var) {
		return {};
	}

	if (!var->value()->is_object()) {
		return {};
	}

	if (!var->value()->object().contains(m_field)) {
		return {};
	}

	return var->value()->object().at(m_field);
}

ysen::lang::ast::ObjectExpression::ObjectExpression(SourceRange source_range, std::vector<KeyValueExpressionPtr> key_value_expressions)
	: Expression(source_range), m_key_value_expressions(std::move(key_value_expressions))
{}

ysen::lang::astvm::Value ysen::lang::ast::ObjectExpression::visit(astvm::Interpreter& vm) const
{
	astvm::Value::Object object{};

	for (const auto &kv : m_key_value_expressions) {
		auto key = kv->key()->visit(vm);
		auto value = kv->value()->visit(vm);
		object[key] = value;
	}

	return object;
}

ysen::lang::ast::KeyValueExpression::KeyValueExpression(SourceRange source_range, ExpressionPtr key, ExpressionPtr value)
	: Expression(source_range), m_key(std::move(key)), m_value(std::move(value))
{}

ysen::lang::ast::NumericRangeExpression::NumericRangeExpression(SourceRange source_range, int min, int max)
	: Expression(source_range), m_min(min), m_max(max)
{}

ysen::lang::astvm::Value ysen::lang::ast::NumericRangeExpression::visit(astvm::Interpreter&) const
{
	astvm::Value::Array arr{};

	for (auto i = min(); i <= max(); ++i) {
		arr.emplace_back(i);
	}

	return arr;
}

ysen::lang::ast::RangedLoopExpression::RangedLoopExpression(
	SourceRange source_range, 
	ExpressionPtr declaration, 
	ExpressionPtr range_expression,
	ExpressionPtr body
)
	: Expression(source_range), m_declaration(std::move(declaration)), m_range_expression(std::move(range_expression)),
	m_body(std::move(body))
{}

ysen::lang::astvm::Value ysen::lang::ast::RangedLoopExpression::visit(astvm::Interpreter& vm) const
{
	auto range = range_expression()->visit(vm);

	if (!(range.is_object() || range.is_string() || range.is_array())) {
		return {}; // TODO error
	}

	astvm::Value last_statement{};
	
	if (range.is_object()) {
		for (auto [key, value] : range.object()) {
			vm.enter_scope("ranged_loop", astvm::ScopeType::Loopable);
			declaration()->visit(vm);

			// TODO: This is ugly, fix it
			auto& [_, snd] = *vm.current_scope()->variables().begin();
			*snd->value() = value;

			last_statement = body()->visit(vm);
			vm.exit_scope();
		}
	}
	else if (range.is_array()) {
		for (auto &value : range.array()) {
			vm.enter_scope("ranged_loop", astvm::ScopeType::Loopable);
			declaration()->visit(vm);

			// TODO: This is ugly, fix it
			auto& [_, snd] = *vm.current_scope()->variables().begin();
			*snd->value() = value;

			last_statement = body()->visit(vm);
			vm.exit_scope();
		}
	}
	else if (range.is_string()) {
		// TODO
	}
	
	return last_statement;
}

ysen::lang::ast::AssignmentExpression::AssignmentExpression(SourceRange source_range, core::String name, ExpressionPtr body)
	: Expression(source_range), m_name(std::move(name)), m_body(std::move(body))
{}

ysen::lang::astvm::Value ysen::lang::ast::AssignmentExpression::visit(astvm::Interpreter& vm) const
{
	auto variable = vm.current_scope()->find_variable(m_name);

	if (!variable) {
		variable = astvm::var(m_name, {});
		vm.current_scope()->declare_variable(variable);
	}

	variable->set_value(m_body->visit(vm));
	return *variable->value();
}

ysen::lang::ast::ElseIfStatement::ElseIfStatement(
	SourceRange source_range, 
	VarDeclarationPtr var_declaration, 
	ExpressionPtr cond, 
	ExpressionPtr body
)
	: Expression(source_range), m_var_declaration(std::move(var_declaration)),
	m_condition(std::move(cond)), m_body(std::move(body))
{}

ysen::lang::astvm::Value ysen::lang::ast::ElseIfStatement::visit(astvm::Interpreter& vm) const
{
	return m_body->visit(vm);
}

ysen::lang::ast::ElseStatement::ElseStatement(SourceRange source_range, ExpressionPtr body)
	: Expression(source_range), m_body(std::move(body))
{}

ysen::lang::astvm::Value ysen::lang::ast::ElseStatement::visit(astvm::Interpreter& vm) const
{
	return m_body->visit(vm);
}

ysen::lang::ast::IfStatement::IfStatement(
	SourceRange source_range, 
	VarDeclarationPtr var_declaration, 
	ExpressionPtr cond, 
	ExpressionPtr body, 
	std::vector<ElseIfStatementPtr> else_ifs,
	ElseStatementPtr else_statement
)
	: Expression(source_range), m_var_declaration(std::move(var_declaration)),
	m_condition(std::move(cond)), m_body(std::move(body)),
	m_else_if_statements(std::move(else_ifs)), m_else_statement(std::move(else_statement))
{}

ysen::lang::astvm::Value ysen::lang::ast::IfStatement::visit(astvm::Interpreter& vm) const
{
	{
		vm.enter_scope("if");
		core::ScopeExit guard{[&vm]() {
			vm.exit_scope();
		}};
		
		if (m_var_declaration) {
			m_var_declaration->visit(vm);
		}

		auto condition = m_condition->visit(vm);

		if (condition.is_trueish()) {
			return m_body->visit(vm);
		}
	}
	

	for (const auto &else_if : m_else_if_statements) {
		vm.enter_scope("else_if");
		core::ScopeExit guard{[&vm]() {
			vm.exit_scope();
		}};
		
		if (else_if->declaration()) {
			else_if->declaration()->visit(vm);
		}

		auto condition = else_if->condition()->visit(vm);

		if (condition.is_trueish()) {
			return else_if->visit(vm);
		}
	}

	if (m_else_statement) {
		return m_else_statement->visit(vm);
	}

	return {};
}

void ysen::lang::ast::IfStatement::generate_bytecode(bytecode::Generator& generator) const
{

	
}
