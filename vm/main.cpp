#include <iostream>
#include <ysen/core/SharedPtr.h>
#include <ysen/core/NonnullOwnPtr.h>
#include <ysen/core/format.h>
#include <ysen/lang/Lexer.h>
#include <ysen/lang/ast/node.h>
#include <ysen/core/String.h>
#include <ysen/core/Optional.h>
#include <ysen/core/random.h>
#include <ysen/fs/io.h>
#include <functional>
#include "ysen/lang/Parser.h"
#include "ysen/lang/astvm/Interpreter.h"
#include "ysen/lang/astvm/Value.h"
#include <format>

using namespace ysen;
using namespace ysen::lang;

int main()
{
	auto lexer = lang::Lexer::lex(R"(
		for (var x : 0..99) {
			print("x={}", x);
		}

		var b = 5;
		var a = b;

		a = 20;
		print('a={}, b={}', a, b);
)", lang::WhitespacePolicy::Ignore, lang::CommentPolicy::Ignore);

	auto parser = core::adopt_nonnull(new lang::Parser);

	try {
		auto top_node = parser->parse(lexer->tokens());
		auto interpreter = core::make_shared<astvm::Interpreter>();

		interpreter->add(astvm::function("print", [](astvm::TreatAsArguments, const std::vector<astvm::Value>& arguments) -> int {
			const auto& fmt = arguments.at(0);
			if (!fmt.is_string() && arguments.size() > 1) {
				return 1;
			}
			core::details::FormatterContext<> context{fmt.to_string()};
			for (auto i = 1u; i < arguments.size(); ++i) {
				context.formatter_arguments().collect(arguments.at(i).to_string());
			}
			core::println(context.format());
			return 0;
		}));
		interpreter->add(astvm::function("to_string", [](const astvm::Value& value) -> core::String {
			return value.to_string();
		}));
		interpreter->add(astvm::function("to_formatted_string", [](const astvm::Value& value) -> core::String {
			return value.to_formatted_string();
		}));
		
		auto ret = interpreter->execute(top_node.ptr());
		core::println("Ret: {}", ret->to_formatted_string());
	}
	catch (lang::ParseError& parse_error) {
		core::println(parse_error.what());
		return -1;
	}

	return 0;
}
