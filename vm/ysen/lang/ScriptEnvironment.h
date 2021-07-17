#pragma once
#include "IEnvironment.h"

namespace ysen::lang {namespace astvm {
		class Interpreter;
	}

	class ScriptEnvironment : public IEnvironment
	{
	public:
		ScriptEnvironment();
	
		astvm::ValuePtr eval(const core::String& code) override;
		astvm::ValuePtr eval_file(const core::String& filename) override;

	private:
		core::SharedPtr<astvm::Interpreter> m_interpreter{};
	};
	
}