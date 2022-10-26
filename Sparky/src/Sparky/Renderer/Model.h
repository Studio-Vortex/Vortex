#pragma once

#include "Sparky/Scene/SceneCamera.h"
#include "Sparky/Renderer/EditorCamera.h"
#include "Sparky/Renderer/VertexArray.h"
#include "Sparky/Renderer/Texture.h"
#include "Sparky/Renderer/Buffer.h"
#include "Sparky/Renderer/Shader.h"

namespace Sparky {

	class Entity;
	struct MeshRendererComponent;

	struct ModelVertex
	{
		Math::vec3 Position;
		Math::vec3 Normal;
		Math::vec4 Color;
		Math::vec2 TextureCoord;
		float TexIndex;
		float TexScale;

		// Editor-only
		int EntityID;
	};

	struct ModelVertexInfo
	{
		Math::vec3 Position;
		Math::vec3 Normal;
		Math::vec2 TextureCoord;

		inline bool operator==(const ModelVertexInfo& other) const
		{
			return Position == other.Position && Normal == other.Normal && TextureCoord == other.TextureCoord;
		}
	};

	struct Mesh
	{
		std::vector<ModelVertexInfo> Vertices;
		std::vector<uint32_t> Indices;
	};

	class Model
	{
	public:
		Model(const std::string& filepath, Entity entity, const Math::vec4& color);
		~Model() = default;

		void OnUpdate(const EditorCamera& camera, const Math::mat4& transform, const MeshRendererComponent& meshRenderer);
		void OnUpdate(const SceneCamera& camera, const Math::mat4& transform, const MeshRendererComponent& meshRenderer);

		inline const std::string& GetPath() const { return m_Filepath; }
		const SharedRef<VertexArray>& GetVertexArray() const { return m_Vao; }

		uint32_t GetQuadCount() const;

		static SharedRef<Model> Create(const std::string& filepath, Entity entity, const Math::vec4& color);

	private:
		inline static SharedRef<Shader> s_ModelShader = nullptr;

	private:
		std::string m_Filepath;
		Math::mat4 m_Transform;
		std::vector<ModelVertex> m_Vertices;
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
