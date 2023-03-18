#include "vxpch.h"
#include "Renderer.h"

#include "Vortex/Core/Base.h"
#include "Vortex/Core/Application.h"

#include "Vortex/Renderer/Renderer2D.h"
#include "Vortex/Renderer/LightSource.h"
#include "Vortex/Renderer/Mesh.h"
#include "Vortex/Renderer/StaticMesh.h"
#include "Vortex/Renderer/Material.h"
#include "Vortex/Animation/Animator.h"

#include <unordered_map>

namespace Vortex {

	static constexpr const char* PBR_SHADER_PATH = "Resources/Shaders/Renderer_PBR.glsl";
	static constexpr const char* PBR_STATIC_SHADER_PATH = "Resources/Shaders/Renderer_PBR_Static.glsl";
	static constexpr const char* EQUIRECTANGULAR_TO_CUBEMAP_SHADER_PATH = "Resources/Shaders/Equirectangular_to_Cubemap.glsl";
	static constexpr const char* IRRADIANCE_CONVOLUTION_SHADER_PATH = "Resources/Shaders/Irradiance_Convolution.glsl";
	static constexpr const char* IBL_PREFILTER_SHADER_PATH = "Resources/Shaders/IBL_Prefilter.glsl";
	static constexpr const char* BRDF_LUT_SHADER_PATH = "Resources/Shaders/Renderer_BRDF_LUT.glsl";
	static constexpr const char* ENVIRONMENT_SHADER_PATH = "Resources/Shaders/Renderer_Environment.glsl";
	static constexpr const char* SKYLIGHT_SHADOW_MAP_SHADER_PATH = "Resources/Shaders/Renderer_SkyLightShadowMap.glsl";
	static constexpr const char* POINT_LIGHT_SHADOW_MAP_SHADER_PATH = "Resources/Shaders/Renderer_PointLightShadowMap.glsl";
	static constexpr const char* SPOT_LIGHT_SHADOW_MAP_SHADER_PATH = "Resources/Shaders/Renderer_SpotLightShadowMap.glsl";
	static constexpr const char* GAUSSIAN_BLUR_SHADER_PATH = "Resources/Shaders/GaussianBlur.glsl";
	static constexpr const char* BLOOM_FINAL_COMPOSITE_SHADER_PATH = "Resources/Shaders/BloomFinalComposite.glsl";
	static constexpr const char* STENCIL_SHADER_PATH = "Resources/Shaders/Renderer_Stencil.glsl";

	static constexpr const char* BRDF_LUT_TEXTURE_PATH = "Resources/Textures/IBL_BRDF_LUT.tga";

	struct RendererInternalData
	{
		ShaderLibrary ShaderLibrary;

		SharedReference<StaticMesh> SkyboxMesh = nullptr;

		static constexpr inline uint32_t MaxPointLights = 50;
		static constexpr inline uint32_t MaxSpotLights = 50;
		
		SceneLightDescription SceneLightDesc{};

		SharedRef<Framebuffer> TargetFramebuffer = nullptr;

		SharedRef<HDRFramebuffer> HDRFramebuffer = nullptr;
		SharedRef<DepthMapFramebuffer> SkylightDepthMapFramebuffer = nullptr;
		std::vector<SharedRef<DepthCubemapFramebuffer>> PointLightDepthMapFramebuffers;
		std::vector<SharedRef<DepthMapFramebuffer>> SpotLightDepthMapFramebuffers;

		SharedRef<GaussianBlurFramebuffer> BlurFramebuffer = nullptr;

		float EnvironmentMapResolution = 512.0f;
		float PrefilterMapResolution = 128.0f;
		float ShadowMapResolution = 1024.0f;
		float SceneExposure = 1.0f;
		float SceneGamma = 2.2f;
		Math::vec3 BloomSettings = Math::vec3(0.2126f /* Threshold */, 0.7152f /* Soft Knee */, 0.0722f /* Unknown */);
		uint32_t BloomSampleSize = 5;

		RenderStatistics RendererStatistics;
		RenderTime RenderTime;
		RendererAPI::TriangleCullMode CullMode = RendererAPI::TriangleCullMode::None;

		SharedReference<Texture2D> BRDF_LUT = nullptr;

		uint32_t RenderFlags = 0;
	};

	static RendererInternalData s_Data;

	void Renderer::Init()
	{
		VX_PROFILE_FUNCTION();

		RenderCommand::Init();

		s_Data.ShaderLibrary.Load("PBR", PBR_SHADER_PATH);
		s_Data.ShaderLibrary.Load("PBR_Static", PBR_STATIC_SHADER_PATH);
		s_Data.ShaderLibrary.Load("EquirectangularToCubemap", EQUIRECTANGULAR_TO_CUBEMAP_SHADER_PATH);
		s_Data.ShaderLibrary.Load("IrradianceConvolution", IRRADIANCE_CONVOLUTION_SHADER_PATH);
		s_Data.ShaderLibrary.Load("IBL_Prefilter", IBL_PREFILTER_SHADER_PATH);
		s_Data.ShaderLibrary.Load("BRDF_LUT", BRDF_LUT_SHADER_PATH);
		s_Data.ShaderLibrary.Load("Environment", ENVIRONMENT_SHADER_PATH);
		s_Data.ShaderLibrary.Load("SkyLightShadowMap", SKYLIGHT_SHADOW_MAP_SHADER_PATH);
		s_Data.ShaderLibrary.Load("PointLightShadowMap", POINT_LIGHT_SHADOW_MAP_SHADER_PATH);
		s_Data.ShaderLibrary.Load("SpotLightShadowMap", SPOT_LIGHT_SHADOW_MAP_SHADER_PATH);
		s_Data.ShaderLibrary.Load("Blur", GAUSSIAN_BLUR_SHADER_PATH);
		s_Data.ShaderLibrary.Load("BloomFinalComposite", BLOOM_FINAL_COMPOSITE_SHADER_PATH);
		s_Data.ShaderLibrary.Load("Stencil", STENCIL_SHADER_PATH);

		TextureProperties brdfImageProps;
		brdfImageProps.Filepath = BRDF_LUT_TEXTURE_PATH;
		brdfImageProps.WrapMode = ImageWrap::Clamp;

		s_Data.BRDF_LUT = Texture2D::Create(brdfImageProps);

		s_Data.SkyboxMesh = StaticMesh::Create(MeshType::Cube);

#if VX_RENDERER_STATISTICS
		ResetStats();
#endif // VX_RENDERER_STATISTICS

		Renderer2D::Init();
	}

	void Renderer::Shutdown()
	{
		VX_PROFILE_FUNCTION();

		Renderer2D::Shutdown();
	}

	void Renderer::OnWindowResize(const Viewport& viewport)
	{
		RenderCommand::SetViewport(viewport);
	}

	void Renderer::BeginScene(const Camera& camera, const TransformComponent& transform, SharedRef<Framebuffer> targetFramebuffer)
	{
		VX_PROFILE_FUNCTION();

		if (targetFramebuffer)
		{
			s_Data.TargetFramebuffer = targetFramebuffer;
			s_Data.TargetFramebuffer->Bind();
		}

		BindShaders(Math::Inverse(transform.GetTransform()), camera.GetProjectionMatrix(), transform.Translation);
		RenderCommand::SetBlendMode(RendererAPI::BlendMode::SrcAlphaOneMinusSrcAlpha);
	}

	void Renderer::BeginScene(const EditorCamera* camera, SharedRef<Framebuffer> targetFramebuffer)
	{
		VX_PROFILE_FUNCTION();

		if (targetFramebuffer)
		{
			s_Data.TargetFramebuffer = targetFramebuffer;
			s_Data.TargetFramebuffer->Bind();
		}

		BindShaders(camera->GetViewMatrix(), camera->GetProjectionMatrix(), camera->GetPosition());
		RenderCommand::SetBlendMode(RendererAPI::BlendMode::SrcAlphaOneMinusSrcAlpha);
	}

	void Renderer::EndScene()
	{
		s_Data.TargetFramebuffer = nullptr;
	}

	void Renderer::Submit(const SharedRef<Shader>& shader, const SharedRef<VertexArray>& vertexArray)
	{
		VX_PROFILE_FUNCTION();

		shader->Enable();
		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
		s_Data.RendererStatistics.DrawCalls++;
	}

	void Renderer::DrawIndexed(const SharedRef<Shader>& shader, const SharedRef<VertexArray>& vertexArray)
	{
		VX_PROFILE_FUNCTION();

		shader->Enable();
		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
		s_Data.RendererStatistics.DrawCalls++;
	}

	void Renderer::RenderLightSource(const TransformComponent& transform, const LightSourceComponent& lightSourceComponent)
	{
		SharedRef<LightSource> lightSource = lightSourceComponent.Source;
		SharedRef<Shader> pbrShader = s_Data.ShaderLibrary.Get("PBR");
		SharedRef<Shader> pbrStaticShader = s_Data.ShaderLibrary.Get("PBR_Static");

		switch (lightSourceComponent.Type)
		{
			case LightType::Directional:
			{
				Math::mat4 orthogonalProjection = Math::Ortho(-75.0f, 75.0f, -75.0f, 75.0f, 0.01f, 500.0f);
				Math::mat4 lightView = Math::LookAt(transform.Translation, transform.GetRotationEuler(), Math::vec3(0.0f, 1.0f, 0.0f));
				Math::mat4 lightProjection = orthogonalProjection * lightView;

				pbrShader->Enable();
				pbrShader->SetFloat3("u_SkyLight.Radiance", lightSource->GetRadiance());
				pbrShader->SetFloat3("u_SkyLight.Direction", Math::Normalize(transform.GetRotationEuler()));
				pbrShader->SetFloat("u_SkyLight.ShadowBias", lightSource->GetShadowBias() / 1'000.0f);
				pbrShader->SetBool("u_SkyLight.SoftShadows", lightSource->GetSoftShadows());
				pbrShader->SetFloat("u_SkyLight.Intensity", lightSource->GetIntensity());
				pbrShader->SetMat4("u_SkyLightProjection", lightProjection);

				pbrStaticShader->Enable();
				pbrStaticShader->SetFloat3("u_SkyLight.Radiance", lightSource->GetRadiance());
				pbrStaticShader->SetFloat3("u_SkyLight.Direction", Math::Normalize(transform.GetRotationEuler()));
				pbrStaticShader->SetFloat("u_SkyLight.ShadowBias", lightSource->GetShadowBias() / 1'000.0f);
				pbrStaticShader->SetBool("u_SkyLight.SoftShadows", lightSource->GetSoftShadows());
				pbrStaticShader->SetFloat("u_SkyLight.Intensity", lightSource->GetIntensity());
				pbrStaticShader->SetMat4("u_SkyLightProjection", lightProjection);

				s_Data.SceneLightDesc.HasSkyLight = true;

				break;
			}
			case LightType::Point:
			{
				uint32_t& i = s_Data.SceneLightDesc.PointLightIndex;

				if (i + 1 > RendererInternalData::MaxPointLights)
					break;

				lightSource->SetPointLightIndex(i);

				pbrShader->Enable();
				pbrShader->SetFloat3("u_PointLights[" + std::to_string(i) +"].Radiance", lightSource->GetRadiance());
				pbrShader->SetFloat3("u_PointLights[" + std::to_string(i) +"].Position", transform.Translation);
				pbrShader->SetFloat("u_PointLights[" + std::to_string(i) +"].Intensity", lightSource->GetIntensity());

				pbrStaticShader->Enable();
				pbrStaticShader->SetFloat3("u_PointLights[" + std::to_string(i) + "].Radiance", lightSource->GetRadiance());
				pbrStaticShader->SetFloat3("u_PointLights[" + std::to_string(i) + "].Position", transform.Translation);
				pbrStaticShader->SetFloat("u_PointLights[" + std::to_string(i) + "].Intensity", lightSource->GetIntensity());

				i++;

				break;
			}
			case LightType::Spot:
			{
				uint32_t& i = s_Data.SceneLightDesc.SpotLightIndex;

				if (i + 1 > RendererInternalData::MaxSpotLights)
					break;

				lightSource->SetSpotLightIndex(i);

				pbrShader->Enable();
				pbrShader->SetFloat3("u_SpotLights[" + std::to_string(i) + "].Radiance", lightSource->GetRadiance());
				pbrShader->SetFloat3("u_SpotLights[" + std::to_string(i) + "].Position", transform.Translation);
				pbrShader->SetFloat3("u_SpotLights[" + std::to_string(i) + "].Direction", transform.GetRotationEuler());
				pbrShader->SetFloat("u_SpotLights[" + std::to_string(i) + "].Intensity", lightSource->GetIntensity());
				pbrShader->SetFloat("u_SpotLights[" + std::to_string(i) + "].CutOff", Math::Cos(Math::Deg2Rad(lightSource->GetCutOff())));
				pbrShader->SetFloat("u_SpotLights[" + std::to_string(i) + "].OuterCutOff", Math::Cos(Math::Deg2Rad(lightSource->GetOuterCutOff())));

				pbrStaticShader->Enable();
				pbrStaticShader->SetFloat3("u_SpotLights[" + std::to_string(i) + "].Radiance", lightSource->GetRadiance());
				pbrStaticShader->SetFloat3("u_SpotLights[" + std::to_string(i) + "].Position", transform.Translation);
				pbrStaticShader->SetFloat3("u_SpotLights[" + std::to_string(i) + "].Direction", transform.GetRotationEuler());
				pbrStaticShader->SetFloat("u_SpotLights[" + std::to_string(i) + "].Intensity", lightSource->GetIntensity());
				pbrStaticShader->SetFloat("u_SpotLights[" + std::to_string(i) + "].CutOff", Math::Cos(Math::Deg2Rad(lightSource->GetCutOff())));
				pbrStaticShader->SetFloat("u_SpotLights[" + std::to_string(i) + "].OuterCutOff", Math::Cos(Math::Deg2Rad(lightSource->GetOuterCutOff())));

				i++;

				break;
			}
		}
	}

	void Renderer::DrawEnvironmentMap(const Math::mat4& view, const Math::mat4& projection, SkyboxComponent& skyboxComponent, SharedReference<Skybox>& skybox)
	{
		const bool framebufferNotCreated = s_Data.HDRFramebuffer == nullptr;

		RenderCommand::SetCullMode(RendererAPI::TriangleCullMode::None);

		// TODO fix this hack!
		if (skybox->ShouldReload() || framebufferNotCreated)
		{
			CreateEnvironmentMap(skyboxComponent, skybox);
		}

		// Render Environment Map
		{
			RenderCommand::DisableDepthMask();

			SharedRef<Shader> environmentShader = s_Data.ShaderLibrary.Get("Environment");
			environmentShader->Enable();
			environmentShader->SetMat4("u_View", Math::mat4(Math::mat3(view)));
			environmentShader->SetMat4("u_Projection", projection);
			environmentShader->SetInt("u_EnvironmentMap", 0);
			environmentShader->SetFloat("u_Gamma", s_Data.SceneGamma);
			environmentShader->SetFloat("u_Exposure", s_Data.SceneExposure);
			environmentShader->SetFloat("u_Intensity", Math::Max(skyboxComponent.Intensity, 0.0f));

			SharedRef<VertexArray> skyboxMeshVA = s_Data.SkyboxMesh->GetSubmeshes()[0].GetVertexArray();

			skyboxMeshVA->Bind();
			s_Data.HDRFramebuffer->BindEnvironmentCubemap();
			RenderCommand::DrawTriangles(skyboxMeshVA, 36);
			RenderCommand::EnableDepthMask();
		}

		SharedRef<Shader> pbrShader = s_Data.ShaderLibrary.Get("PBR");
		pbrShader->Enable();
		pbrShader->SetInt("u_SceneProperties.IrradianceMap", 1);
		s_Data.HDRFramebuffer->BindIrradianceCubemap();
		pbrShader->SetInt("u_SceneProperties.PrefilterMap", 2);
		s_Data.HDRFramebuffer->BindPrefilterCubemap();
		pbrShader->SetInt("u_SceneProperties.BRDFLut", 3);
		s_Data.BRDF_LUT->Bind(3);
		pbrShader->SetFloat("u_SceneProperties.SkyboxIntensity", Math::Max(skyboxComponent.Intensity, 0.0f));

		SharedRef<Shader> pbrStaticShader = s_Data.ShaderLibrary.Get("PBR_Static");
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

	void Renderer::DrawFrustumOutline(const TransformComponent& transform, SceneCamera sceneCamera, const Math::vec4& color)
	{
		Math::mat4 cameraOrientation = Math::ToMat4(transform.GetRotation());

		Math::vec3 corners[4] = {
			 transform.Translation + (Math::vec3(transform.Scale.x, transform.Scale.y, 0) * Math::vec3(0.5f)),
			 transform.Translation + (Math::vec3(-transform.Scale.x, transform.Scale.y, 0) * Math::vec3(0.5f)),
			 transform.Translation + (Math::vec3(transform.Scale.x, -transform.Scale.y, 0) * Math::vec3(0.5f)),
			 transform.Translation + (Math::vec3(-transform.Scale.x, -transform.Scale.y, 0) * Math::vec3(0.5f)),
		};

		// Transform the points to the camera's orientation
		for (uint32_t i = 0; i < 4; i++)
		{
			corners[i] = Math::vec4(corners[i], 1.0f) * cameraOrientation;
		}

		// Draw frustum
		Renderer2D::DrawLine(transform.Translation, corners[0], color);
		Renderer2D::DrawLine(transform.Translation, corners[1], color);
		Renderer2D::DrawLine(transform.Translation, corners[2], color);
		Renderer2D::DrawLine(transform.Translation, corners[3], color);
		Renderer2D::DrawLine(corners[0], corners[1], color);
		Renderer2D::DrawLine(corners[1], corners[3], color);
		Renderer2D::DrawLine(corners[3], corners[2], color);
		Renderer2D::DrawLine(corners[2], corners[0], color);
	}

	SceneLightDescription Renderer::GetSceneLightDescription()
	{
		return s_Data.SceneLightDesc;
	}

	void Renderer::CreateEnvironmentMap(SkyboxComponent& skyboxComponent, SharedReference<Skybox>& skybox)
	{
		s_Data.HDRFramebuffer = HDRFramebuffer::Create({});

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
		SharedRef<Shader> equirectToCubemapShader = s_Data.ShaderLibrary.Get("EquirectangularToCubemap");
		equirectToCubemapShader->Enable();
		equirectToCubemapShader->SetInt("u_EquirectangularMap", 0);
		equirectToCubemapShader->SetMat4("u_Projection", captureProjection);
		skybox->Bind();

		SharedRef<VertexArray> cubeMeshVA = s_Data.SkyboxMesh->GetSubmeshes()[0].GetVertexArray();

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

		SharedRef<Shader> irradianceConvolutionShader = s_Data.ShaderLibrary.Get("IrradianceConvolution");
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

		SharedRef<Shader> iblPrefilterShader = s_Data.ShaderLibrary.Get("IBL_Prefilter");
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

		skybox->SetShouldReload(false);
	}

	void Renderer::CreateShadowMap(LightType type, const SharedRef<LightSource>& lightSource)
	{
		switch (type)
		{
			case LightType::Directional:
			{
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
						BlurAndSubmitFinalSceneComposite(postProcessProps.TargetFramebuffer);

					break;
				}
			}
		}
	}

	void Renderer::RenderToDepthMap(SharedReference<Scene>& contextScene)
	{
		const Scene::SceneMeshes& sceneMeshes = contextScene->GetSceneMeshes();

		auto lightSourceView = contextScene->GetAllEntitiesWith<LightSourceComponent>();

		for (const auto& lightSource : lightSourceView)
		{
			Entity lightSourceEntity{ lightSource, contextScene.Raw() };
			LightSourceComponent& lightSourceComponent = lightSourceEntity.GetComponent<LightSourceComponent>();
			
			switch (lightSourceComponent.Type)
			{
				case LightType::Directional:
				{
					if (!lightSourceComponent.Source->GetCastShadows())
					{
						s_Data.SkylightDepthMapFramebuffer.reset();
						continue;
					}

					RenderDirectionalLightShadow(lightSourceComponent, lightSourceEntity, sceneMeshes);

					break;
				}
				case LightType::Point:
				{
					if (!lightSourceComponent.Source->GetCastShadows())
					{
						continue;
					}

					RenderPointLightShadow(lightSourceComponent, lightSourceEntity, sceneMeshes);

					break;
				}
				case LightType::Spot:
				{
					if (!lightSourceComponent.Source->GetCastShadows())
					{
						continue;
					}

					RenderSpotLightShadow(lightSourceComponent, lightSourceEntity, sceneMeshes);

					break;
				}
			}
		}
	}

	const SharedRef<DepthMapFramebuffer>& Renderer::GetSkyLightDepthFramebuffer()
	{
		return s_Data.SkylightDepthMapFramebuffer;
	}

	void Renderer::BindSkyLightDepthMap()
	{
		if (!s_Data.SkylightDepthMapFramebuffer)
			return;

		{
			SharedRef<Shader> pbrShader = s_Data.ShaderLibrary.Get("PBR");
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
			SharedRef<Shader> pbrStaticShader = s_Data.ShaderLibrary.Get("PBR_Static");
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
		/*SharedRef<Shader> pbrShader = s_Data.ShaderLibrary->Get("PBR");
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
		/*SharedRef<Shader> pbrShader = s_Data.ShaderLibrary->Get("PBR");
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

	void Renderer::BindShaders(const Math::mat4& view, const Math::mat4& projection, const Math::vec3& cameraPosition)
	{
		VX_PROFILE_FUNCTION();

		Math::mat4 viewProjection = projection * view;

		SharedRef<Shader> pbrShader = s_Data.ShaderLibrary.Get("PBR");
		pbrShader->Enable();
		pbrShader->SetMat4("u_ViewProjection", viewProjection);
		pbrShader->SetFloat3("u_SceneProperties.CameraPosition", cameraPosition);
		pbrShader->SetFloat("u_SceneProperties.Exposure", s_Data.SceneExposure);
		pbrShader->SetFloat("u_SceneProperties.Gamma", s_Data.SceneGamma);
		pbrShader->SetFloat3("u_SceneProperties.BloomThreshold", s_Data.BloomSettings);

		SharedRef<Shader> pbrStaticShader = s_Data.ShaderLibrary.Get("PBR_Static");
		pbrStaticShader->Enable();
		pbrStaticShader->SetMat4("u_ViewProjection", viewProjection);
		pbrStaticShader->SetFloat3("u_SceneProperties.CameraPosition", cameraPosition);
		pbrStaticShader->SetFloat("u_SceneProperties.Exposure", s_Data.SceneExposure);
		pbrStaticShader->SetFloat("u_SceneProperties.Gamma", s_Data.SceneGamma);
		pbrStaticShader->SetFloat3("u_SceneProperties.BloomThreshold", s_Data.BloomSettings);

		s_Data.SceneLightDesc.HasSkyLight = false;
		s_Data.SceneLightDesc.PointLightIndex = 0;
		s_Data.SceneLightDesc.SpotLightIndex = 0;
	}

	void Renderer::RenderDirectionalLightShadow(const LightSourceComponent& lightSourceComponent, Entity lightSourceEntity, const Scene::SceneMeshes& sceneMeshes)
	{
		SharedRef<Shader> shadowMapShader = s_Data.ShaderLibrary.Get("SkyLightShadowMap");

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

			if (!s_Data.SkylightDepthMapFramebuffer)
			{
				CreateShadowMap(LightType::Directional, lightSourceComponent.Source);
			}

			s_Data.SkylightDepthMapFramebuffer->Bind();

			shadowMapShader->Enable();
			shadowMapShader->SetMat4("u_LightProjection", lightProjection);
			s_Data.SkylightDepthMapFramebuffer->ClearDepth(1.0f);
			s_Data.SkylightDepthMapFramebuffer->ClearDepthAttachment();
		}

		uint32_t i = 0;

		// Render Meshes
		for (const auto& meshHandle : sceneMeshes.Meshes)
		{
			SharedReference<Mesh> mesh = Project::GetEditorAssetManager()->GetAsset(meshHandle).As<Mesh>();

			Math::mat4 worldSpaceTransform = sceneMeshes.WorldSpaceMeshTransforms[i];
			shadowMapShader->SetMat4("u_Model", worldSpaceTransform);

			if (mesh->HasAnimations() && sceneMeshes.MeshEntities[i].HasComponent<AnimatorComponent>())
			{
				shadowMapShader->SetBool("u_HasAnimations", true);

				const AnimatorComponent& animatorComponent = sceneMeshes.MeshEntities[i].GetComponent<AnimatorComponent>();
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

			auto& submesh = mesh->GetSubmesh();

			submesh.RenderToSkylightShadowMap();

			i++;
		}

		i = 0;

		// Render Static Meshes
		for (const auto& staticMeshHandle : sceneMeshes.StaticMeshes)
		{
			SharedReference<StaticMesh> staticMesh = Project::GetEditorAssetManager()->GetAsset(staticMeshHandle).As<StaticMesh>();

			Math::mat4 worldSpaceTransform = sceneMeshes.WorldSpaceStaticMeshTransforms[i++];
			shadowMapShader->SetMat4("u_Model", worldSpaceTransform);

			auto& submeshes = staticMesh->GetSubmeshes();

			for (auto& submesh : submeshes)
			{
				submesh.RenderToSkylightShadowMap();
			}
		}

		s_Data.SkylightDepthMapFramebuffer->Unbind();
		RenderCommand::SetCullMode(s_Data.CullMode);
	}

	void Renderer::RenderPointLightShadow(const LightSourceComponent& lightSourceComponent, Entity lightSourceEntity, const Scene::SceneMeshes& sceneMeshes)
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

		SharedRef<Shader> pbrShader = s_Data.ShaderLibrary->Get("PBR");
		pbrShader->Enable();
		pbrShader->SetFloat("u_PointLights[" + std::to_string(pointLightIndex) + "].ShadowBias", lightSourceComponent.Source->GetShadowBias() / 1'000.0f);
		pbrShader->SetFloat("u_PointLights[" + std::to_string(pointLightIndex) + "].FarPlane", farPlane);

		SharedRef<Shader> shadowMapShader = s_Data.ShaderLibrary->Get("PointLightShadowMap");
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

	void Renderer::RenderSpotLightShadow(const LightSourceComponent& lightSourceComponent, Entity lightSourceEntity, const Scene::SceneMeshes& sceneMeshes)
	{
		/*float aspectRatio = (float)s_Data.ShadowMapResolution / (float)s_Data.ShadowMapResolution;
		float nearPlane = 0.01f;
		float farPlane = 100.0f;
		Math::mat4 perspectiveProjection = Math::Perspective(Math::Deg2Rad(45.0f), aspectRatio, nearPlane, farPlane);
		TransformComponent& transform = lightSourceEntity.GetTransform();

		Math::mat4 view = Math::LookAt(transform.Translation, transform.Translation + Math::Normalize(transform.GetRotationEuler()), { 0.0f, 1.0f, 0.0f });

		uint32_t spotLightIndex = lightSourceComponent.Source->GetSpotLightIndex();

		SharedRef<Shader> pbrShader = s_Data.ShaderLibrary->Get("PBR");
		pbrShader->Enable();
		pbrShader->SetFloat("u_SpotLights[" + std::to_string(spotLightIndex) + "].ShadowBias", lightSourceComponent.Source->GetShadowBias() / 1'000.0f);

		SharedRef<Shader> shadowMapShader = s_Data.ShaderLibrary->Get("SpotLightShadowMap");
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
		if (!s_Data.BlurFramebuffer)
		{
			Viewport viewport = postProcessProps.ViewportSize;
			CreateBlurFramebuffer(viewport.Width, viewport.Height);
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
		FramebufferProperties props{};
		props.Width = width;
		props.Height = height;
		s_Data.BlurFramebuffer = GaussianBlurFramebuffer::Create(props);
	}

	void Renderer::BlurAndSubmitFinalSceneComposite(SharedRef<Framebuffer> sceneFramebuffer)
	{
		if (!s_Data.BlurFramebuffer)
		{
			const FramebufferProperties& props = sceneFramebuffer->GetProperties();
			CreateBlurFramebuffer(props.Width, props.Height);
		}

		bool horizontal = true;
		SharedRef<Shader> blurShader = s_Data.ShaderLibrary.Get("Blur");
		blurShader->Enable();
		blurShader->SetInt("u_Texture", 5);

		for (uint32_t i = 0; i < s_Data.BloomSampleSize; i++)
		{
			s_Data.BlurFramebuffer->Bind(horizontal);
			blurShader->SetBool("u_Horizontal", horizontal);

			s_Data.BlurFramebuffer->BindColorTexture(!horizontal);

			Renderer2D::DrawUnitQuad();
			horizontal = !horizontal;
		}

		s_Data.BlurFramebuffer->Unbind();
		RenderCommand::Clear();

		SharedRef<Shader> bloomFinalCompositeShader = s_Data.ShaderLibrary.Get("BloomFinalComposite");
		bloomFinalCompositeShader->Enable();

		sceneFramebuffer->BindColorTexture(0);
		bloomFinalCompositeShader->SetInt("u_SceneTexture", 0);

		s_Data.BlurFramebuffer->BindColorTexture(!horizontal);
		bloomFinalCompositeShader->SetInt("u_BloomTexture", 1);

		bloomFinalCompositeShader->SetBool("u_Bloom", true);
		bloomFinalCompositeShader->SetFloat("u_Exposure", s_Data.SceneExposure);
		bloomFinalCompositeShader->SetFloat("u_Gamma", s_Data.SceneGamma);

		Renderer2D::DrawUnitQuad();
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
		s_Data.BloomSettings = props.BloomThreshold;
		s_Data.BloomSampleSize = props.BloomSampleSize;

		ClearFlags();
		s_Data.RenderFlags = props.RenderFlags;

		Application::Get().GetWindow().SetVSync(props.UseVSync);

		if (!props.TriangleCullMode.empty())
		{
			s_Data.CullMode = Utils::TriangleCullModeFromString(props.TriangleCullMode);
		}
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
		return s_Data.BloomSettings;
	}

	void Renderer::SetBloomSettings(const Math::vec3& bloomSettings)
	{
		s_Data.BloomSettings = bloomSettings;
	}

	void Renderer::SetBloomThreshold(float threshold)
	{
		s_Data.BloomSettings.x = threshold;
	}

	void Renderer::SetBloomSoftKnee(float softKnee)
	{
		s_Data.BloomSettings.y = softKnee;
	}

	void Renderer::SetBloomUnknown(float unknown)
	{
		s_Data.BloomSettings.z = unknown;
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

	const ShaderLibrary& Renderer::GetShaderLibrary()
	{
		return s_Data.ShaderLibrary;
	}

}
