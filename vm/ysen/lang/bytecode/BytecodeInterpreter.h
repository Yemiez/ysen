#pragma once
#include <stack>
#include <vector>

#include "Instruction.h"
#include "ysen/core/Optional.h"
#include "ysen/lang/astvm/Value.h"

namespace ysen::lang::bytecode {
	class Block;
	class ExecutableProgram;
	class Register;

	class BytecodeInterpreter
	{
	public:
		astvm::Value execute(const ExecutableProgram& program, const Block* entry_point = nullptr);

		astvm::Value& variable(const core::String& name);
		astvm::Value& accumulator() { return m_accumulator; }
		astvm::Value& register_value(const Register&);

		void set_jump_point(size_t);
		void set_call(const core::String& name);

		void push();
		void pop();
		void pop_stack_frame();
	private:
		const ExecutableProgram* m_executable_program{};
		std::unordered_map<size_t, astvm::Value> m_registers{};
		astvm::Value m_accumulator{};
		core::Optional<size_t> m_jump{};
		core::Optional<const Block*> m_call{};
		std::unordered_map<core::String, astvm::Value> m_variables{};
		struct StackFrame
		{
			const Block* block{};
			InstructionIterator pc{};
		};
		
		std::stack<StackFrame> m_stack_frame{};
		std::stack<astvm::Value> m_stack{};
		
	};
	
}
