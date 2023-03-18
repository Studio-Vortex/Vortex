#include "vxpch.h"
#include "DefaultMeshes.h"

#include "Vortex/Renderer/StaticMesh.h"
#include "Vortex/Asset/AssetManager.h"

namespace Vortex {

	void DefaultMeshes::Init()
	{
		std::string sourcePaths[] =
		{
			"Resources/Default/Meshes/Cube.fbx",
			"Resources/Default/Meshes/Sphere.fbx",
			"Resources/Default/Meshes/Capsule.fbx",
			"Resources/Default/Meshes/Cone.fbx",
			"Resources/Default/Meshes/Cylinder.fbx",
			"Resources/Default/Meshes/Plane.fbx",
			"Resources/Default/Meshes/Torus.fbx",
		};

		for (const auto& sourcePath : sourcePaths)
		{
			MeshImportOptions importOptions = MeshImportOptions();

			if (sourcePath == sourcePaths[2]) // For Capsule only
			{
				importOptions.MeshTransformation.SetRotationEuler({ 0.0f, 0.0f, 90.0f });
			}

			SharedReference<StaticMesh> staticMesh = StaticMesh::Create(sourcePath, TransformComponent(), importOptions);
			// Should we generate a handle here?
			staticMesh->Handle = AssetHandle();
			Project::GetEditorAssetManager()->AddMemoryOnlyAsset(staticMesh);
			DefaultStaticMeshes.push_back(staticMesh->Handle);
		}
	}

	bool DefaultMeshes::IsDefaultStaticMesh(AssetHandle assetHandle)
	{
		for (const auto& staticMeshHandle : DefaultStaticMeshes)
		{
			if (assetHandle == staticMeshHandle)
				return true;
		}

		return false;
	}

}
