#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Core/UUID.h"
#include "Vortex/Core/TimeStep.h"
#include "Vortex/Physics/3D/PhysXTypes.h"
#include "Vortex/Scene/Scene.h"
#include "Vortex/Scene/Entity.h"
#include "Vortex/Scene/Components.h"

#include <unordered_map>

namespace physx {

	class PxScene;
	class PxPhysics;
	class PxMaterial;
	class PxController;
	class PxFixedJoint;
	class PxRigidActor;
	class PxRigidDynamic;
	class PxControllerManager;
	class PxSimulationStatistics;

}

namespace Vortex {

	struct PhysicsBodyData
	{
		UUID EntityUUID = 0;
		Scene* ContextScene = nullptr;
	};

	struct ConstrainedJointData
	{
		UUID EntityUUID = 0;
		bool IsBroken = false;
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
		static void OnSimulationStart(Scene* contextScene);
		static void OnSimulationUpdate(TimeStep delta);
		static void OnSimulationStop(Scene* contextScene);

		static void ReCreateActor(Entity entity);

		static physx::PxScene* GetPhysicsScene();

#ifndef VX_DIST
		static physx::PxSimulationStatistics* GetSimulationStatistics();
#endif

		static void WakeUpActor(Entity entity);
		static void WakeUpActors();

		static bool Raycast(const Math::vec3& origin, const Math::vec3& direction, float maxDistance, RaycastHit* outHitInfo);

		VX_FORCE_INLINE static const std::unordered_map<UUID, physx::PxRigidActor*>& GetActors() { return s_ActiveActors; }
		VX_FORCE_INLINE static const std::unordered_map<UUID, physx::PxController*>& GetControllers() { return s_ActiveControllers; }
		VX_FORCE_INLINE static const std::unordered_map<UUID, physx::PxFixedJoint*> GetFixedJoints() { return s_ActiveFixedJoints; }

		static physx::PxRigidActor* GetActor(UUID entityUUID);
		static physx::PxController* GetController(UUID entityUUID);
		static physx::PxFixedJoint* GetFixedJoint(UUID entityUUID);

		static const std::pair<Math::vec3, Math::vec3>& GetLastReportedFixedJointForces(physx::PxFixedJoint* fixedJoint);

		static const PhysicsBodyData* GetPhysicsBodyData(UUID entityUUID);
		static const ConstrainedJointData* GetConstrainedJointData(UUID entityUUID);

		static void BreakJoint(UUID entityUUID);

		VX_FORCE_INLINE static uint32_t GetPhysicsScenePositionIterations() { return s_PhysicsSolverIterations; }
		VX_FORCE_INLINE static void SetPhysicsScenePositionIterations(uint32_t positionIterations) { s_PhysicsSolverIterations = positionIterations; }

		VX_FORCE_INLINE static uint32_t GetPhysicsSceneVelocityIterations() { return s_PhysicsSolverVelocityIterations; }
		VX_FORCE_INLINE static void SetPhysicsSceneVelocityIterations(uint32_t veloctiyIterations) { s_PhysicsSolverVelocityIterations = veloctiyIterations; }

		VX_FORCE_INLINE static Math::vec3 GetPhysicsSceneGravity() { return s_PhysicsSceneGravity; }
		VX_FORCE_INLINE static void SetPhysicsSceneGravity(const Math::vec3& gravity) { s_PhysicsSceneGravity = gravity; }

	private:
		static void Init();
		static void InitPhysicsSDKInternal();
		static void InitPhysicsSceneInternal();

		static void Shutdown();
		static void ShutdownPhysicsSDKInternal();
		static void ShutdownPhysicsSceneInternal();

		static void CreatePhysicsActor(Entity entity);
		static void DestroyPhysicsActor(Entity entity);

		static void CreateCollider(Entity entity);
		static void CreateFixedJoint(Entity entity);
		static physx::PxController* CreateController(Entity entity);
		static physx::PxMaterial* CreatePhysicsMaterial(const PhysicsMaterialComponent& material);

		static void SetCollisionFilters(physx::PxRigidActor* actor, uint32_t filterGroup, uint32_t filterMask);
		static void UpdateDynamicActorProperties(const RigidBodyComponent& rigidbody, physx::PxRigidDynamic* dynamicActor);

		static void TraverseSceneForUninitializedActors();

	private:
		inline static std::unordered_map<UUID, physx::PxRigidActor*> s_ActiveActors;
		inline static std::unordered_map<UUID, physx::PxController*> s_ActiveControllers;
		inline static std::unordered_map<UUID, physx::PxFixedJoint*> s_ActiveFixedJoints;

		//                                                                               first - linear force, second - angular force
		using LastReportedFixedJointForcesMap = std::unordered_map<physx::PxFixedJoint*, std::pair<Math::vec3, Math::vec3>>;
		inline static LastReportedFixedJointForcesMap s_LastReportedJointForces;

		inline static std::unordered_map<UUID, PhysicsBodyData*> s_PhysicsBodyData;
		inline static std::unordered_map<UUID, ConstrainedJointData*> s_ConstrainedJointData;

		inline constexpr static float s_FixedTimeStep = 1.0f / 100.0f;
		inline static Math::vec3 s_PhysicsSceneGravity = Math::vec3(0.0f, -9.81f, 0.0f);
		inline static uint32_t s_PhysicsSolverIterations = 8;
		inline static uint32_t s_PhysicsSolverVelocityIterations = 2;

		friend class Application;
		friend class Scene;
	};

}
