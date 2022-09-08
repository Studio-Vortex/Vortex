#include "sppch.h"
#include "Renderer.h"

#include "Sparky/Core/Core.h"

#include "Platform/OpenGL/OpenGLShader.h"

namespace Sparky {

	Renderer::SceneData* Renderer::s_SceneData = new Renderer::SceneData();

    void Renderer::Init()
    {
		RenderCommand::Init();
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
		std::dynamic_pointer_cast<OpenGLShader>(shader)->SetUniform("u_ViewProjection", s_SceneData->ViewProjectionMatrix);
		std::dynamic_pointer_cast<OpenGLShader>(shader)->SetUniform("u_Transform", transform);

		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}

}