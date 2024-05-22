#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Renderer/StaticMesh.h"
#include "Vortex/Physics/3D/PhysicsTypes.h"

#include <vector>

namespace Vortex {

	class VORTEX_API CookingFactory
	{
	public:
		static void Init();
		static void Shutdown();

		static CookingResult CookConvexMesh(SharedReference<StaticMesh> staticMesh);
		static CookingResult CookTriangleMesh(SharedReference<StaticMesh> staticMesh);
	};

}
