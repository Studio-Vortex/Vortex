#pragma once

#include "Sparky/Scene/Scene.h"
#include "Sparky/Scene/Entity.h"
#include "Sparky/Scene/Components.h"
#include "Sparky/Core/TimeStep.h"

#include <entt/entt.hpp>

class q3Scene;

namespace Sparky {

	class Physics3D
	{
	public:
		static void CreatePhysicsBody(Entity entity, const TransformComponent& transform, RigidBodyComponent& rigidbody);

		static void OnPhysicsSimulate(entt::registry& sceneRegistry, Scene* contextScene);
		static void OnPhysicsUpdate(TimeStep delta, entt::registry& sceneRegistry, Scene* contextScene);
		static void OnPhysicsStop();

	private:
		inline static q3Scene* s_PhysicsScene = nullptr;
	};

}
