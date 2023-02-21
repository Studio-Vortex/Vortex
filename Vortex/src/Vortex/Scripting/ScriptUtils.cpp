#include "vxpch.h"
#include "ScriptUtils.h"

#include "Vortex/Scripting/ScriptEngine.h"

#include <mono/metadata/object.h>
#include <mono/metadata/reflection.h>

namespace Vortex {

	MonoMethod* ScriptUtils::GetManagedMethodFromName(MonoClass* klass, const std::string& name, uint32_t parameterCount)
	{
		return mono_class_get_method_from_name(klass, name.c_str(), parameterCount);
	}

	MonoObject* ScriptUtils::InstantiateClass(MonoClass* klass)
	{
		MonoObject* object = mono_object_new(ScriptEngine::GetAppDomain(), klass);
		mono_runtime_object_init(object);
		return object;
	}

	MonoObject* ScriptUtils::InvokeMethod(MonoObject* instance, MonoMethod* method, void** params)
	{
		MonoObject* exception = nullptr;
		return mono_runtime_invoke(method, instance, params, &exception);
	}

}
