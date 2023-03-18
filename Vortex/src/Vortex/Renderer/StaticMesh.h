#pragma once

#include "Vortex/Asset/Asset.h"
#include "Vortex/Core/Math/Math.h"
#include "Vortex/Scene/Components.h"
#include "Vortex/Renderer/VertexArray.h"
#include "Vortex/Renderer/Material.h"
#include "Vortex/Renderer/Shader.h"
#include "Vortex/Renderer/Buffer.h"
#include "Vortex/Renderer/MeshImportOptions.h"
#include "Vortex/Renderer/VertexTypes.h"
#include "Vortex/Core/ReferenceCounting/SharedRef.h"

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

	class VORTEX_API StaticMesh : public Asset
	{
	public:
		StaticMesh() = default;
		StaticMesh(const std::string& filepath, const TransformComponent& transform, const MeshImportOptions& importOptions, int entityID);
		StaticMesh(MeshType meshType);
		~StaticMesh() = default;

		void OnUpdate(int entityID = -1);

		const StaticSubmesh& GetSubmesh(uint32_t index) const;
		StaticSubmesh& GetSubmesh(uint32_t index);
		const std::vector<StaticSubmesh>& GetSubmeshes() const { return m_Submeshes; }
		std::vector<StaticSubmesh>& GetSubmeshes() { return m_Submeshes; }

		const Math::AABB& GetBoundingBox() const { return m_BoundingBox; }

		inline const MeshImportOptions& GetImportOptions() const { return m_ImportOptions; }

		ASSET_CLASS_TYPE(StaticMeshAsset)

		static SharedReference<StaticMesh> Create(const std::string& filepath, const TransformComponent& transform, const MeshImportOptions& importOptions = MeshImportOptions(), int entityID = -1);
		static SharedReference<StaticMesh> Create(MeshType meshType);

	private:
		void ProcessNode(const std::string& filepath, aiNode* node, const aiScene* scene, const MeshImportOptions& importOptions, const int entityID);
		StaticSubmesh ProcessMesh(const std::string& filepath, aiMesh* mesh, const aiScene* scene, const MeshImportOptions& importOptions, const int entityID);

		void CreateBoundingBoxFromSubmeshes();

	private:
		std::vector<StaticSubmesh> m_Submeshes;
		MeshImportOptions m_ImportOptions;
		const aiScene* m_Scene;

		Math::AABB m_BoundingBox;
	};

}
