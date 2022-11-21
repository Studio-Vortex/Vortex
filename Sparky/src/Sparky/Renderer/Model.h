#pragma once

#include "Sparky/Scene/Components.h"
#include "Sparky/Scene/SceneCamera.h"
#include "Sparky/Renderer/EditorCamera.h"
#include "Sparky/Renderer/VertexArray.h"
#include "Sparky/Renderer/Material.h"
#include "Sparky/Renderer/Texture.h"
#include "Sparky/Renderer/Buffer.h"
#include "Sparky/Renderer/Shader.h"

#include <string>

namespace Sparky {

	class Entity;

	struct ModelVertex
	{
		Math::vec3 Position;
		Math::vec3 Normal;
		Math::vec4 Tangent;
		Math::vec2 TextureCoord;
		Math::vec2 TexScale;

		// Editor-only
		int EntityID;
	};

	struct ModelVertexInfo
	{
		Math::vec3 Position;
		Math::vec3 Normal;
		Math::vec4 Tangent;
		Math::vec2 TextureCoord;

		inline bool operator==(const ModelVertexInfo& other) const
		{
			return Position == other.Position && Normal == other.Normal && Tangent == other.Tangent && TextureCoord == other.TextureCoord;
		}
	};

	struct Mesh
	{
		std::vector<ModelVertexInfo> Vertices;
		std::vector<uint32_t> Indices;
		std::vector<SharedRef<Texture2D>> Textures;
	};

	class Model
	{
	public:
		// TODO: move to asset system when we have one
		inline static std::vector<std::string> DefaultMeshSourcePaths = {
			"Resources/Meshes/Default/Cube.obj",
			"Resources/Meshes/Default/Sphere.obj",
			"Resources/Meshes/Default/Capsule.obj",
			"Resources/Meshes/Default/Cone.obj",
			"Resources/Meshes/Default/Cylinder.obj",
			"Resources/Meshes/Default/Plane.obj",
			"Resources/Meshes/Default/Torus.obj",
		};

		// Same with this
		static bool IsDefaultMesh(const std::string& path)
		{
			auto it = std::find(DefaultMeshSourcePaths.begin(), DefaultMeshSourcePaths.end(), path);
			bool isDefaultMesh = it != DefaultMeshSourcePaths.end();
			return it != DefaultMeshSourcePaths.end();
		}

		enum class Default
		{
			Cube = 0, Sphere, Capsule, Cone, Cylinder, Plane, Torus,
		};

	public:
		Model() = default;
		Model(const std::string& filepath, const TransformComponent& transform, int entityID);
		Model(const std::string& filepath, const SharedRef<MaterialInstance>& materialInstance, const TransformComponent& transform, int entityID);
		Model(Model::Default defaultMesh, const TransformComponent& transform, int entityID);
		Model(MeshRendererComponent::MeshType meshType);
		~Model() = default;

		void OnUpdate(int entityID = -1, const Math::vec2& scale = Math::vec2(1.0f));

		inline const std::string& GetPath() const { return m_Filepath; }
		const SharedRef<MaterialInstance>& GetMaterial() const { return m_MaterialInstance; }
		const SharedRef<VertexArray>& GetVertexArray() const { return m_Vao; }
		const std::vector<SharedRef<Texture2D>>& GetTextures() const { return m_Textures; }

		uint32_t GetQuadCount() const;

		static SharedRef<Model> Create(const std::string& filepath, const TransformComponent& transform, int entityID);
		static SharedRef<Model> Create(const std::string& filepath, const SharedRef<MaterialInstance>& materialInstance, const TransformComponent& transform, int entityID);
		static SharedRef<Model> Create(Model::Default defaultMesh, const TransformComponent& transform, int entityID);
		static SharedRef<Model> Create(MeshRendererComponent::MeshType meshType);

	private:
		void LoadModelFromFile(const std::string& filepath, const TransformComponent& transform, int entityID);

	private:
		std::string m_Filepath;
		SharedRef<MaterialInstance> m_MaterialInstance;
		std::vector<ModelVertex> m_OriginalVertices;
		std::vector<ModelVertex> m_Vertices;
		std::vector<SharedRef<Texture2D>> m_Textures;

		int m_EntityID = -1;
		Math::vec2 m_TextureScale = Math::vec2(1.0f);

		SharedRef<VertexArray> m_Vao = nullptr;
		SharedRef<VertexBuffer> m_Vbo = nullptr;
		SharedRef<IndexBuffer> m_Ibo = nullptr;
	};

}

namespace std {

	template<> struct hash<Sparky::ModelVertexInfo>
	{
		size_t operator()(const Sparky::ModelVertexInfo& vertex) const
		{
			return ((hash<Sparky::Math::vec3>()(vertex.Position) ^
				(hash<Sparky::Math::vec3>()(vertex.Normal) << 1)) >> 1) ^
				(hash<Sparky::Math::vec2>()(vertex.TextureCoord) << 1);
		}
	};

}
