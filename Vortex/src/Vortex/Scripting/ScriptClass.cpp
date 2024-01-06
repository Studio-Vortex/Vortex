#include "vxpch.h"
#include "ScriptClass.h"

#include "Vortex/Scripting/ScriptEngine.h"
#include "Vortex/Scripting/ScriptUtils.h"

#include <mono/jit/jit.h>

namespace Vortex {

	ScriptClass::ScriptClass(const std::string& classNamespace, const std::string& className, bool isCore)
		: m_ClassNamespace(classNamespace), m_ClassName(className)
	{
		MonoImage* assemblyImage = isCore ? ScriptEngine::GetScriptCoreAssemblyImage() : ScriptEngine::GetAppAssemblyImage();
		m_MonoClass = ScriptUtils::GetClassFromAssemblyImageByName(assemblyImage, classNamespace, className);
	}

	MonoObject* ScriptClass::Instantiate()
	{
		return ScriptUtils::InstantiateManagedClass(m_MonoClass);
	}

	MonoMethod* ScriptClass::GetMethod(const std::string& name, uint32_t parameterCount)
	{
		return ScriptUtils::GetManagedMethodFromName(m_MonoClass, name.c_str(), parameterCount);
	}

	const std::string& ScriptClass::GetClassNamespace() const
	{
		return m_ClassNamespace;
	}

	const std::string& ScriptClass::GetClassNameV() const
	{
		return m_ClassName;
	}

	ScriptField& ScriptClass::GetField(const std::string& fieldName)
	{
		return m_Fields[fieldName];
	}

	const ScriptField& ScriptClass::GetField(const std::string& fieldName) const
	{
		VX_CORE_ASSERT(m_Fields.contains(fieldName), "Invalid field name!");
		return m_Fields.at(fieldName);
	}

	void ScriptClass::SetField(const std::string& fieldName, const ScriptField& scriptField)
	{
		m_Fields[fieldName] = scriptField;
	}

}
