#pragma once

#include "Vortex/Project/Project.h"

#include "Vortex/Scene/Entity.h"
#include "Vortex/Scene/Scene.h"
#include "Vortex/Scene/Components.h"

#include "Vortex/Renderer/Shader.h"
#include "Vortex/Renderer/Camera.h"
#include "Vortex/Renderer/RendererAPI.h"
#include "Vortex/Renderer/RenderCommand.h"

#include "Vortex/ReferenceCounting/SharedRef.h"

#include <vector>

namespace Vortex {

	class Texture2D;
	class Skybox;
	class Mesh;
	class StaticMesh;
	class Material;
	class Framebuffer;

	class Animator;

	class EditorCamera;

	struct VORTEX_API SceneLightDescription
	{
		bool HasEnvironment = false;
		bool HasSkyLight = false;
		uint32_t PointLightIndex = 0;
		uint32_t ActivePointLights = 0;
		uint32_t SpotLightIndex = 0;
		uint32_t ActiveSpotLights = 0;
		uint32_t EmissiveMeshIndex = 0;
		uint32_t ActiveEmissiveMeshes = 0;
	};

	struct VORTEX_API RenderTime
	{
		float ShadowMapRenderTime = 0.0f;
		float PreGeometryPassSortTime = 0.0f;
		float GeometryPassRenderTime = 0.0f;
		float BloomPassRenderTime = 0.0f;
	};

	enum class VORTEX_API PostProcessStage
	{
		None = 0, Bloom,
	};

	struct VORTEX_API PostProcessProperties
	{
		SharedReference<Framebuffer> TargetFramebuffer = nullptr;
		Math::vec3 CameraPosition = {};
		Viewport ViewportInfo = {};
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

		static void BeginScene(const Camera& camera, const Math::mat4& view, const Math::vec3& cameraTranslation, SharedReference<Framebuffer> targetFramebuffer);
		static void BeginScene(const EditorCamera* camera, SharedReference<Framebuffer> targetFramebuffer);
		static void EndScene();

		static void Submit(const SharedReference<Shader>& shader, const SharedReference<VertexArray>& vertexArray);
		static void DrawIndexed(const SharedReference<Shader>& shader, const SharedReference<VertexArray>& vertexArray);

		static void RenderLightSource(const TransformComponent& transform, const LightSourceComponent& lightSourceComponent);
		static void RenderEmissiveEntity(const Math::vec3& translation, const Math::vec3& radiance, float intensity);
		static void DrawEnvironmentMap(const Math::mat4& view, const Math::mat4& projection, SkyboxComponent& skyboxComponent, SharedReference<Skybox>& environment);

		static void DrawFrustum(const std::vector<Math::vec4>& corners, const Math::vec4& color);

		static SceneLightDescription GetSceneLightDescription();

		static void CreateEnvironmentMap(SkyboxComponent& skyboxComponent, SharedReference<Skybox>& environment);
		static void CreateShadowMap(LightType type);

		static void BeginPostProcessingStages(const PostProcessProperties& postProcessProps);

		static void RenderToDepthMap(SharedReference<Scene>& contextScene);
		static const SharedReference<DepthMapFramebuffer>& GetSkyLightDepthFramebuffer();

		static void BindSkyLightDepthMap();
		static void BindPointLightDepthMaps();
		static void BindSpotLightDepthMaps();

		inline static RendererAPI::API GetGraphicsAPI() { return RendererAPI::GetAPI(); }
		inline static void SetGraphicsAPI(const RendererAPI::API& api) { RendererAPI::SetAPI(api); }

		inline static const RendererAPI::Info& GetGraphicsAPIInfo() { return RendererAPI::GetAPIInfo(); }
		inline static void SetGraphicsAPIInfo(const RendererAPI::Info& info) { RendererAPI::SetAPIInfo(info); }

		static RendererAPI::TriangleCullMode GetCullMode();
		static void SetCullMode(RendererAPI::TriangleCullMode cullMode);

		static RenderStatistics GetStats();
		static void ResetStats();
		static RenderTime& GetRenderTime();
		static void ResetRenderTime();
		static void AddToQuadCountStats(uint32_t quadCount);
		static void AddToDrawCallCountStats(uint32_t drawCalls);

		static void SetProperties(const ProjectProperties::RendererProperties& props);

		static void SetEnvironment(SharedReference<Skybox>& environment);

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
		static void SetBloomKnee(float knee);
		static void SetBloomIntensity(float intensity);

		static uint32_t GetBloomSampleSize();
		static void SetBloomSampleSize(uint32_t samples);

		static uint32_t GetFlags();
		static void SetFlags(uint32_t flags);
		static void SetFlag(RenderFlag flag);
		static void ToggleFlag(RenderFlag flag);
		static void DisableFlag(RenderFlag flag);
		static bool IsFlagSet(RenderFlag flag);
		static void ClearFlags();

		static SharedReference<Material> GetWhiteMaterial();
		static SharedReference<Texture2D> GetWhiteTexture();

		static ShaderLibrary& GetShaderLibrary();

	private:
		// Helpers

		static void BindRenderTarget(SharedReference<Framebuffer> renderTarget);
		static void BindShaders(const Math::mat4& view, const Math::mat4& projection, const Math::vec3& cameraTranslation);

		static void RenderDirectionalLightShadow(const LightSourceComponent& lightSourceComponent, Entity lightSourceEntity, SharedReference<SceneGeometry>& sceneMeshes);
		static void RenderPointLightShadow(const LightSourceComponent& lightSourceComponent, Entity lightSourceEntity, SharedReference<SceneGeometry>& sceneMeshes);
		static void RenderSpotLightShadow(const LightSourceComponent& lightSourceComponent, Entity lightSourceEntity, SharedReference<SceneGeometry>& sceneMeshes);

		// Post Processing

		static void ConfigurePostProcessingPipeline(const PostProcessProperties& postProcessProps);
		static std::vector<PostProcessStage> SortPostProcessStages(PostProcessStage* stages, uint32_t count);
		static uint32_t GetPostProcessStageScore(PostProcessStage stage);
		static PostProcessStage FindHighestPriortyStage(PostProcessStage* stages, uint32_t count);
		static void CreateBlurFramebuffer(uint32_t width, uint32_t height);
		static void BlurAndSubmitFinalSceneComposite(SharedReference<Framebuffer> sceneFramebuffer, const Math::vec3& cameraPosition);
	};

}
