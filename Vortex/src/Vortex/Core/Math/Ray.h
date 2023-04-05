#pragma once

#include "Vortex/Core/Math/Math.h"
#include "Vortex/Core/Math/AABB.h"

namespace Vortex::Math {

	struct Ray
	{
		Math::vec3 Origin;
		Math::vec3 Direction;

		Ray(const Math::vec3& origin, const Math::vec3& direction);

		bool IntersectsAABB(const Math::AABB& aabb, float& t);
	};

}
