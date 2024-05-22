#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Math/Math.h"
#include "Vortex/Math/AABB.h"

namespace Vortex::Math {

	struct VORTEX_API Ray
	{
		Math::vec3 Origin;
		Math::vec3 Direction;

		Ray(const Math::vec3& origin, const Math::vec3& direction);

		static Ray Zero();

		bool IntersectsAABB(const Math::AABB& aabb, float& t);
	};

}
