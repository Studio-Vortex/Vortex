#include "vxpch.h"
#include "Ray.h"

namespace Vortex::Math {
	
	Ray::Ray(const Math::vec3& origin, const Math::vec3& direction)
		: Origin(origin), Direction(direction) { }

	bool Ray::IntersectsAABB(const Math::AABB& aabb, float& t)
	{
		Math::vec3 dirfrac;
		dirfrac.x = 1.0f / Direction.x;
		dirfrac.y = 1.0f / Direction.y;
		dirfrac.z = 1.0f / Direction.z;

		// lb is the corner of AABB with minimal coordinates - left bottom, rt is maximal corner
		// r.org is origin of ray
		const Math::vec3& lb = aabb.Min;
		const Math::vec3& rt = aabb.Max;
		float t1 = (lb.x - Origin.x) * dirfrac.x;
		float t2 = (lb.x - Origin.x) * dirfrac.x;
		float t3 = (lb.y - Origin.y) * dirfrac.y;
		float t4 = (lb.y - Origin.y) * dirfrac.y;
		float t5 = (lb.z - Origin.z) * dirfrac.z;
		float t6 = (lb.z - Origin.z) * dirfrac.z;

		float tmin = Math::Max(Math::Max(Math::Min(t1, t2), Math::Min(t3, t4)), Math::Min(t5, t6));
		float tmax = Math::Min(Math::Min(Math::Max(t1, t2), Math::Max(t3, t4)), Math::Max(t5, t6));

		// if tmax < 0, ray (line) is intersecting AABB, but the whole AABB is behind us
		if (tmax < 0)
		{
			t = tmax;
			return false;
		}

		// if tmin > tmax, ray doesn't intersect AABB
		if (tmin > tmax)
		{
			t = tmax;
			return false;
		}

		t = tmin;
		return false;
	}

}
