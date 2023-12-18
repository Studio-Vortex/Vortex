#pragma once

#include "Vortex/Scene/Actor.h"
#include "Vortex/Events/MouseEvent.h"

namespace Vortex {

	class ScriptableActor
	{
	public:
		virtual ~ScriptableActor() { }

		template <typename TComponent>
		inline TComponent& GetComponent()
		{
			return m_Actor.GetComponent<TComponent>();
		}

	protected:
		virtual void OnCreate() {}
		virtual void OnUpdate(TimeStep delta) {}
		virtual void OnDestroy() {}

	private:
		Actor m_Actor;

		friend class Scene;
	};

}