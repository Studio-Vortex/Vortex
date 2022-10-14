#pragma once

#include "Sparky/Core/Base.h"

#include "Sparky/Renderer/RendererAPI.h"
#include "Sparky/Renderer/EditorCamera.h"
#include "Sparky/Renderer/Camera.h"
#include "Sparky/Renderer/RenderCommand.h"
#include "Sparky/Renderer/Shader.h"

namespace Sparky {

	class SPARKY_API Renderer
	{
	public:
		static void Init();
		static void Shutdown();

		static void OnWindowResize(const Viewport& viewport);

		static void BeginScene(const Camera& camera, const Math::mat4& transform);
		static void BeginScene(const EditorCamera& camera);
		static void EndScene();

		static void Submit(const SharedRef<Shader>& shader, const SharedRef<VertexArray>& vertexArray, const Math::mat4& transform = Math::Identity());
		static void Flush();

		static void DrawCube(const Math::mat4& transform, const Math::vec4& color, int entityID = -1);

		inline static RendererAPI::API GetGraphicsAPI() { return RendererAPI::GetAPI(); }
		inline static void SetGraphicsAPI(const RendererAPI::API& api) { RendererAPI::SetAPI(api); }

		inline static const RendererAPI::RendererInfo& GetGraphicsAPIInfo() { return RendererAPI::GetAPIInfo(); }
		inline static void SetGraphicsAPIInfo(const RendererAPI::RendererInfo& info) { RendererAPI::SetAPIInfo(info); }

		static RendererAPI::TriangleCullMode GetCullMode();
		static void SetCullMode(RendererAPI::TriangleCullMode cullMode);

		static RenderStatistics GetStats();
		static void ResetStats();

	private:
		static void StartBatch();
		static void NextBatch();

		static void AddToCubeVertexBuffer(const Math::mat4& transform, const Math::vec4& color, const Math::vec2& normal, const Math::vec2* textureCoords, float textureIndex, float textureScale, int entityID = -1);
	};

}
