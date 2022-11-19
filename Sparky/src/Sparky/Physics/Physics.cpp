#include "sppch.h"
#include "Physics.h"

namespace Sparky {

	namespace Utils {

	}
	
	void Physics::OnSimulationStart(Scene* contextScene)
	{
		s_Foundation = PxCreateFoundation(PX_PHYSICS_VERSION, s_DefaultAllocator, s_ErrorCallback);
	}

	void Physics::OnSimulationUpdate(TimeStep delta, Scene* contextScene)
	{
		
	}

	void Physics::OnSimulationStop()
	{
		
	}

	void Physics::CreatePhysicsBody(Entity entity, const TransformComponent& transform, RigidBodyComponent& rigidbody)
	{
		
	}

	void Physics::DestroyPhysicsBody(Entity entity)
	{
		// Destroy the physics body if it exists

		
	}

}
