#pragma once

#include "Sparky/Scene/SceneCamera.h"
#include "Sparky/Renderer/EditorCamera.h"
#include "Sparky/Renderer/VertexArray.h"
#include "Sparky/Renderer/Buffer.h"
#include "Sparky/Renderer/Shader.h"

namespace Sparky {

	struct ModelVertex
	{
		Math::vec3 Position;
		//Math::vec4 Color;
		Math::vec3 Normal;
		Math::vec2 TextureCoord;

		// Editor-only
		int EntityID;
	};

	struct ModelVertexInfo
	{
		Math::vec3 Position;
		Math::vec3 Normal;
		Math::vec2 TextureCoord;
	};

	class Model
	{
	public:
		Model(const std::string& filepath, const Math::mat4& transform, const Math::vec4& color, int entityID);
		~Model() = default;

		static void Init();

		void OnUpdate(const EditorCamera& camera, const Math::mat4& transform, const Math::vec4& color);
		void OnUpdate(const SceneCamera& camera, const Math::mat4& transform, const Math::vec4& color);

		inline const std::string& GetPath() const { return m_Filepath; }
		const SharedRef<VertexArray>& GetVertexArray() const { return m_Vao; }
		uint32_t GetVertexCount() const { return m_VertexCount; }

		static SharedRef<Model> Create(const std::string& filepath, const Math::mat4& transform, const Math::vec4& color, int entityID);

	private:
		inline static SharedRef<Shader> s_ModelShader = nullptr;

	private:
		std::string m_Filepath;
		std::vector<ModelVertex> m_ModelVertices;
		SharedRef<VertexArray> m_Vao = nullptr;
		SharedRef<VertexBuffer> m_Vbo = nullptr;
		uint32_t m_VertexCount = 0;
	};

}
