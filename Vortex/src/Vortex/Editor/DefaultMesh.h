#pragma once

#include "Vortex/Asset/Asset.h"

#include <unordered_map>
#include <vector>

namespace Vortex {

	class VORTEX_API DefaultMesh
	{
	public:
		static void Init();

		static bool IsDefaultStaticMesh(AssetHandle assetHandle);

		enum class StaticMeshType
		{
			Cube = 0, Sphere, Capsule, Cone, Cylinder, Plane, Torus,
		};

		static StaticMeshType GetStaticMeshType(AssetHandle assetHandle);

		// May need to change this in runtime

		// Index into the vector with EditorResources::DefaultMeshes enum
		inline static std::vector<AssetHandle> DefaultStaticMeshes;

	private:
		inline static std::unordered_map<AssetHandle, StaticMeshType> s_StaticMeshTypes;
	};

}
