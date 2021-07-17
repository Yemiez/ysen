#pragma once
#include "ysen/core/SharedPtr.h"
#include "ysen/core/String.h"

namespace ysen::lang {

	namespace astvm {
		class Value;
		using ValuePtr = core::SharedPtr<Value>;
	}
	
	class IEnvironment
	{
	public:
		virtual ~IEnvironment() = default;
		virtual astvm::ValuePtr eval(const core::String& code) = 0;
		virtual astvm::ValuePtr eval_file(const core::String& code) = 0;
	};
	
}
