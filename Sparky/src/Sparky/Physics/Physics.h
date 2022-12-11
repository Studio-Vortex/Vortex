#pragma once

#include "Sparky/Core/Base.h"
#include "Sparky/Scene/Scene.h"
#include "Sparky/Scene/Entity.h"
#include "Sparky/Scene/Components.h"
#include "Sparky/Core/TimeStep.h"

namespace physx {

	class PxScene;
	class PxPhysics;
	class PxRigidActor;
	class PxRigidDynamic;
	class PxControllerManager;

}

namespace Sparky {

	struct PhysicsBodyData
	{
		UUID EntityUUID;
	};

	struct RaycastHit
	{
		uint64_t EntityID;
		Math::vec3 Position;
		Math::vec3 Normal;
		float Distance;
	};

	class Physics
	{
	public:
		static void CreatePhysicsBody(Entity entity, const TransformComponent& transform, RigidBodyComponent& rigidbody);
		static void DestroyPhysicsBody(Entity entity);

		static void OnSimulationStart(Scene* contextScene);
		static void OnSimulationUpdate(TimeStep delta, Scene* contextScene);
		static void OnSimulationStop();

		static physx::PxScene* GetPhysicsScene();
		static physx::PxPhysics* GetPhysicsFactory();
		static physx::PxControllerManager* GetControllerManager();

		static void UpdateDynamicActorFlags(const RigidBodyComponent& rigidbody, physx::PxRigidDynamic* dynamicActor);

		static uint32_t GetPhysicsPositionIterations() { return s_PhysicsSolverIterations; }
		static void SetPhysicsPositionIterations(uint32_t positionIterations) { s_PhysicsSolverIterations = positionIterations; }

		static uint32_t GetPhysicsVelocityIterations() { return s_PhysicsSolverVelocityIterations; }
		static void SetPhysicsVelocityIterations(uint32_t veloctiyIterations) { s_PhysicsSolverVelocityIterations = veloctiyIterations; }

		static Math::vec3 GetPhysicsSceneGravity() { return s_PhysicsSceneGravity; }
		static void SetPhysicsSceneGravitty(const Math::vec3& gravity) { s_PhysicsSceneGravity = gravity; }

		static void SetCollisionFilters(physx::PxRigidActor* actor, uint32_t filterGroup, uint32_t filterMask);

	private:
		inline constexpr static float s_FixedTimeStep = 1.0f / 100.0f;
		inline static Math::vec3 s_PhysicsSceneGravity = Math::vec3(0.0f, -9.8f, 0.0f);
		inline static uint32_t s_PhysicsSolverIterations = 8;
		inline static uint32_t s_PhysicsSolverVelocityIterations = 2;
	};

}
