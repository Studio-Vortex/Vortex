#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Scripting/ScriptFieldTypes.h"
#include "Vortex/Scripting/MonoAssemblyTypeInfo.h"

#include <cstdint>
#include <string>

#include <mono/metadata/class.h>
#include <mono/metadata/object.h>

#include <vector>

namespace Vortex {

	class ScriptUtils
	{
	public:
		static MonoMethod* GetManagedMethodFromName(MonoClass* klass, const std::string& name, uint32_t parameterCount);
		static MonoObject* InstantiateClass(MonoClass* klass);
		static MonoObject* InvokeMethod(MonoObject* instance, MonoMethod* method, void** params = nullptr);

		static MonoAssembly* LoadMonoAssembly(const std::filesystem::path& filepath, bool loadPdb = false);
		static void PrintAssemblyTypes(MonoAssembly* assembly);
		static std::vector<MonoAssemblyTypeInfo> GetAssemblyTypeInfo(MonoAssembly* assembly);

		static ScriptFieldType MonoTypeToScriptFieldType(MonoType* monoType);
		static const char* ScriptFieldTypeToString(ScriptFieldType type);
		static ScriptFieldType StringToScriptFieldType(std::string_view fieldType);
	};

}
