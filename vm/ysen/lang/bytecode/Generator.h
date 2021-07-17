#pragma once
#include <stack>
#include <vector>


#include "Instruction.h"
#include "Label.h"
#include "Register.h"

namespace ysen::lang::bytecode {

	enum class BlockType
	{
		Returnable,
		Loopable,
		Other,
	};
	
	class Block
	{
	public:
		Block(core::String name, BlockType type)
			: m_label(0, std::move(name)), m_type(type)
		{}
		Block(Label label, BlockType type)
			: m_label(std::move(label)), m_type(type)
		{}

		template<typename T, typename...Ts>
		T& emit(Ts&&...ts)
		{
			auto ptr = new T(std::forward<Ts>(ts)...);
			m_instructions.emplace_back(core::adopt_shared(static_cast<Instruction*>(ptr)));
			return *ptr;
		}

		Instruction& emit(InstructionPtr);
		core::String to_string() const;

		const InstructionList& instructions() const { return m_instructions; }
		const auto& name() const { return m_label.name(); }
		BlockType type() const { return m_type; }

		Block& emit_sub_block(BlockType = BlockType::Other);
	private:
		Label create_sub_label() const;
	private:
		Label m_label;
		InstructionList m_instructions{};
		BlockType m_type{};
		std::vector<core::SharedPtr<Block>> m_children{};
	};

	class ExecutableProgram
	{
	public:
		ExecutableProgram() = default;

		Block& current_block() { return *m_working_stack.top(); }
		Block& emit_block(core::String name, BlockType = BlockType::Other);
		void end_block();
		const auto& blocks() const { return m_blocks; }
		const Block* block_by_name(const core::String& name) const;
		core::String to_string() const;
	private:
		std::stack<core::SharedPtr<Block>> m_working_stack{};
		std::vector<core::SharedPtr<Block>> m_blocks;
	};

	class Generator
	{
	public:
		const auto& registers() const { return m_registers; }
		auto& registers() { return m_registers; }
		const auto& program() const { return m_program; }
		auto& program() { return m_program; }

		template<typename T, typename...Ts>
		T& emit(Ts&&...ts)
		{
			auto ptr = new T(std::forward<Ts>(ts)...);
			emit(core::adopt_shared(static_cast<Instruction*>(ptr)));
			return *ptr;
		}
		Instruction& emit(InstructionPtr);
		Block& emit_block(core::String name);
		void end_block();
		
		Register allocate_register();
	private:
		std::vector<Register> m_registers{};
		ExecutableProgram m_program{};
	};

}
