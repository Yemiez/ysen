#pragma once
#include <vector>

#include "ysen/core/String.h"
#include "Register.h"
#include "ysen/core/SharedPtr.h"
#include "ysen/lang/astvm/Value.h"

namespace ysen::lang::bytecode {
	class Register;
	class BytecodeInterpreter;
	
	class Instruction
	{
	public:
		virtual ~Instruction() = default;

		virtual void execute(BytecodeInterpreter&) const = 0;
		virtual core::String to_string() const = 0;
	};
	using InstructionPtr = core::SharedPtr<Instruction>;
	using InstructionList = std::vector<InstructionPtr>;
	using InstructionIterator = InstructionList::const_iterator;

	// load $1, $2 ($2 = $1)
	class Load : public Instruction
	{
	public:
		Load(Register source)
			: m_source(source)
		{}

		void execute(BytecodeInterpreter&) const override;
		core::String to_string() const override;
		
		const auto& source() const { return m_source; }
	private:
		Register m_source;
	};

	class LoadImmediate : public Instruction
	{
	public:
		LoadImmediate(astvm::Value immediate)
			: m_immediate(std::move(immediate))
		{}

		void execute(BytecodeInterpreter&) const override;
		core::String to_string() const override;
		
		const auto& immediate() const { return m_immediate; }
	private:
		astvm::Value m_immediate{};
	};

	class LoadVariable : public Instruction
	{
	public:
		LoadVariable(core::String name)
			: m_name(std::move(name))
		{}

		void execute(BytecodeInterpreter&) const override;
		core::String to_string() const override;
	private:
		core::String m_name{};
	};

	class Store : public Instruction
	{
	public:
		Store(Register target)
			: m_target(target)
		{}

		void execute(BytecodeInterpreter&) const override;
		core::String to_string() const override;

		const auto& target() const { return m_target; }
	private:
		Register m_target;
	};

	class StoreVariable : public Instruction
	{
	public:
		StoreVariable(core::String name)
			: m_name(std::move(name))
		{}

		void execute(BytecodeInterpreter&) const override;
		core::String to_string() const override;

	private:
		core::String m_name{};
	};

	// add $1, $2, $3 ($3 = $1 + $2)
	class Add : public Instruction
	{
	public:
		Add(Register source)
			: m_source(source)
		{}

		void execute(BytecodeInterpreter&) const override;
		core::String to_string() const override;
		
		const auto& lhs() const { return m_source; }
	private:
		Register m_source;
	};

	class Call : public Instruction
	{
	public:
		Call(core::String name)
			: m_name(std::move(name))
		{}

		void execute(BytecodeInterpreter&) const override;
		core::String to_string() const override;
	private:
		core::String m_name{};
	};

	class Push : public Instruction
	{
	public:
		Push() = default;

		void execute(BytecodeInterpreter&) const override;
		core::String to_string() const override;
	};

	class Pop : public Instruction
	{
	public:
		Pop() = default;

		void execute(BytecodeInterpreter&) const override;
		core::String to_string() const override;
	};

	class Ret : public Instruction
	{
	public:
		Ret() = default;

		void execute(BytecodeInterpreter&) const override;
		core::String to_string() const override;
	};

}
