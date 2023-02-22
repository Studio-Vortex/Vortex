#pragma once

#include "Vortex/Scene/Scene.h"
#include "Vortex/Scene/Entity.h"
#include "Vortex/Scene/Components.h"
#include "Vortex/Core/TimeStep.h"

// Forward declarations

extern "C"
{
	typedef struct _MonoString MonoString;
}

class b2World;
class b2Fixture;

namespace Vortex {

	class RaycastCallback2D;

	struct VORTEX_API RaycastHit2D
	{
		Math::vec2 Point;
		Math::vec2 Normal;
		MonoString* Tag;
		bool Hit;

		RaycastHit2D(const RaycastCallback2D* raycastInfo, Scene* contextScene);
	};

	struct VORTEX_API PhysicsBody2DData
	{
		UUID EntityUUID;
	};

	class VORTEX_API Physics2D
	{
	public:
		static void CreatePhysicsBody(Entity entity, const TransformComponent& transform, RigidBody2DComponent& rb2d);
		static void DestroyPhysicsBody(Entity entity);

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
