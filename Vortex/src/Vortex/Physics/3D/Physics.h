#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Core/TimeStep.h"
#include "Vortex/Physics/3D/PhysXTypes.h"
#include "Vortex/Scene/Scene.h"
#include "Vortex/Scene/Entity.h"
#include "Vortex/Scene/Components.h"

namespace physx {

	class PxScene;
	class PxPhysics;
	class PxMaterial;
	class PxRigidActor;
	class PxRigidDynamic;
	class PxControllerManager;
	class PxSimulationStatistics;

}

namespace Vortex {

	struct PhysicsBodyData
	{
		UUID EntityUUID = 0;
	};

	struct RaycastHit
	{
		uint64_t EntityID;
		Math::vec3 Position;
		Math::vec3 Normal;
		float Distance;
	};

	struct Collision
	{
		uint64_t EntityID;
	};

	class Physics
	{
	public:
		static void Init();
		static void Shutdown();

		static void CreatePhysicsActor(Entity entity);
		static void CreatePhysicsActorFromMesh(Entity entity);
		static void DestroyPhysicsActor(Entity entity);

		static void OnSimulationStart(Scene* contextScene);
		static void OnSimulationUpdate(TimeStep delta, Scene* contextScene);
		static void OnSimulationStop(Scene* contextScene);

		static physx::PxScene* GetPhysicsScene();
		static physx::PxPhysics* GetPhysicsFactory();
		static physx::PxControllerManager* GetControllerManager();

		static void WakeUpActor(Entity entity);
		static void WakeUpActors();

		static void UpdateDynamicActorFlags(const RigidBodyComponent& rigidbody, physx::PxRigidDynamic* dynamicActor);

		static uint32_t GetPhysicsScenePositionIterations() { return s_PhysicsSolverIterations; }
		static void SetPhysicsScenePositionIterations(uint32_t positionIterations) { s_PhysicsSolverIterations = positionIterations; }

		static uint32_t GetPhysicsSceneVelocityIterations() { return s_PhysicsSolverVelocityIterations; }
		static void SetPhysicsSceneVelocityIterations(uint32_t veloctiyIterations) { s_PhysicsSolverVelocityIterations = veloctiyIterations; }

		static Math::vec3 GetPhysicsSceneGravity() { return s_PhysicsSceneGravity; }
		static void SetPhysicsSceneGravity(const Math::vec3& gravity) { s_PhysicsSceneGravity = gravity; }

		static void CreateCollider(Entity entity);
		static physx::PxMaterial* CreatePhysicsMaterial(const PhysicsMaterialComponent& material);

		static void SetCollisionFilters(physx::PxRigidActor* actor, uint32_t filterGroup, uint32_t filterMask);

		static physx::PxSimulationStatistics* GetSimulationStatistics();

	private:
		inline constexpr static float s_FixedTimeStep = 1.0f / 100.0f;
		inline static Math::vec3 s_PhysicsSceneGravity = Math::vec3(0.0f, -9.81f, 0.0f);
		inline static uint32_t s_PhysicsSolverIterations = 8;
		inline static uint32_t s_PhysicsSolverVelocityIterations = 2;
		inline static std::unordered_map<UUID, physx::PxRigidActor*> s_StaticActorsFromMeshes;
	};

}
