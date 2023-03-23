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
		StaticSubmesh(const std::string& name, const std::vector<StaticVertex>& vertices, const std::vector<uint32_t>& indices);
		StaticSubmesh(bool skybox);
		~StaticSubmesh() = default;

		VX_FORCE_INLINE const std::string& GetName() const { return m_MeshName; }

		void Render(AssetHandle materialHandle) const;
		void RenderToSkylightShadowMap() const;

		VX_FORCE_INLINE const SharedReference<VertexArray>& GetVertexArray() const { return m_VertexArray; }
		VX_FORCE_INLINE const SharedReference<VertexBuffer>& GetVertexBuffer() const { return m_VertexBuffer; }
		
		const std::vector<StaticVertex>& GetVertices() const { return m_Vertices; }
		std::vector<StaticVertex>& GetVertices() { return m_Vertices; }

		const std::vector<uint32_t> GetIndices() const { return m_Indices; }

		const Math::AABB& GetBoundingBox() const { return m_BoundingBox; }

	private:
		void CreateAndUploadMesh();
		void CreateBoundingBoxFromVertices();

	private:
		std::vector<StaticVertex> m_Vertices;
		std::vector<uint32_t> m_Indices;

		SharedReference<VertexArray> m_VertexArray = nullptr;
		SharedReference<VertexBuffer> m_VertexBuffer = nullptr;
		SharedReference<IndexBuffer> m_IndexBuffer = nullptr;

		Math::AABB m_BoundingBox;
		std::string m_MeshName;
	};

	class VORTEX_API StaticMesh : public Asset
	{
	public:
		StaticMesh() = default;
		StaticMesh(const std::string& filepath, const TransformComponent& transform, const MeshImportOptions& importOptions, int entityID);
		StaticMesh(MeshType meshType);
		~StaticMesh() override = default;

		void OnUpdate(const SharedReference<MaterialTable>& materialTable, int entityID = -1);

		bool HasSubmesh(uint32_t index) const;

		const StaticSubmesh& GetSubmesh(uint32_t index) const;
		StaticSubmesh& GetSubmesh(uint32_t index);
		const std::unordered_map<uint32_t, StaticSubmesh>& GetSubmeshes() const { return m_Submeshes; }

		const Math::AABB& GetBoundingBox() const { return m_BoundingBox; }
		inline const MeshImportOptions& GetImportOptions() const { return m_ImportOptions; }
		inline bool IsLoaded() const { return m_IsLoaded; }

		ASSET_CLASS_TYPE(StaticMeshAsset)

		static SharedReference<StaticMesh> Create(const std::string& filepath, const TransformComponent& transform, const MeshImportOptions& importOptions = MeshImportOptions(), int entityID = -1);
		static SharedReference<StaticMesh> Create(MeshType meshType);

	private:
		void ProcessNode(const std::string& filepath, aiNode* node, const aiScene* scene, const MeshImportOptions& importOptions, const int entityID);
		StaticSubmesh ProcessMesh(const std::string& filepath, aiMesh* mesh, const aiScene* scene, const MeshImportOptions& importOptions, const int entityID);
		void ProcessVertex(aiMesh* mesh, StaticVertex& vertex, const Math::mat4& transform, uint32_t index);
		AssetHandle LoadMaterialTexture(aiMaterial* material, const std::filesystem::path& directory, uint32_t textureType, uint32_t index);

		void CreateBoundingBoxFromSubmeshes();

	private:
		std::unordered_map<uint32_t, StaticSubmesh> m_Submeshes;
		MeshImportOptions m_ImportOptions;
		const aiScene* m_Scene;

		Math::AABB m_BoundingBox;

		bool m_IsLoaded = false;
	};

}
