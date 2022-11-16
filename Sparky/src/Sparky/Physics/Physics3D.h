#pragma once

#include "Sparky/Scene/Scene.h"
#include "Sparky/Scene/Entity.h"
#include "Sparky/Scene/Components.h"
#include "Sparky/Core/TimeStep.h"

class q3Scene;

namespace Sparky {

	class Physics3D
	{
	public:
		static void CreatePhysicsBody(Entity entity, const TransformComponent& transform, RigidBodyComponent& rigidbody);
		static void DestroyPhysicsBody(Entity entity);

		static void OnPhysicsSimulate(Scene* contextScene);
		static void OnPhysicsUpdate(TimeStep delta, Scene* contextScene);
		static void OnPhysicsStop();

		static q3Scene* GetPhysicsScene() { return s_PhysicsScene; }

		static int32_t GetPhysicsSceneIterations() { return s_PhysicsSceneIterations; }
		static void SetPhysicsSceneIterations(int32_t positionIterations) { s_PhysicsSceneIterations = positionIterations; }

		static Math::vec3 GetPhysicsSceneGravity() { return s_PhysicsSceneGravity; }
		static void SetPhysicsSceneGravitty(const Math::vec3& gravity) { s_PhysicsSceneGravity = gravity; }

	private:
		inline static q3Scene* s_PhysicsScene = nullptr;

		inline static Math::vec3 s_PhysicsSceneGravity = Math::vec3(0.0f, -9.8f, 0.0f);
		inline static int32_t s_PhysicsSceneIterations = 20;
	};

}
