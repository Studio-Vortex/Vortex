#pragma once

#include <cstdint>
#include <string>

extern "C"
{
	typedef struct _MonoClass MonoClass;
	typedef struct _MonoObject MonoObject;
	typedef struct _MonoMethod MonoMethod;
}

namespace Vortex {

	class ScriptUtils
	{
	public:
		static MonoMethod* GetManagedMethodFromName(MonoClass* klass, const std::string& name, uint32_t parameterCount);
		static MonoObject* InstantiateClass(MonoClass* klass);
		static MonoObject* InvokeMethod(MonoObject* instance, MonoMethod* method, void** params = nullptr);
	};

	class ManagedArray
	{
	public:
	};

}
