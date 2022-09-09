#include "Sandbox2D.h"

Sandbox2D::Sandbox2D()
	: Layer("Sandbox2D"), m_CameraController(1280.0f / 720.0f, true), m_SquareColor(Sparky::ColorToVec4(Sparky::Color::LightGreen))
{ }

void Sandbox2D::OnAttach()
{
	m_GridTexture = Sparky::Texture2D::Create("assets/textures/Checkerboard.png");
}

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

	Sparky::Renderer2D::DrawQuad(Math::vec3(0.0f, 0.0f, 0.1f), Math::vec2(1.0f), m_SquareColor);
	Sparky::Renderer2D::DrawQuad(Math::vec3(0.0f, 0.0f, 0.0f), Math::vec2(10.0f), m_GridTexture, 0.0f, 5, Sparky::ColorToVec4(Sparky::Color::LightBlue));

	Sparky::Renderer2D::EndScene();
}

void Sandbox2D::OnGuiRender()
{
	bool show = true;
	Gui::Begin("Settings", &show);
	Gui::ColorEdit4("Square Color", Math::ValuePtr(m_SquareColor));
	Gui::End();
}

void Sandbox2D::OnEvent(Sparky::Event& e)
{
	m_CameraController.OnEvent(e);
}
