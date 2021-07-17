#include "ScriptEnvironment.h"
#include "Parser.h"
#include "astvm/Interpreter.h"
#include "ysen/fs/io.h"

ysen::lang::ScriptEnvironment::ScriptEnvironment()
	: m_interpreter(core::adopt_shared(new astvm::Interpreter{}))
{
	m_interpreter->add(astvm::function("print", [](astvm::VariadicFunction, const std::vector<astvm::Value>& arguments) -> int {
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
	m_interpreter->add(astvm::function("to_string", [](const astvm::Value& value) -> core::String {
		return value.to_string();
	}));
	m_interpreter->add(astvm::function("to_formatted_string", [](const astvm::Value& value) -> core::String {
		return value.to_formatted_string();
	}));
}

ysen::lang::astvm::ValuePtr ysen::lang::ScriptEnvironment::eval(const core::String& code)
{
	auto lexer = Lexer::lex(code);
	auto parser = core::adopt_nonnull(new Parser);
	auto program = parser->parse(lexer->tokens());
	return m_interpreter->execute(program.ptr());
}

ysen::lang::astvm::ValuePtr ysen::lang::ScriptEnvironment::eval_file(const core::String& filename)
{
	auto file_or_error = fs::read_file(filename);
	if (file_or_error.has_value()) {
		return eval(file_or_error.value());
	}

	return nullptr;
}
