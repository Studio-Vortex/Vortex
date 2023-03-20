#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Core/ReferenceCounting/RefCounted.h"

#include "Vortex/Scripting/ScriptField.h"

#include <string>
#include <map>

extern "C"
{
	typedef struct _MonoClass MonoClass;
	typedef struct _MonoObject MonoObject;
	typedef struct _MonoMethod MonoMethod;
}

namespace Vortex {

	class ScriptClass : public RefCounted
	{
	public:
		ScriptClass() = default;
		ScriptClass(const std::string& classNamespace, const std::string& className, bool isCore = false);
		~ScriptClass() = default;

		MonoObject* Instantiate();
		MonoMethod* GetMethod(const std::string& name, int parameterCount);

		VX_FORCE_INLINE std::map<std::string, ScriptField> GetFields() { return m_Fields; }
		VX_FORCE_INLINE MonoClass* GetMonoClass() const { return m_MonoClass; }

	private:
		std::string m_ClassNamespace;
		std::string m_ClassName;

		std::map<std::string, ScriptField> m_Fields;

		MonoClass* m_MonoClass = nullptr;

	private:
		friend class ScriptEngine;
	};

}
