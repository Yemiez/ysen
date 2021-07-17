#include <format>
#include <functional>
#include <iostream>
#include <ysen/core/format.h>
#include <ysen/core/NonnullOwnPtr.h>
#include <ysen/core/Optional.h>
#include <ysen/core/random.h>
#include <ysen/core/SharedPtr.h>
#include <ysen/core/String.h>
#include <ysen/fs/io.h>
#include <ysen/lang/Lexer.h>
#include <ysen/lang/ast/node.h>
#include "ysen/lang/Parser.h"
#include "ysen/lang/astvm/Interpreter.h"
#include "ysen/lang/astvm/Value.h"

#include "ysen/lang/ScriptEnvironment.h"
#include "ysen/lang/bytecode/BytecodeInterpreter.h"
#include "ysen/lang/bytecode/Instruction.h"

using namespace ysen;
using namespace ysen::lang;

void bytecode_test(const char* code)
{
	bytecode::Generator generator;
	auto lexer = Lexer::lex(code);
	Parser p;
	auto node = p.parse(lexer->tokens());
	node->generate_bytecode(generator);

	bytecode::BytecodeInterpreter interpreter;
	auto result = interpreter.execute(generator.program());
	core::println("Exec result: {}", result.to_formatted_string());
}

void ast_test(const char* code)
{
	auto env = core::adopt_nonnull(new ScriptEnvironment);
	core::println("Exec result: {}", env->eval(code)->to_formatted_string());
}

int main()
{
	try {
		auto code = R"(
var a = 5 + 5;
var b = a + 10;

fun testing(a, b) {
	if (a >= 10) {
		ret (a / 2) + b;
	}
	
	ret a + b
}

ret testing(a, b);
)";
		
		bytecode_test(code);
	}
	catch (lang::ParseError& parse_error) {
		core::println(parse_error.what());
		return -1;
	}

	return 0;
}
