#include "vxpch.h"
#include "Renderer.h"

#include "Vortex/Core/Base.h"
#include "Vortex/Renderer/Renderer2D.h"

#include "Vortex/Project/Project.h"
#include "Vortex/Asset/AssetRegistry.h"
//#include "Vortex/Renderer/Model.h"
#include "Vortex/Renderer/LightSource.h"
#include "Vortex/Renderer/Font/Font.h"

namespace Vortex {

	static constexpr const char* BASIC_LIGHTING_SHADER_PATH = "Resources/Shaders/Renderer_BasicLighting.glsl";
	static constexpr const char* PBR_SHADER_PATH = "Resources/Shaders/Renderer_PBR.glsl";
	static constexpr const char* SKYBOX_SHADER_PATH = "Resources/Shaders/Renderer_Skybox.glsl";
	static constexpr const char* REFLECTIVE_SHADER_PATH = "Resources/Shaders/Renderer_Reflection.glsl";
	static constexpr const char* REFRACTIVE_SHADER_PATH = "Resources/Shaders/Renderer_Refraction.glsl";

	static constexpr const char* CAMERA_ICON_PATH = "Resources/Icons/Scene/CameraIcon.png";
	static constexpr const char* DIR_LIGHT_ICON_PATH = "Resources/Icons/Scene/DirLightIcon.png";
	static constexpr const char* POINT_LIGHT_ICON_PATH = "Resources/Icons/Scene/PointLight.png";
	static constexpr const char* SPOT_LIGHT_ICON_PATH = "Resources/Icons/Scene/SpotLight.png";
	static constexpr const char* AUDIO_SOURCE_ICON_PATH = "Resources/Icons/Scene/AudioSourceIcon.png";

	struct RendererInternalData
	{
		SharedRef<ShaderLibrary> ShaderLibrary = nullptr;

		SharedRef<Model> SkyboxMesh = nullptr;

		float RefractiveIndex = 1.52f; // Glass

		static constexpr inline uint32_t MaxDirectionalLights = 1;
		static constexpr inline uint32_t MaxPointLights = 25;
		static constexpr inline uint32_t MaxSpotLights = 25;

		SceneLightDescription SceneLightDesc;

		float SceneExposure = 1.0f;
		float SceneGamma = 2.2f;

		bool RenderWithPBROnly = false;

		RenderStatistics RendererStatistics;
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
		SP_PROFILE_FUNCTION();

		RenderCommand::Init();

		s_Data.ShaderLibrary = ShaderLibrary::Create();
		s_Data.ShaderLibrary->Load("BasicLighting", BASIC_LIGHTING_SHADER_PATH);
		s_Data.ShaderLibrary->Load("PBR", PBR_SHADER_PATH);
		s_Data.ShaderLibrary->Load("Skybox", SKYBOX_SHADER_PATH);
		s_Data.ShaderLibrary->Load("Reflective", REFLECTIVE_SHADER_PATH);
		s_Data.ShaderLibrary->Load("Refractive", REFRACTIVE_SHADER_PATH);

		s_Data.SkyboxMesh = Model::Create(MeshType::Cube);

		s_Data.CameraIcon = Texture2D::Create(CAMERA_ICON_PATH);
		s_Data.PointLightIcon = Texture2D::Create(POINT_LIGHT_ICON_PATH);
		s_Data.SpotLightIcon = Texture2D::Create(SPOT_LIGHT_ICON_PATH);
		s_Data.DirLightIcon = Texture2D::Create(DIR_LIGHT_ICON_PATH);
		s_Data.AudioSourceIcon = Texture2D::Create(AUDIO_SOURCE_ICON_PATH);

#if SP_RENDERER_STATISTICS
		ResetStats();
#endif // SP_RENDERER_STATISTICS

		Renderer2D::Init();
	}

	void Renderer::Shutdown()
	{
		SP_PROFILE_FUNCTION();

		Renderer2D::Shutdown();
	}

	void Renderer::OnWindowResize(const Viewport& viewport)
	{
		RenderCommand::SetViewport(viewport);
	}

	void Renderer::BeginScene(const Camera& camera, const TransformComponent& transform)
	{
		SP_PROFILE_FUNCTION();

		BindShaders(Math::Inverse(transform.GetTransform()), camera.GetProjection(), transform.Translation);
	}

	void Renderer::BeginScene(const EditorCamera& camera)
	{
		SP_PROFILE_FUNCTION();

		BindShaders(camera.GetViewMatrix(), camera.GetProjection(), camera.GetPosition());
	}

	void Renderer::EndScene() { }

	void Renderer::BindShaders(const Math::mat4& view, const Math::mat4& projection, const Math::vec3& cameraPosition)
	{
		SP_PROFILE_FUNCTION();

		Math::mat4 viewProjection = projection * view;

		SharedRef<Shader> basicLightingShader = s_Data.ShaderLibrary->Get("BasicLighting");
		basicLightingShader->Enable();
		basicLightingShader->SetMat4("u_ViewProjection", viewProjection);
		basicLightingShader->SetFloat3("u_SceneProperties.CameraPosition", cameraPosition);
		basicLightingShader->SetFloat("u_SceneProperties.Exposure", s_Data.SceneExposure);

		SharedRef<Shader> pbrShader = s_Data.ShaderLibrary->Get("PBR");
		pbrShader->Enable();
		pbrShader->SetMat4("u_ViewProjection", viewProjection);
		pbrShader->SetFloat3("u_SceneProperties.CameraPosition", cameraPosition);
		pbrShader->SetFloat("u_SceneProperties.Exposure", s_Data.SceneExposure);
		pbrShader->SetFloat("u_SceneProperties.Gamma", s_Data.SceneGamma);

		SharedRef<Shader> reflectiveShader = s_Data.ShaderLibrary->Get("Reflective");
		reflectiveShader->Enable();
		reflectiveShader->SetMat4("u_ViewProjection", viewProjection);
		reflectiveShader->SetInt("u_Skybox", 0);
		reflectiveShader->SetFloat3("u_CameraPosition", cameraPosition);

		SharedRef<Shader> refractiveShader = s_Data.ShaderLibrary->Get("Refractive");
		refractiveShader->Enable();
		refractiveShader->SetMat4("u_ViewProjection", viewProjection);
		refractiveShader->SetInt("u_Skybox", 0);
		refractiveShader->SetFloat3("u_CameraPosition", cameraPosition);
		refractiveShader->SetFloat("u_RefractiveIndex", s_Data.RefractiveIndex);

		s_Data.SceneLightDesc.ActiveDirLights = 0;
		s_Data.SceneLightDesc.ActivePointLights = 0;
		s_Data.SceneLightDesc.ActiveSpotLights = 0;
	}

	void Renderer::Submit(const SharedRef<Shader>& shader, const SharedRef<VertexArray>& vertexArray)
	{
		SP_PROFILE_FUNCTION();

		shader->Enable();
		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
		s_Data.RendererStatistics.DrawCalls++;
	}

	void Renderer::DrawIndexed(const SharedRef<Shader>& shader, const SharedRef<VertexArray>& vertexArray)
	{
		SP_PROFILE_FUNCTION();

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

	void Renderer::RenderLightSource(const LightSourceComponent& lightSourceComponent)
	{
		SharedRef<LightSource> lightSource = lightSourceComponent.Source;
		SharedRef<Shader> basicLightingShader = s_Data.ShaderLibrary->Get("BasicLighting");
		SharedRef<Shader> pbrShader = s_Data.ShaderLibrary->Get("PBR");

		switch (lightSourceComponent.Type)
		{
			case LightSourceComponent::LightType::Directional:
			{
				uint32_t& i = s_Data.SceneLightDesc.ActiveDirLights;

				if (i + 1 > RendererInternalData::MaxDirectionalLights)
					break;

				basicLightingShader->Enable();
				basicLightingShader->SetFloat3(std::format("u_DirectionalLights[{}].Ambient", i).c_str(), lightSource->GetAmbient());
				basicLightingShader->SetFloat3(std::format("u_DirectionalLights[{}].Diffuse", i).c_str(), lightSource->GetDiffuse());
				basicLightingShader->SetFloat3(std::format("u_DirectionalLights[{}].Specular", i).c_str(), lightSource->GetSpecular());
				basicLightingShader->SetFloat3(std::format("u_DirectionalLights[{}].Color", i).c_str(), lightSource->GetColor());
				basicLightingShader->SetFloat3(std::format("u_DirectionalLights[{}].Direction", i).c_str(), lightSource->GetDirection());

				pbrShader->Enable();
				pbrShader->SetFloat3(std::format("u_DirectionalLights[{}].Radiance", i).c_str(), lightSource->GetRadiance());
				pbrShader->SetFloat3(std::format("u_DirectionalLights[{}].Ambient", i).c_str(), lightSource->GetAmbient());
				pbrShader->SetFloat3(std::format("u_DirectionalLights[{}].Diffuse", i).c_str(), lightSource->GetDiffuse());
				pbrShader->SetFloat3(std::format("u_DirectionalLights[{}].Specular", i).c_str(), lightSource->GetSpecular());
				pbrShader->SetFloat3(std::format("u_DirectionalLights[{}].Color", i).c_str(), lightSource->GetColor());
				pbrShader->SetFloat3(std::format("u_DirectionalLights[{}].Direction", i).c_str(), lightSource->GetDirection());

				i++;

				break;
			}
			case LightSourceComponent::LightType::Point:
			{
				uint32_t& i = s_Data.SceneLightDesc.ActivePointLights;

				if (i + 1 > RendererInternalData::MaxPointLights)
					break;

				basicLightingShader->Enable();
				basicLightingShader->SetFloat3(std::format("u_PointLights[{}].Ambient", i).c_str(), lightSource->GetAmbient());
				basicLightingShader->SetFloat3(std::format("u_PointLights[{}].Diffuse", i).c_str(), lightSource->GetDiffuse());
				basicLightingShader->SetFloat3(std::format("u_PointLights[{}].Specular", i).c_str(), lightSource->GetSpecular());
				basicLightingShader->SetFloat3(std::format("u_PointLights[{}].Color", i).c_str(), lightSource->GetColor());
				basicLightingShader->SetFloat3(std::format("u_PointLights[{}].Position", i).c_str(), lightSource->GetPosition());

				Math::vec2 attenuation = lightSource->GetAttenuation();

				basicLightingShader->SetFloat(std::format("u_PointLights[{}].Constant", i).c_str(), 1.0f);
				basicLightingShader->SetFloat(std::format("u_PointLights[{}].Linear", i).c_str(), attenuation.x);
				basicLightingShader->SetFloat(std::format("u_PointLights[{}].Quadratic", i).c_str(), attenuation.y);

				pbrShader->Enable();
				pbrShader->SetFloat3(std::format("u_PointLights[{}].Radiance", i).c_str(), lightSource->GetRadiance());
				pbrShader->SetFloat3(std::format("u_PointLights[{}].Ambient", i).c_str(), lightSource->GetAmbient());
				pbrShader->SetFloat3(std::format("u_PointLights[{}].Diffuse", i).c_str(), lightSource->GetDiffuse());
				pbrShader->SetFloat3(std::format("u_PointLights[{}].Specular", i).c_str(), lightSource->GetSpecular());
				pbrShader->SetFloat3(std::format("u_PointLights[{}].Color", i).c_str(), lightSource->GetColor());
				pbrShader->SetFloat3(std::format("u_PointLights[{}].Position", i).c_str(), lightSource->GetPosition());

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

				basicLightingShader->Enable();
				basicLightingShader->SetFloat3(std::format("u_SpotLights[{}].Ambient", i).c_str(), lightSource->GetAmbient());
				basicLightingShader->SetFloat3(std::format("u_SpotLights[{}].Diffuse", i).c_str(), lightSource->GetDiffuse());
				basicLightingShader->SetFloat3(std::format("u_SpotLights[{}].Specular", i).c_str(), lightSource->GetSpecular());
				basicLightingShader->SetFloat3(std::format("u_SpotLights[{}].Color", i).c_str(), lightSource->GetColor());
				basicLightingShader->SetFloat3(std::format("u_SpotLights[{}].Position", i).c_str(), lightSource->GetPosition());
				basicLightingShader->SetFloat3(std::format("u_SpotLights[{}].Direction", i).c_str(), lightSource->GetDirection());
				basicLightingShader->SetFloat(std::format("u_SpotLights[{}].CutOff", i).c_str(), Math::Cos(Math::Deg2Rad(lightSource->GetCutOff())));
				basicLightingShader->SetFloat(std::format("u_SpotLights[{}].OuterCutOff", i).c_str(), Math::Cos(Math::Deg2Rad(lightSource->GetOuterCutOff())));

				Math::vec2 attenuation = lightSource->GetAttenuation();

				basicLightingShader->SetFloat(std::format("u_SpotLights[{}].Constant", i).c_str(), 1.0f);
				basicLightingShader->SetFloat(std::format("u_SpotLights[{}].Linear", i).c_str(), attenuation.x);
				basicLightingShader->SetFloat(std::format("u_SpotLights[{}].Quadratic", i).c_str(), attenuation.y);

				pbrShader->Enable();
				pbrShader->SetFloat3(std::format("u_SpotLights[{}].Radiance", i).c_str(), lightSource->GetRadiance());
				pbrShader->SetFloat3(std::format("u_SpotLights[{}].Ambient", i).c_str(), lightSource->GetAmbient());
				pbrShader->SetFloat3(std::format("u_SpotLights[{}].Diffuse", i).c_str(), lightSource->GetDiffuse());
				pbrShader->SetFloat3(std::format("u_SpotLights[{}].Specular", i).c_str(), lightSource->GetSpecular());
				pbrShader->SetFloat3(std::format("u_SpotLights[{}].Color", i).c_str(), lightSource->GetColor());
				pbrShader->SetFloat3(std::format("u_SpotLights[{}].Position", i).c_str(), lightSource->GetPosition());
				pbrShader->SetFloat3(std::format("u_SpotLights[{}].Direction", i).c_str(), lightSource->GetDirection());
				pbrShader->SetFloat(std::format("u_SpotLights[{}].CutOff", i).c_str(), Math::Cos(Math::Deg2Rad(lightSource->GetCutOff())));
				pbrShader->SetFloat(std::format("u_SpotLights[{}].OuterCutOff", i).c_str(), Math::Cos(Math::Deg2Rad(lightSource->GetOuterCutOff())));

				pbrShader->SetFloat(std::format("u_SpotLights[{}].Constant", i).c_str(), 1.0f);
				pbrShader->SetFloat(std::format("u_SpotLights[{}].Linear", i).c_str(), attenuation.x);
				pbrShader->SetFloat(std::format("u_SpotLights[{}].Quadratic", i).c_str(), attenuation.y);

				i++;

				break;
			}
		}
	}

	void Renderer::DrawSkybox(const Math::mat4& view, const Math::mat4& projection, const SharedRef<Skybox>& skybox)
	{
		RenderCommand::DisableDepthMask();

		SharedRef<Shader> skyboxShader = s_Data.ShaderLibrary->Get("Skybox");
		skyboxShader->Enable();
		skyboxShader->SetInt("u_EnvironmentMap", 0);
		skyboxShader->SetMat4("u_View", Math::mat4(Math::mat3(view)));
		skyboxShader->SetMat4("u_Projection", projection);

		SharedRef<VertexArray> skyboxMeshVA = s_Data.SkyboxMesh->GetVertexArray();

		skyboxMeshVA->Bind();
		skybox->Bind();
		RenderCommand::DrawTriangles(skyboxMeshVA, 36);
		RenderCommand::EnableDepthMask();
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

	float Renderer::GetRefractiveIndex()
	{
		return s_Data.RefractiveIndex;
	}

	void Renderer::SetRefractiveIndex(float index)
	{
		s_Data.RefractiveIndex = index;
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

	void Renderer::EnablePBR()
	{
		s_Data.RenderWithPBROnly = true;
	}

	void Renderer::DisablePBR()
	{
		s_Data.RenderWithPBROnly = false;
	}

	SharedRef<ShaderLibrary> Renderer::GetShaderLibrary()
	{
		return s_Data.ShaderLibrary;
	}

}
