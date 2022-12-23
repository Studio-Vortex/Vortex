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

	struct AnimatedModelImportOptions
	{
		TransformComponent AnimatedMeshTransformation;

		AnimatedModelImportOptions() = default;
		AnimatedModelImportOptions(const AnimatedModelImportOptions&) = default;

		inline bool operator==(const AnimatedModelImportOptions& other) const
		{
			return AnimatedMeshTransformation.Translation == other.AnimatedMeshTransformation.Translation &&
				AnimatedMeshTransformation.GetRotationEuler() == other.AnimatedMeshTransformation.GetRotationEuler() &&
				AnimatedMeshTransformation.Scale == other.AnimatedMeshTransformation.Scale;
		}
	};

	struct AnimatedModelVertex
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

	class AnimatedMesh
	{
	public:
		AnimatedMesh() = default;
		AnimatedMesh(const std::vector<AnimatedModelVertex>& vertices, const std::vector<uint32_t>& indices, const std::vector<SharedRef<MaterialInstance>>& materials);
		AnimatedMesh(bool skybox = true);
		~AnimatedMesh() = default;

		void Render(const SharedRef<Shader>& shader, const SharedRef<Material>& material);
		void RenderForShadowMap(const SharedRef<Shader>& shader, const SharedRef<Material>& material);

		const SharedRef<VertexArray>& GetVertexArray() const { return m_VertexArray; }
		const SharedRef<VertexBuffer>& GetVertexBuffer() const { return m_VertexBuffer; }
		const std::vector<SharedRef<MaterialInstance>>& GetMaterials() const { return m_Materials; }
		void SetMaterial(const SharedRef<MaterialInstance>& material);
		const std::vector<AnimatedModelVertex>& GetVertices() const { return m_Vertices; }
		std::vector<AnimatedModelVertex>& GetVertices() { return m_Vertices; }

		const SharedRef<VertexArray>& GetShadowMapVertexArray() const { return m_ShadowMapVertexArray; }
		const SharedRef<VertexBuffer>& GetShadowMapVertexBuffer() const { return m_ShadowMapVertexBuffer; }

	private:
		void CreateAndUploadAnimatedMesh();

	private:
		std::vector<AnimatedModelVertex> m_Vertices;
		std::vector<uint32_t> m_Indices;
		std::vector<SharedRef<MaterialInstance>> m_Materials;

		SharedRef<VertexArray> m_VertexArray = nullptr;
		SharedRef<VertexBuffer> m_VertexBuffer = nullptr;
		SharedRef<IndexBuffer> m_IndexBuffer = nullptr;

		SharedRef<VertexArray> m_ShadowMapVertexArray = nullptr;
		SharedRef<VertexBuffer> m_ShadowMapVertexBuffer = nullptr;
		SharedRef<IndexBuffer> m_ShadowMapIndexBuffer = nullptr;
	};

	class AnimatedModel
	{
	public:
		AnimatedModel() = default;
		AnimatedModel(const std::string& filepath, const TransformComponent& transform, const AnimatedModelImportOptions& importOptions, int entityID);
		~AnimatedModel() = default;

		void OnUpdate(int entityID = -1, const Math::vec2& textureScale = Math::vec2(1.0f));
		void Render(const Math::mat4& worldSpaceTransform);
		void RenderForShadowMap(const Math::mat4& worldSpaceTransform);

		const std::string& GetPath() const { return m_Filepath; }

		const SharedRef<VertexArray>& GetVertexArray() const { return m_AnimatedMeshes[0].GetVertexArray(); }
		const SharedRef<Material>& GetMaterial() const { return m_Material; }
		void SetMaterial(const SharedRef<MaterialInstance>& material);

		const std::vector<AnimatedMesh>& GetAnimatedMeshes() const { return m_AnimatedMeshes; }
		std::vector<AnimatedMesh>& GetAnimatedMeshes() { return m_AnimatedMeshes; }

		inline const AnimatedModelImportOptions& GetImportOptions() const { return m_ImportOptions; }

		static SharedRef<AnimatedModel> Create(const std::string& filepath, const TransformComponent& transform, const AnimatedModelImportOptions& importOptions = AnimatedModelImportOptions(), int entityID = -1);

	private:
		void ProcessNode(aiNode* node, const aiScene* scene, const AnimatedModelImportOptions& importOptions);
		AnimatedMesh ProcessAnimatedMesh(aiMesh* AnimatedMesh, const aiScene* scene, const AnimatedModelImportOptions& importOptions, const int entityID);
		std::vector<SharedRef<Texture2D>> LoadMaterialTextures(aiMaterial* material, uint32_t textureType);

	private:
		AnimatedModelImportOptions m_ImportOptions;
		std::string m_Filepath;
		int m_EntityID = -1;
		Math::vec2 m_TextureScale = Math::vec2(1.0f);
		std::vector<AnimatedMesh> m_AnimatedMeshes;
		const aiScene* m_Scene;
		SharedRef<Shader> m_AnimatedMeshShader = nullptr;
		SharedRef<Material> m_Material = nullptr;
	};

}
