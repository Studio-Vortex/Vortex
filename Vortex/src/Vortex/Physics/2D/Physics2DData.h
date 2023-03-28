#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Core/UUID.h"

extern "C"
{
	typedef struct _MonoString MonoString;
}

namespace Vortex {

	class RaycastCallback2D;

	struct VORTEX_API PhysicsBody2DData
	{
		UUID EntityUUID = 0;
	};

	struct VORTEX_API RaycastHit2D
	{
		Math::vec2 Point;
		Math::vec2 Normal;
		MonoString* Tag;
		bool Hit;

		RaycastHit2D(const RaycastCallback2D* raycastInfo, Scene* contextScene);
	};


}
