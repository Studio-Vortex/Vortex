#pragma once

#include "Vortex/Asset/Asset.h"

#include <vector>

namespace Vortex {

	class DefaultMeshes
	{
	public:
		static void Init();

		static bool IsDefaultStaticMesh(AssetHandle assetHandle);

		enum class StaticMeshes
		{
			Cube = 0, Sphere, Capsule, Cone, Cylinder, Plane, Torus,
		};

		// May need to change this in runtime

		// Index into the vector with EditorResources::DefaultMeshes enum
		inline static std::vector<AssetHandle> DefaultStaticMeshes;
	};

}
