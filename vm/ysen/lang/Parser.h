#pragma once
#include <stack>
#include <vector>


#include "Lexer.h"
#include "ast/node.h"
#include "ysen/core/format.h"
#include "ysen/core/SharedPtr.h"
#include "ysen/core/StringView.h"

namespace ysen::lang {
	class Token;

	class Parser
	{
	public:
		Parser() = default;

		ast::ProgramPtr parse(std::vector<Token>);

	private:
		bool eof(int offset = 0) const;
		void unwind();
		Token peek(int offset = 0) const;
		Token consume();

		ast::ExpressionPtr parse_function_call(const Token& token);
		ast::ExpressionPtr parse_array_or_object();
		ast::ExpressionPtr parse_factor();
		ast::ExpressionPtr parse_term();
		ast::ExpressionPtr parse_expression();
		ast::ExpressionPtr parse_var_declaration();
		ast::ExpressionPtr parse_fun_decl_or_expr();
		ast::ExpressionPtr parse_for_ranged_or_conditional();
		ast::ExpressionPtr parse_assignment();
		ast::ExpressionPtr parse_statement_or_expression();
		void parse_inner_block();


		void error(core::StringView);
	private:
		ast::ProgramPtr m_root_node{nullptr};
		std::vector<Token> m_tokens{};
		size_t m_cursor{};
	};

	class ParseError : public std::exception
	{
	public:
		ParseError(core::String message, Token token)
			: m_message(core::format("ParseError: '{}' at token {}", message, token.to_string())),
			m_token(token)
		{}

		char const* what() const override
		{
			return m_message.c_str();
		}

	private:
		core::String m_message{};
		Token m_token{};
	};
	
}
