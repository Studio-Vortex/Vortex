#pragma once

#include <cstdint>
#include <string>

extern "C"
{
	typedef struct _MonoClass MonoClass;
	typedef struct _MonoMethod MonoMethod;
}

namespace Vortex {

	class ScriptUtils
	{
		MonoMethod* GetMethodFromManagedClass(MonoClass* klass, const std::string& name, uint32_t parameterCount);
	};

	class ManagedArray
	{

	};

}
