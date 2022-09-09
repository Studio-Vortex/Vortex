#pragma once

#include "Sparky/Core/Core.h"

#include "Sparky/Renderer/OrthographicCamera.h"
#include "Sparky/Renderer/RenderCommand.h"
#include "Sparky/Renderer/Shader.h"

namespace Sparky {

	class SPARKY_API Renderer
	{
	public:
		static void Init();
		static void Shutdown();

		static void OnWindowResize(const Viewport& viewport);

		static void BeginScene(const OrthographicCamera& camera);
		static void EndScene();

		static void Submit(const SharedRef<Shader>& shader, const SharedRef<VertexArray>& vertexArray, const Math::mat4& transform = Math::Identity());

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