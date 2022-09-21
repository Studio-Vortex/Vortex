#include "Sandbox2D.h"

Sandbox2D::Sandbox2D() :
	Layer("Sandbox2D"), m_CameraController(1280.0f / 720.0f, true),
	m_SquareColor(Sparky::ColorToVec4(Sparky::Color::LightBlue)),
	m_GridColor(Sparky::ColorToVec4(Sparky::Color::White)), m_GridScale(5) { }

void Sandbox2D::OnAttach()
{
	SP_PROFILE_FUNCTION();

	m_GridTexture = Sparky::Texture2D::Create("assets/textures/Checkerboard.png");
}

void Sandbox2D::OnDetach() { }

void Sandbox2D::OnUpdate(Sparky::TimeStep delta)
{
	SP_PROFILE_FUNCTION();

	// Update
	m_CameraController.OnUpdate(delta);

	if (Sparky::Input::IsKeyPressed(Sparky::Key::Escape))
		Sparky::Application::Get().Close();

	// Render
	// Reset stats here
	Sparky::Renderer2D::ResetStats();

	{
		SP_PROFILE_SCOPE("Renderer Prep");
		Sparky::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f });
		Sparky::RenderCommand::Clear();
	}

	{
		SP_PROFILE_SCOPE("Renderer Draw");
		Sparky::Renderer2D::BeginScene(m_CameraController.GetCamera());

		Sparky::Renderer2D::DrawQuad(Math::vec2(), Math::vec2(1.0f), m_SquareColor);
		Sparky::Renderer2D::DrawQuad(Math::vec2(1.5f), Math::vec2(2.0f, 1.0f), Sparky::Color::Purple);
		Sparky::Renderer2D::DrawRotatedQuad(m_RotatedQuadPos, Math::vec2(2.0f), Math::Deg2Rad(m_RotatedQuadRotation -= m_RotatedQuadRotationSpeed * delta), Sparky::Color::LightYellow);
		Sparky::Renderer2D::DrawRotatedQuad({ -2.0f, 2.0f }, Math::vec2(2.0f), Math::Deg2Rad(45.0f), m_GridTexture, 1.0f);
		Sparky::Renderer2D::DrawQuad({ 0.0f, 0.0f, -0.1f }, Math::vec2(20.0f), m_GridTexture, m_GridScale, m_GridColor);
		
		for (float y = -5.0f; y < 5.0f; y += 0.5f)
		{
			for (float x = -5.0f; x < 5.0f; x += 0.5f)
			{
				Math::vec4 color((x + 0.5f) / 10.0f, 0.4f, (y + 0.5f) / 10.0f, 0.7f);
				Sparky::Renderer2D::DrawQuad({ x, y }, Math::vec2(0.45f), color);
			}
		}
		
		Sparky::Renderer2D::EndScene();
	}
}

void Sandbox2D::OnGuiRender()
{
	SP_PROFILE_FUNCTION();

	static bool show = true;
	Gui::Begin("Settings", &show);
	Gui::ColorEdit4("Grid Color", Math::ValuePtr(m_GridColor));
	Gui::SliderFloat("Grid Scale", &m_GridScale, 1, 20, "%.2f");
	Gui::SliderFloat3("Quad Position", Math::ValuePtr(m_RotatedQuadPos), -5.0f, 5.0f, "%.2f");
	Gui::SliderFloat("Quad Rotation Speed", &m_RotatedQuadRotationSpeed, -150.0f, 150.0f, "%.2f");
	Gui::ColorEdit4("Square Color", Math::ValuePtr(m_SquareColor));

	auto stats = Sparky::Renderer2D::GetStats();

	Gui::Text("Renderer2D Stats:");
	Gui::Text("Draw Calls: %i", stats.DrawCalls);
	Gui::Text("Quads:      %i", stats.QuadCount);
	Gui::Text("Triangles:  %i", stats.GetTriangleCount());
	Gui::Text("Vertices:   %i", stats.GetVertexCount());
	Gui::Text("Indices:    %i", stats.GetIndexCount());
	ImGui::End();
}

void Sandbox2D::OnEvent(Sparky::Event& e)
{
	m_CameraController.OnEvent(e);

	if (e.GetEventType() == Sparky::EventType::KeyPressed)
	{
		static bool wireframe{};

		auto& event = (Sparky::KeyPressedEvent&)e;

		if (event.GetKeyCode() == Sparky::Key::Space)
		{
			wireframe = !wireframe;
			
			if (wireframe)
				Sparky::RenderCommand::SetWireframe(true);
			else
				Sparky::RenderCommand::SetWireframe(false);
		}
	}
}
