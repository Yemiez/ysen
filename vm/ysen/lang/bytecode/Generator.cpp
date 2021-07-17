#include "Generator.h"

#include "ysen/core/format.h"

ysen::lang::bytecode::Instruction& ysen::lang::bytecode::Block::emit(InstructionPtr instr)
{
	return *m_instructions.emplace_back(std::move(instr));
}

ysen::core::String ysen::lang::bytecode::Block::to_string() const
{
	core::String formatted{};
	formatted.append(core::format("{}\n", m_label.to_string()));

	for (const auto &instr : m_instructions) {
		formatted.append(core::format("\t{}\n", instr->to_string()));
	}

	return formatted;
}

ysen::lang::bytecode::Block& ysen::lang::bytecode::Block::emit_sub_block(BlockType type)
{
	return *m_children.emplace_back(
		core::adopt_shared(new Block(create_sub_label(), type))
	);
}

ysen::lang::bytecode::Label ysen::lang::bytecode::Block::create_sub_label() const
{
	return Label{ m_children.size() + 1, {} };
}

ysen::lang::bytecode::Block& ysen::lang::bytecode::ExecutableProgram::emit_block(core::String name, BlockType type)
{
	m_working_stack.push(core::adopt_shared(new Block(std::move(name), type)));
	return current_block();
}

void ysen::lang::bytecode::ExecutableProgram::end_block()
{
	auto block = m_working_stack.top();
	m_working_stack.pop();
	m_blocks.emplace_back(block);
}

const ysen::lang::bytecode::Block* ysen::lang::bytecode::ExecutableProgram::block_by_name(const core::String& name
) const
{
	auto iterator = std::find_if(m_blocks.begin(), m_blocks.end(), [&name](auto block) {
		return block->name() == name;
	});

	return iterator != m_blocks.end() ? iterator->ptr() : nullptr;
}

ysen::core::String ysen::lang::bytecode::ExecutableProgram::to_string() const
{
	core::String formatted{};

	for (const auto &block : m_blocks) {
		formatted.append(block->to_string());
		formatted.push('\n');
	}

	return formatted;
}

ysen::lang::bytecode::Instruction& ysen::lang::bytecode::Generator::emit(InstructionPtr instr)
{
	return m_program.current_block().emit(std::move(instr));
}

ysen::lang::bytecode::Block& ysen::lang::bytecode::Generator::emit_block(core::String name) 
{
	return m_program.emit_block(std::move(name));
}

void ysen::lang::bytecode::Generator::end_block()
{
	m_program.end_block();
}

ysen::lang::bytecode::Register ysen::lang::bytecode::Generator::allocate_register()
{
	return m_registers.emplace_back(Register{m_registers.size()});
}

