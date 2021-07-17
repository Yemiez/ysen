#include "Instruction.h"


#include "BytecodeInterpreter.h"
#include "ysen/core/format.h"

void ysen::lang::bytecode::Load::execute(BytecodeInterpreter& vm) const
{
	vm.accumulator() = vm.register_value(m_source);
}

ysen::core::String ysen::lang::bytecode::Load::to_string() const
{
	return core::format("load {}", m_source.to_string());
}

void ysen::lang::bytecode::LoadImmediate::execute(BytecodeInterpreter& vm) const
{
	vm.accumulator() = m_immediate;
}

ysen::core::String ysen::lang::bytecode::LoadImmediate::to_string() const
{
	return core::format("loadi {}", m_immediate.to_formatted_string());
}

void ysen::lang::bytecode::LoadVariable::execute(BytecodeInterpreter& vm) const
{
	vm.accumulator() = vm.variable(m_name);
}

ysen::core::String ysen::lang::bytecode::LoadVariable::to_string() const
{
	return core::format("loadv '{}'", m_name);
}

void ysen::lang::bytecode::Store::execute(BytecodeInterpreter& vm) const
{
	vm.register_value(m_target) = vm.accumulator();
}

ysen::core::String ysen::lang::bytecode::Store::to_string() const
{
	return core::format("store {}", m_target.to_string());
}

void ysen::lang::bytecode::StoreVariable::execute(BytecodeInterpreter& vm) const
{
	vm.variable(m_name) = vm.accumulator();
}

ysen::core::String ysen::lang::bytecode::StoreVariable::to_string() const
{
	return core::format("storev '{}'", m_name);
}

void ysen::lang::bytecode::Add::execute(BytecodeInterpreter& vm) const
{
	vm.accumulator() = vm.accumulator() + vm.register_value(m_source);
}

ysen::core::String ysen::lang::bytecode::Add::to_string() const
{
	return core::format("add {}", m_source.to_string());
}

void ysen::lang::bytecode::Call::execute(BytecodeInterpreter& vm) const
{
	vm.set_call(m_name);
}

ysen::core::String ysen::lang::bytecode::Call::to_string() const
{
	return core::format("call '{}'", m_name);
}

void ysen::lang::bytecode::Push::execute(BytecodeInterpreter& vm) const
{
	vm.push();	
}

ysen::core::String ysen::lang::bytecode::Push::to_string() const
{
	return "push";
}

void ysen::lang::bytecode::Pop::execute(BytecodeInterpreter& vm) const
{
	vm.pop();
}

ysen::core::String ysen::lang::bytecode::Pop::to_string() const
{
	return "pop";
}

void ysen::lang::bytecode::Ret::execute(BytecodeInterpreter& vm) const
{
	vm.pop_stack_frame();	
}

ysen::core::String ysen::lang::bytecode::Ret::to_string() const
{
	return "ret";
}
