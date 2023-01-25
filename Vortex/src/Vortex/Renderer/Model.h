#pragma once

#include "Vortex/Core/Math.h"
#include "Vortex/Scene/Components.h"
#include "Vortex/Renderer/VertexArray.h"
#include "Vortex/Renderer/Material.h"
#include "Vortex/Renderer/Shader.h"
#include "Vortex/Renderer/Buffer.h"
#include "Vortex/Asset/Asset.h"

#include <vector>
#include <unordered_map>
#include <string>

struct aiMesh;
struct aiNode;
struct aiScene;
struct aiMaterial;

namespace Vortex {

#define MAX_BONE_INFLUENCE 4

	struct VORTEX_API ModelImportOptions
	{
		TransformComponent MeshTransformation;

		ModelImportOptions() = default;
		ModelImportOptions(const ModelImportOptions&) = default;

		inline bool operator==(const ModelImportOptions& other) const
		{
			return MeshTransformation.Translation == other.MeshTransformation.Translation &&
				MeshTransformation.GetRotationEuler() == other.MeshTransformation.GetRotationEuler() &&
				MeshTransformation.Scale == other.MeshTransformation.Scale;
		}
	};

	struct VORTEX_API BoneInfo
	{
		uint32_t ID;
		Math::mat4 OffsetMatrix;
	};

	struct VORTEX_API ModelVertex
	{
		Math::vec3 Position;
		Math::vec3 Normal;
		Math::vec3 Tangent;
		Math::vec3 BiTangent;
		Math::vec2 TexCoord;
		Math::vec2 TexScale;
		Math::ivec4 BoneIDs;
		Math::vec4 BoneWeights;

		// Editor-only
		int EntityID;
	};

	class VORTEX_API Mesh
	{
	public:
		Mesh() = default;
		Mesh(const std::vector<ModelVertex>& vertices, const std::vector<uint32_t>& indices, const std::vector<SharedRef<Material>>& materials);
		Mesh(bool skybox = true);
		~Mesh() = default;

		void Render(const SharedRef<Shader>& shader, const SharedRef<Material>& material);
		void RenderToShadowMap(const SharedRef<Shader>& shader, const SharedRef<Material>& material);

		const SharedRef<VertexArray>& GetVertexArray() const { return m_VertexArray; }
		const SharedRef<VertexBuffer>& GetVertexBuffer() const { return m_VertexBuffer; }
		const std::vector<SharedRef<Material>>& GetMaterials() const { return m_Materials; }
		void SetMaterial(const SharedRef<Material>& material);
		const std::vector<ModelVertex>& GetVertices() const { return m_Vertices; }
		std::vector<ModelVertex>& GetVertices() { return m_Vertices; }

		const SharedRef<VertexArray>& GetShadowMapVertexArray() const { return m_ShadowMapVertexArray; }
		const SharedRef<VertexBuffer>& GetShadowMapVertexBuffer() const { return m_ShadowMapVertexBuffer; }

	private:
		void CreateAndUploadMesh();

	private:
		std::vector<ModelVertex> m_Vertices;
		std::vector<uint32_t> m_Indices;
		std::vector<SharedRef<Material>> m_Materials;

		SharedRef<VertexArray> m_VertexArray = nullptr;
		SharedRef<VertexBuffer> m_VertexBuffer = nullptr;
		SharedRef<IndexBuffer> m_IndexBuffer = nullptr;

		SharedRef<VertexArray> m_ShadowMapVertexArray = nullptr;
		SharedRef<VertexBuffer> m_ShadowMapVertexBuffer = nullptr;
		SharedRef<IndexBuffer> m_ShadowMapIndexBuffer = nullptr;
	};

	class VORTEX_API Model : public Asset
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

		// Same with this
		static bool IsDefaultMesh(const std::string& path)
		{
			auto it = std::find(DefaultMeshSourcePaths.begin(), DefaultMeshSourcePaths.end(), path);
			bool isDefaultMesh = it != DefaultMeshSourcePaths.end();
			return isDefaultMesh;
		}

		Model() = default;
		Model(Model::Default defaultMesh, const TransformComponent& transform, const ModelImportOptions& importOptions, int entityID);
		Model(const std::string& filepath, const TransformComponent& transform, const ModelImportOptions& importOptions, int entityID);
		Model(MeshType meshType);
		~Model() override = default;

		void OnUpdate(int entityID = -1, const Math::vec2& textureScale = Math::vec2(1.0f));
		void Render(const Math::mat4& worldSpaceTransform);
		void Render(const Math::mat4& worldSpaceTransform, const AnimatorComponent& animatorComponent);
		void RenderToSkylightShadowMap(const Math::mat4& worldSpaceTransform);
		void RenderToSkylightShadowMap(const Math::mat4& worldSpaceTransform, const AnimatorComponent& animatorComponent);
		void RenderToPointLightShadowMap(const Math::mat4& worldSpaceTransform);
		void RenderToPointLightShadowMap(const Math::mat4& worldSpaceTransform, const AnimatorComponent& animatorComponent);

		const std::string& GetPath() const { return m_Filepath; }

		const SharedRef<VertexArray>& GetVertexArray() const { return m_Meshes[0].GetVertexArray(); }
		const SharedRef<Material>& GetMaterial() const { return m_Material; }
		SharedRef<Material> GetMaterial() { return m_Material; }
		void SetMaterial(const SharedRef<Material>& material);

		const std::vector<Mesh>& GetMeshes() const { return m_Meshes; }
		std::vector<Mesh>& GetMeshes() { return m_Meshes; }

		std::unordered_map<std::string, BoneInfo>& GetBoneInfoMap() { return m_BoneInfoMap; }
		uint32_t& GetBoneCount() { return m_BoneCounter; }

		inline const ModelImportOptions& GetImportOptions() const { return m_ImportOptions; }
		inline bool HasAnimations() const { return m_HasAnimations; }

		static AssetType GetStaticType() { return AssetType::Mesh; }
		AssetType GetAssetType() const override { return AssetType::Mesh; }

		static SharedRef<Model> Create(Model::Default defaultMesh, const TransformComponent& transform, const ModelImportOptions& importOptions = ModelImportOptions(), int entityID = -1);
		static SharedRef<Model> Create(const std::string& filepath, const TransformComponent& transform, const ModelImportOptions& importOptions = ModelImportOptions(), int entityID = -1);
		static SharedRef<Model> Create(MeshType meshType);

	private:
		void ProcessNode(aiNode* node, const aiScene* scene, const ModelImportOptions& importOptions);
		Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene, const ModelImportOptions& importOptions, const int entityID);
		std::vector<SharedRef<Texture2D>> LoadMaterialTextures(aiMaterial* material, uint32_t textureType);

		void SetVertexBoneDataToDefault(ModelVertex& vertex) const;
		void SetVertexBoneData(ModelVertex& vertex, int boneID, float weight) const;
		bool ExtractBoneWeightsForVertices(std::vector<ModelVertex>& vertices, aiMesh* mesh, const aiScene* scene);

	private:
		std::vector<Mesh> m_Meshes;
		SharedRef<Shader> m_MeshShader = nullptr;
		SharedRef<Material> m_Material = nullptr;
		ModelImportOptions m_ImportOptions;
		std::string m_Filepath;
		const aiScene* m_Scene;
		int m_EntityID = -1;
		Math::vec2 m_TextureScale = Math::vec2(1.0f);

		std::unordered_map<std::string, BoneInfo> m_BoneInfoMap;
		uint32_t m_BoneCounter = 0;
		bool m_HasAnimations = false;
	};

}
