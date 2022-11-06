#include "sppch.h"
#include "Renderer.h"

#include "Sparky/Core/Base.h"
#include "Sparky/Renderer/Renderer2D.h"

#include "Sparky/Renderer/LightSource.h"

namespace Sparky {

	static constexpr const char* MODEL_SHADER_PATH = "Resources/Shaders/Renderer_Model.glsl";
	static constexpr const char* REFLECTIVE_SHADER_PATH = "Resources/Shaders/Renderer_Reflection.glsl";
	static constexpr const char* REFRACTIVE_SHADER_PATH = "Resources/Shaders/Renderer_Refraction.glsl";
	static constexpr const char* SKYBOX_SHADER_PATH = "Resources/Shaders/Renderer_Skybox.glsl";

	static constexpr const char* CAMERA_ICON_PATH = "Resources/Icons/Scene/CameraIcon.png";
	static constexpr const char* LIGHT_SOURCE_ICON_PATH = "Resources/Icons/Scene/LightSourceIcon.png";

	struct RendererInternalData
	{
		static constexpr inline uint32_t MaxTextureSlots = 32; // TODO: RendererCapabilities

		SharedRef<Texture2D> WhiteTexture = nullptr; // Default texture

		SharedRef<Shader> ModelShader = nullptr;
		SharedRef<Shader> ReflectiveShader = nullptr;
		SharedRef<Shader> RefractiveShader = nullptr;
		SharedRef<Shader> SkyboxShader = nullptr;

		SharedRef<Model> SkyboxMesh = nullptr;

		std::array<SharedRef<Texture2D>, MaxTextureSlots> TextureSlots;
		uint32_t TextureSlotIndex = 1; // 0 = White Texture

		float RefractiveIndex = 1.52f; // Glass

		static constexpr inline uint32_t MaxPointLights = 2;
		uint32_t PointLightIndex = 0;

		RenderStatistics RendererStatistics;
		RendererAPI::TriangleCullMode CullMode = RendererAPI::TriangleCullMode::None;

		// Editor Resources
		SharedRef<Texture2D> CameraIcon = nullptr;
		SharedRef<Texture2D> LightSourceIcon = nullptr;
	};

	static RendererInternalData s_Data;

	void Renderer::Init()
	{
		SP_PROFILE_FUNCTION();

		RenderCommand::Init();

		s_Data.WhiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		s_Data.WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

		s_Data.ModelShader = Shader::Create(MODEL_SHADER_PATH);
		s_Data.ReflectiveShader = Shader::Create(REFLECTIVE_SHADER_PATH);
		s_Data.RefractiveShader = Shader::Create(REFRACTIVE_SHADER_PATH);

		s_Data.SkyboxShader = Shader::Create(SKYBOX_SHADER_PATH);

		s_Data.SkyboxMesh = Model::Create(MeshRendererComponent::MeshType::Cube);

		s_Data.CameraIcon = Texture2D::Create(CAMERA_ICON_PATH);
		s_Data.LightSourceIcon = Texture2D::Create(LIGHT_SOURCE_ICON_PATH);

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

		s_Data.PointLightIndex = 0;
	}

	void Renderer::BeginScene(const EditorCamera& camera)
	{
		SP_PROFILE_FUNCTION();

		BindShaders(camera.GetViewMatrix(), camera.GetProjection(), camera.GetPosition());

		s_Data.PointLightIndex = 0;
	}

	void Renderer::EndScene()
	{

	}

	void Renderer::BindShaders(const Math::mat4& view, const Math::mat4& projection, const Math::vec3& cameraPosition)
	{
		s_Data.ModelShader->Enable();
		s_Data.ModelShader->SetMat4("u_View", view);
		s_Data.ModelShader->SetMat4("u_Projection", projection);
		s_Data.ModelShader->SetFloat3("u_CameraPosition", cameraPosition);

		s_Data.ReflectiveShader->Enable();
		s_Data.ReflectiveShader->SetMat4("u_ViewProjection", projection * view);
		s_Data.ReflectiveShader->SetInt("u_Skybox", 0);
		s_Data.ReflectiveShader->SetFloat3("u_CameraPosition", cameraPosition);

		s_Data.RefractiveShader->Enable();
		s_Data.RefractiveShader->SetMat4("u_ViewProjection", projection * view);
		s_Data.RefractiveShader->SetInt("u_Skybox", 0);
		s_Data.RefractiveShader->SetFloat3("u_CameraPosition", cameraPosition);
		s_Data.RefractiveShader->SetFloat("u_RefractiveIndex", s_Data.RefractiveIndex);
	}

	void Renderer::Submit(const SharedRef<Shader>& shader, const SharedRef<VertexArray>& vertexArray)
	{
		SP_PROFILE_FUNCTION();

		shader->Enable();
		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
		s_Data.RendererStatistics.DrawCalls++;
	}

	void Renderer::RenderCameraIcon(const TransformComponent& transform, int entityID)
	{
		Renderer2D::DrawQuad(transform.GetTransform() * Math::Scale(Math::vec3(0.75f)), s_Data.CameraIcon, Math::vec2(1.0f), Math::vec4(1.0f), entityID);
	}

	void Renderer::RenderLightSource(const TransformComponent& transform, const LightSourceComponent& light, bool sceneRunning, int entityID)
	{
		SharedRef<LightSource> lightSource = light.Source;

		s_Data.ModelShader->Enable();

		switch (light.Type)
		{
			case LightSourceComponent::LightType::Directional:
			{
				s_Data.ModelShader->SetFloat3("u_DirectionalLight.Ambient", lightSource->GetAmbient());
				s_Data.ModelShader->SetFloat3("u_DirectionalLight.Diffuse", lightSource->GetDiffuse());
				s_Data.ModelShader->SetFloat3("u_DirectionalLight.Specular", lightSource->GetSpecular());
				s_Data.ModelShader->SetFloat3("u_DirectionalLight.Color", lightSource->GetColor());
				s_Data.ModelShader->SetFloat3("u_DirectionalLight.Direction", lightSource->GetDirection());

				break;
			}
			case LightSourceComponent::LightType::Point:
			{
				uint32_t& i = s_Data.PointLightIndex;

				if (i >= s_Data.MaxPointLights)
					break;

				s_Data.ModelShader->SetFloat3(std::format("u_PointLights[{}].Ambient", i).c_str(), lightSource->GetAmbient());
				s_Data.ModelShader->SetFloat3(std::format("u_PointLights[{}].Diffuse", i).c_str(), lightSource->GetDiffuse());
				s_Data.ModelShader->SetFloat3(std::format("u_PointLights[{}].Specular", i).c_str(), lightSource->GetSpecular());
				s_Data.ModelShader->SetFloat3(std::format("u_PointLights[{}].Color", i).c_str(), lightSource->GetColor());
				s_Data.ModelShader->SetFloat3(std::format("u_PointLights[{}].Position", i).c_str(), lightSource->GetPosition());

				Math::vec2 attenuation = lightSource->GetAttenuation();

				s_Data.ModelShader->SetFloat(std::format("u_PointLights[{}].Constant", i).c_str(), 1.0f);
				s_Data.ModelShader->SetFloat(std::format("u_PointLights[{}].Linear", i).c_str(), attenuation.x);
				s_Data.ModelShader->SetFloat(std::format("u_PointLights[{}].Quadratic", i).c_str(), attenuation.y);

				i++;

				break;
			}
			case LightSourceComponent::LightType::Spot:
			{
				s_Data.ModelShader->SetFloat3("u_SpotLight.Ambient", lightSource->GetAmbient());
				s_Data.ModelShader->SetFloat3("u_SpotLight.Diffuse", lightSource->GetDiffuse());
				s_Data.ModelShader->SetFloat3("u_SpotLight.Specular", lightSource->GetSpecular());
				s_Data.ModelShader->SetFloat3("u_SpotLight.Color", lightSource->GetColor());
				s_Data.ModelShader->SetFloat3("u_SpotLight.Position", lightSource->GetPosition());
				s_Data.ModelShader->SetFloat3("u_SpotLight.Direction", lightSource->GetDirection());
				s_Data.ModelShader->SetFloat("u_SpotLight.CutOff", Math::Cos(Math::Deg2Rad(lightSource->GetCutOff())));
				s_Data.ModelShader->SetFloat("u_SpotLight.OuterCutOff", Math::Cos(Math::Deg2Rad(lightSource->GetOuterCutOff())));

				Math::vec2 attenuation = lightSource->GetAttenuation();

				s_Data.ModelShader->SetFloat("u_SpotLight.Constant", 1.0f);
				s_Data.ModelShader->SetFloat("u_SpotLight.Linear", attenuation.x);
				s_Data.ModelShader->SetFloat("u_SpotLight.Quadratic", attenuation.y);

				break;
			}
		}

		if (!sceneRunning)
			Renderer2D::DrawQuad(transform.GetTransform() * Math::Scale(Math::vec3(0.75f)), s_Data.LightSourceIcon, Math::vec2(1.0f), Math::vec4(1.0f), entityID);
	}

	void Renderer::DrawModel(const TransformComponent& transform, const MeshRendererComponent& meshRenderer, int entityID)
	{
		SP_PROFILE_FUNCTION();

		SharedRef<Shader> shader;
		SharedRef<Model> model = meshRenderer.Mesh;
		SharedRef<Material> material = model->GetMaterial();

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
			shader = s_Data.ModelShader;
			shader->Enable();
			shader->SetFloat3("u_Material.Ambient", material->GetAmbient());

			SharedRef<Texture2D> texture = meshRenderer.Texture ? meshRenderer.Texture : s_Data.WhiteTexture;
			uint32_t textureSlot = 1;
			texture->Bind(textureSlot);
			shader->SetInt("u_Texture", textureSlot);

			shader->SetMat4("u_Model", transform.GetTransform());

			if (SharedRef<Texture2D> diffuseMap = material->GetDiffuseMap())
			{
				uint32_t diffuseMapTextureSlot = 1;
				diffuseMap->Bind(diffuseMapTextureSlot);
				shader->SetInt("u_Material.DiffuseMap", diffuseMapTextureSlot);
			}

			if (SharedRef<Texture2D> specularMap = material->GetSpecularMap())
			{
				uint32_t specularMapTextureSlot = 2;
				specularMap->Bind(specularMapTextureSlot);
				shader->SetInt("u_Material.SpecularMap", specularMapTextureSlot);
			}

			if (SharedRef<Texture2D> normalMap = material->GetNormalMap())
			{
				uint32_t normalMapTextureSlot = 3;
				normalMap->Bind(normalMapTextureSlot);
				shader->SetInt("u_Material.NormalMap", normalMapTextureSlot);
			}

			shader->SetFloat("u_Material.Shininess", material->GetShininess());
		}

		Submit(shader, model->GetVertexArray());

#if SP_RENDERER_STATISTICS
		s_Data.RendererStatistics.QuadCount += model->GetQuadCount();
#endif // SP_RENDERER_STATISTICS
	}

	void Renderer::DrawSkybox(const Math::mat4& view, const Math::mat4& projection, const SharedRef<Skybox>& skybox)
	{
		RenderCommand::DisableDepthMask();
		s_Data.SkyboxShader->Enable();
		s_Data.SkyboxShader->SetInt("u_Skybox", 0);
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

		//Math::vec3 translation;
		//Math::vec3 rotation;
		//Math::vec3 scale;
		//Math::DecomposeTransform(transform.GetTransform(), translation, rotation, scale);

		// Translate by the scale instead of translation
		//                                              here
		Renderer2D::DrawRect(transform.GetTransform() * Math::Translate({ transform.Translation.x, transform.Translation.y + (transform.Scale.y / 2.0f), transform.Translation.z }) * Math::Rotate(Math::Deg2Rad(90.0f), { 1.0f, 0.0f, 0.0f }), ColorToVec4(Color::Orange));
		Renderer2D::DrawRect(transform.GetTransform() * Math::Translate({ transform.Translation.x, transform.Translation.y - (transform.Scale.y / 2.0f), transform.Translation.z }) * Math::Rotate(Math::Deg2Rad(90.0f), { 1.0f, 0.0f, 0.0f }), ColorToVec4(Color::Orange));

		Renderer2D::DrawRect(transform.GetTransform() * Math::Translate({ transform.Translation.x, transform.Translation.y, transform.Translation.z + (transform.Scale.z / 2.0f) }), ColorToVec4(Color::Orange));
		Renderer2D::DrawRect(transform.GetTransform() * Math::Translate({ transform.Translation.x, transform.Translation.y, transform.Translation.z - (transform.Scale.z / 2.0f) }), ColorToVec4(Color::Orange));
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

	std::vector<SharedRef<Shader>> Renderer::GetLoadedShaders()
	{
		std::vector<SharedRef<Shader>> shaders;
		shaders.push_back(s_Data.ModelShader);
		shaders.push_back(s_Data.ReflectiveShader);
		shaders.push_back(s_Data.RefractiveShader);
		shaders.push_back(s_Data.SkyboxShader);
		return shaders;
	}

}
