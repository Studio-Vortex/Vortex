#pragma once

#include "Vortex/Editor/EditorCamera.h"
#include "Vortex/Renderer/RendererAPI.h"
#include "Vortex/Renderer/Framebuffer.h"
#include "Vortex/Renderer/RenderCommand.h"
#include "Vortex/Renderer/Camera.h"
#include "Vortex/Renderer/Shader.h"
#include "Vortex/Renderer/Skybox.h"
#include "Vortex/Project/Project.h"
#include "Vortex/Scene/Entity.h"
#include "Vortex/Scene/Scene.h"

#include "Vortex/Scene/Components.h"

#include <vector>

#include <entt/entt.hpp>

namespace Vortex {

	struct SceneLightDescription
	{
		bool HasSkyLight = false;
		uint32_t PointLightIndex = 0;
		uint32_t ActivePointLights = 0;
		uint32_t SpotLightIndex = 0;
		uint32_t ActiveSpotLights = 0;
	};

	struct RenderTime
	{
		float ShadowMapRenderTime = 0.0f;
		float GeometryPassRenderTime = 0.0f;
	};

	enum class PostProcessStage
	{
		None = 0, Bloom,
	};

	struct PostProcessProperties
	{
		SharedRef<Framebuffer> SceneFramebuffer = nullptr;
	};

	class VORTEX_API Renderer
	{
	public:
		static void Init();
		static void Shutdown();

		static void OnWindowResize(const Viewport& viewport);

		static void BeginScene(const Camera& camera, const TransformComponent& transform, const SharedRef<Framebuffer>& targetFramebuffer);
		static void BeginScene(const EditorCamera* camera, const SharedRef<Framebuffer>& targetFramebuffer);
		static void EndScene();

		static void Submit(const SharedRef<Shader>& shader, const SharedRef<VertexArray>& vertexArray);
		static void DrawIndexed(const SharedRef<Shader>& shader, const SharedRef<VertexArray>& vertexArray);

		static void RenderCameraIcon(const TransformComponent& transform, const Math::mat4& cameraView, int entityID = -1);
		static void RenderLightSourceIcon(const TransformComponent& transform, const LightSourceComponent& lightSource, const Math::mat4& cameraView, int entityID = -1);
		static void RenderAudioSourceIcon(const TransformComponent& transform, const Math::mat4& cameraView, int entityID = -1);

		static void RenderLightSource(const TransformComponent& transform, const LightSourceComponent& lightSourceComponent);
		static void DrawEnvironmentMap(const Math::mat4& view, const Math::mat4& projection, SkyboxComponent& skyboxComponent);

		static void DrawFrustumOutline(const TransformComponent& transform, SceneCamera sceneCamera, const Math::vec4& color);

		static SceneLightDescription GetSceneLightDescription();

		static void CreateGaussianBlurFramebuffers(const Math::vec2& viewportSize);

		static void CreateEnvironmentMap(SkyboxComponent& skyboxComponent);
		static void CreateShadowMap(LightType type, const SharedRef<LightSource>& lightSource);

		static void BeginPostProcessStage(PostProcessStage stage, const PostProcessProperties& postProcessProps);

		static void RenderToDepthMap(Scene* contextScene);
		static const SharedRef<DepthMapFramebuffer>& GetSkyLightDepthFramebuffer();

		static void BindSkyLightDepthMap();
		static void BindPointLightDepthMaps();
		static void BindSpotLightDepthMaps();

		inline static RendererAPI::API GetGraphicsAPI() { return RendererAPI::GetAPI(); }
		inline static void SetGraphicsAPI(const RendererAPI::API& api) { RendererAPI::SetAPI(api); }

		inline static const RendererAPI::RendererInfo& GetGraphicsAPIInfo() { return RendererAPI::GetAPIInfo(); }
		inline static void SetGraphicsAPIInfo(const RendererAPI::RendererInfo& info) { RendererAPI::SetAPIInfo(info); }

		static RendererAPI::TriangleCullMode GetCullMode();
		static void SetCullMode(RendererAPI::TriangleCullMode cullMode);

		static RenderStatistics GetStats();
		static void ResetStats();
		static RenderTime& GetRenderTime();
		static void ResetRenderTime();
		static void AddToQuadCountStats(uint32_t quadCount);
		static void AddToDrawCallCountStats(uint32_t drawCalls);

		static void SetProperties(const ProjectProperties::RendererProperties& props);

		static float GetEnvironmentMapResolution();
		static void SetEnvironmentMapResolution(float resolution);

		static float GetPrefilterMapResolution();
		static void SetPrefilterMapResolution(float resolution);

		static float GetShadowMapResolution();
		static void SetShadowMapResolution(float resolution);

		static float GetSceneExposure();
		static void SetSceneExposure(float exposure);

		static float GetSceneGamma();
		static void SetSceneGamma(float gamma);

		static SharedRef<ShaderLibrary> GetShaderLibrary();

	private:
		static void BindShaders(const Math::mat4& view, const Math::mat4& projection, const Math::vec3& cameraPosition);
		static void BlurScene(const SharedRef<Framebuffer>& sceneFramebuffer);
	};

}
