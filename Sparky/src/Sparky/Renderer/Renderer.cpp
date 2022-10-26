#include "sppch.h"
#include "Renderer.h"

#include "Sparky/Core/Base.h"
#include "Sparky/Renderer/Renderer2D.h"

#include <Glad/glad.h>

namespace Sparky {

	static constexpr const char* MODEL_SHADER_PATH = "Assets/Shaders/Renderer_Model.glsl";

	struct RendererInternalData
	{
		static constexpr inline uint32_t MaxTextureSlots = 32; // TODO: RendererCapabilities

		SharedRef<Texture2D> WhiteTexture; // Default texture

		SharedRef<Shader> ModelShader;

		std::array<SharedRef<Texture2D>, MaxTextureSlots> TextureSlots;
		uint32_t TextureSlotIndex = 1; // 0 = White Texture

		RenderStatistics RendererStatistics;
		RendererAPI::TriangleCullMode CullMode = RendererAPI::TriangleCullMode::None;
	};

	static RendererInternalData s_Data;

	void Renderer::Init()
	{
		SP_PROFILE_FUNCTION();

		RenderCommand::Init();

		s_Data.WhiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		s_Data.WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

		// Gpu Sampler2D Array
		int32_t samplers[s_Data.MaxTextureSlots];
		for (size_t i = 0; i < s_Data.MaxTextureSlots; i++)
			samplers[i] = i;

		s_Data.ModelShader = Shader::Create(MODEL_SHADER_PATH);
		s_Data.ModelShader->Enable();
		// Set the sampler2D array on the GPU
		s_Data.ModelShader->SetIntArray("u_Textures", samplers, RendererInternalData::MaxTextureSlots);

		// Set the first texture slot to out default white texture
		s_Data.TextureSlots[0] = s_Data.WhiteTexture;

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

	void Renderer::BeginScene(const Camera& camera, const Math::mat4& transform)
	{
		SP_PROFILE_FUNCTION();

		Math::mat4 viewProjection = camera.GetProjection() * Math::Inverse(transform);

		s_Data.ModelShader->Enable();
		s_Data.ModelShader->SetMat4("u_ViewProjection", viewProjection);

		s_Data.TextureSlotIndex = 1;
	}

	void Renderer::BeginScene(const EditorCamera& camera)
	{
		SP_PROFILE_FUNCTION();
		
		s_Data.ModelShader->Enable();
		s_Data.ModelShader->SetMat4("u_ViewProjection", camera.GetViewProjection());

		s_Data.TextureSlotIndex = 1;
	}

	void Renderer::EndScene()
	{

	}

	void Renderer::Submit(const SharedRef<Shader>& shader, const SharedRef<VertexArray>& vertexArray, const Math::mat4& transform)
	{
		SP_PROFILE_FUNCTION();

		shader->Enable();
		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}

	void Renderer::DrawModel(const EditorCamera& camera, const Math::mat4& transform, const MeshRendererComponent& meshRenderer, int entityID)
	{
		SP_PROFILE_FUNCTION();

		SharedRef<Model> model = meshRenderer.Mesh;

		model->OnUpdate(camera, transform, meshRenderer);

		if (meshRenderer.Texture)
			meshRenderer.Texture->Bind();
		else
			s_Data.WhiteTexture->Bind();

		RenderCommand::DrawIndexed(model->GetVertexArray());
		s_Data.RendererStatistics.DrawCalls++;

#if SP_RENDERER_STATISTICS
		s_Data.RendererStatistics.QuadCount += model->GetQuadCount();
#endif // SP_RENDERER_STATISTICS
	}

	void Renderer::DrawModel(const SceneCamera& camera, const Math::mat4& cameraTransform, const Math::mat4& transform, const MeshRendererComponent& meshRenderer, int entityID)
	{
		SP_PROFILE_FUNCTION();

		SharedRef<Model> model = meshRenderer.Mesh;

		model->OnUpdate(camera, transform, meshRenderer);

		if (meshRenderer.Texture)
			meshRenderer.Texture->Bind();
		else
			s_Data.WhiteTexture->Bind();

		RenderCommand::DrawIndexed(model->GetVertexArray());
		s_Data.RendererStatistics.DrawCalls++;

#if SP_RENDERER_STATISTICS
		s_Data.RendererStatistics.QuadCount += model->GetQuadCount();
#endif // SP_RENDERER_STATISTICS
	}

	void Renderer::DrawCubeWireframe(const TransformComponent& transform)
	{
		SP_PROFILE_FUNCTION();

		//Math::vec3 translation;
		//Math::vec3 rotation;
		//Math::vec3 scale;
		//Math::DecomposeTransform(transform.GetTransform(), translation, rotation, scale);

		Renderer2D::DrawRect(transform.GetTransform() * Math::Translate({ transform.Translation.x, transform.Translation.y + (transform.Scale.y / 2.0f), transform.Translation.z }) * Math::Rotate(Math::Deg2Rad(90.0f), { 1.0f, 0.0f, 0.0f }), ColorToVec4(Color::Orange));
		Renderer2D::DrawRect(transform.GetTransform() * Math::Translate({ transform.Translation.x, transform.Translation.y - (transform.Scale.y / 2.0f), transform.Translation.z }) * Math::Rotate(Math::Deg2Rad(90.0f), { 1.0f, 0.0f, 0.0f }), ColorToVec4(Color::Orange));

		Renderer2D::DrawRect(transform.GetTransform() * Math::Translate({ transform.Translation.x, transform.Translation.y, transform.Translation.z + (transform.Scale.z / 2.0f) }), ColorToVec4(Color::Orange));
		Renderer2D::DrawRect(transform.GetTransform() * Math::Translate({ transform.Translation.x, transform.Translation.y, transform.Translation.z - (transform.Scale.z / 2.0f) }), ColorToVec4(Color::Orange));
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

}
