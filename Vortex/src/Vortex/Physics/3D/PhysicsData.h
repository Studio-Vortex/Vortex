#pragma once

#include "Vortex/Core/UUID.h"
#include "Vortex/Core/Math/Math.h"

namespace Vortex {

	class Scene;

	struct VORTEX_API PhysicsBodyData
	{
		UUID EntityUUID = 0;
		Scene* ContextScene = nullptr;
	};

	struct VORTEX_API ConstrainedJointData
	{
		UUID EntityUUID = 0;
	};

	struct VORTEX_API RaycastHit
	{
		uint64_t EntityID;
		Math::vec3 Position;
		Math::vec3 Normal;
		float Distance;
	};

#define OVERLAP_MAX_COLLIDERS 50

	struct VORTEX_API OverlapHit
	{
		uint64_t EntityID;
	};

	struct VORTEX_API Collision
	{
		uint64_t EntityID;
	};

}