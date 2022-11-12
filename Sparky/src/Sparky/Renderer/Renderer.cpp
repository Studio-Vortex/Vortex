#include "sppch.h"
#include "Renderer.h"

#include "Sparky/Core/Base.h"
#include "Sparky/Renderer/Renderer2D.h"

#include "Sparky/Renderer/LightSource.h"

namespace Sparky {

	static constexpr const char* BASIC_LIGHTING_SHADER_PATH = "Resources/Shaders/Renderer_BasicLighting.glsl";
	static constexpr const char* PBR_SHADER_PATH = "Resources/Shaders/Renderer_PBR.glsl";
	static constexpr const char* SKYBOX_SHADER_PATH = "Resources/Shaders/Renderer_Skybox.glsl";
	static constexpr const char* REFLECTIVE_SHADER_PATH = "Resources/Shaders/Renderer_Reflection.glsl";
	static constexpr const char* REFRACTIVE_SHADER_PATH = "Resources/Shaders/Renderer_Refraction.glsl";

	static constexpr const char* CAMERA_ICON_PATH = "Resources/Icons/Scene/CameraIcon.png";
	static constexpr const char* LIGHT_SOURCE_ICON_PATH = "Resources/Icons/Scene/LightSourceIcon.png";
	static constexpr const char* AUDIO_SOURCE_ICON_PATH = "Resources/Icons/Scene/AudioSourceIcon.png";

	struct RendererInternalData
	{
		SharedRef<Shader> BasicLightingShader = nullptr;
		SharedRef<Shader> PBRShader = nullptr;
		SharedRef<Shader> SkyboxShader = nullptr;
		SharedRef<Shader> ReflectiveShader = nullptr;
		SharedRef<Shader> RefractiveShader = nullptr;

		SharedRef<Model> SkyboxMesh = nullptr;

		float RefractiveIndex = 1.52f; // Glass

		static constexpr inline uint32_t MaxDirectionalLights = 25;
		static constexpr inline uint32_t MaxPointLights = 25;
		static constexpr inline uint32_t MaxSpotLights = 25;

		uint32_t ActiveDirectionalLights = 0;
		uint32_t ActivePointLights = 0;
		uint32_t ActiveSpotLights = 0;

		float SceneExposure = 1.0f;

		RenderStatistics RendererStatistics;
		RendererAPI::TriangleCullMode CullMode = RendererAPI::TriangleCullMode::None;

		// Editor Resources
		SharedRef<Texture2D> CameraIcon = nullptr;
		SharedRef<Texture2D> LightSourceIcon = nullptr;
		SharedRef<Texture2D> AudioSourceIcon = nullptr;
	};

	static RendererInternalData s_Data;

	void Renderer::Init()
	{
		SP_PROFILE_FUNCTION();

		RenderCommand::Init();

		s_Data.BasicLightingShader = Shader::Create(BASIC_LIGHTING_SHADER_PATH);
		s_Data.PBRShader = Shader::Create(PBR_SHADER_PATH);
		s_Data.SkyboxShader = Shader::Create(SKYBOX_SHADER_PATH);
		s_Data.ReflectiveShader = Shader::Create(REFLECTIVE_SHADER_PATH);
		s_Data.RefractiveShader = Shader::Create(REFRACTIVE_SHADER_PATH);

		s_Data.SkyboxMesh = Model::Create(MeshRendererComponent::MeshType::Cube);

		s_Data.CameraIcon = Texture2D::Create(CAMERA_ICON_PATH);
		s_Data.LightSourceIcon = Texture2D::Create(LIGHT_SOURCE_ICON_PATH);
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

	void Renderer::EndScene()
	{
		
	}

	void Renderer::BindShaders(const Math::mat4& view, const Math::mat4& projection, const Math::vec3& cameraPosition)
	{
		SP_PROFILE_FUNCTION();

		Math::mat4 viewProjection = projection * view;

		s_Data.BasicLightingShader->Enable();
		s_Data.BasicLightingShader->SetMat4("u_ViewProjection", viewProjection);
		s_Data.BasicLightingShader->SetFloat3("u_SceneProperties.CameraPosition", cameraPosition);
		s_Data.BasicLightingShader->SetFloat("u_SceneProperties.Exposure", s_Data.SceneExposure);

		s_Data.PBRShader->Enable();
		s_Data.PBRShader->SetMat4("u_ViewProjection", viewProjection);
		s_Data.PBRShader->SetFloat3("u_SceneProperties.CameraPosition", cameraPosition);
		s_Data.PBRShader->SetFloat("u_SceneProperties.Exposure", s_Data.SceneExposure);

		s_Data.ReflectiveShader->Enable();
		s_Data.ReflectiveShader->SetMat4("u_ViewProjection", viewProjection);
		s_Data.ReflectiveShader->SetInt("u_Skybox", 0);
		s_Data.ReflectiveShader->SetFloat3("u_CameraPosition", cameraPosition);

		s_Data.RefractiveShader->Enable();
		s_Data.RefractiveShader->SetMat4("u_ViewProjection", viewProjection);
		s_Data.RefractiveShader->SetInt("u_Skybox", 0);
		s_Data.RefractiveShader->SetFloat3("u_CameraPosition", cameraPosition);
		s_Data.RefractiveShader->SetFloat("u_RefractiveIndex", s_Data.RefractiveIndex);

		s_Data.ActiveDirectionalLights = 0;
		s_Data.ActivePointLights = 0;
		s_Data.ActiveSpotLights = 0;
	}

	void Renderer::Submit(const SharedRef<Shader>& shader, const SharedRef<VertexArray>& vertexArray)
	{
		SP_PROFILE_FUNCTION();

		shader->Enable();
		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
		s_Data.RendererStatistics.DrawCalls++;
	}

	void Renderer::RenderCameraIcon(const TransformComponent& transform, const Math::vec3& cameraPosition, int entityID)
	{
		Renderer2D::DrawQuadBillboard(cameraPosition, transform.Translation, s_Data.CameraIcon, Math::vec2(1.0f), ColorToVec4(Color::White), entityID);
	}

	void Renderer::RenderLightSourceIcon(const TransformComponent& transform, const Math::vec3& cameraPosition, int entityID)
	{
		Renderer2D::DrawQuadBillboard(cameraPosition, transform.Translation, s_Data.LightSourceIcon, Math::vec2(1.0f), ColorToVec4(Color::White), entityID);
	}

	void Renderer::RenderAudioSourceIcon(const TransformComponent& transform, const Math::vec3& cameraPosition, int entityID)
	{
		Renderer2D::DrawQuadBillboard(cameraPosition, transform.Translation, s_Data.AudioSourceIcon, Math::vec2(1.0f), ColorToVec4(Color::White), entityID);
	}

	void Renderer::RenderLightSource(const LightSourceComponent& lightSourceComponent)
	{
		SharedRef<LightSource> lightSource = lightSourceComponent.Source;

		s_Data.BasicLightingShader->Enable();

		switch (lightSourceComponent.Type)
		{
			case LightSourceComponent::LightType::Directional:
			{
				uint32_t& i = s_Data.ActiveDirectionalLights;

				if (i + 1 > RendererInternalData::MaxDirectionalLights)
					break;

				s_Data.BasicLightingShader->SetFloat3(std::format("u_DirectionalLights[{}].Ambient", i).c_str(), lightSource->GetAmbient());
				s_Data.BasicLightingShader->SetFloat3(std::format("u_DirectionalLights[{}].Diffuse", i).c_str(), lightSource->GetDiffuse());
				s_Data.BasicLightingShader->SetFloat3(std::format("u_DirectionalLights[{}].Specular", i).c_str(), lightSource->GetSpecular());
				s_Data.BasicLightingShader->SetFloat3(std::format("u_DirectionalLights[{}].Color", i).c_str(), lightSource->GetColor());
				s_Data.BasicLightingShader->SetFloat3(std::format("u_DirectionalLights[{}].Direction", i).c_str(), lightSource->GetDirection());

				i++;

				break;
			}
			case LightSourceComponent::LightType::Point:
			{
				uint32_t& i = s_Data.ActivePointLights;

				if (i + 1 > s_Data.MaxPointLights)
					break;

				s_Data.BasicLightingShader->SetFloat3(std::format("u_PointLights[{}].Ambient", i).c_str(), lightSource->GetAmbient());
				s_Data.BasicLightingShader->SetFloat3(std::format("u_PointLights[{}].Diffuse", i).c_str(), lightSource->GetDiffuse());
				s_Data.BasicLightingShader->SetFloat3(std::format("u_PointLights[{}].Specular", i).c_str(), lightSource->GetSpecular());
				s_Data.BasicLightingShader->SetFloat3(std::format("u_PointLights[{}].Color", i).c_str(), lightSource->GetColor());
				s_Data.BasicLightingShader->SetFloat3(std::format("u_PointLights[{}].Position", i).c_str(), lightSource->GetPosition());

				Math::vec2 attenuation = lightSource->GetAttenuation();

				s_Data.BasicLightingShader->SetFloat(std::format("u_PointLights[{}].Constant", i).c_str(), 1.0f);
				s_Data.BasicLightingShader->SetFloat(std::format("u_PointLights[{}].Linear", i).c_str(), attenuation.x);
				s_Data.BasicLightingShader->SetFloat(std::format("u_PointLights[{}].Quadratic", i).c_str(), attenuation.y);

				s_Data.PBRShader->SetFloat3(std::format("u_PointLights[{}].Ambient", i).c_str(), lightSource->GetAmbient());
				s_Data.PBRShader->SetFloat3(std::format("u_PointLights[{}].Diffuse", i).c_str(), lightSource->GetDiffuse());
				s_Data.PBRShader->SetFloat3(std::format("u_PointLights[{}].Specular", i).c_str(), lightSource->GetSpecular());
				s_Data.PBRShader->SetFloat3(std::format("u_PointLights[{}].Color", i).c_str(), lightSource->GetColor());
				s_Data.PBRShader->SetFloat3(std::format("u_PointLights[{}].Position", i).c_str(), lightSource->GetPosition());

				s_Data.PBRShader->SetFloat(std::format("u_PointLights[{}].Constant", i).c_str(), 1.0f);
				s_Data.PBRShader->SetFloat(std::format("u_PointLights[{}].Linear", i).c_str(), attenuation.x);
				s_Data.PBRShader->SetFloat(std::format("u_PointLights[{}].Quadratic", i).c_str(), attenuation.y);

				i++;

				break;
			}
			case LightSourceComponent::LightType::Spot:
			{
				uint32_t& i = s_Data.ActiveSpotLights;

				if (i + 1 > RendererInternalData::MaxSpotLights)
					break;

				s_Data.BasicLightingShader->SetFloat3(std::format("u_SpotLights[{}].Ambient", i).c_str(), lightSource->GetAmbient());
				s_Data.BasicLightingShader->SetFloat3(std::format("u_SpotLights[{}].Diffuse", i).c_str(), lightSource->GetDiffuse());
				s_Data.BasicLightingShader->SetFloat3(std::format("u_SpotLights[{}].Specular", i).c_str(), lightSource->GetSpecular());
				s_Data.BasicLightingShader->SetFloat3(std::format("u_SpotLights[{}].Color", i).c_str(), lightSource->GetColor());
				s_Data.BasicLightingShader->SetFloat3(std::format("u_SpotLights[{}].Position", i).c_str(), lightSource->GetPosition());
				s_Data.BasicLightingShader->SetFloat3(std::format("u_SpotLights[{}].Direction", i).c_str(), lightSource->GetDirection());
				s_Data.BasicLightingShader->SetFloat(std::format("u_SpotLights[{}].CutOff", i).c_str(), Math::Cos(Math::Deg2Rad(lightSource->GetCutOff())));
				s_Data.BasicLightingShader->SetFloat(std::format("u_SpotLights[{}].OuterCutOff", i).c_str(), Math::Cos(Math::Deg2Rad(lightSource->GetOuterCutOff())));

				Math::vec2 attenuation = lightSource->GetAttenuation();

				s_Data.BasicLightingShader->SetFloat(std::format("u_SpotLights[{}].Constant", i).c_str(), 1.0f);
				s_Data.BasicLightingShader->SetFloat(std::format("u_SpotLights[{}].Linear", i).c_str(), attenuation.x);
				s_Data.BasicLightingShader->SetFloat(std::format("u_SpotLights[{}].Quadratic", i).c_str(), attenuation.y);

				i++;

				break;
			}
		}
	}

	void Renderer::DrawModel(const TransformComponent& transform, const MeshRendererComponent& meshRenderer, int entityID)
	{
		SP_PROFILE_FUNCTION();

		SharedRef<Shader> shader;
		SharedRef<Model> model = meshRenderer.Mesh;
		SharedRef<Material> material = model->GetMaterial();
		bool pbr = (bool)material->GetAlbedoMap(); // TODO Rework this

		if (meshRenderer.Reflective)
		{
			shader = s_Data.ReflectiveShader;
			shader->Enable();
			shader->SetFloat3("u_Material.Ambient", material->GetAmbient());
		}
		else if (meshRenderer.Refractive)
		{
			shader = s_Data.RefractiveShader;
			shader->Enable();
			shader->SetFloat3("u_Material.Ambient", material->GetAmbient());
		}
		else
		{
			shader = pbr ? s_Data.PBRShader : s_Data.BasicLightingShader;
			shader->Enable();

			if (!pbr)
				shader->SetInt("u_SceneProperties.ActiveDirectionalLights", s_Data.ActiveDirectionalLights);
			shader->SetInt("u_SceneProperties.ActivePointLights", s_Data.ActivePointLights);
			if (!pbr)
				shader->SetInt("u_SceneProperties.ActiveSpotLights", s_Data.ActiveSpotLights);

			if (!pbr)
				shader->SetFloat3("u_Material.Ambient", material->GetAmbient());

			if (SharedRef<Texture2D> diffuseMap = material->GetDiffuseMap(); diffuseMap && !pbr)
			{
				uint32_t diffuseMapTextureSlot = 1;
				diffuseMap->Bind(diffuseMapTextureSlot);
				shader->SetInt("u_Material.DiffuseMap", diffuseMapTextureSlot);
				shader->SetBool("u_Material.HasDiffuseMap", true);
			}
			else if (!pbr)
				shader->SetBool("u_Material.HasDiffuseMap", false);

			if (SharedRef<Texture2D> specularMap = material->GetSpecularMap(); specularMap && !pbr)
			{
				uint32_t specularMapTextureSlot = 2;
				specularMap->Bind(specularMapTextureSlot);
				shader->SetInt("u_Material.SpecularMap", specularMapTextureSlot);
				shader->SetBool("u_Material.HasSpecularMap", true);
			}
			else if (!pbr)
				shader->SetBool("u_Material.HasSpecularMap", false);

			if (SharedRef<Texture2D> normalMap = material->GetNormalMap())
			{
				uint32_t normalMapTextureSlot = 3;
				normalMap->Bind(normalMapTextureSlot);
				shader->SetInt("u_Material.NormalMap", normalMapTextureSlot);
				shader->SetBool("u_Material.HasNormalMap", true);
			}
			else
				shader->SetBool("u_Material.HasNormalMap", false);

			if (!pbr)
				shader->SetFloat("u_Material.Shininess", material->GetShininess());

			if (SharedRef<Texture2D> albedoMap = material->GetAlbedoMap())
			{
				uint32_t albedoMapTextureSlot = 4;
				albedoMap->Bind(albedoMapTextureSlot);
				shader->SetInt("u_Material.AlbedoMap", albedoMapTextureSlot);
				shader->SetBool("u_Material.HasAlbedoMap", true);
			}
			else if (pbr)
			{
				shader->SetBool("u_Material.HasAlbedoMap", false);
				shader->SetFloat3("u_Material.Albedo", material->GetAlbedo());
			}

			if (SharedRef<Texture2D> metallicMap = material->GetMetallicMap())
			{
				uint32_t metallicMapTextureSlot = 5;
				metallicMap->Bind(metallicMapTextureSlot);
				shader->SetInt("u_Material.MetallicMap", metallicMapTextureSlot);
				shader->SetBool("u_Material.HasMetallicMap", true);
			}
			else if (pbr)
			{
				shader->SetBool("u_Material.HasMetallicMap", false);
				shader->SetFloat("u_Material.Metallic", material->GetMetallic());
			}

			if (SharedRef<Texture2D> roughnessMap = material->GetRoughnessMap())
			{
				uint32_t roughnessMapTextureSlot = 6;
				roughnessMap->Bind(roughnessMapTextureSlot);
				shader->SetInt("u_Material.RoughnessMap", roughnessMapTextureSlot);
				shader->SetBool("u_Material.HasRoughnessMap", true);
			}
			else if (pbr)
			{
				shader->SetBool("u_Material.HasRoughnessMap", false);
				shader->SetFloat("u_Material.Roughness", material->GetRoughness());
			}

			if (SharedRef<Texture2D> ambientOcclusionMap = material->GetAmbientOcclusionMap())
			{
				uint32_t ambientOcclusionMapTextureSlot = 7;
				ambientOcclusionMap->Bind(ambientOcclusionMapTextureSlot);
				shader->SetInt("u_Material.AOMap", ambientOcclusionMapTextureSlot);
				shader->SetBool("u_Material.HasAOMap", true);
			}
			else if (pbr)
				shader->SetBool("u_Material.HasAOMap", false);
		}

		shader->SetMat4("u_Model", transform.GetTransform());

		Submit(shader, model->GetVertexArray());

#if SP_RENDERER_STATISTICS
		s_Data.RendererStatistics.QuadCount += model->GetQuadCount();
#endif // SP_RENDERER_STATISTICS
	}

	void Renderer::DrawSkybox(const Math::mat4& view, const Math::mat4& projection, const SharedRef<Skybox>& skybox)
	{
		RenderCommand::DisableDepthMask();
		s_Data.SkyboxShader->Enable();
		s_Data.SkyboxShader->SetInt("u_EnvironmentMap", 0);
		s_Data.SkyboxShader->SetMat4("u_View", Math::mat4(Math::mat3(view)));
		s_Data.SkyboxShader->SetMat4("u_Projection", projection);

		SharedRef<VertexArray> skyboxMeshVA = s_Data.SkyboxMesh->GetVertexArray();

		skyboxMeshVA->Bind();
		skybox->Bind();
		RenderCommand::DrawTriangles(skyboxMeshVA, 36);
		RenderCommand::EnableDepthMask();
	}

	void Renderer::DrawCubeWireframe(const TransformComponent& transform)
	{
		SP_PROFILE_FUNCTION();

		Math::mat4 entityTransform = transform.GetTransform();
		Math::vec3 scale = transform.Scale;
		Math::vec4 color = ColorToVec4(Color::Orange);

		Renderer2D::DrawRect(entityTransform * Math::Translate(Math::vec3(scale.x * 0.5f, 0.0f, 0.0f)) * Math::Rotate(Math::Deg2Rad(90.0f), { 0.0f, 1.0f, 0.0f }), color);
		Renderer2D::DrawRect(entityTransform * Math::Translate(Math::vec3(-scale.x * 0.5f, 0.0f, 0.0f)) * Math::Rotate(Math::Deg2Rad(90.0f), { 0.0f, 1.0f, 0.0f }), color);

		Renderer2D::DrawRect(entityTransform * Math::Translate(Math::vec3(0.0f, scale.y * 0.5f, 0.0f)) * Math::Rotate(Math::Deg2Rad(90.0f), { 1.0f, 0.0f, 0.0f }), color);
		Renderer2D::DrawRect(entityTransform * Math::Translate(Math::vec3(0.0f, -scale.y * 0.5f, 0.0f)) * Math::Rotate(Math::Deg2Rad(90.0f), { 1.0f, 0.0f, 0.0f }), color);

		Renderer2D::DrawRect(entityTransform * Math::Translate(Math::vec3(0.0f, 0.0f, scale.z * 0.5f)), color);
		Renderer2D::DrawRect(entityTransform * Math::Translate(Math::vec3(0.0f, 0.0f, -scale.z * 0.5f)), color);
	}

	void Renderer::DrawFrustum(const TransformComponent& transform, SceneCamera sceneCamera, const Math::vec4& color)
	{
		//4 2.25
		Math::vec3 rotation = transform.Rotation;
		Math::vec3 forwardDirection = Math::Rotate(Math::GetOrientation(rotation.x, rotation.y, rotation.z), { 0.0f, 0.0f, -1.0f });
		Math::mat4 nearClipTransform = transform.GetTransform() * Math::Translate(forwardDirection) / 2.0f;
		Math::mat4 farClipTransform = nearClipTransform * Math::Translate(forwardDirection) / 2.0f;
		Math::vec2 viewportSize = sceneCamera.GetViewportSize();
		Renderer2D::DrawRect(farClipTransform, color);
		Renderer2D::DrawRect(nearClipTransform, color);
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

	std::vector<SharedRef<Shader>> Renderer::GetLoadedShaders()
	{
		std::vector<SharedRef<Shader>> shaders;
		shaders.push_back(s_Data.BasicLightingShader);
		shaders.push_back(s_Data.PBRShader);
		shaders.push_back(s_Data.SkyboxShader);
		shaders.push_back(s_Data.ReflectiveShader);
		shaders.push_back(s_Data.RefractiveShader);
		return shaders;
	}

}
