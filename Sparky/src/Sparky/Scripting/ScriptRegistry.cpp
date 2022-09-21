#include "sppch.h"
#include "ScriptRegistry.h"

#include <mono/metadata/object.h>

namespace Sparky {

#define SP_ADD_INTERNAL_CALL(Name) mono_add_internal_call("Sparky.InternalCalls::" #Name, Name)

	static void NativeLog(MonoString* text, int parameter)
	{
		char* cStr = mono_string_to_utf8(text);
		std::string string(cStr);

		mono_free(cStr);
		SP_CORE_TRACE("text = {}, parameter = {}", string, parameter);
	}

	static void NativeLog_Vector(Math::vec3* parameter, Math::vec3* outParameter)
	{
		SP_CORE_WARN("Vector3 from C# {}", *parameter);

		*outParameter = Math::Cross(*parameter, Math::vec3(parameter->x, parameter->y, -parameter->z));
	}

	void ScriptRegistry::RegisterMethods()
	{
		SP_ADD_INTERNAL_CALL(NativeLog);
		SP_ADD_INTERNAL_CALL(NativeLog_Vector);
	}

}