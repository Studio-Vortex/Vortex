#include "Sandbox2D.h"

#include <Platform/OpenGL/OpenGLShader.h>

Sandbox2D::Sandbox2D()
	: Layer("Sandbox2D"), m_CameraController(1280.0f / 720.0f, true)
{
}

void Sandbox2D::OnAttach()
{

}

void Sandbox2D::OnDetach()
{
	Sparky::Renderer2D::Shutdown();
}

void Sandbox2D::OnUpdate(Sparky::TimeStep ts)
{
	// Update
	if (Sparky::Input::IsKeyPressed(SP_KEY_ESCAPE))
		Sparky::Application::Get().CloseApplication();

	m_CameraController.OnUpdate(ts);

	// Render
	Sparky::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f });
	Sparky::RenderCommand::Clear();

	Sparky::Renderer2D::BeginScene(m_CameraController.GetCamera());

	Sparky::Renderer2D::DrawQuad({ 0.0f, 0.0f }, { 1.0f, 1.0f }, m_SquareColor);

	Sparky::Renderer2D::EndScene();

	// TODO: Add these functions Shader::SetMat4, Shader::SetFloat4
	//std::dynamic_pointer_cast<Sparky::OpenGLShader>(m_FlatColorShader)->Enable();
	//std::dynamic_pointer_cast<Sparky::OpenGLShader>(m_FlatColorShader)->SetUniform("u_Color", m_SquareColor);
}

void Sandbox2D::OnGuiRender()
{
	Gui::Begin("Settings");
	Gui::ColorEdit4("Square Color", Sparky::Math::ValuePtr(m_SquareColor));
	Gui::End();
}

void Sandbox2D::OnEvent(Sparky::Event& e)
{
	m_CameraController.OnEvent(e);
}
