#include "Sandbox2D.h"


static constexpr uint32_t s_MapWidth = 24;
static constexpr const char* s_MapTiles =
"WWWWWWWWWWWWWWWWWWWWWWWW"
"WWWWWWWWDDDDDDDDDWWWWWWW"
"WWWWWWDDDDDDDDDDDDDWWWWW"
"WWWWWDDDDDDDDDDDDDDDWWWW"
"WWWWDDDDDDDWDDDDDDDDDWWW"
"WWWDDDDDDWWWWDDDDDWDDDWW"
"WWDDDDDDDDWWWDDDDWWWDDDW"
"WWDDDDDDDDDWDDDDDWWDDDDW"
"WWWDDDDDDDDDDDDDDDDDDDWW"
"WWWWDDDDDDDDWWWDDDDDDWWW"
"WWWWWDDDDDDDDWWDDDDDWWWW"
"WWWWWWWDDDDDDDDDDDWWWWWW"
"WWWWWWWWWWWWWWWWWWWWWWWW";


Sandbox2D::Sandbox2D() :
	Layer("Sandbox2D"), m_CameraController(1280.0f / 720.0f, true),
	m_SquareColor(Sparky::ColorToVec4(Sparky::Color::LightBlue)),
	m_GridColor(Sparky::ColorToVec4(Sparky::Color::White)), m_GridScale(5) { }

void Sandbox2D::OnAttach()
{
	SP_PROFILE_FUNCTION();

	m_GridTexture = Sparky::Texture2D::Create("assets/textures/Checkerboard.png");
	m_SpriteSheet = Sparky::Texture2D::Create("assets/game/textures/RPGpack_sheet_2X.png");

	m_Stairs = Sparky::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0, 11 }, m_SpriteSize);
	m_Tree = Sparky::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 2, 1 }, m_SpriteSize, {1, 2});

	m_MapWidth = s_MapWidth;
	m_MapHeight = strlen(s_MapTiles) / s_MapWidth;

	m_TextureMap['D'] = Sparky::SubTexture2D::CreateFromCoords(m_SpriteSheet, {6, 11}, m_SpriteSize);
	m_TextureMap['W'] = Sparky::SubTexture2D::CreateFromCoords(m_SpriteSheet, {11, 11}, m_SpriteSize);

	m_CameraController.SetZoomLevel(5.0f);
}

void Sandbox2D::OnDetach() { }

void Sandbox2D::OnUpdate(Sparky::TimeStep delta)
{
	SP_PROFILE_FUNCTION();

	// Update
	m_CameraController.OnUpdate(delta);

	if (Sparky::Input::IsKeyPressed(SP_KEY_ESCAPE))
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
		Sparky::Renderer2D::DrawRotatedQuad(m_RotatedQuadPos, Math::vec2(2.0f), Math::Deg2Rad(m_RotatedQuadRotation += -m_RotatedQuadRotationSpeed * delta), Sparky::Color::LightYellow);
		Sparky::Renderer2D::DrawRotatedQuad({ -2.0f, 2.0f }, Math::vec2(2.0f), Math::Deg2Rad(45.0f), 1.0f, m_GridTexture);
		Sparky::Renderer2D::DrawQuad({ 0.0f, 0.0f, -0.1f }, Math::vec2(20.0f), m_GridScale, m_GridTexture, m_GridColor);
		Sparky::Renderer2D::EndScene();

		Sparky::Renderer2D::BeginScene(m_CameraController.GetCamera());
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
	static bool dockspaceOpen = true;
	static bool opt_fullscreen = true;
	static bool opt_padding = false;
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

	// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
	// because it would be confusing to have two docking targets within each others.
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	if (opt_fullscreen)
	{
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	}
	else
	{
		dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
	}

	// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
	// and handle the pass-thru hole, so we ask Begin() to not render a background.
	if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
		window_flags |= ImGuiWindowFlags_NoBackground;

	// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
	// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
	// all active windows docked into it will lose their parent and become undocked.
	// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
	// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
	if (!opt_padding)
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
	if (!opt_padding)
		ImGui::PopStyleVar();

	if (opt_fullscreen)
		ImGui::PopStyleVar(2);

	// Submit the DockSpace
	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
	{
		ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
	}

	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit"))
				Sparky::Application::Get().Close();
			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}

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

	uint32_t textureID = m_GridTexture->GetRendererID();
	Gui::Image((void*)textureID, ImVec2{ 256, 256 });
	Gui::End();

	ImGui::End();
}

void Sandbox2D::OnEvent(Sparky::Event& e)
{
	m_CameraController.OnEvent(e);

	if (e.GetEventType() == Sparky::EventType::KeyPressed)
	{
		static bool wireframe{};

		auto& event = (Sparky::KeyPressedEvent&)e;

		if (event.GetKeyCode() == SP_KEY_SPACE)
		{
			wireframe = !wireframe;
			
			if (wireframe)
				Sparky::RenderCommand::SetWireframe(true);
			else
				Sparky::RenderCommand::SetWireframe(false);
		}
	}
}
