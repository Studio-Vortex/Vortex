#pragma once

#include "Sparky/Scene/Scene.h"
#include "Sparky/Scene/Entity.h"
#include "Sparky/Scene/Components.h"
#include "Sparky/Core/TimeStep.h"

#include <PhysX/PxPhysicsAPI.h>

namespace Sparky {

	class Physics
	{
	public:
		static void CreatePhysicsBody(Entity entity, const TransformComponent& transform, RigidBodyComponent& rigidbody);
		static void DestroyPhysicsBody(Entity entity);

		static void OnSimulationStart(Scene* contextScene);
		static void OnSimulationUpdate(TimeStep delta, Scene* contextScene);
		static void OnSimulationStop();

		static int32_t GetPhysicsSceneIterations() { return s_PhysicsSceneIterations; }
		static void SetPhysicsSceneIterations(int32_t positionIterations) { s_PhysicsSceneIterations = positionIterations; }

		static Math::vec3 GetPhysicsSceneGravity() { return s_PhysicsSceneGravity; }
		static void SetPhysicsSceneGravitty(const Math::vec3& gravity) { s_PhysicsSceneGravity = gravity; }

	private:
		inline static physx::PxDefaultAllocator s_DefaultAllocator;
		inline static physx::PxDefaultErrorCallback s_ErrorCallback;
		inline static physx::PxFoundation* s_Foundation = nullptr;
		inline static physx::PxPhysics* s_Physics = nullptr;
		inline static physx::PxDefaultCpuDispatcher* s_Dispatcher = nullptr;
		inline static physx::PxScene* s_PhysicsScene = nullptr;
		inline static physx::PxTolerancesScale s_ToleranceScale;

		inline static Math::vec3 s_PhysicsSceneGravity = Math::vec3(0.0f, -9.8f, 0.0f);
		inline static int32_t s_PhysicsSceneIterations = 20;
	};

}
