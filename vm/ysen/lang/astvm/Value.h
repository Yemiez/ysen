#pragma once
#include <unordered_map>
#include <vector>
#include "ysen/core/fnv1a.h"
#include "ysen/core/SharedPtr.h"
#include "ysen/Core/String.h"

namespace ysen::lang::astvm {
	class Value;
}

namespace std {
	template<>
	struct hash<ysen::lang::astvm::Value>
	{
		size_t operator()(const ysen::lang::astvm::Value&) const noexcept;
	};
}

namespace ysen::lang::astvm {
	class Function;
	using FunctionPtr = core::SharedPtr<Function>;

	namespace details {
		template<typename T>
		struct IsCoreString : std::bool_constant<false> {};
		template<>
		struct IsCoreString<core::String> : std::bool_constant<true> {};
		
		template<typename T>
		struct ValueTraits
		{
			static constexpr auto IS_TRIVIAL = std::is_trivial_v<T>;
			static constexpr auto IS_INTEGER = std::is_integral_v<T>;
			static constexpr auto IS_FLOATING = std::is_floating_point_v<T>;
			static constexpr auto IS_STRING	 = IsCoreString<T>::value;
		};
	}

	class BadValueCast : public std::exception
	{
	public:
		char const* what() const override { return "Bad value cast"; }
	};

	class Value
	{
	public:
		enum class ValueType
		{
			Undefined,
			Null,
			Array,
			Object,
			String,
			Function,
			Bool,
			Int,
			Float,
			Double,
		};

		using Array = std::vector<Value>;
		using Object = std::unordered_map<Value, Value>;
		
	public:
		Value() = default;
		Value(const Value&);
		Value(Value&&) noexcept;
		Value(Array);
		Value(Object);
		Value(core::String);
		Value(const char*);
		Value(bool);
		Value(int);
		Value(float);
		Value(double);
		Value(FunctionPtr);

		core::String to_string() const;
		core::String to_formatted_string() const;
		
		const Array& array() const { return m_array; }
		Array& array() { return m_array; }
		const Object& object() const { return m_object; }
		Object& object() { return m_object; }
		const core::String& string() const { return m_string; }
		core::String& string() { return m_string; }
		const FunctionPtr& function() const { return m_function; }
		FunctionPtr& function() { return m_function; }
		
		template<typename T>
		T get() const;
		template<typename T>
		T cast() const;
		
		size_t hash() const;

		bool is_undefined() const { return m_type == ValueType::Undefined; }
		bool is_null() const { return m_type == ValueType::Null; }
		bool is_function() const { return m_type == ValueType::Function; }
		bool is_string() const { return m_type == ValueType::String; }
		bool is_array() const { return m_type == ValueType::Array; }
		bool is_object() const { return m_type == ValueType::Object; }
		bool is_trivial() const { return m_type >= ValueType::Bool && m_type <= ValueType::Double; }
		
	public: // Operators
		bool operator<(const Value&) const;
		bool operator==(const Value&) const;
		Value operator+(const Value&) const;
		Value operator-(const Value&) const;
		Value operator/(const Value&) const;
		Value operator*(const Value&) const;

		void reset();
		Value& operator=(const Value&);
		Value& operator=(Value&&) noexcept;
		Value& operator=(Array);
		Value& operator=(Object);
		Value& operator=(core::String);
		Value& operator=(const char*);
		Value& operator=(bool);
		Value& operator=(int);
		Value& operator=(float);
		Value& operator=(double);
		Value& operator=(FunctionPtr);
	private:
		template<typename Op>
		Value same_type_bin_op(const Value&, Op&& op) const;

		template<typename Op>
		Value casted_type_bin_op(const Value&, Op&& op) const;
		
	private:
		ValueType m_type{ValueType::Undefined};
		Array m_array{};
		Object m_object{};
		core::String m_string{};
		FunctionPtr m_function{};
		union
		{
			bool b;
			int i;
			float f;
			double d;
		} m_trivial{};
	};

	using ValuePtr = core::SharedPtr<Value>;

	template <typename T>
	T Value::get() const
	{
		using Traits = details::ValueTraits<T>;

		if constexpr (Traits::IS_TRIVIAL) {
			if (m_type == ValueType::Bool) {
				return static_cast<T>(m_trivial.b);
			}
			if (m_type == ValueType::Int) {
				return static_cast<T>(m_trivial.i);
			}
			if (m_type == ValueType::Float) {
				return static_cast<T>(m_trivial.f);
			}
			return static_cast<T>(m_trivial.d);
		}
		else if constexpr (Traits::IS_STRING) {
			if (m_type == ValueType::String) {
				return m_string;
			}
		}

		throw BadValueCast();
	}

	template <typename T>
	T Value::cast() const
	{
		if constexpr (details::IsCoreString<T>::value) {
			return to_string();			
		}
		else if constexpr (std::is_trivial_v<T>) {
			switch (m_type) {
			case ValueType::Undefined: return {};
			case ValueType::Null: return {};
			case ValueType::Array: return {};
			case ValueType::Object: return {};
			case ValueType::String: 
				if constexpr (std::is_integral_v<T>) {
					return static_cast<T>(m_string.to_integer());
				}
				else if constexpr (std::is_floating_point_v<T>) {
					return static_cast<T>(m_string.to_double());
				}
				else if constexpr (std::is_same_v<T, bool>) {
					return static_cast<T>(m_string.to_boolean());
				}
				break;
			case ValueType::Bool: return static_cast<T>(m_trivial.b);
			case ValueType::Int: return static_cast<T>(m_trivial.i);
			case ValueType::Float: return static_cast<T>(m_trivial.f);
			case ValueType::Double: return static_cast<T>(m_trivial.d);
			default: return {};
			}
		}
		else if constexpr (std::is_same_v<T, Object>) {
			if (m_type != ValueType::Object) {
				throw std::exception("Cannot cast to object");
			}

			return m_object;
		}
		else if constexpr (std::is_same_v<T, Array>) {
			if (m_type != ValueType::Array) {
				throw std::exception("Cannot cast to Array");
			}

			return m_array;
		}
		throw BadValueCast();
	}

	template <typename Op>
	Value Value::same_type_bin_op(const Value& other, Op&& op) const
	{
		switch (m_type) {
		case ValueType::Undefined: return {};
		case ValueType::Null: return *this;
		case ValueType::Array: throw std::exception("Unimplemented array merge"); // TODO merge
		case ValueType::Object: throw std::exception("Unimplemented object merge"); // TODO merge
		case ValueType::String:
			{
				if constexpr (std::is_same_v<Op, std::plus<>>) {
					return op(string(), other.string());
				}

				return {};
			}
		case ValueType::Bool: return {};
		case ValueType::Int: return op(get<int>(), other.get<int>());
		case ValueType::Float: return op(get<float>(), other.get<float>());
		case ValueType::Double: return op(get<double>(), other.get<double>());
		default: ;
		}

		throw std::exception();
	}

	template <typename Op>
	Value Value::casted_type_bin_op(const Value& other, Op&& op) const
	{
		switch (m_type) {
		case ValueType::Undefined: return {}; // TODO, undefined + 5 = 5 maybe?
		case ValueType::Null: return *this; // TODO, NULL + 5 = 5 maybe?
		case ValueType::Array: throw std::exception("Unimplemented array merge"); // TODO merge
		case ValueType::Object: throw std::exception("Unimplemented object merge"); // TODO merge
		case ValueType::String:
			{
				if constexpr (std::is_same_v<Op, std::plus<>>) {
					return op(string(), other.cast<core::String>());
				}

				return {};
			}
		case ValueType::Bool: return {};
		case ValueType::Int: return op(get<int>(), other.cast<int>());
		case ValueType::Float: return op(get<float>(), other.cast<float>());
		case ValueType::Double: return op(get<double>(), other.cast<double>());
		default: ;
		}

		throw std::exception();
	}
}
