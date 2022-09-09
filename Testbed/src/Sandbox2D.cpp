#include "Sandbox2D.h"

Sandbox2D::Sandbox2D() :
	Layer("Sandbox2D"), m_CameraController(1280.0f / 720.0f, true),
	m_SquareColor(Sparky::ColorToVec4(Sparky::Color::LightGreen)),
	m_GridColor(Sparky::ColorToVec4(Sparky::Color::LightBlue)), m_GridScale(5)
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
	SP_PROFILE_FUNCTION();

	// Update
	{
		SP_PROFILE_SCOPE("CameraController::OnUpdate");
		m_CameraController.OnUpdate(ts);
	}

	if (Sparky::Input::IsKeyPressed(SP_KEY_ESCAPE))
		Sparky::Application::Get().CloseApplication();

	// Render
	{
		SP_PROFILE_SCOPE("Renderer Prep");
		Sparky::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f });
		Sparky::RenderCommand::Clear();
	}

	{
		SP_PROFILE_SCOPE("Renderer Draw");
		Sparky::Renderer2D::BeginScene(m_CameraController.GetCamera());
		Sparky::Renderer2D::DrawQuad(Math::vec3(0.0f, 0.0f, 0.1f), Math::vec2(1.0f), m_SquareColor);
		Sparky::Renderer2D::DrawQuad(Math::vec3(1.5f, 0.5f, 0.1f), Math::vec2(1.0f, 2.0f), Sparky::Color::LightRed);
		Sparky::Renderer2D::DrawQuad(Math::vec3(0.0f), Math::vec2(10.0f), m_GridTexture, m_GridColor, m_GridScale);
		Sparky::Renderer2D::EndScene();
	}
}

void Sandbox2D::OnGuiRender()
{
	SP_PROFILE_FUNCTION();

	static bool show = true;
	Gui::Begin("Settings", &show);
	Gui::ColorEdit4("Grid Color", Math::ValuePtr(m_GridColor));
	Gui::SliderInt("Grid Scale", &m_GridScale, 1, 20);
	Gui::ColorEdit4("Square Color", Math::ValuePtr(m_SquareColor));
	Gui::End();
}

void Sandbox2D::OnEvent(Sparky::Event& e)
{
	m_CameraController.OnEvent(e);
}