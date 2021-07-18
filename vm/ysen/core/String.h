#pragma once
#include <cstring>
#include <ostream>
#include "fnv1a.h"

namespace ysen::core {
	
	template<typename T>
	class BasicString;
	using String = BasicString<char>;

	static String to_string(int);
	static String to_string(unsigned int);
	static String to_string(float);
	static String to_string(double);
	
	template<typename T>
	class BasicString
	{
	public:
		using CharType = T;
		static constexpr auto NPOS = static_cast<unsigned int>(-1);

		BasicString() = default;
		BasicString(const T*);
		BasicString(const T*, size_t);
		BasicString(const BasicString&);
		BasicString(BasicString&&) noexcept;
		~BasicString();

		bool empty() const { return length() == 0; }
		
		const T* begin() const { return m_buffer; }
		T* begin() { return m_buffer; }
		const T* end() const { return m_buffer + m_length; }
		T* end() { return m_buffer + m_length; }
		
		const T* c_str() const { return m_buffer; }
		T* buffer() { return m_buffer; }
		size_t capacity() const { return m_capacity; }
		size_t length() const { return m_length; }

		void push(T); // pushes to the end
		T pop(); // pop last element
		T shift(); // pop first element

		void append(const T*);
		void append(const T*, size_t);
		void prepend(const T*);
		void prepend(const T*, size_t);
		void append(const BasicString&);
		void prepend(const BasicString&);
		
		BasicString substr(int);
		BasicString substr(size_t);
		BasicString substr(int, size_t);
		BasicString substr(size_t, size_t);

		BasicString& operator=(const BasicString&);
		BasicString& operator=(BasicString&&) noexcept;
		BasicString& operator=(const T*);

		T at(size_t) const;
		
		// find first
		size_t find(T) const;
		size_t find(const T*) const;
		size_t find(const BasicString&) const;

		bool contains(T s) const { return find(s) != NPOS; }
		bool contains(const T* s) const { return find(s) != NPOS; }
		bool contains(const BasicString& s) const { return find(s) != NPOS; }

		bool operator!=(const T*) const;
		bool operator!=(const BasicString&) const;
		bool operator==(const T*) const;
		bool operator==(const BasicString&) const;
		bool operator<(const T*) const;
		bool operator<(const BasicString&) const;
		bool operator>(const T*) const;
		bool operator>(const BasicString&) const;
		String& operator+=(const String&);
		String operator+(const String&) const;
		
		void resize(size_t, T);

		template<typename...Ts>
		bool is_equal_to_any_of(const Ts&...ts) const;
		
	public:
		// Conversion
		int to_integer() const { return std::strtol(begin(), nullptr, 10); }
		unsigned int to_unsigned_integer() const { return std::strtoul(begin(), nullptr, 10); }
		float to_float() const { return std::strtof(begin(), nullptr); }
		double to_double() const { return std::strtod(begin(), nullptr); }
		bool to_boolean() const;

		BasicString to_lowercase() const;
		BasicString to_uppercase() const;

		template<typename Callable>
		BasicString custom_string_convert(Callable f) const;

		template<typename Callable>
		size_t count_occurrences(Callable f) const;

		bool is_integer() const;
		bool is_float() const;

		size_t hash() const;
	protected:
		void copy_buffer(const T*, size_t);
		void delete_buffer();
		void grow_to(size_t, bool);
		void grow_capacity(size_t);
		void ensure_capacity(size_t);
		size_t compute_hash() const;
		
	private:
		T* m_buffer{nullptr};
		size_t m_capacity{};
		size_t m_length{};
		mutable size_t m_hash{};
		mutable bool m_hashed{};
	};

	template <typename T>
	BasicString<T>::BasicString(const T* buffer)
	{
		copy_buffer(buffer, ::strlen(buffer));
	}

	template <typename T>
	BasicString<T>::BasicString(const T* buffer, size_t length)
	{
		copy_buffer(buffer, length);
	}

	template <typename T>
	BasicString<T>::BasicString(const BasicString& other)
	{
		copy_buffer(other.c_str(), other.length());
	}

	template <typename T>
	BasicString<T>::BasicString(BasicString&& other) noexcept
		: m_buffer(other.m_buffer), m_capacity(other.m_capacity), m_length(other.m_length)
	{
		other.m_buffer = nullptr;
		other.m_length = 0;
		other.m_capacity = 0;
		other.m_hashed = false;
	}

	template <typename T>
	BasicString<T>::~BasicString()
	{
		delete_buffer();
	}

	template <typename T>
	void BasicString<T>::push(T value)
	{
		ensure_capacity(m_length + 8);
		m_buffer[m_length] = value;
		m_buffer[m_length + 1] = 0;
		++m_length;
		m_hashed = false;
	}

	template <typename T>
	T BasicString<T>::pop()
	{
		auto v = m_buffer[--m_length];
		m_buffer[m_length] = 0;
		m_hashed = false;
		return v;
	}

	template <typename T>
	T BasicString<T>::shift()
	{
		auto v = *m_buffer;
		for (auto i = 1u; i < m_length; ++i) {
			m_buffer[i - 1] = m_buffer[i];
		}
		--m_length;
		m_hashed = false;
		return v;
	}

	template <typename T>
	void BasicString<T>::append(const T* buffer)
	{
		append(buffer, ::strlen(buffer));		
	}

	template <typename T>
	void BasicString<T>::append(const T* buffer, size_t length)
	{
		ensure_capacity(m_length + length + 1);
		::memcpy(end(), buffer, length);
		m_length += length;
		*end() = 0;
		m_hashed = false;
	}

	template <typename T>
	void BasicString<T>::prepend(const T* buffer)
	{
		prepend(buffer, ::strlen(buffer));
	}

	template <typename T>
	void BasicString<T>::prepend(const T* buffer, size_t length)
	{
		ensure_capacity(m_length + length);

		for (int i = m_length; i > -1; --i) {
			m_buffer[i + length - 1] = m_buffer[i];
		}
		
		::memcpy(m_buffer, buffer, length - 1);
		m_length += length;
		*end() = 0;
		m_hashed = false;
	}

	template <typename T>
	void BasicString<T>::append(const BasicString& other)
	{
		append(other.c_str(), other.length());
	}

	template <typename T>
	void BasicString<T>::prepend(const BasicString& other)
	{
		prepend(other.c_str(), other.length());
	}

	template <typename T>
	BasicString<T> BasicString<T>::substr(int offset)
	{
		if (offset < 0) {
			offset = m_length + offset;
		}
		
		return BasicString{ m_buffer + offset, m_length - offset };
	}

	template <typename T>
	BasicString<T> BasicString<T>::substr(size_t offset)
	{
		return BasicString{ m_buffer + offset, m_length - offset };
	}

	template <typename T>
	BasicString<T> BasicString<T>::substr(int offset, size_t length)
	{
		if (offset < 0) {
			offset = m_length + offset;
		}
		return BasicString{ m_buffer + offset, length };
	}

	template <typename T>
	BasicString<T> BasicString<T>::substr(size_t offset, size_t length)
	{
		return BasicString{ m_buffer + offset, length };
	}

	template <typename T>
	BasicString<T>& BasicString<T>::operator=(const BasicString& other)
	{
		if (this == &other) {
			return *this;
		}
		
		copy_buffer(other.c_str(), other.length());
		return *this;
	}

	template <typename T>
	BasicString<T>& BasicString<T>::operator=(BasicString&& other) noexcept
	{
		if (this == &other) {
			return *this;
		}
		delete_buffer();
		std::swap(m_buffer, other.m_buffer);
		std::swap(m_length, other.m_length);
		std::swap(m_capacity, other.m_capacity);
		return *this;
	}

	template <typename T>
	BasicString<T>& BasicString<T>::operator=(const T* buffer)
	{
		if (buffer) {
			copy_buffer(buffer, ::strlen(buffer));
		}
		else {
			delete_buffer();
		}
		return *this;
	}

	template <typename T>
	T BasicString<T>::at(size_t i) const
	{
		return m_buffer[i];
	}

	template <typename T>
	size_t BasicString<T>::find(T value) const
	{
		for (auto i = 0u; i < m_length; ++i) {
			if (at(i) == value) {
				return i;
			}
		}

		return NPOS;
	}

	template <typename T>
	size_t BasicString<T>::find(const T* str) const
	{
		auto length = ::strlen(str);
		if (length > m_length) return NPOS;

		for (auto i = 0u; i < m_length - length; ++i) {
			auto submatch{true};
			for (auto j = i; j < length; ++j) {
				if (at(i) != str[j]) {
					break;
				}
			}

			if (submatch) {
				return i;
			}
		}

		return NPOS;
	}

	template <typename T>
	size_t BasicString<T>::find(const BasicString& s) const
	{
		return find(s.c_str());
	}

	template <typename T>
	bool BasicString<T>::operator!=(const T* s) const
	{
		return !(*this == s);
	}

	template <typename T>
	bool BasicString<T>::operator!=(const BasicString& s) const
	{
		return !(*this == s);
	}

	template <typename T>
	bool BasicString<T>::operator==(const T* rhs) const
	{
		return ::strcmp(c_str(), rhs) == 0;
	}

	template <typename T>
	bool BasicString<T>::operator==(const BasicString& other) const
	{
		return ::strcmp(c_str(), other.c_str()) == 0;
	}

	template <typename T>
	bool BasicString<T>::operator<(const T* other) const
	{
		return ::strcmp(c_str(), other) < 0;
	}

	template <typename T>
	bool BasicString<T>::operator<(const BasicString& other) const
	{
		return ::strcmp(c_str(), other.c_str()) < 0;
	}

	template <typename T>
	bool BasicString<T>::operator>(const T* other) const
	{
		return ::strcmp(c_str(), other) > 0;
	}

	template <typename T>
	bool BasicString<T>::operator>(const BasicString& other) const
	{
		return ::strcmp(c_str(), other.c_str()) > 0;
	}

	template <typename T>
	String& BasicString<T>::operator+=(const String& other)
	{
		append(other);
		return *this;
	}

	template <typename T>
	String BasicString<T>::operator+(const String& other) const
	{
		auto tmp{*this};
		return tmp += other;
	}

	template <typename T>
	void BasicString<T>::resize(size_t length, T init)
	{
		if (length < m_length) {
			return;
		}
		
		grow_capacity(length);

		if (m_length > 0) {
			::memset(&m_buffer[m_length], init, length - m_length);
		}
		else {
			::memset(m_buffer, init, length);
		}

		m_length = length;
		m_hashed = false;
	}

	template <typename T>
	template <typename ... Ts>
	bool BasicString<T>::is_equal_to_any_of(const Ts&... ts) const
	{
		return ((ts == *this) || ...);
	}

	template <typename T>
	bool BasicString<T>::to_boolean() const
	{
		return !empty();
	}

	template <typename T>
	BasicString<T> BasicString<T>::to_lowercase() const
	{
		return custom_string_convert([](const auto& c) {
			return std::tolower(static_cast<unsigned char>(c));
		});
	}

	template <typename T>
	BasicString<T> BasicString<T>::to_uppercase() const
	{
		return custom_string_convert([](const auto& c) {
			return std::tolower(static_cast<unsigned char>(c));
		});
	}

	template <typename T>
	template <typename Callable>
	BasicString<T> BasicString<T>::custom_string_convert(Callable f) const
	{
		String s{};
		s.resize(length(), 0);

		for (const auto &c : *this) {
			s.push(f(c));
		}

		return s;
	}

	template <typename T>
	template <typename Callable>
	size_t BasicString<T>::count_occurrences(Callable f) const
	{
		auto count{0u};
		for (const auto& c : *this) {
			if (f(c)) {
				++count;
			}
		}
		return count;
	}

	template <typename T>
	bool BasicString<T>::is_integer() const
	{
		return count_occurrences([](auto&& c) {
			return ::isdigit(static_cast<unsigned char>(c));
		}) == length();
	}

	template <typename T>
	bool BasicString<T>::is_float() const
	{
		auto digits = count_occurrences([](auto&& c) {
			return ::isdigit(static_cast<unsigned char>(c));
		});

		if (digits == length()) {
			return true;
		}

		return (digits == length() - 1) && find('.') != NPOS;
	}

	template <typename T>
	size_t BasicString<T>::hash() const
	{
		if (m_hashed) {
			return m_hash;
		}

		return compute_hash();
	}

	template <typename T>
	void BasicString<T>::copy_buffer(const T* buffer, size_t length)
	{
		ensure_capacity(length + 1);
		::memcpy(m_buffer, buffer, length);
		m_length = length;
		m_hashed = false;
	}

	template <typename T>
	void BasicString<T>::delete_buffer()
	{
		if (m_buffer) {
			delete[] m_buffer;
			m_capacity = m_length = 0;
			m_buffer = nullptr;
			m_hashed = false;
		}
	}

	template <typename T>
	void BasicString<T>::grow_to(size_t length, bool grow_capacity)
	{
		m_capacity = m_length = length;

		if (grow_capacity) {
			m_capacity *= 2;
		}
		
		m_buffer = new T[m_capacity]{0};
	}

	template <typename T>
	void BasicString<T>::grow_capacity(size_t capacity)
	{
		auto *buffer = new T[capacity]{0};

		if (m_buffer != nullptr) {
			::memcpy(buffer, m_buffer, m_length);
			delete[] m_buffer; // Manual deletion
		}

		m_buffer = buffer;
		m_capacity = capacity;
		m_hashed = false;
	}

	template <typename T>
	void BasicString<T>::ensure_capacity(size_t requirement)
	{
		if (requirement > m_capacity) {
			grow_capacity(requirement * 2);
		}
	}

	template <typename T>
	size_t BasicString<T>::compute_hash() const
	{
		if (empty()) {
			return 0;
		}

		m_hash = core::fnv1a(reinterpret_cast<const unsigned char*>(c_str()), length() * sizeof(T));
		m_hashed = true;
		return m_hash;
	}

	static String to_string(int value)
	{
		char buffer[256]{0};
		sprintf(buffer, "%d", value);
		return buffer;
	}
	
	static String to_string(unsigned int value)
	{
		char buffer[256]{0};
		sprintf(buffer, "%u", value);
		return buffer;
	}
	
	static String to_string(float value)
	{
		char buffer[256]{0};
		sprintf(buffer, "%f", value);
		return buffer;
	}
	
	static String to_string(double value)
	{
		char buffer[256]{0};
		sprintf(buffer, "%f", value);
		return buffer;
	}

	inline std::ostream &operator<<(std::ostream &os, String const &m) { 
	    os.write(m.c_str(), m.length());
		return os;
	}

	inline bool is_whitespace(char c)
	{
		return ::iswspace(c);
	}

	inline bool is_alphabetical(char c)
	{
		return ::isalpha(c);
	}

	inline bool is_alpha_numeric(char c)
	{
		return ::isalnum(c);
	}

	inline bool is_numeric(char c)
	{
		return ::isdigit(c);
	}
}

namespace std {

	template<>
	struct hash<ysen::core::String>
	{
		size_t operator()(const ysen::core::String &value) const noexcept
		{
			return value.hash();
		}
	};
	
}
