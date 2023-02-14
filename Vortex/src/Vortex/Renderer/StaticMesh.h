#pragma once

#include "Vortex/Core/Math.h"
#include "Vortex/Scene/Components.h"
#include "Vortex/Renderer/VertexArray.h"
#include "Vortex/Renderer/Material.h"
#include "Vortex/Renderer/Shader.h"
#include "Vortex/Renderer/Buffer.h"
#include "Vortex/Renderer/MeshImportOptions.h"
#include "Vortex/Renderer/VertexTypes.h"

#include <vector>
#include <string>

struct aiMesh;
struct aiNode;
struct aiScene;
struct aiMaterial;

namespace Vortex {

	class VORTEX_API StaticSubmesh
	{
	public:
		StaticSubmesh() = default;
		StaticSubmesh(const std::string& name, const std::vector<StaticVertex>& vertices, const std::vector<uint32_t>& indices, const SharedRef<Material>& material);
		StaticSubmesh(bool skybox = true);
		~StaticSubmesh() = default;

		VX_FORCE_INLINE const std::string& GetName() const { return m_MeshName; }

		void Render() const;
		void RenderToSkylightShadowMap();

		const SharedRef<VertexArray>& GetVertexArray() const { return m_VertexArray; }
		const SharedRef<VertexBuffer>& GetVertexBuffer() const { return m_VertexBuffer; }
		const SharedRef<Material>& GetMaterial() const { return m_Material; }
		void SetMaterial(const SharedRef<Material>& material);
		
		const std::vector<StaticVertex>& GetVertices() const { return m_Vertices; }
		std::vector<StaticVertex>& GetVertices() { return m_Vertices; }

		const std::vector<uint32_t> GetIndices() const { return m_Indices; }

		const Math::AABB& GetBoundingBox() const { return m_BoundingBox; }

	private:
		void CreateAndUploadMesh();
		void CreateBoundingBoxFromVertices();

	private:
		std::string m_MeshName;
		std::vector<StaticVertex> m_Vertices;
		std::vector<uint32_t> m_Indices;
		SharedRef<Material> m_Material = nullptr;

		SharedRef<VertexArray> m_VertexArray = nullptr;
		SharedRef<VertexBuffer> m_VertexBuffer = nullptr;
		SharedRef<IndexBuffer> m_IndexBuffer = nullptr;

		Math::AABB m_BoundingBox;
	};

	class VORTEX_API StaticMesh
	{
	public:
		enum class Default
		{
			Cube = 0, Sphere, Capsule, Cone, Cylinder, Plane, Torus,
		};

		// TODO: move to asset system when we have one
		inline static std::vector<std::string> DefaultMeshSourcePaths = {
			"Resources/Meshes/Cube.fbx",
			"Resources/Meshes/Sphere.fbx",
			"Resources/Meshes/Capsule.fbx",
			"Resources/Meshes/Cone.fbx",
			"Resources/Meshes/Cylinder.fbx",
			"Resources/Meshes/Plane.fbx",
			"Resources/Meshes/Torus.fbx",
		};

		// ditto
		static bool IsDefaultMesh(const std::string& path)
		{
			auto it = std::find(DefaultMeshSourcePaths.begin(), DefaultMeshSourcePaths.end(), path);
			bool isDefaultMesh = it != DefaultMeshSourcePaths.end();
			return isDefaultMesh;
		}

		StaticMesh() = default;
		StaticMesh(StaticMesh::Default defaultMesh, const TransformComponent& transform, const MeshImportOptions& importOptions, int entityID);
		StaticMesh(const std::string& filepath, const TransformComponent& transform, const MeshImportOptions& importOptions, int entityID);
		StaticMesh(const std::vector<StaticVertex>& vertices, const std::vector<Index>& indices, const Math::mat4& transform = Math::Identity());
		StaticMesh(MeshType meshType);
		~StaticMesh() = default;

		void OnUpdate(int entityID = -1);

		const std::string& GetPath() const { return m_Filepath; }

		const StaticSubmesh& GetSubmesh(uint32_t index) const;
		StaticSubmesh& GetSubmesh(uint32_t index);
		const std::vector<StaticSubmesh>& GetSubmeshes() const { return m_Submeshes; }
		std::vector<StaticSubmesh>& GetSubmeshes() { return m_Submeshes; }

		const Math::AABB& GetBoundingBox() const { return m_BoundingBox; }

		inline const MeshImportOptions& GetImportOptions() const { return m_ImportOptions; }

		static SharedRef<StaticMesh> Create(StaticMesh::Default defaultMesh, const TransformComponent& transform, const MeshImportOptions& importOptions = MeshImportOptions(), int entityID = -1);
		static SharedRef<StaticMesh> Create(const std::string& filepath, const TransformComponent& transform, const MeshImportOptions& importOptions = MeshImportOptions(), int entityID = -1);
		static SharedRef<StaticMesh> Create(const std::vector<StaticVertex>& vertices, const std::vector<Index>& indices, const Math::mat4& transform = Math::Identity());
		static SharedRef<StaticMesh> Create(MeshType meshType);

	private:
		void ProcessNode(aiNode* node, const aiScene* scene, const MeshImportOptions& importOptions, const int entityID);
		StaticSubmesh ProcessMesh(aiMesh* mesh, const aiScene* scene, const MeshImportOptions& importOptions, const int entityID);

		void CreateBoundingBoxFromSubmeshes();

	private:
		std::vector<StaticSubmesh> m_Submeshes;
		MeshImportOptions m_ImportOptions;
		std::string m_Filepath;
		const aiScene* m_Scene;

		Math::AABB m_BoundingBox;
	};

}
