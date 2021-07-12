#pragma once
#include <ysen/core/String.h>

namespace ysen::core {

	template<typename T>
	class BasicStringView
	{
	public:
		BasicStringView() = default;
		BasicStringView(const BasicStringView&) = default;
		BasicStringView(BasicStringView&&) = default;
		BasicStringView(const T*);
		BasicStringView(const BasicString<T>&);

		size_t length() const { return m_length; }
		const T* c_str() const { return m_str; }
	private:
		const T* m_str{nullptr};
		size_t m_length{0};
	};

	template <typename T>
	BasicStringView<T>::BasicStringView(const T* str)
		: m_str(str), m_length(::strlen(str))
	{}

	template <typename T>
	BasicStringView<T>::BasicStringView(const BasicString<T>& str)
		: m_str(str.c_str()), m_length(str.length())
	{}

	using StringView = BasicStringView<char>;
	
}