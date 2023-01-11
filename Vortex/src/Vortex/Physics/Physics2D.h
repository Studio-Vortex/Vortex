#pragma once

#include "Vortex/Scene/Scene.h"
#include "Vortex/Scene/Entity.h"
#include "Vortex/Scene/Components.h"
#include "Vortex/Core/TimeStep.h"

// Forward declarations
class b2World;
class b2Fixture;

namespace Vortex {

	struct PhysicsBody2DData
	{
		UUID EntityUUID;
	};

	class Physics2D
	{
	public:
		static void CreatePhysicsBody(Entity entity, const TransformComponent& transform, RigidBody2DComponent& rb2d);
		static void DestroyPhysicsBody(Entity entity);

		static void OnSimulationStart(Scene* contextScene);
		static void OnSimulationUpdate(TimeStep delta, Scene* contextScene);
		static void OnSimulationStop();

		static b2World* GetPhysicsScene() { return s_PhysicsScene; }

		static int32_t GetPhysicsWorld2DVeloctiyIterations() { return s_PhysicsWorld2DVeloctityIterations; }
		static void SetPhysicsWorldVelocityIterations(int32_t veloctiyIterations) { s_PhysicsWorld2DVeloctityIterations = veloctiyIterations; }

		static int32_t GetPhysicsWorld2DPositionIterations() { return s_PhysicsWorld2DPositionIterations; }
		static void SetPhysicsWorldPositionIterations(int32_t positionIterations) { s_PhysicsWorld2DPositionIterations = positionIterations; }

		static Math::vec2 GetPhysicsWorld2DGravity() { return s_PhysicsWorld2DGravity; }
		static void SetPhysicsWorldGravitty(const Math::vec2& gravity) { s_PhysicsWorld2DGravity = gravity; }

	private:
		inline static b2World* s_PhysicsScene = nullptr;

		inline static Math::vec2 s_PhysicsWorld2DGravity = Math::vec2(0.0f, -9.8f);
		inline static int32_t s_PhysicsWorld2DVeloctityIterations = 6;
		inline static int32_t s_PhysicsWorld2DPositionIterations = 2;

		inline static std::unordered_map<b2Fixture*, UniqueRef<PhysicsBody2DData>> s_PhysicsBodyDataMap;
	};

}
