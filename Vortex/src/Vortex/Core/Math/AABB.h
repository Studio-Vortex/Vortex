#pragma once

#include "Vortex/Core/Math/Math.h"

namespace Vortex::Math {

	struct AABB
	{
		Math::vec3 Min;
		Math::vec3 Max;

		AABB()
			: Min(0.0f), Max(0.0f) { }

		AABB(const Math::vec3& min, const Math::vec3& max)
			: Min(min), Max(max) { }
	};

}
