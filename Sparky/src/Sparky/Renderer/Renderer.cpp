#include "sppch.h"
#include "Renderer.h"

#include "Sparky/Core/Core.h"
#include "Sparky/Renderer/Renderer2D.h"

namespace Sparky {

	Renderer::SceneData* Renderer::s_SceneData = new Renderer::SceneData();

    void Renderer::Init()
    {
		SP_PROFILE_FUNCTION();

		RenderCommand::Init();
		Renderer2D::Init();
    }

	void Renderer::Shutdown()
	{
		Renderer2D::Shutdown();
	}

    void Renderer::OnWindowResize(const Viewport& viewport)
    {
		RenderCommand::SetViewport(viewport);
    }

    void Renderer::BeginScene(const OrthographicCamera& camera)
	{
		s_SceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix();
	}

	void Renderer::EndScene()
	{

	}

	void Renderer::Submit(const SharedRef<Shader>& shader, const SharedRef<VertexArray>& vertexArray, const Math::mat4& transform)
	{
		shader->Enable();
		shader->SetMat4("u_ViewProjection", s_SceneData->ViewProjectionMatrix);
		shader->SetMat4("u_Transform", transform);

		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}

}