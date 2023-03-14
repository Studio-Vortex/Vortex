#include "vxpch.h"
#include "CookingFactory.h"

#include "Vortex/Physics/3D/Physics.h"

#include <PhysX/PxPhysicsAPI.h>

namespace Vortex {

	static physx::PxCooking* s_CookingSDK = nullptr;

	void CookingFactory::Init()
	{
		physx::PxCookingParams cookingParams = physx::PxCookingParams(*Physics::GetTolerancesScale());
		cookingParams.midphaseDesc = physx::PxMeshMidPhase::eBVH34;

		s_CookingSDK = PxCreateCooking(PX_PHYSICS_VERSION, *Physics::GetFoundation(), cookingParams);
		VX_CORE_ASSERT(s_CookingSDK, "Failed to Initialize PhysX Cooking!");
	}

	void CookingFactory::Shutdown()
	{
		s_CookingSDK->release();
		s_CookingSDK = nullptr;
	}

}
