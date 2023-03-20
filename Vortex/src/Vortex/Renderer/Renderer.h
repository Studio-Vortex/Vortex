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

	struct VORTEX_API SceneLightDescription
	{
		bool HasSkyLight = false;
		uint32_t PointLightIndex = 0;
		uint32_t ActivePointLights = 0;
		uint32_t SpotLightIndex = 0;
		uint32_t ActiveSpotLights = 0;
	};

	struct VORTEX_API RenderTime
	{
		float ShadowMapRenderTime = 0.0f;
		float GeometryPassRenderTime = 0.0f;
		float BloomPassRenderTime = 0.0f;
	};

	enum class PostProcessStage
	{
		None = 0, Bloom,
	};

	struct VORTEX_API PostProcessProperties
	{
		SharedRef<Framebuffer> TargetFramebuffer = nullptr;
		Viewport ViewportSize = {};
		PostProcessStage* Stages = nullptr;
		uint32_t StageCount = 0;
	};

	enum class VORTEX_API RenderFlag
	{
		None = 0,
		EnableBloom = BIT(1),
	};

	class VORTEX_API Renderer
	{
	public:
		static void Init();
		static void Shutdown();

		static void OnWindowResize(const Viewport& viewport);

		static void BeginScene(const Camera& camera, const TransformComponent& transform, SharedRef<Framebuffer> targetFramebuffer);
		static void BeginScene(const EditorCamera* camera, SharedRef<Framebuffer> targetFramebuffer);
		static void EndScene();

		static void Submit(SharedReference<Shader>& shader, SharedReference<VertexArray>& vertexArray);
		static void DrawIndexed(SharedReference<Shader>& shader, SharedReference<VertexArray>& vertexArray);

		static void RenderLightSource(const TransformComponent& transform, const LightSourceComponent& lightSourceComponent);
		static void DrawEnvironmentMap(const Math::mat4& view, const Math::mat4& projection, SkyboxComponent& skyboxComponent, SharedReference<Skybox>& skybox);

		static void DrawFrustumOutline(const TransformComponent& transform, SceneCamera sceneCamera, const Math::vec4& color);

		static SceneLightDescription GetSceneLightDescription();

		static void CreateEnvironmentMap(SkyboxComponent& skyboxComponent, SharedReference<Skybox>& skybox);
		static void CreateShadowMap(LightType type);

		static void BeginPostProcessingStages(const PostProcessProperties& postProcessProps);

		static void RenderToDepthMap(SharedReference<Scene>& contextScene);
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

		static Math::vec3 GetBloomSettings();
		static void SetBloomSettings(const Math::vec3& threshold);
		static void SetBloomThreshold(float threshold);
		static void SetBloomSoftKnee(float softKnee);
		static void SetBloomUnknown(float unknown);

		static uint32_t GetBloomSampleSize();
		static void SetBloomSampleSize(uint32_t samples);

		static uint32_t GetFlags();
		static void SetFlags(uint32_t flags);
		static void SetFlag(RenderFlag flag);
		static void ToggleFlag(RenderFlag flag);
		static void DisableFlag(RenderFlag flag);
		static bool IsFlagSet(RenderFlag flag);
		static void ClearFlags();

		static SharedReference<Texture2D> GetWhiteTexture();

		static ShaderLibrary& GetShaderLibrary();

	private:
		// Helpers

		static void BindShaders(const Math::mat4& view, const Math::mat4& projection, const Math::vec3& cameraPosition);

		static void RenderDirectionalLightShadow(const LightSourceComponent& lightSourceComponent, Entity lightSourceEntity, SharedReference<Scene::SceneGeometry>& sceneMeshes);
		static void RenderPointLightShadow(const LightSourceComponent& lightSourceComponent, Entity lightSourceEntity, SharedReference<Scene::SceneGeometry>& sceneMeshes);
		static void RenderSpotLightShadow(const LightSourceComponent& lightSourceComponent, Entity lightSourceEntity, SharedReference<Scene::SceneGeometry>& sceneMeshes);

		// Post Processing

		static void ConfigurePostProcessingPipeline(const PostProcessProperties& postProcessProps);
		static std::vector<PostProcessStage> SortPostProcessStages(PostProcessStage* stages, uint32_t count);
		static uint32_t GetPostProcessStageScore(PostProcessStage stage);
		static PostProcessStage FindHighestPriortyStage(PostProcessStage* stages, uint32_t count);
		static void CreateBlurFramebuffer(uint32_t width, uint32_t height);
		static void BlurAndSubmitFinalSceneComposite(SharedRef<Framebuffer> sceneFramebuffer);
	};

}
