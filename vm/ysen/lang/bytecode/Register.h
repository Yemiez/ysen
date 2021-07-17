#pragma once
#include "ysen/core/String.h"

namespace ysen::lang::bytecode {

	class Register
	{
	public:
		Register(size_t index);
		
		size_t index() const { return m_index; }
		core::String to_string() const;
	private:
		size_t m_index{};
	};

}
