#include "vxpch.h"
#include "CookingFactory.h"

#include "Vortex/Physics/3D/Physics.h"

#include <PhysX/PxPhysicsAPI.h>

namespace Vortex {

	struct CookingFactorInternalData
	{
		physx::PxCooking* CookingSDK = nullptr;
	};

	static CookingFactorInternalData s_Data;

	void CookingFactory::Init()
	{
		physx::PxCookingParams cookingParams = physx::PxCookingParams(*((physx::PxTolerancesScale*)Physics::GetTolerancesScale()));
		cookingParams.midphaseDesc = physx::PxMeshMidPhase::eBVH34;

		s_Data.CookingSDK = PxCreateCooking(PX_PHYSICS_VERSION, *((physx::PxFoundation*)Physics::GetFoundation()), cookingParams);
		VX_CORE_ASSERT(s_CookingSDK, "Failed to Initialize PhysX Cooking!");
	}

	void CookingFactory::Shutdown()
	{
		s_Data.CookingSDK->release();
		s_Data.CookingSDK = nullptr;
	}

	CookingResult CookingFactory::CookConvexMesh(SharedReference<StaticMesh> staticMesh)
	{
		VX_CORE_ASSERT(false, "Not implemented yet!");
		return CookingResult::Failure;
	}

	CookingResult CookingFactory::CookTriangleMesh(SharedReference<StaticMesh> staticMesh)
	{
		VX_CORE_ASSERT(false, "Not implemented yet!");
		return CookingResult::Failure;
	}

}
