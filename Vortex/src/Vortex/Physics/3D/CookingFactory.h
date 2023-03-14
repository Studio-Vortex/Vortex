#pragma once

#include "Vortex/Renderer/StaticMesh.h"
#include "Vortex/Physics/3D/PhysXTypes.h"

#include <vector>

namespace Vortex {

	enum class MeshColliderType : uint8_t { Triangle = 0, Convex = 1, None = 3 };

	class CookingFactory
	{
	public:
		static void Init();
		static void Shutdown();

		static CookingResult CookConvexMesh(SharedRef<StaticMesh> staticMesh, const std::vector<uint32_t>& submeshIndices);
		static CookingResult CookTriangleMesh(SharedRef<StaticMesh> staticMesh, const std::vector<uint32_t>& submeshIndices);
	};

}
