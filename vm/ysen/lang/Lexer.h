#pragma once
#include <string_view>
#include <vector>


#include "ysen/core/NonnullOwnPtr.h"
#include "ysen/Core/String.h"

namespace ysen::lang {

#define TOKEN_TYPE_ENUMERATOR \
	__TOKEN_TYPE_ENUMERATOR(None, none) \
	__TOKEN_TYPE_ENUMERATOR(Unknown, unknown) \
	__TOKEN_TYPE_ENUMERATOR(Whitespace, whitespace) \
	__TOKEN_TYPE_ENUMERATOR(SimpleComment, simple_comment) \
	__TOKEN_TYPE_ENUMERATOR(MultilineComment, multiline_comment) \
	__TOKEN_TYPE_ENUMERATOR(Identifier, identifier) \
	__TOKEN_TYPE_ENUMERATOR(Keyword, keyword) \
	__TOKEN_TYPE_ENUMERATOR(String, string) \
	__TOKEN_TYPE_ENUMERATOR(Integer, integer) \
	__TOKEN_TYPE_ENUMERATOR(FloatingPointNumber, floating_point_number) \
	__TOKEN_TYPE_ENUMERATOR(SemiColon, semi_colon) \
	__TOKEN_TYPE_ENUMERATOR(Colon, colon) \
	__TOKEN_TYPE_ENUMERATOR(Equals, equals) \
	__TOKEN_TYPE_ENUMERATOR(ParenOpen, paren_open) \
	__TOKEN_TYPE_ENUMERATOR(ParenClose, paren_close) \
	__TOKEN_TYPE_ENUMERATOR(SquigglyOpen, squiggly_open) \
	__TOKEN_TYPE_ENUMERATOR(SquigglyClose, squiggly_close) \
	__TOKEN_TYPE_ENUMERATOR(BracketOpen, bracket_open) \
	__TOKEN_TYPE_ENUMERATOR(BracketClose, bracket_close) \
	__TOKEN_TYPE_ENUMERATOR(Comma, comma) \
	__TOKEN_TYPE_ENUMERATOR(BinOp, bin_op) \
	__TOKEN_TYPE_ENUMERATOR(Dot, dot) \
	
	enum class TokenType
	{
#define __TOKEN_TYPE_ENUMERATOR(c, ...) c,
		TOKEN_TYPE_ENUMERATOR
#undef __TOKEN_TYPE_ENUMERATOR
	};

	static core::String to_string(TokenType);
	
	class SourcePosition
	{
	public:
		SourcePosition();
		SourcePosition(uint32_t, uint32_t);

		core::String to_string() const;
		
		void set_row(uint32_t);
		void set_column(uint32_t);
		const auto& row() const { return m_row; }
		const auto& column() const { return m_column; }

		SourcePosition increment() const;
		SourcePosition new_line() const;
	private:
		uint32_t m_row{0},
			m_column{0};
	};

	class SourceRange
	{
	public:
		SourceRange() = default;
		SourceRange(SourcePosition, SourcePosition);

		SourcePosition start_position() const { return m_start_position; }
		SourcePosition end_position() const { return m_end_position; }

		core::String to_string() const;
	private:
		SourcePosition m_start_position{};
		SourcePosition m_end_position{};
	};
	
	class Token
	{
	public:
		Token() = default;
		Token(SourcePosition, SourcePosition, TokenType, core::String);

		core::String to_string() const;
		
		SourcePosition start_position() const { return m_start_position; }
		SourcePosition end_position() const { return m_end_position; }
		SourceRange source_range() const { return { start_position(), end_position() }; }
		TokenType type() const { return m_type; }
		const core::String& content() const { return m_content; }

#define __TOKEN_TYPE_ENUMERATOR(camel, snake) bool is_##snake() const { return m_type == TokenType::camel; } 
		TOKEN_TYPE_ENUMERATOR
#undef __TOKEN_TYPE_ENUMERATOR

		template<typename...Ts>
		bool is_any_of(const Ts&...ts) const
		{
			return ((ts == m_type) || ...);
		}
	private:
		SourcePosition m_start_position{};
		SourcePosition m_end_position{};
		TokenType m_type{};
		core::String m_content{};
	};

	enum class WhitespacePolicy
	{
		Ignore,
		Keep
	};

	enum class CommentPolicy
	{
		Ignore,
		Keep
	};
	
	class Lexer
	{
	private:
		Lexer() = default;
	public:
		static core::NonnullOwnPtr<Lexer> lex(core::String, WhitespacePolicy = WhitespacePolicy::Ignore, CommentPolicy = CommentPolicy::Ignore);

		const auto& tokens() const { return m_tokens; }
		const auto& code() const { return m_code; }
		auto whitespace_policy() const { return m_whitespace_policy; }
		auto comment_policy() const { return m_comment_policy; }
		
	private:
		bool eof(int offset = 0) const;
		void lex_ws();

		void lex_simple_comment();
		void lex_multiline_comment();
		void lex_id();
		void lex_number();
		void lex_string();
		void lex_other();
		void lex_impl(core::String, WhitespacePolicy, CommentPolicy);
		char peek(int offset = 0) const;
		char consume();
		
		template<typename Callable>
		std::tuple<SourcePosition, SourcePosition, core::String> consume_while(Callable&& pred)
		{
			auto start = m_position;
			core::String content;
			do {
				content.push(consume());
			} while(!eof() && pred(peek()));
			return { start, m_position, content };
		}

		Token& emit_token(SourcePosition, SourcePosition, TokenType, core::String);
		Token& emit_token(SourcePosition, SourcePosition, TokenType, char);
	private:
		core::String m_code{};
		std::vector<Token> m_tokens{};
		SourcePosition m_position{};
		size_t m_cursor{};
		WhitespacePolicy m_whitespace_policy{};
		CommentPolicy m_comment_policy{};
	};
	
}
