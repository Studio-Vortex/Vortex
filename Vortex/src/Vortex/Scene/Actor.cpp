#include "vxpch.h"
#include "Actor.h"

#include "Vortex/Scripting/ScriptEngine.h"

namespace Vortex {

	void Actor::SetActive(bool active)
	{
		GetComponent<TagComponent>().IsActive = active;

		const Actor self = *this;

		auto callMethod = [&](auto method) {
			if (!ScriptEngine::HasValidScriptClass(self))
				return;
			
			if (!ScriptEngine::ScriptInstanceHasMethod(self, method))
				return;
			
			ScriptEngine::Invoke(method, self);
		};

		if (active)
		{
			m_Scene->ActiveateChildren(self);

			// Invoke Actor.OnEnable
			if (m_Scene->IsRunning())
			{
				callMethod(ManagedMethod::OnEnable);
			}
		}
		else
		{
			m_Scene->DeactiveateChildren(self);

			// Invoke Actor.OnDisable
			if (m_Scene->IsRunning())
			{
				callMethod(ManagedMethod::OnDisable);
			}
		}
	}

}
