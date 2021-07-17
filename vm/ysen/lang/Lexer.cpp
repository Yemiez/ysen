#include "Lexer.h"

#include <set>
#include <ysen/core/format.h>

ysen::core::String ysen::lang::to_string(TokenType type)
{
	switch(type) {
#define __TOKEN_TYPE_ENUMERATOR(Camel, ...) case TokenType::Camel: return #Camel; 
	TOKEN_TYPE_ENUMERATOR
#undef __TOKEN_TYPE_ENUMERATOR
	}

	return {};
}

ysen::lang::SourcePosition::SourcePosition() = default;
ysen::lang::SourcePosition::SourcePosition(uint32_t row, uint32_t column)
	: m_row(row), m_column(column)
{}

ysen::core::String ysen::lang::SourcePosition::to_string() const
{
	return core::format("{{ row={}, col={} }}", row(), column());
}

void ysen::lang::SourcePosition::set_row(uint32_t row)
{
	m_row = row;
}

void ysen::lang::SourcePosition::set_column(uint32_t column)
{
	m_column = column;
}

ysen::lang::SourcePosition ysen::lang::SourcePosition::increment() const
{
	return { m_row, m_column + 1 };
}

ysen::lang::SourcePosition ysen::lang::SourcePosition::new_line() const
{
	return { m_row + 1, 0 };
}

ysen::lang::SourceRange::SourceRange(SourcePosition start, SourcePosition end)
	: m_start_position(start), m_end_position(end)
{}

ysen::core::String ysen::lang::SourceRange::to_string() const
{
	return core::format("{}:{}-{}:{}", start_position().row(), start_position().column(), end_position().row(), end_position().column());
}

ysen::lang::Token::Token(SourcePosition start, SourcePosition end, TokenType type, core::String content)
	: m_start_position(start), m_end_position(end), m_type(type), m_content(std::move(content))
{}

ysen::core::String ysen::lang::Token::to_string() const
{
	return core::format("Token{{ '{}', {}, start={}, end={} }}", content(), lang::to_string(type()), start_position().to_string(), end_position().to_string());
}

ysen::core::NonnullOwnPtr<ysen::lang::Lexer> ysen::lang::Lexer::lex(core::String code, WhitespacePolicy whitespace_policy, CommentPolicy comment_policy)
{
	auto lexer = core::adopt_nonnull(new Lexer());
	lexer->lex_impl(std::move(code), whitespace_policy, comment_policy);
	return lexer;
}

bool ysen::lang::Lexer::eof(int offset) const
{
	return m_cursor + offset >= m_code.length();
}

void ysen::lang::Lexer::lex_ws()
{
	auto [start, end, content] = consume_while(core::is_whitespace);

	if (m_whitespace_policy == WhitespacePolicy::Keep) {
		emit_token(start, end, TokenType::Whitespace, std::move(content));
	}
}

void ysen::lang::Lexer::lex_simple_comment()
{
	auto [start, end, content] = consume_while([](auto&& c) {
		return c != '\n';
	});

	if (comment_policy() == CommentPolicy::Keep) {
		emit_token(start, end, TokenType::SimpleComment, std::move(content));
	}
}

void ysen::lang::Lexer::lex_multiline_comment()
{
	auto [start, _, content] = consume_while([&](auto&& c) {
		return c != '*' && peek(1) != '/';
	});
	consume();

	if (comment_policy() == CommentPolicy::Keep) {
		emit_token(start, m_position, TokenType::MultilineComment, std::move(content));
	}
}

void ysen::lang::Lexer::lex_id()
{
	static std::set<core::String> keyword_set{
		"var",
		"if",
		"else",
		"while",
		"for",
		"class",
		"fun",
		"ret",
		"int",
		"float",
		"string",
		"continue",
		"break",
		"require",
		"true",
		"false",
	};
	
	// We already know the very first character is either alphabetical or _
	// Hence we can check alpha_numeric here, since numbers are allowed anywhere
	// but at the start of an identifier
	auto [start, end, content] = consume_while([](auto&& c) {
		return c == '_' || core::is_alpha_numeric(c);
	});

	auto type = TokenType::Identifier;
	if (keyword_set.contains(content)) {
		type = TokenType::Keyword;
	}

	emit_token(start, end, type, std::move(content));
}

void ysen::lang::Lexer::lex_number()
{
	auto start = m_position;
	core::String content{};
	while (!eof()) {
		auto ch = peek();

		if (ch == '.' && peek(1) == '.') {
			break;
		}
		if (ch == '.' || core::is_numeric(ch)) {
			content.push(consume());
		}
		else {
			break;
		}
	}
	auto end = m_position;

	auto type = TokenType::Integer;
	if (content.contains('.')) {
		type = TokenType::FloatingPointNumber;
	}

	emit_token(start, end, type, std::move(content));
}

void ysen::lang::Lexer::lex_string()
{
	auto start = m_position;
	core::String content{};
	auto delim = consume();

	while (!eof()) {
		if (peek() == '\\') {
			switch (peek(1)) {
			case '\'': 
				content.push('\'');
				break;
			case '"': 
				content.push('\'');
				break;
			case 'n':
				content.push('\n');
				break;
			case 't':
				content.push('\t');
				break;
			case 'r':
				content.push('\r');
				break;
			default:
				break;
			}

			consume(); consume(); // consume x2
			continue;
		}
		else if (peek() == delim) {
			consume();
			break;
		}

		content.push(consume());
	}

	emit_token(start, m_position, TokenType::String, std::move(content));
}

void ysen::lang::Lexer::lex_other()
{
	switch (peek()) {
	case '.':
		emit_token(m_position, m_position.increment(), TokenType::Dot, consume());
		break;
	case ',':
		emit_token(m_position, m_position.increment(), TokenType::Comma, consume());
		break;
		
	case '+':
	case '-':
	case '*':
	case '/':
	case '%':
	case '>':
	case '<':
		if (!eof(1) && peek(1) == '=') {
			core::String op{};
			auto start = m_position;
			op.push(consume());
			op.push(consume());
			emit_token(start, m_position, TokenType::BinOp, std::move(op));
			break;
		}
		
		emit_token(m_position, m_position.increment(), TokenType::BinOp, consume());
		break;

	case '=':
		emit_token(m_position, m_position.increment(), TokenType::Equals, consume());
		break;

	case ':':
		emit_token(m_position, m_position.increment(), TokenType::Colon, consume());
		break;
	case ';':
		emit_token(m_position, m_position.increment(), TokenType::SemiColon, consume());
		break;

	case '(':
		emit_token(m_position, m_position.increment(), TokenType::ParenOpen, consume());
		break;
	case ')':
		emit_token(m_position, m_position.increment(), TokenType::ParenClose, consume());
		break;
	case '{':
		emit_token(m_position, m_position.increment(), TokenType::SquigglyOpen, consume());
		break;
	case '}':
		emit_token(m_position, m_position.increment(), TokenType::SquigglyClose, consume());
		break;

	case '[':
		emit_token(m_position, m_position.increment(), TokenType::BracketOpen, consume());
		break;
	case ']':
		emit_token(m_position, m_position.increment(), TokenType::BracketClose, consume());
		break;

	default:
		emit_token(m_position, m_position.increment(), TokenType::Unknown, consume());
	}
}

void ysen::lang::Lexer::lex_impl(core::String code, WhitespacePolicy whitespace_policy, CommentPolicy comment_policy)
{
	m_whitespace_policy = whitespace_policy;
	m_comment_policy = comment_policy;
	m_code = std::move(code);
	m_cursor = 0;
	m_tokens.clear();
	m_position = {};

	while (!eof()) {
		auto ch = peek();
		if (core::is_whitespace(ch)) {
			lex_ws();
		}
		else if (core::is_alphabetical(ch) || ch == '_') {
			lex_id();
		}
		else if (core::is_numeric(ch)) {
			lex_number();
		}
		else if (ch == '\'') {
			lex_string();
		}
		else if (ch == '/' && peek(1) == '/') {
			lex_simple_comment();
		}
		else if (ch == '/' && peek(1) == '*') {
			lex_multiline_comment();
		}
		else if (ch == '\"' || ch == '\'') {
			lex_string();
		}
		else {
			lex_other();
		}
	}
}

char ysen::lang::Lexer::peek(int offset) const
{
	return m_code.at(m_cursor + offset);
}

char ysen::lang::Lexer::consume()
{
	if (eof()) return 0;
	
	auto c = peek();
	m_position = c == '\n'
		? m_position.new_line()
		: m_position.increment();
	
	++m_cursor;
	return c;
}

ysen::lang::Token& ysen::lang::Lexer::emit_token(SourcePosition start, SourcePosition end, TokenType type, core::String content)
{
	return m_tokens.emplace_back(start, end, type, std::move(content));
}

ysen::lang::Token& ysen::lang::Lexer::emit_token(SourcePosition start, SourcePosition end, TokenType type, char c)
{
	core::String content{};
	content.push(c);
	return m_tokens.emplace_back(start, end, type, std::move(content));
}
