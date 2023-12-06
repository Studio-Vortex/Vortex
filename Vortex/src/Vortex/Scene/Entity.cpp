#include "vxpch.h"
#include "Entity.h"

#include "Vortex/Scripting/ScriptEngine.h"

namespace Vortex {

	void Entity::SetActive(bool active)
	{
		GetComponent<TagComponent>().IsActive = active;

		const Entity self = *this;

		const bool sceneRunning = m_Scene->IsRunning();
		const bool hasScript = self.HasComponent<ScriptComponent>();
		const auto classExistsFn = [&](auto e) { return ScriptEngine::EntityClassExists(e.GetComponent<ScriptComponent>().ClassName); };
		const bool callMethod = sceneRunning && hasScript && classExistsFn(self);

		if (!active)
		{
			m_Scene->DeactiveateChildren(self);

			if (callMethod)
			{
				// Call Entity.OnDisabled
				ScriptEngine::CallMethod(ManagedMethod::OnDisabled, self);
			}

			return;
		}

		m_Scene->ActiveateChildren(self);

		if (callMethod)
		{
			// Call Entity.OnEnabled
			ScriptEngine::CallMethod(ManagedMethod::OnEnabled, self);
		}
	}

}
