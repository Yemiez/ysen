#pragma once
#include <functional>
#include <map>
#include "../ast/node.h"
#include "Value.h"

namespace ysen::lang::astvm {

	struct TreatAsArguments {};
	
	namespace details {
		template<typename Fty>
		struct FunctionLambda {};
	}
	
	class Interpreter;
	
	class FunctionParameter
	{
	public:
		FunctionParameter(core::String name, core::String type_name, const ast::AstNode* node);
		auto& name() const { return m_name; }
		auto& type_name() const { return m_type_name; }
		auto& ast_node() const { return m_ast_node; }
	private:
		core::String m_name{};
		core::String m_type_name{};
		const ast::AstNode* m_ast_node{};
	};
	using FunctionParameterPtr = core::SharedPtr<FunctionParameter>;
	using FunctionParameterList = std::vector<FunctionParameterPtr>;
	
	class Function
	{
	public:
		using FunctionSignature = std::function<Value(Interpreter&, const std::vector<Value>&)>;
		
	public:
		Function(core::String name, FunctionParameterList parameters, const ast::AstNode* ast_node);
		Function(core::String name, FunctionParameterList parameters, FunctionSignature function);
		
		auto& name() const { return m_name; }
		auto& parameters() const { return m_parameters; }
		auto& ast_node() const { return m_ast_node; }

		Value invoke(Interpreter&, const std::vector<Value>& arguments) const;

		template<typename Fty>
		std::function<Fty> cast(Interpreter&) const; 
	private:
		core::String m_name{};
		FunctionParameterList m_parameters{};
		const ast::AstNode* m_ast_node{};
		FunctionSignature m_callable{};
	};

	using FunctionPtr = core::SharedPtr<Function>;
	using FunctionMap = std::map<core::String, FunctionPtr>;

	class Variable
	{
	public:
		Variable(core::String name, ValuePtr value, const ast::AstNode* ast_node);

		auto& name() const { return m_name; }
		const auto& value() const { return m_value; }
		auto& value() { return m_value; }
		auto& ast_node() const { return m_ast_node; }

		void set_value(Value value);
	private:
		core::String m_name{};
		ValuePtr m_value{};
		const ast::AstNode* m_ast_node{};
	};
	using VariablePtr = core::SharedPtr<Variable>;
	using VariableMap = std::map<core::String, VariablePtr>;

	class Scope
	{
	public:
		Scope(Scope *parent, core::String name);

		auto* parent() const { return m_parent; }
		auto& name() const { return m_name; }
		core::String qualified_name() const;

		auto& functions() const { return m_functions; }
		const auto& variables() const { return m_variables; }
		auto& variables() { return m_variables; }

		void declare_function(FunctionPtr);
		void declare_variable(VariablePtr);

		FunctionPtr find_function(const core::String& name);
		VariablePtr find_variable(const core::String& name);
	private:
		Scope *m_parent{};
		core::String m_name{};
		FunctionMap m_functions{};
		VariableMap m_variables{};
	};
	using ScopePtr = core::SharedPtr<Scope>;
	using ScopeList = std::vector<ScopePtr>;

	class Interpreter
	{
	public:
		Interpreter();
		ValuePtr execute(const ast::AstNode* node);

		const auto& current_scope() const { return m_scopes.back(); }
		auto& current_scope() { return m_scopes.back(); }
		void enter_scope(core::String name);
		void exit_scope();

		// Unpacks argument list into current scope.
		void unpack_arguments(const std::vector<Value>& arguments, const FunctionParameterList& parameters);

		void add(VariablePtr);
		void add(FunctionPtr);

	private:
		ScopeList m_scopes{};
	};

	inline ValuePtr value(Value value)
	{
		return core::make_shared<Value>(std::move(value));
	}

	inline VariablePtr var(core::String name, ValuePtr value)
	{
		return core::make_shared<Variable>(std::move(name), std::move(value), nullptr);
	}

	inline FunctionPtr function(core::String name, FunctionParameterList parameters, const ast::AstNode* ast_node)
	{
		return core::make_shared<Function>(std::move(name), std::move(parameters), ast_node);
	}

	namespace details {
		template<typename Ret, typename...Args>
		struct FunctionLambda<Ret(Args...)>
		{
			using Cont = std::vector<Value>;
			
			FunctionLambda(Interpreter& vm, const Function& function)
				: vm(vm), function(function)
			{}
			
			Ret operator()(Args&&... args) const
			{
				auto cont = pack(std::forward<Args>(args)...);
				return function.invoke(vm, cont).cast<Ret>();
			}

			Interpreter& vm;
			const Function& function;

			template<typename...Ts>
			Cont pack(Ts&&...args) const
			{
				Cont cont{};
				pack_ex(cont, std::forward<Ts>(args)...);
				return cont;
			}

			template<typename Arg, typename...Rest>
			void pack_ex(Cont &cont, Arg&& arg, Rest&&...rest) const
			{
				cont.emplace_back(std::forward<Arg>(arg));
				pack_ex(cont, std::forward<Rest>(rest)...);
			}

			void pack_ex(Cont &cont) const
			{}
			
		};

		template<typename Ret, typename...Args>
		struct FunctionTraitsBase
		{
			static constexpr size_t ARITY = sizeof... (Args);
			using ReturnType = Ret;
			using Signature = ReturnType(Args...);

			template<size_t Index>
			struct ArgTypeAt_
			{
				using Type = std::tuple_element_t<Index, std::tuple<Args...>>;
			};

			template<size_t Index>
			using ArgTypeAt = typename ArgTypeAt_<Index>::Type;
		};

		template<typename T>
		struct FunctionTraits : FunctionTraits<decltype(&T::operator())> {};
		template<typename R, typename...A>
		struct FunctionTraits<R(A...)> : FunctionTraitsBase<R, A...> {};
		template<typename R, typename...A>
		struct FunctionTraits<R(A...) const> : FunctionTraitsBase<R, A...> {};
		template<typename R, typename...A>
		struct FunctionTraits<R(A...) const &> : FunctionTraitsBase<R, A...> {};
		template<typename R, typename...A>
		struct FunctionTraits<R(A...) const &&> : FunctionTraitsBase<R, A...> {};
		template<typename C, typename R, typename...A>
		struct FunctionTraits<R(C::*)(A...)> : FunctionTraits<R(A...)> {};
		template<typename C, typename R, typename...A>
		struct FunctionTraits<R(C::*)(A...) const> : FunctionTraits<R(A...)> {};
		template<typename C, typename R, typename...A>
		struct FunctionTraits<R(C::*)(A...) const &> : FunctionTraits<R(A...)> {};
		template<typename C, typename R, typename...A>
		struct FunctionTraits<R(C::*)(A...) const &&> : FunctionTraits<R(A...)> {};
		
		struct ParamBuilder
		{
			template<size_t Index, size_t Max, typename Traits>
			struct ParamBuilderEx
			{
				static void build(FunctionParameterList& list)
				{
					list.emplace_back(core::adopt_shared(new FunctionParameter(
						{}, 
						typeid(typename Traits::template ArgTypeAt<Index>).name(), 
						nullptr
					)));
					ParamBuilderEx<Index + 1, Max, Traits>::build(list);
				}
			};

			template<size_t Max, typename Traits>
			struct ParamBuilderEx<Max, Max, Traits>
			{
				static void build(FunctionParameterList& list) {}
			};
			
			template<typename Traits>
			static FunctionParameterList build()
			{
				FunctionParameterList list;
				ParamBuilderEx<0, Traits::ARITY, Traits>::build(list);
				return list;
			}
		};

		template<typename T1, typename T2 = std::remove_cvref_t<T1>>
		struct ArgumentDeducer
		{
			static T2 deduce(const Value& v)
			{
				return v.cast<T2>();
			}
		};

		template<typename T>
		struct ArgumentDeducer<T, Value>
		{
			static Value deduce(const Value& v)
			{
				return v;
			}
		};

		
		
		template<size_t Pos, size_t Size, typename Traits>
		struct Caller
		{
			template<typename Callable, typename...Deduced>
			static auto invoke(const std::vector<Value>& arguments, Callable &&callable, Deduced&&...deduced)
			{
				using ArgumentType = typename Traits::template ArgTypeAt<Pos>;
				
				return Caller<Pos + 1, Size, Traits>::invoke(
					arguments,
					std::forward<Callable>(callable),
					std::forward<Deduced>(deduced)...,
					ArgumentDeducer<ArgumentType>::deduce(arguments.at(Pos))
				);
			}
		};

		template<size_t Size, typename Traits>
		struct Caller<Size, Size, Traits>
		{
			template<typename Callable, typename...Deduced>
			static auto invoke(const std::vector<Value>&, Callable &&callable, Deduced&&...deduced)
			{
				return std::forward<Callable>(callable)(std::forward<Deduced>(deduced)...);
			}
		};
		
		struct FunctionBuilder
		{
			template<typename Callable, typename Traits = FunctionTraits<Callable>>
			static FunctionPtr build(core::String name, Callable&& callable)
			{
				return core::adopt_shared(new Function(
					std::move(name),
					ParamBuilder::build<Traits>(),
					build_lambda(std::forward<Callable>(callable))
				));
			}

			template<typename Callable, typename Traits = FunctionTraits<Callable>>
			static auto build_lambda(Callable&& callable)
			{
				return [callable = std::function<typename Traits::Signature>{callable}](Interpreter& vm, const std::vector<Value>& arguments) -> Value {
					using NewTraits = FunctionTraits<decltype(callable)>;

					if constexpr (std::is_same_v<typename NewTraits::template ArgTypeAt<0>, TreatAsArguments>) {
						return callable(TreatAsArguments{}, arguments);
					}
					if constexpr (!std::is_same_v<typename NewTraits::template ArgTypeAt<0>, TreatAsArguments>) {
						return Caller<0, NewTraits::ARITY, NewTraits>::invoke(arguments, callable);
					}
				};
			}
		};
	}

	template <typename Fty>
	std::function<Fty> Function::cast(Interpreter& vm) const
	{
		return details::FunctionLambda<Fty>{vm, *this};
	}

	template<typename Callable>
	FunctionPtr function(core::String name, Callable&& callable)
	{
		return details::FunctionBuilder::build(std::move(name), std::forward<Callable>(callable));
	}
}
