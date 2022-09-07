#pragma once

#include "Sparky/Core.h"

#include "Sparky/Renderer/OrthographicCamera.h"
#include "Sparky/Renderer/RenderCommand.h"
#include "Sparky/Renderer/Shader.h"

namespace Sparky {

	class SPARKY_API Renderer
	{
	public:
		static void BeginScene(const OrthographicCamera& camera);
		static void EndScene();

		static void Submit(const std::shared_ptr<Shader>& shader, const std::shared_ptr<VertexArray>& vertexArray, const Math::mat4& transform = Math::Identity());

		inline static RendererAPI::API GetGraphicsAPI() { return RendererAPI::GetAPI(); }
		inline static void SetGraphicsAPI(const RendererAPI::API& api) { RendererAPI::SetAPI(api); }

	private:
		struct SceneData
		{
			Math::mat4 ViewProjectionMatrix;
		};

	private:
		static SceneData* s_SceneData;
	};

}