#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Core/UUID.h"

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

	class ManagedArray
	{
	public:
		ManagedArray(MonoClass* elementKlass, uintptr_t arrayLength);

		template <typename TElementType>
		VX_FORCE_INLINE void FillFromVector(const std::vector<TElementType>& vector)
		{
			VX_PROFILE_FUNCTION();

			uintptr_t arrayLength = Size();

			VX_CORE_ASSERT(vector.size() == arrayLength, "Inconsistent sizes!");

			for (uintptr_t i = 0; i < arrayLength; i++)
			{
				MonoClass* elementClass = GetElementClass();
				int32_t elementSize = GetElementSize();
				MonoType* elementType = GetElementType();

				if (TypeIsReferenceOrByRef())
				{
					SetValue(i, elementClass, vector[i]);
				}
			}
		}

		uintptr_t Size() const;

		void SetValue(uintptr_t index, MonoClass* elementKlass, MonoObject* value);
		void SetValue(uintptr_t index, MonoClass* elementKlass, UUID value);

		MonoArray* GetHandle() const;

	private:
		void SetValueInternal(uintptr_t index, const MonoObject* boxed) const;
		MonoClass* GetArrayClass() const;
		MonoClass* GetElementClass() const;
		int32_t GetElementSize() const;
		MonoType* GetElementType() const;
		bool TypeIsReferenceOrByRef() const;

	private:
		MonoArray* m_ManagedArray = nullptr;
	};

}
