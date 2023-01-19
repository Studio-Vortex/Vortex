#include "vxpch.h"
#include "Renderer.h"

#include "Vortex/Core/Base.h"
#include "Vortex/Asset/AssetRegistry.h"
#include "Vortex/Core/Application.h"

#include "Vortex/Renderer/Renderer2D.h"
#include "Vortex/Renderer/LightSource.h"
#include "Vortex/Renderer/Model.h"

#include "Vortex/Editor/EditorResources.h"

namespace Vortex {

	static constexpr const char* PBR_SHADER_PATH = "Resources/Shaders/Renderer_PBR.glsl";
	static constexpr const char* EQUIRECTANGULAR_TO_CUBEMAP_SHADER_PATH = "Resources/Shaders/Equirectangular_to_Cubemap.glsl";
	static constexpr const char* IRRADIANCE_CONVOLUTION_SHADER_PATH = "Resources/Shaders/Irradiance_Convolution.glsl";
	static constexpr const char* IBL_PREFILTER_SHADER_PATH = "Resources/Shaders/IBL_Prefilter.glsl";
	static constexpr const char* BRDF_LUT_SHADER_PATH = "Resources/Shaders/Renderer_BRDF_LUT.glsl";
	static constexpr const char* ENVIRONMENT_SHADER_PATH = "Resources/Shaders/Renderer_Environment.glsl";
	static constexpr const char* SKYLIGHT_SHADOW_MAP_SHADER_PATH = "Resources/Shaders/Renderer_SkyLightShadowMap.glsl";
	static constexpr const char* POINT_LIGHT_SHADOW_MAP_SHADER_PATH = "Resources/Shaders/Renderer_PointLightShadowMap.glsl";
	static constexpr const char* SPOT_LIGHT_SHADOW_MAP_SHADER_PATH = "Resources/Shaders/Renderer_SpotLightShadowMap.glsl";
	static constexpr const char* GAUSSIAN_BLUR_SHADER_PATH = "Resources/Shaders/GaussianBlur.glsl";
	static constexpr const char* STENCIL_SHADER_PATH = "Resources/Shaders/Renderer_Stencil.glsl";

	static constexpr const char* BRDF_LUT_TEXTURE_PATH = "Resources/Textures/IBL_BRDF_LUT.tga";

	struct RendererInternalData
	{
		SharedRef<ShaderLibrary> ShaderLibrary = nullptr;

		SharedRef<Model> SkyboxMesh = nullptr;

		static constexpr inline uint32_t MaxPointLights = 50;
		static constexpr inline uint32_t MaxSpotLights = 50;
		
		SceneLightDescription SceneLightDesc{};

		SharedRef<HDRFramebuffer> HDRFramebuffer = nullptr;
		SharedRef<DepthMapFramebuffer> SkylightDepthMapFramebuffer = nullptr;
		std::vector<SharedRef<DepthCubemapFramebuffer>> PointLightDepthMapFramebuffers;
		std::vector<SharedRef<DepthMapFramebuffer>> SpotLightDepthMapFramebuffers;

		SharedRef<Framebuffer> GaussianBlurFramebuffers[2] = { nullptr, nullptr };

		float EnvironmentMapResolution = 512.0f;
		float PrefilterMapResolution = 128.0f;
		float ShadowMapResolution = 1024.0f;
		float SceneExposure = 1.0f;
		float SceneGamma = 2.2f;

		RenderStatistics RendererStatistics;
		RenderTime RenderTime;
		RendererAPI::TriangleCullMode CullMode = RendererAPI::TriangleCullMode::None;

		SharedRef<Texture2D> BRDF_LUT = nullptr;
	};

	static RendererInternalData s_Data;

	void Renderer::Init()
	{
		VX_PROFILE_FUNCTION();

		RenderCommand::Init();

		s_Data.ShaderLibrary = ShaderLibrary::Create();
		s_Data.ShaderLibrary->Load("PBR", PBR_SHADER_PATH);
		s_Data.ShaderLibrary->Load("EquirectangularToCubemap", EQUIRECTANGULAR_TO_CUBEMAP_SHADER_PATH);
		s_Data.ShaderLibrary->Load("IrradianceConvolution", IRRADIANCE_CONVOLUTION_SHADER_PATH);
		s_Data.ShaderLibrary->Load("IBL_Prefilter", IBL_PREFILTER_SHADER_PATH);
		s_Data.ShaderLibrary->Load("BRDF_LUT", BRDF_LUT_SHADER_PATH);
		s_Data.ShaderLibrary->Load("Environment", ENVIRONMENT_SHADER_PATH);
		s_Data.ShaderLibrary->Load("SkyLightShadowMap", SKYLIGHT_SHADOW_MAP_SHADER_PATH);
		s_Data.ShaderLibrary->Load("PointLightShadowMap", POINT_LIGHT_SHADOW_MAP_SHADER_PATH);
		s_Data.ShaderLibrary->Load("SpotLightShadowMap", SPOT_LIGHT_SHADOW_MAP_SHADER_PATH);
		s_Data.ShaderLibrary->Load("Blur", GAUSSIAN_BLUR_SHADER_PATH);
		s_Data.ShaderLibrary->Load("Stencil", STENCIL_SHADER_PATH);

		s_Data.BRDF_LUT = Texture2D::Create(BRDF_LUT_TEXTURE_PATH, TextureWrap::Clamp);

		s_Data.SkyboxMesh = Model::Create(MeshType::Cube);

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

	void Renderer::BeginScene(const Camera& camera, const TransformComponent& transform)
	{
		VX_PROFILE_FUNCTION();

		BindShaders(Math::Inverse(transform.GetTransform()), camera.GetProjectionMatrix(), transform.Translation);
	}

	void Renderer::BeginScene(const EditorCamera* camera)
	{
		VX_PROFILE_FUNCTION();

		BindShaders(camera->GetViewMatrix(), camera->GetProjectionMatrix(), camera->GetPosition());
	}

	void Renderer::EndScene()
	{
	}

	void Renderer::BindShaders(const Math::mat4& view, const Math::mat4& projection, const Math::vec3& cameraPosition)
	{
		VX_PROFILE_FUNCTION();

		Math::mat4 viewProjection = projection * view;

		SharedRef<Shader> pbrShader = s_Data.ShaderLibrary->Get("PBR");
		pbrShader->Enable();
		pbrShader->SetMat4("u_ViewProjection", viewProjection);
		pbrShader->SetFloat3("u_SceneProperties.CameraPosition", cameraPosition);
		pbrShader->SetFloat("u_SceneProperties.Exposure", s_Data.SceneExposure);
		pbrShader->SetFloat("u_SceneProperties.Gamma", s_Data.SceneGamma);

		s_Data.SceneLightDesc.HasSkyLight = false;
		s_Data.SceneLightDesc.PointLightIndex = 0;
		s_Data.SceneLightDesc.SpotLightIndex = 0;
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

	void Renderer::RenderCameraIcon(const TransformComponent& transform, const Math::mat4& cameraView, int entityID)
	{
		Renderer2D::DrawQuadBillboard(cameraView, transform.Translation, EditorResources::CameraIcon, Math::vec2(1.0f), ColorToVec4(Color::White), entityID);
	}

	void Renderer::RenderLightSourceIcon(const TransformComponent& transform, const LightSourceComponent& lightSource, const Math::mat4& cameraView, int entityID)
	{
		switch (lightSource.Type)
		{
			case LightType::Directional:
				Renderer2D::DrawQuadBillboard(cameraView, transform.Translation, EditorResources::SkyLightIcon, Math::vec2(1.0f), ColorToVec4(Color::White), entityID);
				break;
			case LightType::Point:
				Renderer2D::DrawQuadBillboard(cameraView, transform.Translation, EditorResources::PointLightIcon, Math::vec2(1.0f), ColorToVec4(Color::White), entityID);
				break;
			case LightType::Spot:
				Renderer2D::DrawQuadBillboard(cameraView, transform.Translation, EditorResources::SpotLightIcon, Math::vec2(1.0f), ColorToVec4(Color::White), entityID);
				break;
		}
	}

	void Renderer::RenderAudioSourceIcon(const TransformComponent& transform, const Math::mat4& cameraView, int entityID)
	{
		Renderer2D::DrawQuadBillboard(cameraView, transform.Translation, EditorResources::AudioSourceIcon, Math::vec2(1.0f), ColorToVec4(Color::White), entityID);
	}

	void Renderer::RenderLightSource(const TransformComponent& transform, const LightSourceComponent& lightSourceComponent)
	{
		SharedRef<LightSource> lightSource = lightSourceComponent.Source;
		SharedRef<Shader> pbrShader = s_Data.ShaderLibrary->Get("PBR");

		switch (lightSourceComponent.Type)
		{
			case LightType::Directional:
			{
				pbrShader->Enable();
				pbrShader->SetFloat3("u_SkyLight.Radiance", lightSource->GetRadiance());
				pbrShader->SetFloat3("u_SkyLight.Direction", Math::Normalize(transform.GetRotationEuler()));
				pbrShader->SetFloat("u_SkyLight.ShadowBias", lightSource->GetShadowBias() / 1'000.0f);
				pbrShader->SetBool("u_SkyLight.SoftShadows", lightSource->GetSoftShadows());
				pbrShader->SetFloat("u_SkyLight.Intensity", lightSource->GetIntensity());
				s_Data.SceneLightDesc.HasSkyLight = true;

				Math::mat4 orthogonalProjection = Math::Ortho(-50.0f, 50.0f, -50.0f, 50.0f, 0.01f, 500.0f);
				Math::mat4 lightView = Math::LookAt(transform.Translation, Math::Normalize(transform.GetRotationEuler()), Math::vec3(0.0f, 1.0f, 0.0f));
				Math::mat4 lightProjection = orthogonalProjection * lightView;
				pbrShader->SetMat4("u_SkyLightProjection", lightProjection);

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
				pbrShader->SetFloat3("u_SpotLights[" + std::to_string(i) + "].Direction", Math::Normalize(transform.GetRotationEuler()));
				pbrShader->SetFloat("u_SpotLights[" + std::to_string(i) + "].Intensity", lightSource->GetIntensity());
				pbrShader->SetFloat("u_SpotLights[" + std::to_string(i) + "].CutOff", Math::Cos(Math::Deg2Rad(lightSource->GetCutOff())));
				pbrShader->SetFloat("u_SpotLights[" + std::to_string(i) + "].OuterCutOff", Math::Cos(Math::Deg2Rad(lightSource->GetOuterCutOff())));

				i++;

				break;
			}
		}
	}

	void Renderer::DrawEnvironmentMap(const Math::mat4& view, const Math::mat4& projection, SkyboxComponent& skyboxComponent)
	{
		SharedRef<Skybox> skybox = skyboxComponent.Source;

		bool framebufferNotCreated = s_Data.HDRFramebuffer == nullptr;

		RenderCommand::SetCullMode(RendererAPI::TriangleCullMode::None);

		// TODO fix this hack!
		if (skybox->PathChanged() || framebufferNotCreated)
		{
			CreateEnvironmentMap(skyboxComponent);
		}

		// Render Environment Map
		{
			RenderCommand::DisableDepthMask();

			SharedRef<Shader> environmentShader = s_Data.ShaderLibrary->Get("Environment");
			environmentShader->Enable();
			environmentShader->SetMat4("u_View", Math::mat4(Math::mat3(view)));
			environmentShader->SetMat4("u_Projection", projection);
			environmentShader->SetInt("u_EnvironmentMap", 0);
			environmentShader->SetFloat("u_Gamma", s_Data.SceneGamma);
			environmentShader->SetFloat("u_Exposure", s_Data.SceneExposure);
			environmentShader->SetFloat("u_Intensity", Math::Max(skyboxComponent.Intensity, 0.0f));

			SharedRef<VertexArray> skyboxMeshVA = s_Data.SkyboxMesh->GetVertexArray();

			skyboxMeshVA->Bind();
			s_Data.HDRFramebuffer->BindEnvironmentCubemap();
			RenderCommand::DrawTriangles(skyboxMeshVA, 36);
			RenderCommand::EnableDepthMask();
		}

		SharedRef<Shader> pbrShader = s_Data.ShaderLibrary->Get("PBR");
		pbrShader->Enable();
		pbrShader->SetInt("u_SceneProperties.IrradianceMap", 1);
		s_Data.HDRFramebuffer->BindIrradianceCubemap();
		pbrShader->SetInt("u_SceneProperties.PrefilterMap", 2);
		s_Data.HDRFramebuffer->BindPrefilterCubemap();
		pbrShader->SetInt("u_SceneProperties.BRDFLut", 3);
		s_Data.BRDF_LUT->Bind(3);
		pbrShader->SetFloat("u_SceneProperties.SkyboxIntensity", Math::Max(skyboxComponent.Intensity, 0.0f));

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

	void Renderer::CreateGaussianBlurFramebuffers(const Math::vec2& viewportSize)
	{
		FramebufferProperties framebufferProps{};
		framebufferProps.Attachments = { FramebufferTextureFormat::RGBA8 };
		framebufferProps.Width = viewportSize.x;
		framebufferProps.Height = viewportSize.y;

		for (uint32_t i = 0; i < 2; i++)
		{
			s_Data.GaussianBlurFramebuffers[i] = Framebuffer::Create(framebufferProps);
		}
	}

	void Renderer::CreateEnvironmentMap(SkyboxComponent& skyboxComponent)
	{
		SharedRef<Skybox> skybox = skyboxComponent.Source;

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
		SharedRef<Shader> equirectToCubemapShader = s_Data.ShaderLibrary->Get("EquirectangularToCubemap");
		equirectToCubemapShader->Enable();
		equirectToCubemapShader->SetInt("u_EquirectangularMap", 0);
		equirectToCubemapShader->SetMat4("u_Projection", captureProjection);
		skybox->Bind();

		SharedRef<VertexArray> cubeMeshVA = s_Data.SkyboxMesh->GetVertexArray();

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

		SharedRef<Shader> irradianceConvolutionShader = s_Data.ShaderLibrary->Get("IrradianceConvolution");
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

		SharedRef<Shader> iblPrefilterShader = s_Data.ShaderLibrary->Get("IBL_Prefilter");
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
			uint32_t mipWidth = static_cast<uint32_t>(s_Data.PrefilterMapResolution * std::pow(0.5, mip));
			uint32_t mipHeight = static_cast<uint32_t>(s_Data.PrefilterMapResolution * std::pow(0.5, mip));
			s_Data.HDRFramebuffer->BindAndSetRenderbufferStorage(mipWidth, mipHeight);

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

		skybox->SetPathChanged(false);
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
				FramebufferProperties depthCubemapProps{};
				depthCubemapProps.Width = s_Data.ShadowMapResolution;
				depthCubemapProps.Height = s_Data.ShadowMapResolution;
				SharedRef<DepthCubemapFramebuffer> framebuffer = DepthCubemapFramebuffer::Create(depthCubemapProps);
				s_Data.PointLightDepthMapFramebuffers.push_back(framebuffer);
				s_Data.SceneLightDesc.ActivePointLights++;

				break;
			}
			case LightType::Spot:
			{
				FramebufferProperties depthFramebufferProps{};
				depthFramebufferProps.Width = s_Data.ShadowMapResolution;
				depthFramebufferProps.Height = s_Data.ShadowMapResolution;
				SharedRef<DepthMapFramebuffer> framebuffer = DepthMapFramebuffer::Create(depthFramebufferProps);
				s_Data.SpotLightDepthMapFramebuffers.push_back(framebuffer);
				s_Data.SceneLightDesc.ActiveSpotLights++;

				break;
			}
		}
	}

	void Renderer::RenderToDepthMap(Scene* contextScene)
	{
		auto meshRendererView = contextScene->GetAllEntitiesWith<MeshRendererComponent>();
		auto lightSourceView = contextScene->GetAllEntitiesWith<LightSourceComponent>();

		for (auto& lightSource : lightSourceView)
		{
			Entity lightSourceEntity{ lightSource, contextScene };
			LightSourceComponent& lightSourceComponent = lightSourceEntity.GetComponent<LightSourceComponent>();
			
			switch (lightSourceComponent.Type)
			{
				case LightType::Directional:
				{
					if (!lightSourceComponent.Source->GetCastShadows())
						continue;

					// Configure shader
					Math::mat4 orthogonalProjection = Math::Ortho(-50.0f, 50.0f, -50.0f, 50.0f, 0.01f, 500.0f);
					TransformComponent& transform = lightSourceEntity.GetTransform();
					Math::mat4 lightView = Math::LookAt(transform.Translation, Math::Normalize(transform.GetRotationEuler()), Math::vec3(0.0f, 1.0f, 0.0f));
					Math::mat4 lightProjection = orthogonalProjection * lightView;
					SharedRef<Shader> shadowMapShader = s_Data.ShaderLibrary->Get("SkyLightShadowMap");

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
							meshRendererComponent.Mesh->RenderToSkylightShadowMap(worldSpaceTransform, meshRendererEntity.GetComponent<AnimatorComponent>());
							continue;
						}

						meshRendererComponent.Mesh->RenderToSkylightShadowMap(worldSpaceTransform);
					}

					s_Data.SkylightDepthMapFramebuffer->Unbind();
					RenderCommand::SetCullMode(s_Data.CullMode);

					break;
				}
				case LightType::Point:
				{
					if (!lightSourceComponent.Source->GetCastShadows())
						continue;

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

					break;
				}
				case LightType::Spot:
				{
					if (!lightSourceComponent.Source->GetCastShadows())
						continue;

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
		s_Data.SkylightDepthMapFramebuffer->BindDepthTexture(4);
		SharedRef<Shader> pbrShader = s_Data.ShaderLibrary->Get("PBR");
		pbrShader->Enable();
		pbrShader->SetInt("u_SkyLight.ShadowMap", 4);
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
		Application::Get().GetWindow().SetVSync(props.UseVSync);

		if (!props.TriangleCullMode.empty())
			s_Data.CullMode = Utils::TriangleCullModeFromString(props.TriangleCullMode);
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

	SharedRef<ShaderLibrary> Renderer::GetShaderLibrary()
	{
		return s_Data.ShaderLibrary;
	}

}
