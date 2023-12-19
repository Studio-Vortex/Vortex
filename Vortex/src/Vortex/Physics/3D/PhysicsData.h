#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Core/UUID.h"

#include "Vortex/Math/Math.h"

namespace Vortex {

	class Scene;

	struct VORTEX_API PhysicsBodyData
	{
		UUID ActorUUID = 0;
		Scene* ContextScene = nullptr;
	};

	struct VORTEX_API ConstrainedJointData
	{
		UUID ActorUUID = 0;
	};

	struct VORTEX_API RaycastHit
	{
		uint64_t ActorID;
		Math::vec3 Position;
		Math::vec3 Normal;
		float Distance;
	};

#define OVERLAP_MAX_COLLIDERS 50

	struct VORTEX_API OverlapHit
	{
		uint64_t ActorID;
	};

	struct VORTEX_API Collision
	{
		uint64_t ActorID;
	};

}