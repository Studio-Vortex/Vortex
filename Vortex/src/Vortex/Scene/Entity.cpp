#include "vxpch.h"
#include "Entity.h"

#include "Vortex/Scripting/ScriptEngine.h"

namespace Vortex {

	void Entity::SetActive(bool active)
	{
		GetComponent<TagComponent>().IsActive = active;

		Entity self = *this;

		const bool shouldCallActivationMethod = m_Scene->IsRunning()
			&& self.HasComponent<ScriptComponent>()
			&& ScriptEngine::EntityClassExists(self.GetComponent<ScriptComponent>().ClassName);

		if (active)
		{
			m_Scene->ActiveateChildren(self);

			if (shouldCallActivationMethod)
			{
				ScriptEngine::OnEnabled(self);
			}
		}
		else
		{
			m_Scene->DeactiveateChildren(self);

			if (shouldCallActivationMethod)
			{
				ScriptEngine::OnDisabled(self);
			}
		}
	}

}
