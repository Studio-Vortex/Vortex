#include "vxpch.h"
#include "ScriptUtils.h"

#include "Vortex/Scripting/ManagedString.h"
#include "Vortex/Scripting/ScriptFieldTypesMap.h"
#include "Vortex/Scripting/ScriptEngine.h"

#include <mono/metadata/object.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/reflection.h>
#include <mono/metadata/mono-debug.h>

namespace Vortex {

	MonoMethod* ScriptUtils::GetManagedMethodFromName(MonoClass* klass, const std::string& name, uint32_t parameterCount)
	{
		MonoMethod* method = mono_class_get_method_from_name(klass, name.c_str(), parameterCount);
		return method;
	}

	MonoObject* ScriptUtils::InstantiateManagedClass(MonoClass* klass)
	{
		MonoObject* object = mono_object_new(ScriptEngine::GetAppDomain(), klass);
		mono_runtime_object_init(object);
		return object;
	}

	RT_ScriptInvokeResult ScriptUtils::InvokeManagedMethod(MonoObject* instance, MonoMethod* method, void** params)
	{
		RT_ScriptInvokeResult result;
		result.ReturnValue = mono_runtime_invoke(method, instance, params, &result.Exception);
		return result;
	}

	MonoAssembly* ScriptUtils::LoadMonoAssembly(const std::filesystem::path& filepath, bool loadPdb)
	{
		UniqueBuffer fileData = FileSystem::ReadBinary(filepath);
		if (!fileData)
		{
			return nullptr;
		}

		// NOTE: We can't use this image for anything other than loading the assembly
		//       because this image doesn't have a reference to the assembly
		MonoImageOpenStatus status;
		MonoImage* image = mono_image_open_from_data_full(fileData.As<char>(), fileData.Size(), 1, &status, 0);
		if (status != MONO_IMAGE_OK)
		{
			const char* errorMessage = mono_image_strerror(status);
			VX_CONSOLE_LOG_ERROR("Mono Assembly Error: {}", errorMessage);
			return nullptr;
		}

		if (loadPdb)
		{
			std::filesystem::path assemblyPath = filepath;
			std::filesystem::path pdbPath = assemblyPath.replace_extension(".pdb");

			if (FileSystem::Exists(pdbPath))
			{
				UniqueBuffer pdbFileData = FileSystem::ReadBinary(pdbPath);
				mono_debug_open_image_from_memory(image, pdbFileData.As<const mono_byte>(), pdbFileData.Size());
				VX_CONSOLE_LOG_INFO("PDB Loaded : {}", pdbPath);
			}
		}

		std::string pathString = filepath.string();
		MonoAssembly* assembly = mono_assembly_load_from_full(image, pathString.c_str(), &status, 0);
		mono_image_close(image);

		return assembly;
	}

	void ScriptUtils::PrintAssemblyTypes(MonoAssembly* assembly)
	{
		MonoImage* image = mono_assembly_get_image(assembly);
		const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
		const int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

		for (int32_t i = 0; i < numTypes; i++)
		{
			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

			const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
			const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);
			VX_CONSOLE_LOG_TRACE("{}.{}", nameSpace, name);
		}
	}

	std::vector<ScriptAssemblyTypedefInfo> ScriptUtils::GetAssemblyTypeInfo(MonoAssembly* assembly)
	{
		std::vector<ScriptAssemblyTypedefInfo> result;

		MonoImage* image = mono_assembly_get_image(assembly);
		const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
		int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

		for (int32_t i = 0; i < numTypes; i++)
		{
			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

			const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
			const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);

			MonoClass* monoClass = mono_class_from_name(image, nameSpace, name);

			if (name[0] == '<' || !monoClass)
				continue;

			int fieldCount = mono_class_num_fields(monoClass);

			result.emplace_back(nameSpace, name, fieldCount);
		}

		return result;
	}

	MonoClass* ScriptUtils::GetClassFromAssemblyImageByName(MonoImage* assemblyImage, const std::string& classNamespace, const std::string& className)
	{
		return mono_class_from_name(assemblyImage, classNamespace.c_str(), className.c_str());
	}

	struct MonoExceptionInfo
	{
		std::string Typename;
		std::string Source;
		std::string Message;
		std::string StackTrace;
	};

	static MonoExceptionInfo GetExceptionInfo(MonoObject* exception)
	{
		MonoClass* exceptionClass = mono_object_get_class(exception);
		MonoType* exceptionType = mono_class_get_type(exceptionClass);

		auto GetExceptionStringFn = [exception, exceptionClass](const char* stringName) -> std::string {
			MonoProperty* property = mono_class_get_property_from_name(exceptionClass, stringName);

			if (property == nullptr)
				return "";

			MonoMethod* getterMethod = mono_property_get_get_method(property);

			if (getterMethod == nullptr)
				return "";

			MonoString* string = (MonoString*)mono_runtime_invoke(getterMethod, exception, NULL, NULL);
			ManagedString mstring(string);
			return mstring.String();
		};

		MonoExceptionInfo result;
		result.Typename = mono_type_get_name(exceptionType);
		result.Source = GetExceptionStringFn("Source");
		result.Message = GetExceptionStringFn("Message");
		result.StackTrace = GetExceptionStringFn("StackTrace");
		return result;
	}

	void ScriptUtils::RT_HandleInvokeResult(const RT_ScriptInvokeResult& result)
	{
		if (result.Exception == nullptr)
		{
			return;
		}
		
		MonoExceptionInfo exceptionInfo = GetExceptionInfo(result.Exception);
		VX_CONSOLE_LOG_ERROR("[Script Engine] {}: {}. Source {}, StackTrace: {}", exceptionInfo.Typename, exceptionInfo.Message, exceptionInfo.Source, exceptionInfo.StackTrace);
	}

	bool ScriptUtils::RT_CheckError(MonoError* error)
	{
		if (mono_error_ok(error))
		{
			return false;
		}

		unsigned short errorCode = mono_error_get_error_code(error);
		const char* errorMessage = mono_error_get_message(error);

		VX_CONSOLE_LOG_ERROR("[Script Engine] Error: ({}): {}", errorCode, errorMessage);

		mono_error_cleanup(error);

		return true;
	}

	ScriptFieldType ScriptUtils::MonoTypeToScriptFieldType(MonoType* monoType)
	{
		const std::string typeName = mono_type_get_name(monoType);

		auto it = s_ScriptFieldTypeMap.find(typeName);
		if (it == s_ScriptFieldTypeMap.end())
		{
			return ScriptFieldType::None;
		}

		return it->second;
	}

	const char* ScriptUtils::ScriptFieldTypeToString(ScriptFieldType type)
	{
		switch (type)
		{
			case ScriptFieldType::None:        return "None";
			case ScriptFieldType::Float:       return "Float";
			case ScriptFieldType::Double:      return "Double";
			case ScriptFieldType::Bool:        return "Bool";
			case ScriptFieldType::Char:        return "Char";
			case ScriptFieldType::String:      return "String";
			case ScriptFieldType::Short:       return "Short";
			case ScriptFieldType::Int:         return "Int";
			case ScriptFieldType::Long:        return "Long";
			case ScriptFieldType::Byte:        return "Byte";
			case ScriptFieldType::UShort:      return "UShort";
			case ScriptFieldType::UInt:        return "UInt";
			case ScriptFieldType::ULong:       return "ULong";
			case ScriptFieldType::Vector2:     return "Vector2";
			case ScriptFieldType::Vector3:     return "Vector3";
			case ScriptFieldType::Vector4:     return "Vector4";
			case ScriptFieldType::Color3:      return "Color3";
			case ScriptFieldType::Color4:      return "Color4";
			case ScriptFieldType::Actor:      return "Entity";
			case ScriptFieldType::AssetHandle: return "AssetHandle";
		}

		VX_CORE_ASSERT(false, "Unknown Script Field Type!");
		return "None";
	}

	ScriptFieldType ScriptUtils::StringToScriptFieldType(std::string_view fieldType)
	{
		if (fieldType == "None")        return ScriptFieldType::None;
		if (fieldType == "Float")       return ScriptFieldType::Float;
		if (fieldType == "Double")      return ScriptFieldType::Double;
		if (fieldType == "Bool")        return ScriptFieldType::Bool;
		if (fieldType == "Char")        return ScriptFieldType::Char;
		if (fieldType == "String")      return ScriptFieldType::String;
		if (fieldType == "Short")       return ScriptFieldType::Short;
		if (fieldType == "Int")         return ScriptFieldType::Int;
		if (fieldType == "Long")        return ScriptFieldType::Long;
		if (fieldType == "Byte")        return ScriptFieldType::Byte;
		if (fieldType == "UShort")      return ScriptFieldType::UShort;
		if (fieldType == "UInt")        return ScriptFieldType::UInt;
		if (fieldType == "ULong")       return ScriptFieldType::ULong;
		if (fieldType == "Vector2")     return ScriptFieldType::Vector2;
		if (fieldType == "Vector3")     return ScriptFieldType::Vector3;
		if (fieldType == "Vector4")     return ScriptFieldType::Vector4;
		if (fieldType == "Color3")      return ScriptFieldType::Color3;
		if (fieldType == "Color4")      return ScriptFieldType::Color4;
		if (fieldType == "Entity")      return ScriptFieldType::Actor;
		if (fieldType == "AssetHandle") return ScriptFieldType::AssetHandle;

		VX_CORE_ASSERT(false, "Unknown Script Field Type!");
		return ScriptFieldType::None;
	}

}
