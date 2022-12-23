#pragma once

#include "Vortex/Scene/Components.h"
#include "Vortex/Renderer/VertexArray.h"
#include "Vortex/Renderer/Material.h"
#include "Vortex/Renderer/Shader.h"
#include "Vortex/Renderer/Buffer.h"
#include "Vortex/Core/Math.h"

struct aiMesh;
struct aiNode;
struct aiScene;
struct aiMaterial;

namespace Assimp {

	class Importer;

}

#include <vector>
#include <string>

namespace Vortex {

	struct ModelImportOptions
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

	struct ModelVertex
	{
		Math::vec3 Position;
		Math::vec3 Normal;
		Math::vec3 Tangent;
		Math::vec3 BiTangent;
		Math::vec2 TexCoord;
		Math::vec2 TexScale;

		// Editor-only
		int EntityID;
	};

	class Mesh
	{
	public:
		Mesh() = default;
		Mesh(const std::vector<ModelVertex>& vertices, const std::vector<uint32_t>& indices, const std::vector<SharedRef<MaterialInstance>>& materials);
		Mesh(bool skybox = true);
		~Mesh() = default;

		void Render(const SharedRef<Shader>& shader, const SharedRef<Material>& material);
		void RenderForShadowMap(const SharedRef<Shader>& shader, const SharedRef<Material>& material);

		const SharedRef<VertexArray>& GetVertexArray() const { return m_VertexArray; }
		const SharedRef<VertexBuffer>& GetVertexBuffer() const { return m_VertexBuffer; }
		const std::vector<SharedRef<MaterialInstance>>& GetMaterials() const { return m_Materials; }
		void SetMaterial(const SharedRef<MaterialInstance>& material);
		const std::vector<ModelVertex>& GetVertices() const { return m_Vertices; }
		std::vector<ModelVertex>& GetVertices() { return m_Vertices; }

		const SharedRef<VertexArray>& GetShadowMapVertexArray() const { return m_ShadowMapVertexArray; }
		const SharedRef<VertexBuffer>& GetShadowMapVertexBuffer() const { return m_ShadowMapVertexBuffer; }

	private:
		void CreateAndUploadMesh();

	private:
		std::vector<ModelVertex> m_Vertices;
		std::vector<uint32_t> m_Indices;
		std::vector<SharedRef<MaterialInstance>> m_Materials;

		SharedRef<VertexArray> m_VertexArray = nullptr;
		SharedRef<VertexBuffer> m_VertexBuffer = nullptr;
		SharedRef<IndexBuffer> m_IndexBuffer = nullptr;

		SharedRef<VertexArray> m_ShadowMapVertexArray = nullptr;
		SharedRef<VertexBuffer> m_ShadowMapVertexBuffer = nullptr;
		SharedRef<IndexBuffer> m_ShadowMapIndexBuffer = nullptr;
	};

	class Model
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
		~Model() = default;

		void OnUpdate(int entityID = -1, const Math::vec2& textureScale = Math::vec2(1.0f));
		void Render(const Math::mat4& worldSpaceTransform);
		void RenderForShadowMap(const Math::mat4& worldSpaceTransform);

		const std::string& GetPath() const { return m_Filepath; }

		const SharedRef<VertexArray>& GetVertexArray() const { return m_Meshes[0].GetVertexArray(); }
		const SharedRef<Material>& GetMaterial() const { return m_Material; }
		void SetMaterial(const SharedRef<MaterialInstance>& material);

		const std::vector<Mesh>& GetMeshes() const { return m_Meshes; }
		std::vector<Mesh>& GetMeshes() { return m_Meshes; }

		inline const ModelImportOptions& GetImportOptions() const { return m_ImportOptions; }

		static SharedRef<Model> Create(Model::Default defaultMesh, const TransformComponent& transform, const ModelImportOptions& importOptions = ModelImportOptions(), int entityID = -1);
		static SharedRef<Model> Create(const std::string& filepath, const TransformComponent& transform, const ModelImportOptions& importOptions = ModelImportOptions(), int entityID = -1);
		static SharedRef<Model> Create(MeshType meshType);

	private:
		void ProcessNode(aiNode* node, const aiScene* scene, const ModelImportOptions& importOptions);
		Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene, const ModelImportOptions& importOptions, const int entityID);
		std::vector<SharedRef<Texture2D>> LoadMaterialTextures(aiMaterial* material, uint32_t textureType);

	private:
		ModelImportOptions m_ImportOptions;
		std::string m_Filepath;
		int m_EntityID = -1;
		Math::vec2 m_TextureScale = Math::vec2(1.0f);
		std::vector<Mesh> m_Meshes;
		const aiScene* m_Scene;
		SharedRef<Shader> m_MeshShader = nullptr;
		SharedRef<Material> m_Material = nullptr;
	};

}
