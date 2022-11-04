#pragma once

#include "Sparky/Renderer/RendererAPI.h"
#include "Sparky/Renderer/EditorCamera.h"
#include "Sparky/Renderer/Camera.h"
#include "Sparky/Renderer/RenderCommand.h"
#include "Sparky/Renderer/Shader.h"
#include "Sparky/Renderer/Model.h"
#include "Sparky/Renderer/Skybox.h"

#include "Sparky/Scene/Components.h"

#include <vector>

namespace Sparky {

	class SPARKY_API Renderer
	{
	public:
		static void Init();
		static void Shutdown();

		static void OnWindowResize(const Viewport& viewport);

		static void BeginScene(const Camera& camera, const TransformComponent& transform);
		static void BeginScene(const EditorCamera& camera);
		static void EndScene();

		static void Submit(const SharedRef<Shader>& shader, const SharedRef<VertexArray>& vertexArray);

		static void RenderCameraIcon(const TransformComponent& transform, const CameraComponent& camera, int entityID = -1);
		static void RenderLightSource(const TransformComponent& transform, const LightSourceComponent& light, bool sceneRunning, int entityID = -1);
		static void DrawModel(const TransformComponent& transform, const MeshRendererComponent& meshRenderer, int entityID = -1);
		static void DrawSkybox(const Math::mat4& view, const Math::mat4& projection, const SharedRef<Skybox>& skybox);

		static void DrawCubeWireframe(const TransformComponent& transform);
		static void DrawFrustum(const TransformComponent& transform, SceneCamera sceneCamera, const Math::vec4& color);

		inline static RendererAPI::API GetGraphicsAPI() { return RendererAPI::GetAPI(); }
		inline static void SetGraphicsAPI(const RendererAPI::API& api) { RendererAPI::SetAPI(api); }

		inline static const RendererAPI::RendererInfo& GetGraphicsAPIInfo() { return RendererAPI::GetAPIInfo(); }
		inline static void SetGraphicsAPIInfo(const RendererAPI::RendererInfo& info) { RendererAPI::SetAPIInfo(info); }

		static RendererAPI::TriangleCullMode GetCullMode();
		static void SetCullMode(RendererAPI::TriangleCullMode cullMode);

		static RenderStatistics GetStats();
		static void ResetStats();

		static float GetRefractiveIndex();
		static void SetRefractiveIndex(float index);

		static std::vector<SharedRef<Shader>> GetLoadedShaders();

	private:
		static void BindShaders(const Math::mat4& view, const Math::mat4& projection, const Math::vec3& cameraPosition);
	};

}
