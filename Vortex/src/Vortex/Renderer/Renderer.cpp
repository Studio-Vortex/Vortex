#include "vxpch.h"
#include "Renderer.h"

#include "Vortex/Core/Base.h"
#include "Vortex/Core/Application.h"

#include "Vortex/Module/Module.h"

#include "Vortex/Animation/Animator.h"

#include "Vortex/Renderer/Renderer2D.h"
#include "Vortex/Renderer/BloomPass.h"
#include "Vortex/Renderer/Texture.h"
#include "Vortex/Renderer/Skybox.h"
#include "Vortex/Renderer/Mesh.h"
#include "Vortex/Renderer/StaticMesh.h"
#include "Vortex/Renderer/Material.h"
#include "Vortex/Renderer/Framebuffer.h"

#include "Vortex/Editor/EditorCamera.h"

namespace Vortex {

	struct RendererInternalData
	{
		ShaderLibrary ShaderLibrary;

		SharedReference<StaticMesh> SkyboxMesh = nullptr;
		SharedReference<Skybox> CurrentEnvironment = nullptr;

		SharedReference<Material> WhiteMaterial = nullptr;

		static constexpr inline uint32_t MaxPointLights = 50;
		static constexpr inline uint32_t MaxSpotLights = 50;
		
		SceneLightDescription SceneLightDesc{};

		SharedReference<Framebuffer> TargetFramebuffer = nullptr;

		SharedReference<HDRFramebuffer> HDRFramebuffer = nullptr;
		SharedReference<DepthMapFramebuffer> SkylightDepthMapFramebuffer = nullptr;
		std::vector<SharedReference<DepthCubemapFramebuffer>> PointLightDepthMapFramebuffers;
		std::vector<SharedReference<DepthMapFramebuffer>> SpotLightDepthMapFramebuffers;

		float EnvironmentMapResolution = 512.0f;
		float PrefilterMapResolution = 128.0f;
		float ShadowMapResolution = 1024.0f;
		float SceneExposure = 1.0f;
		float SceneGamma = 2.2f;

		BloomRenderPass BloomRenderPass;
		struct BloomSettings
		{
			float Threshold = 0.2126f;
			float Knee = 0.7152f;
			float Intensity = 0.0722f;
		} BloomSettings;
		uint32_t BloomSampleSize = 5;

		RenderStatistics RendererStatistics;
		RenderTime RenderTime;
		RendererAPI::TriangleCullMode CullMode = RendererAPI::TriangleCullMode::None;

		SharedReference<Texture2D> BRDF_LUT = nullptr;

		uint32_t RenderFlags = 0;

		SubModule Module;
	};

	static RendererInternalData s_Data;

	void Renderer::Init()
	{
		VX_PROFILE_FUNCTION();

		RenderCommand::Init();

		// Mesh shaders
		s_Data.ShaderLibrary.Load("PBR", "Resources/Shaders/Renderer_PBR.glsl");
		s_Data.ShaderLibrary.Load("PBR_Static", "Resources/Shaders/Renderer_PBR_Static.glsl");
		
		// PBR shaders
		s_Data.ShaderLibrary.Load("EquirectangularToCubemap", "Resources/Shaders/PBR_Equirectangular_to_Cubemap.glsl");
		s_Data.ShaderLibrary.Load("IrradianceConvolution", "Resources/Shaders/PBR_Irradiance_Convolution.glsl");
		s_Data.ShaderLibrary.Load("IBL_Prefilter", "Resources/Shaders/PBR_IBL_Prefilter.glsl");
		s_Data.ShaderLibrary.Load("BRDF_LUT", "Resources/Shaders/PBR_BRDF_LUT.glsl");
		s_Data.ShaderLibrary.Load("Environment", "Resources/Shaders/PBR_Environment.glsl");

		// Shadow map shaders
		s_Data.ShaderLibrary.Load("SkyLightShadowMap", "Resources/Shaders/Renderer_SkyLightShadow.glsl");
		s_Data.ShaderLibrary.Load("PointLightShadowMap", "Resources/Shaders/Renderer_PointLightShadow.glsl");
		s_Data.ShaderLibrary.Load("SpotLightShadowMap", "Resources/Shaders/Renderer_SpotLightShadow.glsl");
		
		s_Data.ShaderLibrary.Load("Stencil", "Resources/Shaders/Renderer_Stencil.glsl");

		// Bloom shaders
		s_Data.ShaderLibrary.Load("Bloom_Upsample", "Resources/Shaders/Bloom_Upsample.glsl");
		s_Data.ShaderLibrary.Load("Bloom_Downsample", "Resources/Shaders/Bloom_Downsample.glsl");
		s_Data.ShaderLibrary.Load("Bloom", "Resources/Shaders/Bloom.glsl");
		s_Data.ShaderLibrary.Load("Bloom_FinalComposite", "Resources/Shaders/Bloom_FinalComposite.glsl");

		// BRDF look up texture
		TextureProperties brdfImageProps;
		brdfImageProps.Filepath = "Resources/Textures/IBL_BRDF_LUT.tga";
		brdfImageProps.WrapMode = ImageWrap::Clamp;
		s_Data.BRDF_LUT = Texture2D::Create(brdfImageProps);

		SharedReference<Shader> shader = s_Data.ShaderLibrary.Get("PBR_Static");
		MaterialProperties materialProps{};
		materialProps.Albedo = Math::vec3(1.0f, 1.0f, 1.0f);
		s_Data.WhiteMaterial = Material::Create(shader, materialProps);

		s_Data.SkyboxMesh = StaticMesh::Create(MeshType::Cube);

#if VX_RENDERER_STATISTICS
		ResetStats();
#endif // VX_RENDERER_STATISTICS

		Renderer2D::Init();

		SubModuleProperties moduleProps;
		moduleProps.ModuleName = "Renderer";
		moduleProps.APIVersion = Version(1, 0, 0);
		moduleProps.RequiredModules = {};
		s_Data.Module.Init(moduleProps);

		Application::Get().AddModule(s_Data.Module);
	}

	void Renderer::Shutdown()
	{
		VX_PROFILE_FUNCTION();

		s_Data.SkyboxMesh.Reset();
		s_Data.WhiteMaterial.Reset();

		s_Data.BRDF_LUT.Reset();

		s_Data.BloomRenderPass.Destroy();

		Renderer2D::Shutdown();

		Application::Get().RemoveModule(s_Data.Module);
		s_Data.Module.Shutdown();
	}

	void Renderer::OnWindowResize(const Viewport& viewport)
	{
		RenderCommand::SetViewport(viewport);
		s_Data.BloomRenderPass.Destroy();
		CreateBlurFramebuffer(viewport.Width, viewport.Height);
	}

	void Renderer::BeginScene(const Camera& camera, const Math::mat4& view, const Math::vec3& translation, SharedReference<Framebuffer> targetFramebuffer)
	{
		VX_PROFILE_FUNCTION();

		BindRenderTarget(targetFramebuffer);

		BindShaders(view, camera.GetProjectionMatrix(), translation);
		RenderCommand::SetBlendMode(RendererAPI::BlendMode::SrcAlphaOneMinusSrcAlpha);
	}

	void Renderer::BeginScene(const EditorCamera* camera, SharedReference<Framebuffer> targetFramebuffer)
	{
		VX_PROFILE_FUNCTION();

		BindRenderTarget(targetFramebuffer);

		BindShaders(camera->GetViewMatrix(), camera->GetProjectionMatrix(), camera->GetPosition());
		RenderCommand::SetBlendMode(RendererAPI::BlendMode::SrcAlphaOneMinusSrcAlpha);
	}

	void Renderer::EndScene()
	{
		s_Data.TargetFramebuffer = nullptr;
	}

	void Renderer::Submit(const SharedReference<Shader>& shader, const SharedReference<VertexArray>& vertexArray)
	{
		VX_PROFILE_FUNCTION();

		shader->Enable();
		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
		s_Data.RendererStatistics.DrawCalls++;
	}

	void Renderer::DrawIndexed(const SharedReference<Shader>& shader, const SharedReference<VertexArray>& vertexArray)
	{
		VX_PROFILE_FUNCTION();

		shader->Enable();
		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
		s_Data.RendererStatistics.DrawCalls++;
	}

	void Renderer::RenderLightSource(const TransformComponent& transform, const LightSourceComponent& lightSourceComponent)
	{
		SharedReference<Shader> shaders[] = { s_Data.ShaderLibrary.Get("PBR"), s_Data.ShaderLibrary.Get("PBR_Static") };
		const uint32_t shaderCount = VX_ARRAYCOUNT(shaders);

		switch (lightSourceComponent.Type)
		{
			case LightType::Directional:
			{
				Math::mat4 orthogonalProjection = Math::Ortho(-75.0f, 75.0f, -75.0f, 75.0f, 0.01f, 500.0f);
				Math::mat4 lightView = Math::LookAt(transform.Translation, transform.GetRotationEuler(), Math::vec3(0.0f, 1.0f, 0.0f));
				Math::mat4 lightProjection = orthogonalProjection * lightView;

				for (uint32_t i = 0; i < shaderCount; i++)
				{
					auto& shader = shaders[i];

					shader->Enable();
					shader->SetFloat3("u_SkyLight.Radiance", lightSourceComponent.Radiance);
					shader->SetFloat3("u_SkyLight.Direction", Math::Normalize(transform.GetRotationEuler()));
					shader->SetFloat("u_SkyLight.ShadowBias", lightSourceComponent.ShadowBias / 1'000.0f);
					shader->SetBool("u_SkyLight.SoftShadows", lightSourceComponent.SoftShadows);
					shader->SetFloat("u_SkyLight.Intensity", lightSourceComponent.Intensity);
					shader->SetMat4("u_SkyLightProjection", lightProjection);
				}

				s_Data.SceneLightDesc.HasSkyLight = true;

				break;
			}
			case LightType::Point:
			{
				uint32_t& pointLightIndex = s_Data.SceneLightDesc.PointLightIndex;

				if (pointLightIndex > RendererInternalData::MaxPointLights - 1)
					break;

				for (uint32_t i = 0; i < shaderCount; i++)
				{
					auto& shader = shaders[i];

					shader->Enable();
					shader->SetFloat3("u_PointLights[" + std::to_string(pointLightIndex) + "].Radiance", lightSourceComponent.Radiance);
					shader->SetFloat3("u_PointLights[" + std::to_string(pointLightIndex) + "].Position", transform.Translation);
					shader->SetFloat("u_PointLights[" + std::to_string(pointLightIndex) + "].Intensity", lightSourceComponent.Intensity);
				}

				pointLightIndex++;

				break;
			}
			case LightType::Spot:
			{
				uint32_t& spotLightIndex = s_Data.SceneLightDesc.SpotLightIndex;

				if (spotLightIndex > RendererInternalData::MaxSpotLights - 1)
					break;

				for (uint32_t i = 0; i < shaderCount; i++)
				{
					auto& shader = shaders[i];

					shader->Enable();
					shader->SetFloat3("u_SpotLights[" + std::to_string(spotLightIndex) + "].Radiance", lightSourceComponent.Radiance);
					shader->SetFloat3("u_SpotLights[" + std::to_string(spotLightIndex) + "].Position", transform.Translation);
					shader->SetFloat3("u_SpotLights[" + std::to_string(spotLightIndex) + "].Direction", transform.GetRotationEuler());
					shader->SetFloat("u_SpotLights[" + std::to_string(spotLightIndex) + "].Intensity", lightSourceComponent.Intensity);
					shader->SetFloat("u_SpotLights[" + std::to_string(spotLightIndex) + "].CutOff", Math::Cos(Math::Deg2Rad(lightSourceComponent.Cutoff)));
					shader->SetFloat("u_SpotLights[" + std::to_string(spotLightIndex) + "].OuterCutOff", Math::Cos(Math::Deg2Rad(lightSourceComponent.OuterCutoff)));
				}

				spotLightIndex++;

				break;
			}
		}
	}

	void Renderer::DrawEnvironmentMap(const Math::mat4& view, const Math::mat4& projection, SkyboxComponent& skyboxComponent, SharedReference<Skybox>& environment)
	{
		if (!s_Data.SceneLightDesc.HasEnvironment)
			return;

		RenderCommand::SetCullMode(RendererAPI::TriangleCullMode::None);

		// Render Environment Map
		{
			RenderCommand::DisableDepthMask();

			SharedReference<Shader> environmentShader = s_Data.ShaderLibrary.Get("Environment");
			environmentShader->Enable();
			environmentShader->SetMat4("u_View", Math::mat4(Math::mat3(view)));
			environmentShader->SetMat4("u_Projection", projection);
			environmentShader->SetInt("u_EnvironmentMap", 0);
			environmentShader->SetFloat("u_Gamma", s_Data.SceneGamma);
			environmentShader->SetFloat("u_Exposure", s_Data.SceneExposure);
			environmentShader->SetFloat("u_Intensity", Math::Max(skyboxComponent.Intensity, 0.0f));

			SharedReference<VertexArray> skyboxMeshVA = s_Data.SkyboxMesh->GetSubmeshes().at(0).GetVertexArray();

			skyboxMeshVA->Bind();
			s_Data.HDRFramebuffer->BindEnvironmentCubemap();
			RenderCommand::DrawTriangles(skyboxMeshVA, 36);
			RenderCommand::EnableDepthMask();
		}

		SharedReference<Shader> pbrShader = s_Data.ShaderLibrary.Get("PBR");
		pbrShader->Enable();
		pbrShader->SetInt("u_SceneProperties.IrradianceMap", 1);
		s_Data.HDRFramebuffer->BindIrradianceCubemap();
		pbrShader->SetInt("u_SceneProperties.PrefilterMap", 2);
		s_Data.HDRFramebuffer->BindPrefilterCubemap();
		pbrShader->SetInt("u_SceneProperties.BRDFLut", 3);
		s_Data.BRDF_LUT->Bind(3);
		pbrShader->SetFloat("u_SceneProperties.SkyboxIntensity", Math::Max(skyboxComponent.Intensity, 0.0f));

		SharedReference<Shader> pbrStaticShader = s_Data.ShaderLibrary.Get("PBR_Static");
		pbrStaticShader->Enable();
		pbrStaticShader->SetInt("u_SceneProperties.IrradianceMap", 1);
		s_Data.HDRFramebuffer->BindIrradianceCubemap();
		pbrStaticShader->SetInt("u_SceneProperties.PrefilterMap", 2);
		s_Data.HDRFramebuffer->BindPrefilterCubemap();
		pbrStaticShader->SetInt("u_SceneProperties.BRDFLut", 3);
		s_Data.BRDF_LUT->Bind(3);
		pbrStaticShader->SetFloat("u_SceneProperties.SkyboxIntensity", Math::Max(skyboxComponent.Intensity, 0.0f));

		RenderCommand::SetCullMode(s_Data.CullMode);
	}

	void Renderer::DrawFrustum(const std::vector<Math::vec4>& corners, const Math::vec4& color)
	{
		// Near Rect
		Renderer2D::DrawLine(Math::vec3(corners[0]), Math::vec3(corners[4]), color);
		Renderer2D::DrawLine(Math::vec3(corners[2]), Math::vec3(corners[6]), color);
		Renderer2D::DrawLine(Math::vec3(corners[4]), Math::vec3(corners[6]), color);
		Renderer2D::DrawLine(Math::vec3(corners[2]), Math::vec3(corners[0]), color);

		// Far Rect
		Renderer2D::DrawLine(Math::vec3(corners[1]), Math::vec3(corners[5]), color);
		Renderer2D::DrawLine(Math::vec3(corners[3]), Math::vec3(corners[7]), color);
		Renderer2D::DrawLine(Math::vec3(corners[5]), Math::vec3(corners[7]), color);
		Renderer2D::DrawLine(Math::vec3(corners[3]), Math::vec3(corners[1]), color);

		// Connect Rects
		Renderer2D::DrawLine(Math::vec3(corners[0]), Math::vec3(corners[1]), color);
		Renderer2D::DrawLine(Math::vec3(corners[2]), Math::vec3(corners[3]), color);
		Renderer2D::DrawLine(Math::vec3(corners[4]), Math::vec3(corners[5]), color);
		Renderer2D::DrawLine(Math::vec3(corners[6]), Math::vec3(corners[7]), color);
	}

	SceneLightDescription Renderer::GetSceneLightDescription()
	{
		return s_Data.SceneLightDesc;
	}

	void Renderer::CreateEnvironmentMap(SkyboxComponent& skyboxComponent, SharedReference<Skybox>& environment)
	{
		s_Data.HDRFramebuffer = HDRFramebuffer::Create(FramebufferProperties{});

		Math::mat4 rotationMatrix = Math::Rotate(Math::Deg2Rad(skyboxComponent.Rotation), { 0.0f, 1.0f, 0.0f });

		Math::mat4 captureProjection = Math::Perspective(Math::Deg2Rad(90.0f), 1.0f, 0.1f, 10.0f);
		Math::mat4 captureViews[] =
		{
		   Math::LookAt(Math::vec3(0.0f, 0.0f, 0.0f), Math::vec3(1.0f,  0.0f,  0.0f), Math::vec3(0.0f, -1.0f,  0.0f)) * rotationMatrix,
		   Math::LookAt(Math::vec3(0.0f, 0.0f, 0.0f), Math::vec3(-1.0f, 0.0f, 0.0f),  Math::vec3(0.0f, -1.0f,  0.0f)) * rotationMatrix,
		   Math::LookAt(Math::vec3(0.0f, 0.0f, 0.0f), Math::vec3(0.0f,  1.0f,  0.0f), Math::vec3(0.0f,  0.0f,  1.0f)) * rotationMatrix,
		   Math::LookAt(Math::vec3(0.0f, 0.0f, 0.0f), Math::vec3(0.0f, -1.0f,  0.0f), Math::vec3(0.0f,  0.0f, -1.0f)) * rotationMatrix,
		   Math::LookAt(Math::vec3(0.0f, 0.0f, 0.0f), Math::vec3(0.0f,  0.0f,  1.0f), Math::vec3(0.0f, -1.0f,  0.0f)) * rotationMatrix,
		   Math::LookAt(Math::vec3(0.0f, 0.0f, 0.0f), Math::vec3(0.0f,  0.0f, -1.0f), Math::vec3(0.0f, -1.0f,  0.0f)) * rotationMatrix
		};

		s_Data.HDRFramebuffer->CreateEnvironmentCubemap(s_Data.EnvironmentMapResolution);

		// convert HDR equirectangular environment map to cubemap equivalent
		SharedReference<Shader> equirectToCubemapShader = s_Data.ShaderLibrary.Get("EquirectangularToCubemap");
		equirectToCubemapShader->Enable();
		equirectToCubemapShader->SetInt("u_EquirectangularMap", 0);
		equirectToCubemapShader->SetMat4("u_Projection", captureProjection);
		environment->Bind();

		SharedReference<VertexArray> cubeMeshVA = s_Data.SkyboxMesh->GetSubmeshes().at(0).GetVertexArray();

		{
			// don't forget to configure the viewport to the capture dimensions.
			Viewport viewport;
			viewport.TopLeftXPos = 0;
			viewport.TopLeftYPos = 0;
			viewport.Width = (uint32_t)s_Data.EnvironmentMapResolution;
			viewport.Height = (uint32_t)s_Data.EnvironmentMapResolution;

			RenderCommand::SetViewport(viewport);
		}

		s_Data.HDRFramebuffer->Bind();
		for (uint32_t i = 0; i < 6; i++)
		{
			equirectToCubemapShader->SetMat4("u_View", captureViews[i]);
			s_Data.HDRFramebuffer->SetEnvironmentCubemapFramebufferTexture(i);
			s_Data.HDRFramebuffer->ClearColorAndDepthAttachments();

			// Render a unit cube
			cubeMeshVA->Bind();
			RenderCommand::DrawTriangles(cubeMeshVA, 36);
		}
		s_Data.HDRFramebuffer->Unbind();

		// Generate mip maps
		s_Data.HDRFramebuffer->BindAndGenerateEnvironmentMipMap();

		// Create Irradiance Map
		uint32_t irradianceTexSize = 32;
		s_Data.HDRFramebuffer->CreateIrradianceCubemap(irradianceTexSize);
		s_Data.HDRFramebuffer->RescaleAndBindFramebuffer(irradianceTexSize, irradianceTexSize);

		SharedReference<Shader> irradianceConvolutionShader = s_Data.ShaderLibrary.Get("IrradianceConvolution");
		irradianceConvolutionShader->Enable();
		irradianceConvolutionShader->SetInt("u_EnvironmentMap", 0);
		irradianceConvolutionShader->SetMat4("u_Projection", captureProjection);
		s_Data.HDRFramebuffer->BindEnvironmentCubemap();

		{
			// don't forget to configure the viewport to the capture dimensions.
			Viewport viewport;
			viewport.TopLeftXPos = 0;
			viewport.TopLeftYPos = 0;
			viewport.Width = irradianceTexSize;
			viewport.Height = irradianceTexSize;

			RenderCommand::SetViewport(viewport);
		}

		s_Data.HDRFramebuffer->Bind();
		for (uint32_t i = 0; i < 6; i++)
		{
			irradianceConvolutionShader->SetMat4("u_View", captureViews[i]);
			s_Data.HDRFramebuffer->SetIrradianceCubemapFramebufferTexture(i);
			s_Data.HDRFramebuffer->ClearColorAndDepthAttachments();

			// Render a unit cube
			cubeMeshVA->Bind();
			RenderCommand::DrawTriangles(cubeMeshVA, 36);
		}
		s_Data.HDRFramebuffer->Unbind();

		// Create Prefiltered Envrionment Map
		s_Data.HDRFramebuffer->CreatePrefilteredEnvironmentCubemap(s_Data.PrefilterMapResolution);

		SharedReference<Shader> iblPrefilterShader = s_Data.ShaderLibrary.Get("IBL_Prefilter");
		iblPrefilterShader->Enable();
		iblPrefilterShader->SetInt("u_EnvironmentMap", 0);
		iblPrefilterShader->SetMat4("u_Projection", captureProjection);
		iblPrefilterShader->SetFloat("u_EnvironmentMapResolution", s_Data.EnvironmentMapResolution);
		s_Data.HDRFramebuffer->BindEnvironmentCubemap();

		// Render each mip level
		uint32_t maxMipLevels = 5;
		s_Data.HDRFramebuffer->Bind();
		for (uint32_t mip = 0; mip < maxMipLevels; mip++)
		{
			// Resize framebuffer according to mip-level size
			uint32_t mipWidth = (uint32_t)s_Data.PrefilterMapResolution * std::pow(0.5, mip);
			uint32_t mipHeight = (uint32_t)s_Data.PrefilterMapResolution * std::pow(0.5, mip);
			s_Data.HDRFramebuffer->BindAndSetRenderbufferStorage(mipWidth, mipHeight);

			// don't forget to set viewport to mip level
			{
				Viewport viewport;
				viewport.TopLeftXPos = 0;
				viewport.TopLeftYPos = 0;
				viewport.Width = mipWidth;
				viewport.Height = mipHeight;

				RenderCommand::SetViewport(viewport);
			}

			float roughness = (float)mip / (float)(maxMipLevels - 1);
			iblPrefilterShader->SetFloat("u_Roughness", roughness);
			for (uint32_t i = 0; i < 6; i++)
			{
				iblPrefilterShader->SetMat4("u_View", captureViews[i]);
				s_Data.HDRFramebuffer->SetPrefilterCubemapFramebufferTexture(i, mip);
				s_Data.HDRFramebuffer->ClearColorAndDepthAttachments();

				// Render a unit cube
				cubeMeshVA->Bind();
				RenderCommand::DrawTriangles(cubeMeshVA, 36);
			}
		}

		s_Data.HDRFramebuffer->Unbind();

		environment->SetShouldReload(false);
	}

	void Renderer::CreateShadowMap(LightType type)
	{
		switch (type)
		{
			case LightType::Directional:
			{
				s_Data.SkylightDepthMapFramebuffer.Reset();
				FramebufferProperties depthFramebufferProps{};
				depthFramebufferProps.Width = s_Data.ShadowMapResolution;
				depthFramebufferProps.Height = s_Data.ShadowMapResolution;
				s_Data.SkylightDepthMapFramebuffer = DepthMapFramebuffer::Create(depthFramebufferProps);

				break;
			}
			case LightType::Point:
			{
				/*FramebufferProperties depthCubemapProps{};
				depthCubemapProps.Width = s_Data.ShadowMapResolution;
				depthCubemapProps.Height = s_Data.ShadowMapResolution;
				SharedRef<DepthCubemapFramebuffer> framebuffer = DepthCubemapFramebuffer::Create(depthCubemapProps);
				s_Data.PointLightDepthMapFramebuffers.push_back(framebuffer);
				s_Data.SceneLightDesc.ActivePointLights++;*/

				break;
			}
			case LightType::Spot:
			{
				/*FramebufferProperties depthFramebufferProps{};
				depthFramebufferProps.Width = s_Data.ShadowMapResolution;
				depthFramebufferProps.Height = s_Data.ShadowMapResolution;
				SharedRef<DepthMapFramebuffer> framebuffer = DepthMapFramebuffer::Create(depthFramebufferProps);
				s_Data.SpotLightDepthMapFramebuffers.push_back(framebuffer);
				s_Data.SceneLightDesc.ActiveSpotLights++;*/

				break;
			}
		}
	}

	void Renderer::BeginPostProcessingStages(const PostProcessProperties& postProcessProps)
	{
		ConfigurePostProcessingPipeline(postProcessProps);

		std::vector<PostProcessStage> postProcessStages = SortPostProcessStages(postProcessProps.Stages, postProcessProps.StageCount);

		for (auto postProcessStage : postProcessStages)
		{
			VX_CORE_ASSERT(postProcessStage != PostProcessStage::None, "Unknown Post Process Stage!");

			switch (postProcessStage)
			{
				case PostProcessStage::Bloom:
				{
					if (IsFlagSet(RenderFlag::EnableBloom))
						BlurAndSubmitFinalSceneComposite(postProcessProps.TargetFramebuffer, postProcessProps.CameraPosition);

					break;
				}
			}
		}
	}

	void Renderer::RenderToDepthMap(SharedReference<Scene>& contextScene)
	{
		auto& sceneMeshes = contextScene->GetSceneMeshes();

		auto lightSourceView = contextScene->GetAllEntitiesWith<LightSourceComponent>();

		if (!s_Data.SkylightDepthMapFramebuffer)
		{
			CreateShadowMap(LightType::Directional);
		}

		for (const auto& lightSource : lightSourceView)
		{
			Entity lightSourceEntity{ lightSource, contextScene.Raw() };
			LightSourceComponent& lightSourceComponent = lightSourceEntity.GetComponent<LightSourceComponent>();

			switch (lightSourceComponent.Type)
			{
				case LightType::Directional:
				{
					if (!lightSourceComponent.CastShadows)
					{
						s_Data.SkylightDepthMapFramebuffer.Reset();
						continue;
					}

					RenderDirectionalLightShadow(lightSourceComponent, lightSourceEntity, sceneMeshes);

					break;
				}
				case LightType::Point:
				{
					if (!lightSourceComponent.CastShadows)
					{
						continue;
					}

					RenderPointLightShadow(lightSourceComponent, lightSourceEntity, sceneMeshes);

					break;
				}
				case LightType::Spot:
				{
					if (!lightSourceComponent.CastShadows)
					{
						continue;
					}

					RenderSpotLightShadow(lightSourceComponent, lightSourceEntity, sceneMeshes);

					break;
				}
			}
		}
	}

	const SharedReference<DepthMapFramebuffer>& Renderer::GetSkyLightDepthFramebuffer()
	{
		return s_Data.SkylightDepthMapFramebuffer;
	}

	void Renderer::BindSkyLightDepthMap()
	{
		if (!s_Data.SkylightDepthMapFramebuffer)
			return;

		{
			SharedReference<Shader> pbrShader = s_Data.ShaderLibrary.Get("PBR");
			pbrShader->Enable();

			// TEMPORARY FIX
			{
				pbrShader->SetInt("u_SceneProperties.ActivePointLights", s_Data.SceneLightDesc.PointLightIndex);
				pbrShader->SetInt("u_SceneProperties.ActiveSpotLights", s_Data.SceneLightDesc.SpotLightIndex);
			}

			s_Data.SkylightDepthMapFramebuffer->BindDepthTexture(4);
			pbrShader->Enable();
			pbrShader->SetInt("u_SkyLight.ShadowMap", 4);
		}

		{
			SharedReference<Shader> pbrStaticShader = s_Data.ShaderLibrary.Get("PBR_Static");
			pbrStaticShader->Enable();

			// TEMPORARY FIX
			{
				pbrStaticShader->SetInt("u_SceneProperties.ActivePointLights", s_Data.SceneLightDesc.PointLightIndex);
				pbrStaticShader->SetInt("u_SceneProperties.ActiveSpotLights", s_Data.SceneLightDesc.SpotLightIndex);
			}

			s_Data.SkylightDepthMapFramebuffer->BindDepthTexture(4);
			pbrStaticShader->Enable();
			pbrStaticShader->SetInt("u_SkyLight.ShadowMap", 4);
		}
	}

	void Renderer::BindPointLightDepthMaps()
	{
		/*SharedReference<Shader> pbrShader = s_Data.ShaderLibrary->Get("PBR");
		pbrShader->Enable();

		const uint32_t startSlot = 12;
		uint32_t index = 0;

		for (const auto& framebuffer : s_Data.PointLightDepthMapFramebuffers)
		{
			uint32_t currentSlot = startSlot + index;
			framebuffer->BindDepthTexture(currentSlot);
			pbrShader->SetInt("u_PointLightShadowMaps[" + std::to_string(index) + "]", currentSlot);
			index++;
		}*/
	}

	void Renderer::BindSpotLightDepthMaps()
	{
		/*SharedReference<Shader> pbrShader = s_Data.ShaderLibrary->Get("PBR");
		pbrShader->Enable();

		const uint32_t startSlot = 12;
		uint32_t index = 0;

		for (const auto& framebuffer : s_Data.SpotLightDepthMapFramebuffers)
		{
			uint32_t currentSlot = startSlot + index;
			framebuffer->BindDepthTexture(currentSlot);
			pbrShader->SetInt("u_SpotLightShadowMaps[" + std::to_string(index) + "]", currentSlot);
			index++;
		}*/
	}

	void Renderer::BindRenderTarget(SharedReference<Framebuffer>& renderTarget)
	{
		VX_CORE_ASSERT(renderTarget, "Invalid Render Target!");

		if (renderTarget)
		{
			s_Data.TargetFramebuffer = renderTarget;
			renderTarget->Bind();
		}
	}

	void Renderer::BindShaders(const Math::mat4& view, const Math::mat4& projection, const Math::vec3& cameraPosition)
	{
		VX_PROFILE_FUNCTION();

		Math::mat4 viewProjection = projection * view;

		Math::vec3 bloomSettings = { s_Data.BloomSettings.Threshold, s_Data.BloomSettings.Knee, s_Data.BloomSettings.Intensity };

		SharedReference<Shader> pbrShader = s_Data.ShaderLibrary.Get("PBR");
		pbrShader->Enable();
		pbrShader->SetMat4("u_ViewProjection", viewProjection);
		pbrShader->SetFloat3("u_SceneProperties.CameraPosition", cameraPosition);
		pbrShader->SetFloat("u_SceneProperties.Exposure", s_Data.SceneExposure);
		pbrShader->SetFloat("u_SceneProperties.Gamma", s_Data.SceneGamma);
		pbrShader->SetFloat3("u_SceneProperties.BloomThreshold", bloomSettings);

		SharedReference<Shader> pbrStaticShader = s_Data.ShaderLibrary.Get("PBR_Static");
		pbrStaticShader->Enable();
		pbrStaticShader->SetMat4("u_ViewProjection", viewProjection);
		pbrStaticShader->SetFloat3("u_SceneProperties.CameraPosition", cameraPosition);
		pbrStaticShader->SetFloat("u_SceneProperties.Exposure", s_Data.SceneExposure);
		pbrStaticShader->SetFloat("u_SceneProperties.Gamma", s_Data.SceneGamma);
		pbrStaticShader->SetFloat3("u_SceneProperties.BloomThreshold", bloomSettings);

		s_Data.SceneLightDesc.HasSkyLight = false;
		s_Data.SceneLightDesc.PointLightIndex = 0;
		s_Data.SceneLightDesc.SpotLightIndex = 0;
	}

	void Renderer::RenderDirectionalLightShadow(const LightSourceComponent& lightSourceComponent, Entity lightSourceEntity, SharedReference<Scene::SceneGeometry>& sceneMeshes)
	{
		SharedReference<Shader> shadowMapShader = s_Data.ShaderLibrary.Get("SkyLightShadowMap");

		// Configure shader
		{
			Math::mat4 orthogonalProjection = Math::Ortho(-75.0f, 75.0f, -75.0f, 75.0f, 0.01f, 500.0f);
			TransformComponent& transform = lightSourceEntity.GetTransform();
			Math::mat4 lightView = Math::LookAt(transform.Translation, Math::Normalize(transform.GetRotationEuler()), Math::vec3(0.0f, 1.0f, 0.0f));
			Math::mat4 lightProjection = orthogonalProjection * lightView;

			RenderCommand::SetCullMode(RendererAPI::TriangleCullMode::Front);

			{
				// don't forget to configure the viewport to the shadow resolution
				Viewport viewport;
				viewport.TopLeftXPos = 0;
				viewport.TopLeftYPos = 0;
				viewport.Width = (uint32_t)s_Data.ShadowMapResolution;
				viewport.Height = (uint32_t)s_Data.ShadowMapResolution;

				RenderCommand::SetViewport(viewport);
			}

			s_Data.SkylightDepthMapFramebuffer->Bind();

			shadowMapShader->Enable();
			shadowMapShader->SetMat4("u_LightProjection", lightProjection);
			s_Data.SkylightDepthMapFramebuffer->ClearDepth(1.0f);
			s_Data.SkylightDepthMapFramebuffer->ClearDepthAttachment();
		}

		uint32_t i = 0;

		// Render Meshes
		for (const auto& mesh : sceneMeshes->Meshes)
		{
			Math::mat4 worldSpaceTransform = sceneMeshes->WorldSpaceMeshTransforms[i];
			shadowMapShader->SetMat4("u_Model", worldSpaceTransform);

			if (mesh->HasAnimations() && sceneMeshes->MeshEntities[i].HasComponent<AnimatorComponent>())
			{
				shadowMapShader->SetBool("u_HasAnimations", true);

				const AnimatorComponent& animatorComponent = sceneMeshes->MeshEntities[i].GetComponent<AnimatorComponent>();
				const std::vector<Math::mat4>& transforms = animatorComponent.Animator->GetFinalBoneMatrices();

				for (uint32_t c = 0; c < transforms.size(); c++)
				{
					shadowMapShader->SetMat4("u_FinalBoneMatrices[" + std::to_string(c) + "]", transforms[c]);
				}
			}
			else
			{
				shadowMapShader->SetBool("u_HasAnimations", false);
			}

			const Submesh& submesh = mesh->GetSubmesh();

			submesh.RenderToSkylightShadowMap();

			i++;
		}

		i = 0;

		// Render Static Meshes
		for (const auto& staticMesh : sceneMeshes->StaticMeshes)
		{
			Math::mat4 worldSpaceTransform = sceneMeshes->WorldSpaceStaticMeshTransforms[i++];
			shadowMapShader->SetMat4("u_Model", worldSpaceTransform);

			const auto& submeshes = staticMesh->GetSubmeshes();

			for (const auto& [submeshIndex, submesh] : submeshes)
			{
				submesh.RenderToSkylightShadowMap();
			}
		}

		s_Data.SkylightDepthMapFramebuffer->Unbind();
		RenderCommand::SetCullMode(s_Data.CullMode);
	}

	void Renderer::RenderPointLightShadow(const LightSourceComponent& lightSourceComponent, Entity lightSourceEntity, SharedReference<Scene::SceneGeometry>& sceneMeshes)
	{
		// Configure shader
		/*float aspectRatio = (float)s_Data.ShadowMapResolution / (float)s_Data.ShadowMapResolution;
		float nearPlane = 0.01f;
		float farPlane = 100.0f;
		Math::mat4 perspectiveProjection = Math::Perspective(Math::Deg2Rad(90.0f), aspectRatio, nearPlane, farPlane);
		TransformComponent& transform = lightSourceEntity.GetTransform();

		Math::mat4 shadowTransforms[6]{};
		shadowTransforms[0] = perspectiveProjection * Math::LookAt(transform.Translation, transform.Translation + Math::vec3(1.0, 0.0, 0.0), Math::vec3(0.0, -1.0, 0.0));
		shadowTransforms[1] = perspectiveProjection * Math::LookAt(transform.Translation, transform.Translation + Math::vec3(-1.0, 0.0, 0.0), Math::vec3(0.0, -1.0, 0.0));
		shadowTransforms[2] = perspectiveProjection * Math::LookAt(transform.Translation, transform.Translation + Math::vec3(0.0, 1.0, 0.0), Math::vec3(0.0, 0.0, 1.0));
		shadowTransforms[3] = perspectiveProjection * Math::LookAt(transform.Translation, transform.Translation + Math::vec3(0.0, -1.0, 0.0), Math::vec3(0.0, 0.0, -1.0));
		shadowTransforms[4] = perspectiveProjection * Math::LookAt(transform.Translation, transform.Translation + Math::vec3(0.0, 0.0, 1.0), Math::vec3(0.0, -1.0, 0.0));
		shadowTransforms[5] = perspectiveProjection * Math::LookAt(transform.Translation, transform.Translation + Math::vec3(0.0, 0.0, -1.0), Math::vec3(0.0, -1.0, 0.0));

		uint32_t pointLightIndex = lightSourceComponent.Source->GetPointLightIndex();

		SharedReference<Shader> pbrShader = s_Data.ShaderLibrary->Get("PBR");
		pbrShader->Enable();
		pbrShader->SetFloat("u_PointLights[" + std::to_string(pointLightIndex) + "].ShadowBias", lightSourceComponent.Source->GetShadowBias() / 1'000.0f);
		pbrShader->SetFloat("u_PointLights[" + std::to_string(pointLightIndex) + "].FarPlane", farPlane);

		SharedReference<Shader> shadowMapShader = s_Data.ShaderLibrary->Get("PointLightShadowMap");
		shadowMapShader->Enable();
		for (uint32_t i = 0; i < 6; i++)
			shadowMapShader->SetMat4("u_ShadowTransforms[" + std::to_string(i) + "]", shadowTransforms[i]);

		shadowMapShader->SetFloat3("u_LightPosition", transform.Translation);
		shadowMapShader->SetFloat("u_FarPlane", farPlane);

		RenderCommand::SetCullMode(RendererAPI::TriangleCullMode::None);

		{
			// don't forget to configure the viewport to the shadow resolution
			Viewport viewport;
			viewport.TopLeftXPos = 0;
			viewport.TopLeftYPos = 0;
			viewport.Width = (uint32_t)s_Data.ShadowMapResolution;
			viewport.Height = (uint32_t)s_Data.ShadowMapResolution;

			RenderCommand::SetViewport(viewport);
		}

		uint32_t framebufferCount = s_Data.PointLightDepthMapFramebuffers.size();
		uint32_t pointLightCount = s_Data.SceneLightDesc.ActivePointLights;

		if (framebufferCount < pointLightCount || framebufferCount == 0)
		{
			CreateShadowMap(LightType::Point, lightSourceComponent.Source);
			framebufferCount = s_Data.PointLightDepthMapFramebuffers.size();
		}

		for (uint32_t i = 0; i < framebufferCount; i++)
		{
			SharedRef<DepthCubemapFramebuffer> pointLightDepthFramebuffer = s_Data.PointLightDepthMapFramebuffers[i];
			pointLightDepthFramebuffer->Bind();
			pointLightDepthFramebuffer->ClearDepthAttachment();

			// Render to shadow map
			for (auto& meshRenderer : meshRendererView)
			{
				Entity meshRendererEntity{ meshRenderer, contextScene };

				// Skip if not active
				if (!meshRendererEntity.IsActive())
					continue;

				MeshRendererComponent& meshRendererComponent = meshRendererEntity.GetComponent<MeshRendererComponent>();
				Math::mat4 worldSpaceTransform = contextScene->GetWorldSpaceTransformMatrix(meshRendererEntity);
				shadowMapShader->SetMat4("u_Model", worldSpaceTransform);

				if (meshRendererComponent.Mesh->HasAnimations() && meshRendererEntity.HasComponent<AnimatorComponent>())
				{
					meshRendererComponent.Mesh->RenderToPointLightShadowMap(worldSpaceTransform, meshRendererEntity.GetComponent<AnimatorComponent>());
					continue;
				}

				meshRendererComponent.Mesh->RenderToPointLightShadowMap(worldSpaceTransform);
			}

			pointLightDepthFramebuffer->Unbind();
		}

		RenderCommand::SetCullMode(s_Data.CullMode);*/
	}

	void Renderer::RenderSpotLightShadow(const LightSourceComponent& lightSourceComponent, Entity lightSourceEntity, SharedReference<Scene::SceneGeometry>& sceneMeshes)
	{
		/*float aspectRatio = (float)s_Data.ShadowMapResolution / (float)s_Data.ShadowMapResolution;
		float nearPlane = 0.01f;
		float farPlane = 100.0f;
		Math::mat4 perspectiveProjection = Math::Perspective(Math::Deg2Rad(45.0f), aspectRatio, nearPlane, farPlane);
		TransformComponent& transform = lightSourceEntity.GetTransform();

		Math::mat4 view = Math::LookAt(transform.Translation, transform.Translation + Math::Normalize(transform.GetRotationEuler()), { 0.0f, 1.0f, 0.0f });

		uint32_t spotLightIndex = lightSourceComponent.Source->GetSpotLightIndex();

		SharedReference<Shader> pbrShader = s_Data.ShaderLibrary->Get("PBR");
		pbrShader->Enable();
		pbrShader->SetFloat("u_SpotLights[" + std::to_string(spotLightIndex) + "].ShadowBias", lightSourceComponent.Source->GetShadowBias() / 1'000.0f);

		SharedReference<Shader> shadowMapShader = s_Data.ShaderLibrary->Get("SpotLightShadowMap");
		shadowMapShader->Enable();

		RenderCommand::SetCullMode(RendererAPI::TriangleCullMode::None);

		{
			// don't forget to configure the viewport to the shadow resolution
			Viewport viewport;
			viewport.TopLeftXPos = 0;
			viewport.TopLeftYPos = 0;
			viewport.Width = (uint32_t)s_Data.ShadowMapResolution;
			viewport.Height = (uint32_t)s_Data.ShadowMapResolution;

			RenderCommand::SetViewport(viewport);
		}

		uint32_t framebufferCount = s_Data.SpotLightDepthMapFramebuffers.size();
		uint32_t spotLightCount = s_Data.SceneLightDesc.ActiveSpotLights;

		if (framebufferCount < spotLightCount)
		{
			CreateShadowMap(LightType::Spot, lightSourceComponent.Source);
			framebufferCount = s_Data.SpotLightDepthMapFramebuffers.size();
		}

		for (uint32_t i = 0; i < framebufferCount; i++)
		{
			SharedRef<DepthMapFramebuffer> spotLightDepthFramebuffer = s_Data.SpotLightDepthMapFramebuffers[i];
			spotLightDepthFramebuffer->Bind();
			spotLightDepthFramebuffer->ClearDepthAttachment();

			for (auto& meshRenderer : meshRendererView)
			{
				Entity meshRendererEntity{ meshRenderer, contextScene };

				// Skip if not active
				if (!meshRendererEntity.IsActive())
					continue;

				MeshRendererComponent& meshRendererComponent = meshRendererEntity.GetComponent<MeshRendererComponent>();
				Math::mat4 worldSpaceTransform = contextScene->GetWorldSpaceTransformMatrix(meshRendererEntity);
				shadowMapShader->SetMat4("u_Model", worldSpaceTransform);

				if (meshRendererComponent.Mesh->HasAnimations() && meshRendererEntity.HasComponent<AnimatorComponent>())
				{
					meshRendererComponent.Mesh->RenderToPointLightShadowMap(worldSpaceTransform, meshRendererEntity.GetComponent<AnimatorComponent>());
					continue;
				}

				meshRendererComponent.Mesh->RenderToPointLightShadowMap(worldSpaceTransform);
			}

			spotLightDepthFramebuffer->Unbind();
		}

		RenderCommand::SetCullMode(s_Data.CullMode);*/
	}

	void Renderer::ConfigurePostProcessingPipeline(const PostProcessProperties& postProcessProps)
	{
		static bool init = false;
		if (init == false)//should be checking if the framebuffer wasn't created yet
		{
			Viewport viewport = postProcessProps.ViewportSize;
			CreateBlurFramebuffer(viewport.Width, viewport.Height);
			init = true;
		}
	}

	std::vector<PostProcessStage> Renderer::SortPostProcessStages(PostProcessStage* stages, uint32_t count)
	{
		std::vector<PostProcessStage> result(count);
		for (uint32_t i = 0; i < count; i++)
		{
			VX_CORE_ASSERT(stages[i] != PostProcessStage::None, "Unknown post process stage!");
			result[i] = stages[i];
		}

		std::sort(result.begin(), result.end(), [](PostProcessStage lhs, PostProcessStage rhs)
		{
			return GetPostProcessStageScore(lhs) > GetPostProcessStageScore(rhs);
		});

		return result;
	}

	uint32_t Renderer::GetPostProcessStageScore(PostProcessStage stage)
	{
		switch (stage)
		{
			case PostProcessStage::Bloom: return 1;
		}

		VX_CORE_ASSERT(false, "Unknown post process stage!");
		return 0;
	}

	PostProcessStage Renderer::FindHighestPriortyStage(PostProcessStage* stages, uint32_t count)
	{
		uint32_t highestScore = 0;
		PostProcessStage result = PostProcessStage::None;

		for (uint32_t i = 0; i < count; i++)
		{
			uint32_t score = GetPostProcessStageScore(stages[i]);
			if (score > highestScore)
			{
				highestScore = score;
				result = stages[i];
			}
		}

		return result;
	}

	void Renderer::CreateBlurFramebuffer(uint32_t width, uint32_t height)
	{
		s_Data.BloomRenderPass.InitRenderPass({ (float)width, (float)height });
	}

	void Renderer::BlurAndSubmitFinalSceneComposite(SharedReference<Framebuffer> sceneFramebuffer, const Math::vec3& cameraPosition)
	{
		s_Data.BloomRenderPass.RenderPass(cameraPosition);
	}

	RendererAPI::TriangleCullMode Renderer::GetCullMode()
	{
		return s_Data.CullMode;
	}

	void Renderer::SetCullMode(RendererAPI::TriangleCullMode cullMode)
	{
		s_Data.CullMode = cullMode;
	}

	RenderStatistics Renderer::GetStats()
	{
		return s_Data.RendererStatistics;
	}

	void Renderer::ResetStats()
	{
		memset(&s_Data.RendererStatistics, 0, sizeof(s_Data.RendererStatistics));
	}

	RenderTime& Renderer::GetRenderTime()
	{
		return s_Data.RenderTime;
	}

	void Renderer::ResetRenderTime()
	{
		memset(&s_Data.RenderTime, 0, sizeof(s_Data.RenderTime));
	}

	void Renderer::AddToQuadCountStats(uint32_t quadCount)
	{
		s_Data.RendererStatistics.QuadCount += quadCount;
	}

	void Renderer::AddToDrawCallCountStats(uint32_t drawCalls)
	{
		s_Data.RendererStatistics.DrawCalls += drawCalls;
	}

	void Renderer::SetProperties(const ProjectProperties::RendererProperties& props)
	{
		Renderer2D::SetLineWidth(props.LineWidth);
		s_Data.EnvironmentMapResolution = props.EnvironmentMapResolution;
		s_Data.PrefilterMapResolution = props.PrefilterMapResolution;
		s_Data.ShadowMapResolution = props.ShadowMapResolution;
		s_Data.SceneExposure = props.Exposure;
		s_Data.SceneGamma = props.Gamma;
		s_Data.BloomSettings.Threshold = props.BloomThreshold.x;
		s_Data.BloomSettings.Knee = props.BloomThreshold.y;
		s_Data.BloomSettings.Intensity = props.BloomThreshold.z;
		s_Data.BloomSampleSize = props.BloomSampleSize;

		ClearFlags();
		s_Data.RenderFlags = props.RenderFlags;

		Application::Get().GetWindow().SetVSync(props.UseVSync);

		if (!props.TriangleCullMode.empty())
		{
			s_Data.CullMode = Utils::TriangleCullModeFromString(props.TriangleCullMode);
		}
	}

	void Renderer::SetEnvironment(SharedReference<Skybox>& environment)
	{
		s_Data.CurrentEnvironment = environment;
		s_Data.SceneLightDesc.HasEnvironment = environment != nullptr;
	}

	float Renderer::GetEnvironmentMapResolution()
	{
		return s_Data.EnvironmentMapResolution;
	}

	void Renderer::SetEnvironmentMapResolution(float resolution)
	{
		s_Data.EnvironmentMapResolution = resolution;
	}

	float Renderer::GetPrefilterMapResolution()
	{
		return s_Data.PrefilterMapResolution;
	}

	void Renderer::SetPrefilterMapResolution(float resolution)
	{
		s_Data.PrefilterMapResolution = resolution;
	}

	float Renderer::GetShadowMapResolution()
	{
		return s_Data.ShadowMapResolution;
	}

	void Renderer::SetShadowMapResolution(float resolution)
	{
		s_Data.ShadowMapResolution = resolution;
	}

	float Renderer::GetSceneExposure()
	{
		return s_Data.SceneExposure;
	}

	void Renderer::SetSceneExposure(float exposure)
	{
		s_Data.SceneExposure = exposure;
	}

	float Renderer::GetSceneGamma()
	{
		return s_Data.SceneGamma;
	}

	void Renderer::SetSceneGamma(float gamma)
	{
		s_Data.SceneGamma = gamma;
	}

	Math::vec3 Renderer::GetBloomSettings()
	{
		return { s_Data.BloomSettings.Threshold, s_Data.BloomSettings.Knee, s_Data.BloomSettings.Intensity };
	}

	void Renderer::SetBloomSettings(const Math::vec3& bloomSettings)
	{
		s_Data.BloomSettings.Threshold = bloomSettings.x;
		s_Data.BloomSettings.Knee = bloomSettings.y;
		s_Data.BloomSettings.Intensity = bloomSettings.z;
	}

	void Renderer::SetBloomThreshold(float threshold)
	{
		s_Data.BloomSettings.Threshold = threshold;
	}

	void Renderer::SetBloomKnee(float knee)
	{
		s_Data.BloomSettings.Knee = knee;
	}

	void Renderer::SetBloomIntensity(float intensity)
	{
		s_Data.BloomSettings.Intensity = intensity;
	}

	uint32_t Renderer::GetBloomSampleSize()
	{
		return s_Data.BloomSampleSize;
	}

	void Renderer::SetBloomSampleSize(uint32_t samples)
	{
		s_Data.BloomSampleSize = samples;
	}

	uint32_t Renderer::GetFlags()
	{
		return s_Data.RenderFlags;
	}

	void Renderer::SetFlags(uint32_t flags)
	{
		ClearFlags();
		s_Data.RenderFlags = flags;
	}

	void Renderer::SetFlag(RenderFlag flag)
	{
		s_Data.RenderFlags |= (uint32_t)flag;
	}

	void Renderer::ToggleFlag(RenderFlag flag)
	{
		s_Data.RenderFlags ^= (uint32_t)flag;
	}

	void Renderer::DisableFlag(RenderFlag flag)
	{
		s_Data.RenderFlags &= (uint32_t)flag;
	}

	bool Renderer::IsFlagSet(RenderFlag flag)
	{
		return s_Data.RenderFlags & (uint32_t)flag;
	}

	void Renderer::ClearFlags()
	{
		memset(&s_Data.RenderFlags, 0, sizeof(uint32_t));
	}

	SharedReference<Material> Renderer::GetWhiteMaterial()
	{
		return s_Data.WhiteMaterial;
	}

	SharedReference<Texture2D> Renderer::GetWhiteTexture()
	{
		return Renderer2D::GetWhiteTexture();
	}

	ShaderLibrary& Renderer::GetShaderLibrary()
	{
		return s_Data.ShaderLibrary;
	}

}
