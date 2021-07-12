#pragma once
#include <any>
#include <ysen/Core/String.h>
#include <vector>
#include <iostream>

namespace ysen {namespace lang {namespace astvm {
	class Value;
}}}

namespace ysen::core {

	namespace details {
		template<typename T>
		struct Formatter;
		
		template<typename...Args>
		String format_impl(const String&, Args&&...);
	}

	template<typename...Args>
	String format(const String &fmt, Args&&...args)
	{
		return details::format_impl(fmt, std::forward<Args>(args)...);
	}

	
	template<typename...Args>
	void print(const String& fmt, Args&&...args)
	{
		std::cout << format(fmt, std::forward<Args>(args)...);
	}

	template<typename...Args>
	void println(const String& fmt, Args&&...args)
	{
		std::cout << format(fmt, std::forward<Args>(args)...) << '\n';
	}
}


namespace ysen::core::details {

	class FormatterArgs
	{
	public:
		template<typename...Args>
		void collect(Args&&...args)
		{
			collect_ex(std::forward<Args>(args)...);
		}

		auto size() const { return m_args.size(); }
		const auto& at(uint32_t idx) { return m_args.at(idx); }

		auto pop() -> String
		{
			if (m_args.empty()) {
				return {};
			}

			auto arg = m_args.at(0);
			m_args.erase(m_args.begin());
			return arg;
		}
		
	private:
		template<typename T, typename...Args>
		void collect_ex(T&& t, Args&& ...args)
		{
			m_args.push_back(Formatter<std::remove_cvref_t<T>>::format(t));
			collect_ex(std::forward<Args>(args)...);
		}
		static void collect_ex() {}

	private:
		std::vector<String> m_args{};
	};

	template<typename...Args>
	class FormatterContext
	{
	public:
		FormatterContext(String fmt, Args&&...args)
			: m_fmt(fmt), m_arguments()
		{
			m_arguments.collect(std::forward<Args>(args)...);
		}

		String format()
		{
			auto iterator = m_fmt.begin();
			auto end = m_fmt.end();
			
			for (;iterator != end;) {
				if (*iterator == '{' && *(iterator + 1) == '}') {
					auto arg = m_arguments.pop();
					m_output.append(arg);
					iterator += 2;
				}
				else if (*iterator == '{' && *(iterator + 1) == '{') {
					m_output.push('{');
					iterator += 2;
				}
				else if (*iterator == '}' && *(iterator + 1) == '}') {
					m_output.push('}');
					iterator += 2;
				}
				else {
					m_output.push(*(iterator++));
				}
			}

			return m_output;
		}

		auto& formatter_arguments() { return m_arguments; }

	private:
		String m_output{};
		String m_fmt{};
		FormatterArgs m_arguments{};
	};
	
	template<typename...Args>
	String format_impl(const String& fmt, Args&& ...args)
	{
		FormatterContext<Args...> ctx{fmt, std::forward<Args>(args)...};
		return ctx.format();
	}
	
	template<>
	struct Formatter<int>
	{
		static String format(int x) { return to_string(x); }
	};
	template<>
	struct Formatter<unsigned int>
	{
		static String format(unsigned int x) { return to_string(x); }
	};
	template<>
	struct Formatter<float>
	{
		static String format(float x) { return to_string(x); }
	};
	template<>
	struct Formatter<double>
	{
		static String format(double x) { return to_string(x); }
	};

	template<>
	struct Formatter<String>
	{
		static String format(const String& cs) { return cs; }
	};

	template<>
	struct Formatter<std::string_view>
	{
		static String format(const std::string_view& cs) { return cs.data(); }
	};

	template<size_t S>
	struct Formatter<char[S]>
	{
		static String format(const char* cs) { return cs; }
	};

	template<>
	struct Formatter<const char*>
	{
		static String format(const char* cs) { return cs; }
	};

	
	template<>
	struct Formatter<char>
	{
		static String format(const char& c) { String s; s.push(c); return s; }
	};

	template<>
	struct Formatter<std::any>
	{
		static String format(const std::any& value)
		{
			if (value.type() == typeid(int)) {
				return to_string(std::any_cast<int>(value));
			}
			if (value.type() == typeid(float)) {
				return to_string(std::any_cast<float>(value));
			}
			if (value.type() == typeid(double)) {
				return to_string(std::any_cast<double>(value));
			}
			if (value.type() == typeid(String)) {
				return std::any_cast<String>(value);
			}

			return core::format("<unknown {}>", value.type().name());
		}
	};

	template<>
	struct Formatter<lang::astvm::Value>
	{
		static String format(const lang::astvm::Value& value);
	};
}