#include "vxpch.h"
#include "Actor.h"

#include "Vortex/Scripting/ScriptEngine.h"

namespace Vortex {

	void Actor::SetActive(bool active)
	{
		GetComponent<TagComponent>().IsActive = active;

		const Actor self = *this;

		const bool sceneRunning = m_Scene->IsRunning();
		const bool hasScript = self.HasComponent<ScriptComponent>();
		const auto classExistsFn = [&](auto e) { return ScriptEngine::ActorClassExists(e.GetComponent<ScriptComponent>().ClassName); };
		const bool callMethod = sceneRunning && hasScript && classExistsFn(self);

		if (!active)
		{
			m_Scene->DeactiveateChildren(self);

			if (callMethod)
			{
				// Invoke Actor.OnDisabled
				ScriptEngine::Invoke(ManagedMethod::OnDisable, self);
			}

			return;
		}

		m_Scene->ActiveateChildren(self);

		if (callMethod)
		{
			// Invoke Actor.OnEnabled
			ScriptEngine::Invoke(ManagedMethod::OnEnable, self);
		}
	}

}
