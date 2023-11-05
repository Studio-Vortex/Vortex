#include "vxpch.h"
#include "DefaultMeshes.h"

#include "Vortex/Renderer/StaticMesh.h"
#include "Vortex/Asset/AssetManager.h"

namespace Vortex {

	void DefaultMeshes::Init()
	{
		std::string sourcePaths[] =
		{
			"Resources/Meshes/Default/Cube.fbx",
			"Resources/Meshes/Default/Sphere.fbx",
			"Resources/Meshes/Default/Capsule.fbx",
			"Resources/Meshes/Default/Cone.fbx",
			"Resources/Meshes/Default/Cylinder.fbx",
			"Resources/Meshes/Default/Plane.fbx",
			"Resources/Meshes/Default/Torus.fbx",
		};

		SharedReference<EditorAssetManager> editorAssetManager = Project::GetEditorAssetManager();

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

			editorAssetManager->AddMemoryOnlyAsset(staticMesh);
			AssetMetadata& metadata = editorAssetManager->GetMutableMetadata(staticMesh->Handle);
			metadata.Filepath = sourcePath;

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
