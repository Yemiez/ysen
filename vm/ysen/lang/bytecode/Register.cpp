#include "Register.h"

#include "ysen/core/format.h"

ysen::lang::bytecode::Register::Register(size_t index)
	: m_index(index)
{}

ysen::core::String ysen::lang::bytecode::Register::to_string() const
{
	return core::format("${}", m_index);
}
