#include "vxpch.h"
#include "Renderer.h"

#include "Vortex/Core/Base.h"
#include "Vortex/Project/Project.h"
#include "Vortex/Asset/AssetRegistry.h"
#include "Vortex/Core/Application.h"

#include "Vortex/Renderer/Renderer2D.h"
#include "Vortex/Renderer/LightSource.h"
#include "Vortex/Renderer/Model.h"

namespace Vortex {

	static constexpr const char* PBR_SHADER_PATH = "Resources/Shaders/Renderer_PBR.glsl";
	static constexpr const char* EQUIRECTANGULAR_TO_CUBEMAP_SHADER_PATH = "Resources/Shaders/Equirectangular_to_Cubemap.glsl";
	static constexpr const char* IRRADIANCE_CONVOLUTION_SHADER_PATH = "Resources/Shaders/Irradiance_Convolution.glsl";
	static constexpr const char* IBL_PREFILTER_SHADER_PATH = "Resources/Shaders/IBL_Prefilter.glsl";
	static constexpr const char* BRDF_LUT_SHADER_PATH = "Resources/Shaders/BRDF_LUT.glsl";
	static constexpr const char* SKYBOX_SHADER_PATH = "Resources/Shaders/Renderer_Skybox.glsl";
	static constexpr const char* SHADOW_MAP_SHADER_PATH = "Resources/Shaders/Renderer_ShadowMap.glsl";
	static constexpr const char* STENCIL_SHADER_PATH = "Resources/Shaders/Renderer_Stencil.glsl";

	static constexpr const char* CAMERA_ICON_PATH = "Resources/Icons/Scene/CameraIcon.png";
	static constexpr const char* DIR_LIGHT_ICON_PATH = "Resources/Icons/Scene/DirLightIcon.png";
	static constexpr const char* POINT_LIGHT_ICON_PATH = "Resources/Icons/Scene/PointLight.png";
	static constexpr const char* SPOT_LIGHT_ICON_PATH = "Resources/Icons/Scene/SpotLight.png";
	static constexpr const char* AUDIO_SOURCE_ICON_PATH = "Resources/Icons/Scene/AudioSourceIcon.png";

	struct RendererInternalData
	{
		SharedRef<ShaderLibrary> ShaderLibrary = nullptr;

		SharedRef<Model> SkyboxMesh = nullptr;

		static constexpr inline uint32_t MaxDirectionalLights = 1;
		static constexpr inline uint32_t MaxPointLights = 100;
		static constexpr inline uint32_t MaxSpotLights = 50;

		static constexpr inline uint32_t MaxShadowWidth = 4096;
		static constexpr inline uint32_t MaxShadowHeight = 4096;
		
		SceneLightDescription SceneLightDesc;

		SharedRef<HDRFramebuffer> HDRFramebuffer = nullptr;
		SharedRef<DepthMapFramebuffer> DepthMapFramebuffer = nullptr;

		float SceneExposure = 1.0f;
		float SceneGamma = 2.2f;

		RenderStatistics RendererStatistics;
		RenderTime RenderTime;
		RendererAPI::TriangleCullMode CullMode = RendererAPI::TriangleCullMode::None;

		// Editor Resources
		SharedRef<Texture2D> CameraIcon = nullptr;
		SharedRef<Texture2D> PointLightIcon = nullptr;
		SharedRef<Texture2D> SpotLightIcon = nullptr;
		SharedRef<Texture2D> DirLightIcon = nullptr;
		SharedRef<Texture2D> AudioSourceIcon = nullptr;
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
		s_Data.ShaderLibrary->Load("Skybox", SKYBOX_SHADER_PATH);
		s_Data.ShaderLibrary->Load("ShadowMap", SHADOW_MAP_SHADER_PATH);
		s_Data.ShaderLibrary->Load("Stencil", STENCIL_SHADER_PATH);

		s_Data.SkyboxMesh = Model::Create(MeshType::Cube);

		s_Data.CameraIcon = Texture2D::Create(CAMERA_ICON_PATH);
		s_Data.PointLightIcon = Texture2D::Create(POINT_LIGHT_ICON_PATH);
		s_Data.SpotLightIcon = Texture2D::Create(SPOT_LIGHT_ICON_PATH);
		s_Data.DirLightIcon = Texture2D::Create(DIR_LIGHT_ICON_PATH);
		s_Data.AudioSourceIcon = Texture2D::Create(AUDIO_SOURCE_ICON_PATH);

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

		BindShaders(Math::Inverse(transform.GetTransform()), camera.GetProjection(), transform.Translation);
	}

	void Renderer::BeginScene(const EditorCamera& camera)
	{
		VX_PROFILE_FUNCTION();

		BindShaders(camera.GetViewMatrix(), camera.GetProjection(), camera.GetPosition());
	}

	void Renderer::EndScene() { }

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

		s_Data.SceneLightDesc.ActiveDirLights = 0;
		s_Data.SceneLightDesc.ActivePointLights = 0;
		s_Data.SceneLightDesc.ActiveSpotLights = 0;
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
		Renderer2D::DrawQuadBillboard(cameraView, transform.Translation, s_Data.CameraIcon, Math::vec2(1.0f), ColorToVec4(Color::White), entityID);
	}

	void Renderer::RenderLightSourceIcon(const TransformComponent& transform, const LightSourceComponent& lightSource, const Math::mat4& cameraView, int entityID)
	{
		switch (lightSource.Type)
		{
			case LightSourceComponent::LightType::Directional:
				Renderer2D::DrawQuadBillboard(cameraView, transform.Translation, s_Data.DirLightIcon, Math::vec2(1.0f), ColorToVec4(Color::White), entityID);
				break;
			case LightSourceComponent::LightType::Point:
				Renderer2D::DrawQuadBillboard(cameraView, transform.Translation, s_Data.PointLightIcon, Math::vec2(1.0f), ColorToVec4(Color::White), entityID);
				break;
			case LightSourceComponent::LightType::Spot:
				Renderer2D::DrawQuadBillboard(cameraView, transform.Translation, s_Data.SpotLightIcon, Math::vec2(1.0f), ColorToVec4(Color::White), entityID);
				break;
		}
	}

	void Renderer::RenderAudioSourceIcon(const TransformComponent& transform, const Math::mat4& cameraView, int entityID)
	{
		Renderer2D::DrawQuadBillboard(cameraView, transform.Translation, s_Data.AudioSourceIcon, Math::vec2(1.0f), ColorToVec4(Color::White), entityID);
	}

	void Renderer::RenderLightSource(const TransformComponent& transform, const LightSourceComponent& lightSourceComponent)
	{
		SharedRef<LightSource> lightSource = lightSourceComponent.Source;
		SharedRef<Shader> pbrShader = s_Data.ShaderLibrary->Get("PBR");

		switch (lightSourceComponent.Type)
		{
			case LightSourceComponent::LightType::Directional:
			{
				uint32_t& i = s_Data.SceneLightDesc.ActiveDirLights;

				if (i + 1 > RendererInternalData::MaxDirectionalLights)
					break;

				pbrShader->Enable();
				pbrShader->SetFloat3(std::format("u_DirectionalLights[{}].Radiance", i).c_str(), lightSource->GetRadiance());
				pbrShader->SetFloat3(std::format("u_DirectionalLights[{}].Direction", i).c_str(), Math::Normalize(transform.GetRotationEuler()));

				Math::mat4 orthogonalProjection = Math::Ortho(-35.0f, 35.0f, -35.0f, 35.0f, 0.01f, 100.0f);
				Math::mat4 lightView = Math::LookAt(transform.Translation, Math::Normalize(transform.GetRotationEuler()), Math::vec3(0.0f, 1.0f, 0.0f));
				Math::mat4 lightProjection = orthogonalProjection * lightView;
				pbrShader->SetMat4("u_LightProjection", lightProjection);
				pbrShader->SetFloat("u_SkylightShadowSettings.ShadowBias", lightSource->GetShadowBias() / 1'000.0f);

				i++;

				break;
			}
			case LightSourceComponent::LightType::Point:
			{
				uint32_t& i = s_Data.SceneLightDesc.ActivePointLights;

				if (i + 1 > RendererInternalData::MaxPointLights)
					break;

				pbrShader->Enable();
				pbrShader->SetFloat3(std::format("u_PointLights[{}].Radiance", i).c_str(), lightSource->GetRadiance());
				pbrShader->SetFloat3(std::format("u_PointLights[{}].Position", i).c_str(), transform.Translation);

				Math::vec2 attenuation = lightSource->GetAttenuation();
				pbrShader->SetFloat(std::format("u_PointLights[{}].Constant", i).c_str(), 1.0f);
				pbrShader->SetFloat(std::format("u_PointLights[{}].Linear", i).c_str(), attenuation.x);
				pbrShader->SetFloat(std::format("u_PointLights[{}].Quadratic", i).c_str(), attenuation.y);

				i++;

				break;
			}
			case LightSourceComponent::LightType::Spot:
			{
				uint32_t& i = s_Data.SceneLightDesc.ActiveSpotLights;

				if (i + 1 > RendererInternalData::MaxSpotLights)
					break;

				pbrShader->Enable();
				pbrShader->SetFloat3(std::format("u_SpotLights[{}].Radiance", i).c_str(), lightSource->GetRadiance());
				pbrShader->SetFloat3(std::format("u_SpotLights[{}].Position", i).c_str(), transform.Translation);
				pbrShader->SetFloat3(std::format("u_SpotLights[{}].Direction", i).c_str(), Math::Normalize(transform.GetRotationEuler()));
				pbrShader->SetFloat(std::format("u_SpotLights[{}].CutOff", i).c_str(), Math::Cos(Math::Deg2Rad(lightSource->GetCutOff())));
				pbrShader->SetFloat(std::format("u_SpotLights[{}].OuterCutOff", i).c_str(), Math::Cos(Math::Deg2Rad(lightSource->GetOuterCutOff())));

				Math::vec2 attenuation = lightSource->GetAttenuation();
				pbrShader->SetFloat(std::format("u_SpotLights[{}].Constant", i).c_str(), 1.0f);
				pbrShader->SetFloat(std::format("u_SpotLights[{}].Linear", i).c_str(), attenuation.x);
				pbrShader->SetFloat(std::format("u_SpotLights[{}].Quadratic", i).c_str(), attenuation.y);

				i++;

				break;
			}
		}
	}

	void Renderer::DrawSkybox(const Math::mat4& view, const Math::mat4& projection, const SkyboxComponent& skyboxComponent)
	{
		SharedRef<Skybox> skybox = skyboxComponent.Source;

		bool framebufferNotCreated = s_Data.HDRFramebuffer == nullptr;

		// TODO fix this hack!
		if (skybox->PathChanged() || framebufferNotCreated)
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

			s_Data.HDRFramebuffer->CreateEnvironmentCubemap();

			// convert HDR equirectangular environment map to cubemap equivalent
			SharedRef<Shader> equirectToCubemapShader = s_Data.ShaderLibrary->Get("EquirectangularToCubemap");
			equirectToCubemapShader->Enable();
			equirectToCubemapShader->SetInt("u_EquirectangularMap", 0);
			equirectToCubemapShader->SetMat4("u_Projection", captureProjection);
			skybox->Bind();

			SharedRef<VertexArray> cubeMeshVA = s_Data.SkyboxMesh->GetVertexArray();

			// don't forget to configure the viewport to the capture dimensions.
			RenderCommand::SetViewport(Viewport{ 0, 0, 4096, 4096 });
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

			// Create Irradiance Map
			s_Data.HDRFramebuffer->CreateIrradianceCubemap();
			s_Data.HDRFramebuffer->RescaleAndBindFramebuffer(32, 32);

			SharedRef<Shader> irradianceConvolutionShader = s_Data.ShaderLibrary->Get("IrradianceConvolution");
			s_Data.HDRFramebuffer->BindEnvironmentCubemap();
			irradianceConvolutionShader->Enable();
			irradianceConvolutionShader->SetInt("u_EnvironmentMap", 0);
			irradianceConvolutionShader->SetMat4("u_Projection", captureProjection);

			RenderCommand::SetViewport(Viewport{ 0, 0, 32, 32 }); // don't forget to configure the viewport to the capture dimensions.
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
			s_Data.HDRFramebuffer->CreatePrefilteredEnvironmentCubemap();

			s_Data.HDRFramebuffer->BindEnvironmentCubemap();
			SharedRef<Shader> iblPrefilterShader = s_Data.ShaderLibrary->Get("IBL_Prefilter");
			iblPrefilterShader->Enable();
			iblPrefilterShader->SetInt("u_EnvironmentMap", 0);
			iblPrefilterShader->SetMat4("u_Projection", captureProjection);

			uint32_t maxMipLevels = 5;
			s_Data.HDRFramebuffer->Bind();
			for (uint32_t mip = 0; mip < maxMipLevels; mip++)
			{
				// Resize framebuffer according to mip-level size
				uint32_t mipWidth = static_cast<uint32_t>(128 * std::pow(0.5, mip));
				uint32_t mipHeight = static_cast<uint32_t>(128 * std::pow(0.5, mip));
				s_Data.HDRFramebuffer->BindAndSetRenderbufferStorage(mipWidth, mipHeight);
				RenderCommand::SetViewport(Viewport{ 0, 0, mipWidth, mipHeight });

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

			// Create BRDF Look-Up Texture
			s_Data.HDRFramebuffer->CreateBRDFLutTexture();
			s_Data.HDRFramebuffer->RescaleAndBindFramebuffer(4096, 4096);
			s_Data.HDRFramebuffer->SetBRDFLutFramebufferTexture();

			RenderCommand::SetViewport(Viewport{ 0, 0, 4096, 4096 });
			SharedRef<Shader> brdfLutShader = s_Data.ShaderLibrary->Get("BRDF_LUT");
			brdfLutShader->Enable();
			s_Data.HDRFramebuffer->ClearColorAndDepthAttachments();
			Renderer2D::DrawUnitQuadAtOrigin();

			s_Data.HDRFramebuffer->Unbind();

			skybox->SetPathChanged(false);
		}

		// Render Environment Map
		RenderCommand::DisableDepthMask();

		SharedRef<Shader> skyboxShader = s_Data.ShaderLibrary->Get("Skybox");
		skyboxShader->Enable();
		skyboxShader->SetMat4("u_View", Math::mat4(Math::mat3(view)) * Math::Rotate(Math::Deg2Rad(skyboxComponent.Rotation), { 0.0f, 1.0f, 0.0f }));
		skyboxShader->SetMat4("u_Projection", projection);
		skyboxShader->SetInt("u_EnvironmentMap", 0);
		skyboxShader->SetFloat("u_Gamma", s_Data.SceneGamma);
		skyboxShader->SetFloat("u_Exposure", s_Data.SceneExposure);
		skyboxShader->SetFloat("u_Intensity", Math::Max(skyboxComponent.Intensity, 0.0f));

		SharedRef<VertexArray> skyboxMeshVA = s_Data.SkyboxMesh->GetVertexArray();

		skyboxMeshVA->Bind();
		skybox->Bind();
		RenderCommand::DrawTriangles(skyboxMeshVA, 36);
		RenderCommand::EnableDepthMask();

		SharedRef<Shader> pbrShader = s_Data.ShaderLibrary->Get("PBR");
		pbrShader->Enable();
		pbrShader->SetInt("u_SceneProperties.IrradianceMap", 1);
		pbrShader->SetInt("u_SceneProperties.PrefilterMap", 2);
		pbrShader->SetInt("u_SceneProperties.BRDFLut", 3);

		s_Data.HDRFramebuffer->BindIrradianceCubemap();
		s_Data.HDRFramebuffer->BindPrefilterCubemap();
		s_Data.HDRFramebuffer->BindBRDFLutTexture();
	}

	void Renderer::DrawFrustum(const TransformComponent& transform, SceneCamera sceneCamera, const Math::vec4& color)
	{
		Math::vec3 rotation = transform.GetRotationEuler();
		Math::vec3 forwardDirection = Math::Rotate(Math::GetOrientation(rotation.x, rotation.y, rotation.z), { 0.0f, 0.0f, -1.0f });

		Math::vec3 corners[4] = {
			transform.Translation + forwardDirection + (Math::vec3(transform.Scale.x, transform.Scale.y, 0) * Math::vec3(0.5f)),
			transform.Translation + forwardDirection + (Math::vec3(-transform.Scale.x, transform.Scale.y, 0) * Math::vec3(0.5f)),
			transform.Translation + forwardDirection + (Math::vec3(transform.Scale.x, -transform.Scale.y, 0) * Math::vec3(0.5f)),
			transform.Translation + forwardDirection + (Math::vec3(-transform.Scale.x, -transform.Scale.y, 0) * Math::vec3(0.5f)),
		};

		Renderer2D::DrawLine(transform.Translation, corners[0], color);
		Renderer2D::DrawLine(transform.Translation, corners[1], color);
		Renderer2D::DrawLine(transform.Translation, corners[2], color);
		Renderer2D::DrawLine(transform.Translation, corners[3], color);
		Renderer2D::DrawRect(transform.GetTransform() * Math::Translate(forwardDirection), color);
	}

	SceneLightDescription Renderer::GetSceneLightDescription()
	{
		return s_Data.SceneLightDesc;
	}

	void Renderer::CreateSkyLightShadowMapFramebuffer()
	{
		FramebufferProperties depthFramebufferProps;
		depthFramebufferProps.Width = s_Data.MaxShadowWidth;
		depthFramebufferProps.Height = s_Data.MaxShadowHeight;
		s_Data.DepthMapFramebuffer = DepthMapFramebuffer::Create(depthFramebufferProps);
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
				case LightSourceComponent::LightType::Directional:
				{
					if (!lightSourceComponent.Source->ShouldCastShadows())
						continue;

					Math::mat4 orthogonalProjection = Math::Ortho(-35.0f, 35.0f, -35.0f, 35.0f, 0.01f, 100.0f);
					TransformComponent& transform = lightSourceEntity.GetTransform();
					Math::mat4 lightView = Math::LookAt(transform.Translation, Math::Normalize(transform.GetRotationEuler()), Math::vec3(0.0f, 1.0f, 0.0f));
					Math::mat4 lightProjection = orthogonalProjection * lightView;
					SharedRef<Shader> shadowMapShader = s_Data.ShaderLibrary->Get("ShadowMap");

					RenderCommand::SetCullMode(RendererAPI::TriangleCullMode::Front);
					RenderCommand::SetViewport(Viewport{ 0, 0, s_Data.MaxShadowWidth, s_Data.MaxShadowHeight });
					s_Data.DepthMapFramebuffer->Bind();
					shadowMapShader->Enable();
					shadowMapShader->SetMat4("u_LightProjection", lightProjection);
					s_Data.DepthMapFramebuffer->ClearDepth(1.0f);
					s_Data.DepthMapFramebuffer->ClearDepthAttachment();

					for (auto& meshRenderer : meshRendererView)
					{
						Entity meshRendererEntity{ meshRenderer, contextScene };

						// Skip rendering to depth map if not active
						if (!meshRendererEntity.IsActive())
						{
							continue;
						}

						MeshRendererComponent& meshRendererComponent = meshRendererEntity.GetComponent<MeshRendererComponent>();
						Math::mat4 worldSpaceTransform = contextScene->GetWorldSpaceTransformMatrix(meshRendererEntity);
						shadowMapShader->SetMat4("u_Model", worldSpaceTransform);

						if (meshRendererComponent.Mesh->HasAnimations() && meshRendererEntity.HasComponent<AnimatorComponent>())
						{
							meshRendererComponent.Mesh->RenderForShadowMap(worldSpaceTransform, meshRendererEntity.GetComponent<AnimatorComponent>());
							continue;
						}

						meshRendererComponent.Mesh->RenderForShadowMap(worldSpaceTransform);
					}

					s_Data.DepthMapFramebuffer->Unbind();
					RenderCommand::SetCullMode(s_Data.CullMode);

					break;
				}
				case LightSourceComponent::LightType::Point:
				{
					if (!lightSourceComponent.Source->ShouldCastShadows())
						continue;

					break;
				}
				case LightSourceComponent::LightType::Spot:
				{
					if (!lightSourceComponent.Source->ShouldCastShadows())
						continue;

					break;
				}
			}
		}
	}

	const SharedRef<DepthMapFramebuffer>& Renderer::GetDepthMapFramebuffer()
	{
		return s_Data.DepthMapFramebuffer;
	}

	void Renderer::BindDepthMap()
	{
		s_Data.DepthMapFramebuffer->BindDepthTexture(4);
		SharedRef<Shader> pbrShader = s_Data.ShaderLibrary->Get("PBR");
		pbrShader->Enable();
		pbrShader->SetInt("u_SkylightShadowSettings.ShadowMap", 4);
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
