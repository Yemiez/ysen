#pragma once
#include <functional>

namespace ysen::core {

	class ScopeExit
	{
	public:
		ScopeExit(std::function<void()> callable)
			: m_callable(std::move(callable))
		{}
		~ScopeExit()
		{
			if (m_callable) {
				m_callable();
			}
		}

	private:
		std::function<void()> m_callable{};
	};
	
}
