#include "BytecodeInterpreter.h"

#include <format>

#include "Register.h"
#include "ysen/lang/ast/node.h"

ysen::lang::astvm::Value ysen::lang::bytecode::BytecodeInterpreter::execute(const ExecutableProgram& program, const Block* entry_point)
{
	if (entry_point == nullptr) {
		entry_point = program.block_by_name("main");
	}

	if (!entry_point) {
		return astvm::undefined();
	}

	m_executable_program = &program;
	m_stack_frame.push({ entry_point, entry_point->instructions().begin() });
	while (!m_stack_frame.empty() && m_stack_frame.top().pc != m_stack_frame.top().block->instructions().end()) {
		const auto *block = m_stack_frame.top().block;
		auto& pc = m_stack_frame.top().pc;

		core::String s{std::format("{:20}\t\t\tacc={}", (*pc)->to_string().c_str(), accumulator().to_formatted_string().c_str()).c_str()};
		core::println(s);

		auto b = m_stack_frame.size();
		(*pc)->execute(*this);

		if (b != m_stack_frame.size()) {
			continue; // a ret
		}
		
		++pc;

		if (m_jump.has_value()) {
			pc = block->instructions().begin() + m_jump.release_value();
		}
		else if (m_call.has_value()) {
			const auto* frame_block = m_call.release_value();
			m_stack_frame.push({ frame_block, frame_block->instructions().begin() });
		}
	}

	return accumulator();
}

ysen::lang::astvm::Value& ysen::lang::bytecode::BytecodeInterpreter::variable(const core::String& name)
{
	return m_variables[name];
}

ysen::lang::astvm::Value& ysen::lang::bytecode::BytecodeInterpreter::register_value(const Register& reg)
{
	return m_registers[reg.index()];
}

void ysen::lang::bytecode::BytecodeInterpreter::set_jump_point(size_t jump)
{
	m_jump = jump;
}

void ysen::lang::bytecode::BytecodeInterpreter::set_call(const core::String& name)
{
	m_call = m_executable_program->block_by_name(name);
}

void ysen::lang::bytecode::BytecodeInterpreter::push()
{
	m_stack.push(m_accumulator);	
}

void ysen::lang::bytecode::BytecodeInterpreter::pop()
{
	m_accumulator = m_stack.top();
	m_stack.pop();
}

void ysen::lang::bytecode::BytecodeInterpreter::pop_stack_frame()
{
	m_stack_frame.pop();
}
