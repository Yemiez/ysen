#pragma once
#include "ysen/core/format.h"
#include "ysen/Core/String.h"

namespace ysen::lang::bytecode {

	class Label
	{
	public:
		Label(size_t position, core::String name)
			: m_position(position), m_name(std::move(name))
		{}

		size_t position() const { return m_position; }
		const core::String& name() const { return m_name; }
		core::String to_string() const;
	private:
		size_t m_position{};
		core::String m_name{};
	};

	inline core::String Label::to_string() const
	{
		if (m_name.empty()) {
			return core::format(":loc_{}", m_position);
		}

		return core::format(":{}", m_name);
	}

}
