#include "vxpch.h"
#include "ScriptClass.h"

#include "Vortex/Scripting/ScriptUtils.h"
#include "Vortex/Scripting/ScriptEngine.h"

namespace Vortex {

	ScriptClass::ScriptClass(const std::string& classNamespace, const std::string& className, bool isCore)
		: m_ClassNamespace(classNamespace), m_ClassName(className)
	{
		MonoImage* assemblyImage = isCore ? ScriptEngine::GetCoreAssemblyImage() : ScriptEngine::GetAppAssemblyImage();
		m_MonoClass = mono_class_from_name(assemblyImage, classNamespace.c_str(), className.c_str());
	}

	MonoObject* ScriptClass::Instantiate()
	{
		return ScriptEngine::InstantiateClass(m_MonoClass);
	}

	MonoMethod* ScriptClass::GetMethod(const std::string& name, int parameterCount)
	{
		return ScriptUtils::GetManagedMethodFromName(m_MonoClass, name.c_str(), parameterCount);
	}

}
