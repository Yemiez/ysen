#include "format.h"

#include "ysen/lang/astvm/Value.h"

ysen::core::String ysen::core::details::Formatter<ysen::lang::astvm::Value>::format(const lang::astvm::Value& value)
{
	return value.to_string();
}
