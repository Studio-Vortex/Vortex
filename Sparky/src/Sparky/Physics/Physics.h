#pragma once

#include "Sparky/Scene/Scene.h"
#include "Sparky/Scene/Entity.h"
#include "Sparky/Scene/Components.h"
#include "Sparky/Core/TimeStep.h"

namespace physx {

	class PxScene;
	class PxRigidDynamic;

}

namespace Sparky {

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

		static int32_t GetPhysicsSceneIterations() { return s_PhysicsSceneIterations; }
		static void SetPhysicsSceneIterations(int32_t positionIterations) { s_PhysicsSceneIterations = positionIterations; }

		static Math::vec3 GetPhysicsSceneGravity() { return s_PhysicsSceneGravity; }
		static void SetPhysicsSceneGravitty(const Math::vec3& gravity) { s_PhysicsSceneGravity = gravity; }

	private:
		static void UpdateActorFlags(const RigidBodyComponent& rigidbody, physx::PxRigidDynamic* dynamicActor);

	private:
		inline static Math::vec3 s_PhysicsSceneGravity = Math::vec3(0.0f, -9.8f, 0.0f);
		inline static int32_t s_PhysicsSceneIterations = 20;
	};

	enum class FilterGroup : uint32_t
	{
		Static = BIT(0),
		Dynamic = BIT(1),
		Kinematic = BIT(2),
		All = Static | Dynamic | Kinematic
	};

	enum class BroadphaseType
	{
		SweepAndPrune,
		MultiBoxPrune,
		AutomaticBoxPrune
	};

	enum class FrictionType
	{
		Patch,
		OneDirectional,
		TwoDirectional
	};

	enum class CookingResult
	{
		Success,
		ZeroAreaTestFailed,
		PolygonLimitReached,
		LargeTriangle,
		InvalidMesh,
		Failure,
		None
	};

	enum class ForceMode : uint8_t
	{
		Force = 0,
		Impulse,
		VelocityChange,
		Acceleration
	};

	enum class ActorLockFlag : uint8_t
	{
		TranslationX = BIT(0), TranslationY = BIT(1), TranslationZ = BIT(2), Translation = TranslationX | TranslationY | TranslationZ,
		RotationX = BIT(3), RotationY = BIT(4), RotationZ = BIT(5), Rotation = RotationX | RotationY | RotationZ
	};

}
