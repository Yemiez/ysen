#pragma once
#include <ysen/core/SharedPtr.h>

namespace ysen::core {

	template<typename T>
	class Optional
	{
	public:
		Optional() = default;
		Optional(const Optional&) = default;
		Optional(Optional&&) noexcept;
		Optional(T);

		bool has_value() const { return m_has_value; }
		void reset();
		T release_value();
		T value() const { return m_value; }

		Optional& operator=(const Optional&) = default;
		Optional& operator=(Optional&&) noexcept;
		Optional& operator=(T);
		
	private:
		bool m_has_value{false};
		T m_value{};
	};

	template <typename T>
	Optional<T>::Optional(Optional&& other) noexcept
		: m_has_value(other.m_has_value), m_value(std::move(other.m_value))
	{
		other = {};
	}

	template <typename T>
	Optional<T>::Optional(T value)
		: m_has_value(true), m_value(std::move(value))
	{}

	template <typename T>
	void Optional<T>::reset()
	{
		*this = {};
	}

	template <typename T>
	T Optional<T>::release_value()
	{
		T tmp{};
		std::swap(tmp, m_value);
		m_has_value = false;
		return std::move(tmp);
	}

	template <typename T>
	Optional<T>& Optional<T>::operator=(Optional&& other) noexcept
	{
		if (this == &other) {
			return *this;
		}
		
		m_has_value = other.m_has_value;
		m_value = other.m_value;
		return *this;
	}

	template <typename T>
	Optional<T>& Optional<T>::operator=(T value)
	{
		m_has_value = true;
		m_value = std::move(value);
		return *this;
	}

}
