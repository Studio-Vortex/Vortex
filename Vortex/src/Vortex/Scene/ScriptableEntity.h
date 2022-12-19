#pragma once

#include "Vortex/Scene/Entity.h"
#include "Vortex/Events/MouseEvent.h"

#include <entt/entt.hpp>

namespace Vortex {

	class ScriptableEntity
	{
	public:
		virtual ~ScriptableEntity() { }

		template <typename TComponent>
		inline TComponent& GetComponent()
		{
			return m_Entity.GetComponent<TComponent>();
		}

	protected:
		virtual void OnCreate() {}
		virtual void OnUpdate(TimeStep delta) {}
		virtual void OnDestroy() {}

	private:
		Entity m_Entity;

		friend class Scene;
	};

}