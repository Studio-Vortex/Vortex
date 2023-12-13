#include "vxpch.h"
#include "DefaultMesh.h"

#include "Vortex/Renderer/StaticMesh.h"
#include "Vortex/Renderer/Material.h"

#include "Vortex/Asset/AssetManager.h"

namespace Vortex {

	void DefaultMesh::Init()
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

		bool firstIteration = true;
		uint32_t i = 0;

		for (const auto& sourcePath : sourcePaths)
		{
			MeshImportOptions importOptions = MeshImportOptions();

			if (sourcePath == sourcePaths[2]) // For Capsule only
			{
				importOptions.MeshTransformation.SetRotationEuler({ 0.0f, 0.0f, 90.0f });
			}

			SharedReference<StaticMesh> staticMesh = StaticMesh::Create(sourcePath, TransformComponent(), importOptions);
			
			// Nasty hack to set the default material handle
			if (firstIteration) {
				AssetHandle defaultMaterialHandle = staticMesh->m_MaterialHandles[0];
				Material::SetDefaultMaterialHandle(defaultMaterialHandle);
				firstIteration = false;
			}

			// Should we generate a handle here?
			staticMesh->Handle = AssetHandle();

			VX_CORE_ASSERT(!s_StaticMeshTypes.contains(staticMesh->Handle), "default static mesh was already created with assest handle!");
			s_StaticMeshTypes[staticMesh->Handle] = (StaticMeshType)i++;

			editorAssetManager->AddMemoryOnlyAsset(staticMesh);
			AssetMetadata& metadata = editorAssetManager->GetMutableMetadata(staticMesh->Handle);
			metadata.Filepath = sourcePath;

			DefaultStaticMeshes.push_back(staticMesh->Handle);
		}
	}

	bool DefaultMesh::IsDefaultStaticMesh(AssetHandle assetHandle)
	{
		for (const auto& staticMeshHandle : DefaultStaticMeshes)
		{
			if (assetHandle == staticMeshHandle)
				return true;
		}

		return false;
	}

    DefaultMesh::StaticMeshType DefaultMesh::GetStaticMeshType(AssetHandle assetHandle)
    {
		VX_CORE_ASSERT(IsDefaultStaticMesh(assetHandle), "expected default static mesh!");
		VX_CORE_ASSERT(s_StaticMeshTypes.contains(assetHandle), "invalid asset handle!");

		return s_StaticMeshTypes[assetHandle];
    }

}
