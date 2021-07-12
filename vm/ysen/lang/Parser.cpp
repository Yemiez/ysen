#include <ysen/lang/Parser.h>
#include <ysen/lang/ast/node.h>

#include "ysen/core/format.h"

ysen::lang::ast::ProgramPtr ysen::lang::Parser::parse(std::vector<Token> tokens)
{
	m_tokens = std::move(tokens);
	m_root_node = core::adopt_shared(new ast::Program{{}});

	parse_inner_block();
	return m_root_node;
}

bool ysen::lang::Parser::eof(int offset) const
{
	return m_cursor + offset >= m_tokens.size();
}

void ysen::lang::Parser::unwind()
{
	if (m_cursor != 0) {
		--m_cursor;		
	}
}

ysen::lang::Token ysen::lang::Parser::peek(int offset) const
{
	return m_tokens[m_cursor + offset];
}
ysen::lang::Token ysen::lang::Parser::consume()
{
	auto tok = peek();
	++m_cursor;
	return tok;
}

ysen::lang::ast::ExpressionPtr ysen::lang::Parser::parse_function_call(const Token& token)
{
	consume(); // consume (

	std::vector<ast::ExpressionPtr> arguments{};
	while (!eof() && !peek().is_paren_close()) {
		arguments.emplace_back(parse_expression());

		if (peek().is_paren_close()) {
			break;
		}

		if (peek().is_comma()) {
			consume();
			continue;
		}

		throw ParseError("Unexpected token", peek());
	}

	consume(); // Consume )

	return core::dynamic_shared_cast<ast::Expression>(
		core::adopt_shared(
			new ast::FunctionCallExpression(
				{ token.start_position(), peek(-1).end_position() }, token.content(), std::move(arguments)
			)
		)
	);
}

ysen::lang::ast::ExpressionPtr ysen::lang::Parser::parse_array_or_object()
{
	auto start = consume().start_position();

	std::vector<ast::ExpressionPtr> expressions{};
	std::vector<ast::KeyValueExpressionPtr> object_map{};
	bool is_object{false};
	while (!eof() && !peek().is_bracket_close()) {
		auto expr = parse_expression();

		if (eof()) {
			throw ParseError("Unexpected EOF when parsing array or object", peek(-1));
		}

		if (is_object || peek().is_colon()) {
			if (!peek().is_colon()) {
				throw ParseError("Cannot mix array and object notation", peek());
			}
			
			is_object = true;
			consume(); // :
			auto value = parse_expression();
			object_map.emplace_back(core::adopt_shared(new ast::KeyValueExpression({}, std::move(expr), std::move(value))));
		}
		else {
			expressions.emplace_back(std::move(expr));
		}

		if (peek().is_comma()) {
			consume();
		}
		else if (!peek().is_bracket_close()) {
			throw ParseError("Unexpected token when parsing array or object", peek());
		}
	}
	 
	if (eof()) {
		throw ParseError("Unexpected EOF when parsing array or object", peek(-1));
	}
	
	if (!peek().is_bracket_close()) {
		throw ParseError("Unexpected token when parsing array or object", peek());
	}

	auto end = consume().end_position(); // ]

	if (is_object) {
		return core::dynamic_shared_cast<ast::Expression>(
			core::adopt_shared(new ast::ObjectExpression({start, end}, std::move(object_map)))
		);
	}

	return core::dynamic_shared_cast<ast::Expression>(
		core::adopt_shared(new ast::ArrayExpression{{start, end}, std::move(expressions)})	
	);
}

ysen::lang::ast::ExpressionPtr ysen::lang::Parser::parse_factor()
{
	auto token = consume();
	if (token.is_integer()) {
		auto number = token.content().to_integer();

		if (peek().is_dot() && !eof(1) && peek(1).is_dot() && !eof(2) && peek(2).is_integer()) {
			consume(); // ..
			consume();
			auto max_tok = consume();

			return core::dynamic_shared_cast<ast::Expression>(
				core::adopt_shared(new ast::NumericRangeExpression(
					{token.start_position(), max_tok.end_position()},
					number,
					max_tok.content().to_integer()
				))
			);
		}

		return core::dynamic_shared_cast<ast::Expression>(
			core::adopt_shared(new ast::IntegerExpression(token.source_range(), number))
		);
	}
	else if (token.is_floating_point_number()) {
		auto number = token.content().to_float();

		return core::dynamic_shared_cast<ast::Expression>(
			core::adopt_shared(new ast::FloatExpression(token.source_range(), number))
		);
	}
	else if (token.is_string()) {
		return core::dynamic_shared_cast<ast::Expression>(
			core::adopt_shared(new ast::StringExpression(token.source_range(), token.content()))
		);
	}
	else if (token.is_paren_open()) {
		auto node = parse_expression();
		auto ending_paren = consume();
		return node;
	}
	else if (token.is_identifier()) {
		// TODO: Check here if function call!
		if (!eof() && peek().is_paren_open()) {
			return parse_function_call(token);
		}

		if (!eof() && peek().is_dot() && !eof(1) && peek(1).is_identifier()) {
			SourceRange range{token.start_position(), peek(1).end_position()};
			consume(); // .
			auto field = consume();
			return core::dynamic_shared_cast<ast::Expression>(
				core::adopt_shared(new ast::AccessExpression(range, token.content(), field.content()))
			);
		}

		return core::dynamic_shared_cast<ast::Expression>(
			core::adopt_shared(new ast::IdentifierExpression(token.source_range(), token.content()))
		);
	}
	else if (token.is_string()) {
		return core::dynamic_shared_cast<ast::Expression>(
			core::adopt_shared(new ast::StringExpression(token.source_range(), token.content()))
		);
	}
	else if (token.is_squiggly_open()) {
		auto scope = core::adopt_shared(new ast::ScopeStatement{ {} });

		while (!eof() && !peek().is_squiggly_close()) {
			auto expr = parse_statement_or_expression();

			if (!expr) {
				continue;
			}

			scope->emit(core::reinterpret_shared_cast<ast::Statement>(expr));
		}

		if (eof()) {
			throw ParseError("Unexpected EOF when parsing '{' block", peek(-1));
		}

		if (!peek().is_squiggly_close()) {
			throw ParseError("Unexpected token when expected closing '}'", peek());
		}
 
		consume(); // }
		return core::dynamic_shared_cast<ast::Expression>(scope);
	}
	else if (token.is_keyword() && token.content() == "ret") {
		auto expr = parse_expression();
		return core::dynamic_shared_cast<ast::Expression>(
			core::adopt_shared(new ast::ReturnExpression({ token.start_position(), expr->source_range().end_position() }, std::move(expr)))
		);
	}
	else if (token.is_keyword() && token.content() == "fun") {
		// Parse function
		unwind();
		return parse_fun_decl_or_expr();
	}
	else if (token.is_bracket_open()) {
		unwind();
		return parse_array_or_object();
	}

	throw ParseError("Unknown token when parsing factor", token);
}

ysen::lang::ast::ExpressionPtr ysen::lang::Parser::parse_term()
{
	auto node = parse_factor();

	while (!eof() && peek().is_bin_op() && peek().content().is_equal_to_any_of("*", "/")) {
		auto token = consume();
		auto op{ ast::BinOp::Division };

		if (token.content() == "*") {
			op = ast::BinOp::Multiplication;
		}

		node = core::dynamic_shared_cast<ast::Expression>(
			core::adopt_shared(new ast::BinOpExpression(token.source_range(), node, parse_factor(), op))
		);
	}

	return node;
}

ysen::lang::ast::ExpressionPtr ysen::lang::Parser::parse_expression()
{
	auto node = parse_term();

	while (!eof() && peek().is_bin_op()) {
		auto token = consume();
		ast::BinOp op{};

		if (token.content() == "+") {
			op = ast::BinOp::Addition;
		}
		else if (token.content() == "-") {
			op = ast::BinOp::Subtraction;
		}

		node = core::dynamic_shared_cast<ast::Expression>(
			core::adopt_shared(new ast::BinOpExpression(token.source_range(), node, parse_term(), op))
		);
	}

	return node;
}

ysen::lang::ast::ExpressionPtr ysen::lang::Parser::parse_var_declaration()
{
	auto start = peek().start_position();
	consume();

	if (!peek().is_identifier()) {
		throw ParseError("var without identifier", peek());
	}

	auto name = consume().content();

	if (peek().is_semi_colon() || peek().is_colon()) {
		return core::dynamic_shared_cast<ast::Expression>(
			core::adopt_shared(new ast::VarDeclaration(SourceRange{ start, consume().end_position() }, std::move(name)))
		);
	}

	consume(); // Consume '='
	auto expression = parse_expression();
	auto declaration = core::adopt_shared(
		new ast::VarDeclaration{ {start, expression->source_range().end_position()}, std::move(name), expression }
	);

	return core::dynamic_shared_cast<ast::Expression>(declaration);
}

ysen::lang::ast::ExpressionPtr ysen::lang::Parser::parse_fun_decl_or_expr()
{
	auto start = consume().start_position();
	core::String name{};
	auto is_anon_expr = false;

	if (!peek().is_identifier()) {
		is_anon_expr = true;
	}
	else {
		name = consume().content();
	}

	if (!peek().is_paren_open()) {
		throw ParseError("fun without paren", peek());
	}

	consume();

	std::vector<ast::FunctionParameterExpressionPtr> params{};
	while (!eof() && !peek().is_paren_close()) {
		if (!peek().is_identifier()) {
			error("Unknown token in param list");
			throw ParseError(core::format("Unknown token in param list"), peek());
		}
		auto name_tok = consume();
		auto parameter_name = name_tok.content();
		core::String type_name{};
		auto end_tok = name_tok;
		
		if (eof()) {
			throw ParseError("Unexpected EOF", peek(-1));
		}

		if (peek().is_colon() && peek(1).is_any_of(TokenType::Keyword, TokenType::Identifier)) {
			consume(); // :
			end_tok = consume();
			type_name = end_tok.content();
		}

		params.emplace_back(core::adopt_shared(new ast::FunctionParameterExpression{
			{name_tok.start_position(), end_tok.end_position()},
			std::move(parameter_name),
			std::move(type_name),
			false
		}));

		if (peek().is_comma()) {
			consume();		
		}
	}

	if (eof()) {
		throw ParseError("Unexpected EOF after param list", peek(-1));
	}

	if (!peek().is_paren_close()) {
		throw ParseError("Expected ) after parameter list", peek());
	}
	
	consume(); // consume )

	auto body = parse_statement_or_expression();

	if (is_anon_expr) {
		return core::dynamic_shared_cast<ast::Expression>(
			core::adopt_shared(
				new ast::FunctionExpression(
					{ start, body->source_range().end_position() },
					std::move(params),
					body
				)
			)
		);
	}
	
	return core::dynamic_shared_cast<ast::Expression>(
		core::adopt_shared(
			new ast::FunctionDeclarationStatement(
				{ start, body->source_range().end_position() },
				std::move(name),
				std::move(params),
				body
			)
		)
	);
}

ysen::lang::ast::ExpressionPtr ysen::lang::Parser::parse_for_ranged_or_conditional()
{
	auto start_pos = consume().start_position(); // for

	if (eof()) {
		throw ParseError("Unexpected EOF", peek(-1));
	}
	if (!peek().is_paren_open()) {
		throw ParseError("Unexpected token when expecting opening parentheses", peek());
	}

	consume();

	auto decl = parse_var_declaration();

	/*
	// TODO: this isn't needed for now because of a quick hack in parse_var_declaration()
	if (!peek().is_colon()) {
		throw ParseError("Unexpected token when expecting colon", peek());	
	}
	consume(); // :
	*/
	
	auto expr = parse_expression();

	if (!peek().is_paren_close()) {
		throw ParseError("Unexpected token when expecting closing parentheses", peek());
	}
	auto paren_close = consume(); // )
	
	auto body = parse_statement_or_expression();

	return core::dynamic_shared_cast<ast::Expression>(
		core::adopt_shared(new ast::RangedLoopExpression({start_pos, paren_close.end_position()}, decl, expr, body))
	);
}

ysen::lang::ast::ExpressionPtr ysen::lang::Parser::parse_assignment()
{
	auto start_pos = peek().start_position();
	auto identifier = consume().content();
	consume(); // =
	auto expr = parse_expression();

	return core::dynamic_shared_cast<ast::Expression>(
		core::adopt_shared(new ast::AssignmentExpression({start_pos, expr->source_range().end_position()}, identifier, expr))
	);
}

ysen::lang::ast::ExpressionPtr ysen::lang::Parser::parse_statement_or_expression()
{
	if (peek().is_keyword() && peek().content() == "var") {
		return parse_var_declaration();
	}
	if (peek().is_keyword() && peek().content() == "fun" && peek(1).is_identifier()) {
		return parse_fun_decl_or_expr();
	}
	if (peek().is_keyword() && peek().content() == "for") {
		return parse_for_ranged_or_conditional();
	}
	if (peek().is_semi_colon()) {
		consume();
		return {};
	}
	if (peek().is_identifier() && peek(1).is_equals()) {
		return parse_assignment();
	}

	auto expression = parse_expression();
	return expression;
}

void ysen::lang::Parser::parse_inner_block()
{
	while (!eof()) {
		if (peek().is_semi_colon()) {
			consume();
		}
		else if (auto node = parse_statement_or_expression(); node) {
			m_root_node->emit(core::reinterpret_shared_cast<ast::AstNode>(node));
		}
	}
}

void ysen::lang::Parser::error(core::StringView error)
{
	core::print("Error: {}\n", error.c_str());
}
