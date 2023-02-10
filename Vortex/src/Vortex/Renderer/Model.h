#pragma once

#include "Vortex/Core/Math.h"
#include "Vortex/Scene/Components.h"
#include "Vortex/Renderer/VertexArray.h"
#include "Vortex/Renderer/Material.h"
#include "Vortex/Renderer/Shader.h"
#include "Vortex/Renderer/Buffer.h"

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

	struct VORTEX_API Vertex
	{
		Math::vec3 Position;
		Math::vec4 Color;
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

	struct VORTEX_API Index
	{
		uint32_t i0, i1, i2;
	};

	class VORTEX_API Submesh
	{
	public:
		Submesh() = default;
		Submesh(const std::string& name, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, const SharedRef<Material>& material);
		Submesh(bool skybox = true);
		~Submesh() = default;

		VX_FORCE_INLINE const std::string& GetName() const { return m_MeshName; }

		void Render() const;
		void RenderToSkylightShadowMap();

		const SharedRef<VertexArray>& GetVertexArray() const { return m_VertexArray; }
		const SharedRef<VertexBuffer>& GetVertexBuffer() const { return m_VertexBuffer; }
		const SharedRef<Material>& GetMaterial() const { return m_Material; }
		void SetMaterial(const SharedRef<Material>& material);
		
		const std::vector<Vertex>& GetVertices() const { return m_Vertices; }
		std::vector<Vertex>& GetVertices() { return m_Vertices; }

		const std::vector<uint32_t> GetIndices() const { return m_Indices; }

	private:
		void CreateAndUploadMesh();

	private:
		std::string m_MeshName;
		std::vector<Vertex> m_Vertices;
		std::vector<uint32_t> m_Indices;
		SharedRef<Material> m_Material = nullptr;

		SharedRef<VertexArray> m_VertexArray = nullptr;
		SharedRef<VertexBuffer> m_VertexBuffer = nullptr;
		SharedRef<IndexBuffer> m_IndexBuffer = nullptr;
	};

	class VORTEX_API Model
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
		Model(const std::vector<Vertex>& vertices, const std::vector<Index>& indices, const Math::mat4& transform = Math::Identity());
		Model(MeshType meshType);
		~Model() = default;

		void OnUpdate(int entityID = -1);

		const std::string& GetPath() const { return m_Filepath; }

		const Submesh& GetSubmesh(uint32_t index) const;
		Submesh& GetSubmesh(uint32_t index);
		const std::vector<Submesh>& GetSubmeshes() const { return m_Submeshes; }
		std::vector<Submesh>& GetSubmeshes() { return m_Submeshes; }

		std::unordered_map<std::string, BoneInfo>& GetBoneInfoMap() { return m_BoneInfoMap; }
		uint32_t& GetBoneCount() { return m_BoneCounter; }

		inline const ModelImportOptions& GetImportOptions() const { return m_ImportOptions; }
		inline bool HasAnimations() const { return m_HasAnimations; }

		static SharedRef<Model> Create(Model::Default defaultMesh, const TransformComponent& transform, const ModelImportOptions& importOptions = ModelImportOptions(), int entityID = -1);
		static SharedRef<Model> Create(const std::string& filepath, const TransformComponent& transform, const ModelImportOptions& importOptions = ModelImportOptions(), int entityID = -1);
		static SharedRef<Model> Create(const std::vector<Vertex>& vertices, const std::vector<Index>& indices, const Math::mat4& transform = Math::Identity());
		static SharedRef<Model> Create(MeshType meshType);

	private:
		void ProcessNode(aiNode* node, const aiScene* scene, const ModelImportOptions& importOptions, const int entityID);
		Submesh ProcessMesh(aiMesh* mesh, const aiScene* scene, const ModelImportOptions& importOptions, const int entityID);

		void SetVertexBoneDataToDefault(Vertex& vertex) const;
		void SetVertexBoneData(Vertex& vertex, int boneID, float weight) const;
		bool ExtractBoneWeightsForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene);

	private:
		std::vector<Submesh> m_Submeshes;
		ModelImportOptions m_ImportOptions;
		std::string m_Filepath;
		const aiScene* m_Scene;

		std::unordered_map<std::string, BoneInfo> m_BoneInfoMap;
		uint32_t m_BoneCounter = 0;
		bool m_HasAnimations = false;
	};

}
