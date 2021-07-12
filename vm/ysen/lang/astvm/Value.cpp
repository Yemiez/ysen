#include "Value.h"
#include <functional>
#include "Interpreter.h"

ysen::lang::astvm::Value::Value(const Value& other)
	: m_type(other.m_type), m_array(other.m_array), m_object(other.m_object), m_string(other.m_string),
	m_function(other.m_function), m_trivial(other.m_trivial)
{}

ysen::lang::astvm::Value::Value(Value&& other) noexcept
	: m_type(other.m_type), m_array(std::move(other.m_array)), m_object(std::move(other.m_object)),
	m_string(std::move(other.m_string)), m_function(std::move(other.m_function)), m_trivial(other.m_trivial)
{}

ysen::lang::astvm::Value::Value(Array v)
	: m_type(ValueType::Array), m_array(std::move(v))
{}

ysen::lang::astvm::Value::Value(Object v)
	: m_type(ValueType::Object), m_object(std::move(v))
{}

ysen::lang::astvm::Value::Value(core::String v)
	: m_type(ValueType::String), m_string(std::move(v))
{}

ysen::lang::astvm::Value::Value(const char* v)
	: m_type(ValueType::String), m_string(v)
{}

ysen::lang::astvm::Value::Value(bool v)
	: m_type(ValueType::Bool), m_trivial{.b = v}
{}

ysen::lang::astvm::Value::Value(int v)
	: m_type(ValueType::Int), m_trivial{.i = v}
{}

ysen::lang::astvm::Value::Value(float v)
	: m_type(ValueType::Float), m_trivial{.f = v}
{}

ysen::lang::astvm::Value::Value(double v)
	: m_type(ValueType::Double), m_trivial{.d = v}
{}

ysen::lang::astvm::Value::Value(FunctionPtr v)
	: m_type(ValueType::Function), m_function(std::move(v))
{}

ysen::core::String ysen::lang::astvm::Value::to_string() const
{
	switch (m_type) {
	case ValueType::Undefined: return "undefined";
	case ValueType::Null: return "null";
	case ValueType::Array: return "Array";
	case ValueType::Object: return "Object";
	case ValueType::Function: return "Function";
	case ValueType::String: return m_string;
	case ValueType::Bool: return core::to_string(m_trivial.b);
	case ValueType::Int: return core::to_string(m_trivial.i);
	case ValueType::Float: return core::to_string(m_trivial.f);
	case ValueType::Double: return core::to_string(m_trivial.d);
	default: return "unknown";
	}
}

ysen::core::String ysen::lang::astvm::Value::to_formatted_string() const
{
	if (is_string()) {
		core::String builder{};
		builder.push('"');
		builder.append(m_string);
		builder.push('"');
		return builder;
	}
	if (is_array()) {
		core::String builder{};
		builder.append("[");

		core::String array_list{};
		for (const auto& elem : m_array) {
			if (!array_list.empty()) {
				array_list.push(' ');
			}
			array_list.append(elem.to_formatted_string());
			array_list.append(",");
		}
		
		builder.append(array_list);
		builder.append("]");
		return builder;
	}
	if (is_object()) {
		core::String builder{};
		builder.append("[");

		core::String object_list{};
		for (const auto& [key, value] : m_object) {
			if (!object_list.empty()) {
				object_list.push(' ');
			}

			object_list.append(key.to_formatted_string());
			object_list.append(":");
			object_list.append(value.to_formatted_string());
			object_list.append(",");
		}

		builder.append(object_list);
		builder.append("]");
		return builder;
	}
	
	return to_string();
}

size_t ysen::lang::astvm::Value::hash() const
{
	switch (m_type) {
		case ValueType::String: return m_string.hash();
		case ValueType::Int: return core::fnv1a_trivial(m_trivial.i);
		case ValueType::Float: return core::fnv1a_trivial(m_trivial.f);
		case ValueType::Double: return core::fnv1a_trivial(m_trivial.d);
		default: return 0;
	}
}

bool ysen::lang::astvm::Value::operator<(const Value& other) const
{
	switch (m_type) {
	case ValueType::Undefined:
	case ValueType::Null:
		{
			if (other.is_null() || other.is_undefined()) {
				return false;
			}

			if (other.is_trivial() && other.cast<int>() != 0) {
				return true;
			}

			return true;
		}
		break;
	case ValueType::Array:
		{
			if (other.is_array()) {
				return false;
			}

			if (other.is_trivial() && other.cast<int>() > 1) {
				return true;
			}

			return false;
		}
	case ValueType::Object: 
		{
			if (other.is_object()) {
				return false;
			}

			if (other.is_trivial() && other.cast<int>() > 1) {
				return true;
			}

			return false;
		}
	case ValueType::String:
		{
			if (other.is_string()) {
				return m_string < other.m_string;
			}

			if (other.is_trivial() && m_string.is_integer()) {
				return m_string.to_integer() < other.cast<int>();
			}

			return false;
		}
	case ValueType::Function:
		{
			return m_function->name() < other.m_function->name();	
		}
	case ValueType::Bool:
		{
			if (other.m_type == ValueType::Bool) {
				return m_trivial.b < other.m_trivial.b;
			}

			if (other.is_trivial()) {
				return m_trivial.b < other.cast<bool>();
			}

			return false;
		}
	case ValueType::Int:
		{
			if (other.m_type == ValueType::Int) {
				return m_trivial.i < other.m_trivial.i;
			}

			if (other.is_trivial()) {
				return m_trivial.i < other.cast<int>();
			}

			if (other.is_string() && other.string().is_integer()) {
				return m_trivial.i < other.string().to_integer();
			}
			
			return false;
		}
	case ValueType::Float:
		{
			if (other.m_type == ValueType::Float) {
				return m_trivial.f < other.m_trivial.f;
			}

			if (other.is_trivial()) {
				return m_trivial.f < other.cast<float>();
			}

			if (other.is_string() && (other.string().is_integer() || other.string().is_float())) {
				return m_trivial.f < other.string().to_float();
			}

			return false;
		}
	case ValueType::Double:
		{
			if (other.m_type == ValueType::Float) {
				return m_trivial.f < other.m_trivial.f;
			}

			if (other.is_trivial()) {
				return m_trivial.f < other.cast<float>();
			}

			if (other.is_string() && (other.string().is_integer() || other.string().is_float())) {
				return m_trivial.f < other.string().to_float();
			}

			return false;
		}
	default: ;
	}
	
	return false;
}

bool ysen::lang::astvm::Value::operator==(const Value& other) const
{
	if (m_type != other.m_type) {
		return false;
	}

	switch (m_type) {
	case ValueType::Undefined: return other.is_undefined();
	case ValueType::Null: return other.is_null();
	case ValueType::String: return string() == other.string();
	case ValueType::Bool: return get<bool>() == other.get<bool>();
	case ValueType::Int: return get<int>() == other.get<int>();
	case ValueType::Float: return std::abs(get<float>() - other.get<float>()) < 1e-9f;
	case ValueType::Double: return std::abs(get<double>() - other.get<double>()) < 1e-9;
	default: ;
	}

	return false;
}

ysen::lang::astvm::Value ysen::lang::astvm::Value::operator+(const Value& other) const
{
	if (m_type == other.m_type) {
		return same_type_bin_op(other, std::plus<>{});
	}

	return casted_type_bin_op(other, std::plus<>{});
}

ysen::lang::astvm::Value ysen::lang::astvm::Value::operator-(const Value& other) const
{
	if (m_type == other.m_type) {
		return same_type_bin_op(other, std::minus<>{});
	}

	return casted_type_bin_op(other, std::minus<>{});
}

ysen::lang::astvm::Value ysen::lang::astvm::Value::operator/(const Value& other) const
{
	if (m_type == other.m_type) {
		return same_type_bin_op(other, std::divides<>{});
	}

	return casted_type_bin_op(other, std::divides<>{});
}

ysen::lang::astvm::Value ysen::lang::astvm::Value::operator*(const Value& other) const
{
	if (m_type == other.m_type) {
		return same_type_bin_op(other, std::multiplies<>{});
	}

	return casted_type_bin_op(other, std::multiplies<>{});
}

void ysen::lang::astvm::Value::reset()
{
	m_array.clear();
	m_object.clear();
	m_string = {};
	m_trivial = {};
	m_function.release();
}

ysen::lang::astvm::Value& ysen::lang::astvm::Value::operator=(const Value& v)
{
	reset();
	m_type = v.m_type;
	m_array = v.m_array;
	m_object = v.m_object;
	m_string = v.m_string;
	m_function = v.m_function;
	m_trivial = v.m_trivial;
	return *this;	
}
ysen::lang::astvm::Value& ysen::lang::astvm::Value::operator=(Value&& v) noexcept
{
	reset();
	m_type = v.m_type;
	m_array = std::move(v.m_array);
	m_object = std::move(v.m_object);
	m_string = std::move(v.m_string);
	m_function = std::move(v.m_function);
	std::swap(m_trivial, v.m_trivial);
	return *this;	
}
ysen::lang::astvm::Value& ysen::lang::astvm::Value::operator=(Array v)
{
	reset();
	m_type = ValueType::Array;
	m_array = std::move(v);
	return *this;
}
ysen::lang::astvm::Value& ysen::lang::astvm::Value::operator=(Object v)
{
	reset();
	m_type = ValueType::Object;
	m_object = std::move(v);
	return *this;
}
ysen::lang::astvm::Value& ysen::lang::astvm::Value::operator=(core::String v)
{
	reset();
	m_type = ValueType::String;
	m_string = std::move(v);
	return *this;
}
ysen::lang::astvm::Value& ysen::lang::astvm::Value::operator=(const char* v)
{
	reset();
	m_type = ValueType::String;
	m_string = v;
	return *this;
}
ysen::lang::astvm::Value& ysen::lang::astvm::Value::operator=(bool v)
{
	reset();
	m_type = ValueType::Bool;
	m_trivial.b = v;
	return *this;
}
ysen::lang::astvm::Value& ysen::lang::astvm::Value::operator=(int v)
{
	reset();
	m_type = ValueType::Int;
	m_trivial.i = v;
	return *this;
}
ysen::lang::astvm::Value& ysen::lang::astvm::Value::operator=(float v)
{
	reset();
	m_type = ValueType::Float;
	m_trivial.f = v;
	return *this;
}
ysen::lang::astvm::Value& ysen::lang::astvm::Value::operator=(double v)
{
	reset();
	m_type = ValueType::Double;
	m_trivial.d = v;
	return *this;
}

ysen::lang::astvm::Value& ysen::lang::astvm::Value::operator=(FunctionPtr v)
{
	reset();
	m_type = ValueType::Function;
	m_function = std::move(v);
	return *this;
}

size_t std::hash<ysen::lang::astvm::Value>::operator()(const ysen::lang::astvm::Value& v) const noexcept
{
	return v.hash();
}
