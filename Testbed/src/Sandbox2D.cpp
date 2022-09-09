#include "Sandbox2D.h"

Sandbox2D::Sandbox2D()
	: Layer("Sandbox2D"), m_CameraController(1280.0f / 720.0f, true) { }

void Sandbox2D::OnAttach() { }

void Sandbox2D::OnDetach()
{
	Sparky::Renderer2D::Shutdown();
}

void Sandbox2D::OnUpdate(Sparky::TimeStep ts)
{
	// Update
	m_CameraController.OnUpdate(ts);

	if (Sparky::Input::IsKeyPressed(SP_KEY_ESCAPE))
		Sparky::Application::Get().CloseApplication();

	// Render
	Sparky::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f });
	Sparky::RenderCommand::Clear();

	Sparky::Renderer2D::BeginScene(m_CameraController.GetCamera());

	Sparky::Renderer2D::DrawQuad(Math::vec3(), {1.0f, 1.0f}, m_SquareColor);
	Sparky::Renderer2D::DrawQuad({ -1.0f, 1.0f }, { 3.0f, 6.0f }, { 1.0, 0.4, 0.2 });

	Sparky::Renderer2D::EndScene();
}

void Sandbox2D::OnGuiRender()
{
	Gui::Begin("Settings");
	Gui::ColorEdit4("Square Color", Math::ValuePtr(m_SquareColor));
	Gui::End();
}

void Sandbox2D::OnEvent(Sparky::Event& e)
{
	m_CameraController.OnEvent(e);
}
