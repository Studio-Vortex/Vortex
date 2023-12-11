#include "vxpch.h"
#include "ScriptUtils.h"

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
		int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

		for (int32_t i = 0; i < numTypes; i++)
		{
			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

			const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
			const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);
			VX_CONSOLE_LOG_TRACE("{}.{}", nameSpace, name);
		}
	}

	std::vector<MonoAssemblyTypeInfo> ScriptUtils::GetAssemblyTypeInfo(MonoAssembly* assembly)
	{
		std::vector<MonoAssemblyTypeInfo> result;

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
			case ScriptFieldType::Entity:      return "Entity";
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
		if (fieldType == "Entity")      return ScriptFieldType::Entity;
		if (fieldType == "AssetHandle") return ScriptFieldType::AssetHandle;

		VX_CORE_ASSERT(false, "Unknown Script Field Type!");
		return ScriptFieldType::None;
	}

	ManagedArray::ManagedArray(MonoClass* elementKlass, uintptr_t arrayLength)
	{
		m_ManagedArray = mono_array_new(ScriptEngine::GetAppDomain(), elementKlass, arrayLength);
		VX_CORE_ASSERT(m_ManagedArray, "Failed to create managed array!");
	}

	uintptr_t ManagedArray::Size() const
	{
		return mono_array_length(m_ManagedArray);
	}

	void ManagedArray::SetValue(uintptr_t index, MonoClass* elementKlass, MonoObject* value)
	{
		// TODO
	}

	void ManagedArray::SetValue(uintptr_t index, MonoClass* elementKlass, UUID value)
	{
		MonoObject* boxed = ScriptUtils::InstantiateClass(elementKlass);
		ScriptEngine::EntityConstructorRuntime(value, boxed);
		SetValueInternal(index, boxed);
	}

	void ManagedArray::SetValueInternal(uintptr_t index, const MonoObject* boxed) const
	{
		mono_array_setref(m_ManagedArray, index, const_cast<MonoObject*>(boxed));
	}

	MonoClass* ManagedArray::GetArrayClass() const
	{
		return mono_object_get_class((MonoObject*)m_ManagedArray);
	}

	MonoClass* ManagedArray::GetElementClass() const
	{
		return mono_class_get_element_class(GetArrayClass());
	}

	int32_t ManagedArray::GetElementSize() const
	{
		return mono_array_element_size(GetArrayClass());
	}

	MonoType* ManagedArray::GetElementType() const
	{
		return mono_class_get_type(GetElementClass());
	}

	bool ManagedArray::TypeIsReferenceOrByRef() const
	{
		MonoType* elementType = GetElementType();
		return mono_type_is_reference(elementType) || mono_type_is_byref(elementType);
	}

	MonoArray* ManagedArray::GetHandle() const
	{
		return m_ManagedArray;
	}

}
