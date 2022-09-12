#pragma once

#include "Sparky/Scene/Entity.h"

#include <entt/entt.hpp>

namespace Sparky {

	class ScriptableEntity
	{
	public:
		template <typename TComponent>
		inline TComponent& GetComponent()
		{
			return m_Entity.GetComponent<TComponent>();
		}

	private:
		Entity m_Entity;

		friend class Scene;
	};

}