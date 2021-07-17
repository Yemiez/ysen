#include "Interpreter.h"
#include "Value.h"
#include "ysen/core/format.h"

ysen::lang::astvm::FunctionParameter::FunctionParameter(core::String name, core::String type_name, const ast::AstNode* node)
	: m_name(std::move(name)), m_type_name(std::move(type_name)), m_ast_node(node)
{}

ysen::lang::astvm::Function::Function(core::String name, FunctionParameterList parameters, const ast::AstNode* ast_node)
	: m_name(std::move(name)), m_parameters(std::move(parameters)), m_ast_node(ast_node)
{
	m_callable = [this](Interpreter& vm, const std::vector<Value>& arguments) {
		core::println("calling function '{}'", this->name());
		vm.unpack_arguments(arguments, this->parameters());
		return m_ast_node->is_function_declaration() ?
			dynamic_cast<const ast::FunctionDeclarationStatement*>(m_ast_node)->body()->visit(vm) :
			dynamic_cast<const ast::FunctionExpression*>(m_ast_node)->body()->visit(vm);
	};
}

ysen::lang::astvm::Function::Function(core::String name, FunctionParameterList parameters, FunctionSignature function)
	: m_name(std::move(name)), m_parameters(std::move(parameters)), m_callable(std::move(function))
{}

ysen::lang::astvm::Value ysen::lang::astvm::Function::invoke(Interpreter& vm, const std::vector<Value>& arguments) const
{
	vm.enter_scope(m_name, ScopeType::Returnable);
	auto ret = m_callable(vm, arguments);
	vm.exit_scope();
	return ret;
}

ysen::lang::astvm::Variable::Variable(core::String name, ValuePtr value, const ast::AstNode* ast_node)
	: m_name(std::move(name)), m_value(std::move(value)), m_ast_node(ast_node)
{}

void ysen::lang::astvm::Variable::set_value(Value value)
{
	*m_value = std::move(value);
}

ysen::lang::astvm::Scope::Scope(Scope* parent, core::String name, ScopeType type)
	: m_parent(parent), m_name(std::move(name)), m_scope_type(type)
{}

ysen::core::String ysen::lang::astvm::Scope::qualified_name() const
{
	auto parent_qualified = m_parent ? m_parent->qualified_name() : "";
	if (!parent_qualified.empty()) {
		parent_qualified.append(":");
	}
	parent_qualified.append(m_name);
	return parent_qualified;
}

void ysen::lang::astvm::Scope::declare_function(FunctionPtr fn)
{
	m_functions[fn->name()] = std::move(fn);
}

void ysen::lang::astvm::Scope::declare_variable(VariablePtr var)
{
	m_variables[var->name()] = std::move(var);
}

ysen::lang::astvm::FunctionPtr ysen::lang::astvm::Scope::find_function(const core::String& name)
{
	if (m_functions.contains(name)) {
		return m_functions[name];
	}

	if (m_variables.contains(name) && m_variables[name]->value()->is_function()) {
		return nullptr;
	}

	return m_parent ? m_parent->find_function(name) : nullptr;
}

ysen::lang::astvm::VariablePtr ysen::lang::astvm::Scope::find_variable(const core::String& name)
{
	if (m_variables.contains(name)) {
		return m_variables[name];
	}

	return m_parent ? m_parent->find_variable(name) : nullptr;
}

void ysen::lang::astvm::Scope::mark_return()
{
	m_returning = true;

	if (m_scope_type == ScopeType::Returnable) {
		return;
	}

	if (m_parent != nullptr) {
		m_parent->mark_return();	
	}
}

ysen::lang::astvm::Interpreter::Interpreter()
{
	enter_scope("global");
}

ysen::lang::astvm::ValuePtr ysen::lang::astvm::Interpreter::execute(const ast::AstNode* node)
{
	return core::make_shared<Value>(node->visit(*this));
}

void ysen::lang::astvm::Interpreter::enter_scope(core::String name, ScopeType type)
{
	m_scopes.emplace_back(core::adopt_shared(new Scope{ m_scopes.empty() ? nullptr : m_scopes.back().ptr(), std::move(name), type }));
}

void ysen::lang::astvm::Interpreter::exit_scope()
{
	if (!m_scopes.empty()) {
		m_scopes.pop_back();
	}
}

void ysen::lang::astvm::Interpreter::unpack_arguments(const std::vector<Value>& arguments, const FunctionParameterList& parameters)
{
	auto index{0u};
	for (const auto& arg : arguments) {
		auto value = astvm::value(arg);
		
		if (index < parameters.size()) {
			auto name = parameters.at(index)->name();
			current_scope()->declare_variable(astvm::var(std::move(name), value));
		}

		// Always have __argc availability!
		current_scope()->declare_variable(
			astvm::var(core::format("__arg{}", index++), value)
		);
	}

	current_scope()->declare_variable(astvm::var("__argc", astvm::value(static_cast<int>(arguments.size()))));

	core::println("Unpacked {} arguments in scope '{}'", arguments.size(), current_scope()->qualified_name());
}

void ysen::lang::astvm::Interpreter::add(VariablePtr v)
{
	m_scopes[0]->declare_variable(std::move(v));
}

void ysen::lang::astvm::Interpreter::add(FunctionPtr f)
{
	m_scopes[0]->declare_function(std::move(f));
}
