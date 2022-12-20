#pragma once

#include "Vortex/Renderer/RendererAPI.h"
#include "Vortex/Renderer/EditorCamera.h"
#include "Vortex/Renderer/Framebuffer.h"
#include "Vortex/Renderer/RenderCommand.h"
#include "Vortex/Renderer/Camera.h"
#include "Vortex/Renderer/Shader.h"
#include "Vortex/Renderer/Skybox.h"

#include "Vortex/Scene/Components.h"

#include <vector>

namespace Vortex {

	struct SceneLightDescription
	{
		uint32_t ActiveDirLights;
		uint32_t ActivePointLights;
		uint32_t ActiveSpotLights;
	};

	class VORTEX_API Renderer
	{
	public:
		static void Init();
		static void Shutdown();

		static void OnWindowResize(const Viewport& viewport);

		static void BeginScene(const Camera& camera, const TransformComponent& transform);
		static void BeginScene(const EditorCamera& camera);
		static void EndScene();

		static void Submit(const SharedRef<Shader>& shader, const SharedRef<VertexArray>& vertexArray);
		static void SubmitToShadowMap(const SharedRef<Model>& model, const Math::mat4& worldSpaceTransform);
		static void DrawIndexed(const SharedRef<Shader>& shader, const SharedRef<VertexArray>& vertexArray);

		static void RenderCameraIcon(const TransformComponent& transform, const Math::mat4& cameraView, int entityID = -1);
		static void RenderLightSourceIcon(const TransformComponent& transform, const LightSourceComponent& lightSource, const Math::mat4& cameraView, int entityID = -1);
		static void RenderAudioSourceIcon(const TransformComponent& transform, const Math::mat4& cameraView, int entityID = -1);

		static void RenderLightSource(const TransformComponent& transform, const LightSourceComponent& lightSourceComponent);
		static void DrawSkybox(const Math::mat4& view, const Math::mat4& projection, SharedRef<Skybox>& skybox);

		static void DrawFrustum(const TransformComponent& transform, SceneCamera sceneCamera, const Math::vec4& color);

		static SceneLightDescription GetSceneLightDescription();

		static const SharedRef<DepthMapFramebuffer>& GetDepthMapFramebuffer();
		static void BindDepthMap();

		inline static RendererAPI::API GetGraphicsAPI() { return RendererAPI::GetAPI(); }
		inline static void SetGraphicsAPI(const RendererAPI::API& api) { RendererAPI::SetAPI(api); }

		inline static const RendererAPI::RendererInfo& GetGraphicsAPIInfo() { return RendererAPI::GetAPIInfo(); }
		inline static void SetGraphicsAPIInfo(const RendererAPI::RendererInfo& info) { RendererAPI::SetAPIInfo(info); }

		static RendererAPI::TriangleCullMode GetCullMode();
		static void SetCullMode(RendererAPI::TriangleCullMode cullMode);

		static RenderStatistics GetStats();
		static void ResetStats();

		static float GetSceneExposure();
		static void SetSceneExposure(float exposure);

		static float GetSceneGamma();
		static void SetSceneGamma(float gamma);

		static SharedRef<ShaderLibrary> GetShaderLibrary();

	private:
		static void BindShaders(const Math::mat4& view, const Math::mat4& projection, const Math::vec3& cameraPosition);
	};

}
