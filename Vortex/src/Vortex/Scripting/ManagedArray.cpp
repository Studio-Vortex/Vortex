#include "vxpch.h"
#include "ManagedArray.h"

#include "Vortex/Scripting/ScriptEngine.h"
#include "Vortex/Scripting/ScriptUtils.h"

namespace Vortex {

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
		MonoObject* boxed = ScriptUtils::InstantiateManagedClass(elementKlass);
		ScriptEngine::RT_ActorConstructor(value, boxed);
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
