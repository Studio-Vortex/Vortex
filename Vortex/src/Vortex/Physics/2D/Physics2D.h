#pragma once

#include "Vortex/Core/TimeStep.h"

#include "Vortex/Scene/Scene.h"
#include "Vortex/Scene/Actor.h"
#include "Vortex/Scene/Components.h"

#include "Vortex/Physics/2D/Physics2DData.h"

class b2World;
class b2Fixture;

namespace Vortex {

	class VORTEX_API Physics2D
	{
	public:
		static void CreatePhysicsBody(Actor actor, const TransformComponent& transform, RigidBody2DComponent& rb2d);
		static void DestroyPhysicsBody(Actor actor);

		static void OnSimulationStart(Scene* contextScene);
		static void OnSimulationUpdate(TimeStep delta, Scene* contextScene);
		static void OnSimulationStop();

		static uint64_t Raycast(const Math::vec2& start, const Math::vec2& end, RaycastHit2D* outResult, bool drawDebugLine);

		static b2World* GetPhysicsScene() { return s_PhysicsScene; }

		static uint32_t GetPhysicsWorldVelocityIterations() { return s_PhysicsWorld2DVeloctityIterations; }
		static void SetPhysicsWorldVelocityIterations(uint32_t veloctiyIterations) { s_PhysicsWorld2DVeloctityIterations = veloctiyIterations; }

		static uint32_t GetPhysicsWorldPositionIterations() { return s_PhysicsWorld2DPositionIterations; }
		static void SetPhysicsWorldPositionIterations(uint32_t positionIterations) { s_PhysicsWorld2DPositionIterations = positionIterations; }

		static Math::vec2 GetPhysicsWorldGravity() { return s_PhysicsWorld2DGravity; }
		static void SetPhysicsWorldGravitty(const Math::vec2& gravity) { s_PhysicsWorld2DGravity = gravity; }

	private:
		inline static Scene* s_ContextScene = nullptr;
		inline static b2World* s_PhysicsScene = nullptr;

		inline static Math::vec2 s_PhysicsWorld2DGravity = Math::vec2(0.0f, -9.81f);
		inline static uint32_t s_PhysicsWorld2DVeloctityIterations = 6;
		inline static uint32_t s_PhysicsWorld2DPositionIterations = 2;

		inline static std::unordered_map<b2Fixture*, UniqueRef<PhysicsBody2DData>> s_PhysicsBodyDataMap;
	};

}
