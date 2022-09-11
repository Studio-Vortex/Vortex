#include "EditorLayer.h"

namespace Sparky {

	EditorLayer::EditorLayer() :
		Layer("EditorLayer"), m_CameraController(1280.0f / 720.0f, true),
		m_SquareColor(Sparky::ColorToVec4(Sparky::Color::LightBlue)),
		m_GridColor(Sparky::ColorToVec4(Sparky::Color::White)), m_GridScale(5) { }

	void EditorLayer::OnAttach()
	{
		SP_PROFILE_FUNCTION();

		m_GridTexture = Sparky::Texture2D::Create("assets/textures/Checkerboard.png");

		Sparky::FramebufferProperties properties;
		properties.Width = 1280.0f;
		properties.Height = 720.0f;

		m_Framebuffer = Sparky::Framebuffer::Create(properties);
	}

	void EditorLayer::OnDetach() { }

	void EditorLayer::OnUpdate(Sparky::TimeStep delta)
	{
		SP_PROFILE_FUNCTION();

		// Update
		if (m_ViewportFocused)
			m_CameraController.OnUpdate(delta);

		if (Sparky::Input::IsKeyPressed(SP_KEY_ESCAPE))
			Sparky::Application::Get().Close();

		// Render
		// Reset stats here
		Sparky::Renderer2D::ResetStats();

		{
			SP_PROFILE_SCOPE("Renderer Prep");
			m_Framebuffer->Bind();
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

			for (float y = -5.0f; y < 5.0f; y += 0.5f)
			{
				for (float x = -5.0f; x < 5.0f; x += 0.5f)
				{
					Math::vec4 color((x + 0.5f) / 10.0f, 0.4f, (y + 0.5f) / 10.0f, 0.7f);
					Sparky::Renderer2D::DrawQuad({ x, y }, Math::vec2(0.45f), color);
				}
			}

			Sparky::Renderer2D::EndScene();

			m_Framebuffer->Unbind();
		}
	}

	void EditorLayer::OnGuiRender()
	{
		SP_PROFILE_FUNCTION();

		static bool show = true;
		static bool dockspaceOpen = true;
		static bool opt_fullscreen = true;
		static bool opt_padding = false;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			const ImGuiViewport* viewport = Gui::GetMainViewport();
			Gui::SetNextWindowPos(viewport->WorkPos);
			Gui::SetNextWindowSize(viewport->WorkSize);
			Gui::SetNextWindowViewport(viewport->ID);
			Gui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			Gui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}
		else
		{
			dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
		}

		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		if (!opt_padding)
			Gui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		Gui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
		if (!opt_padding)
			Gui::PopStyleVar();

		if (opt_fullscreen)
			Gui::PopStyleVar(2);

		ImGuiIO& io = Gui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = Gui::GetID("MyDockSpace");
			Gui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		if (Gui::BeginMenuBar())
		{
			if (Gui::BeginMenu("File"))
			{
				if (Gui::MenuItem("Exit"))
					Sparky::Application::Get().Close();
				Gui::EndMenu();
			}

			Gui::EndMenuBar();
		}

		Gui::Begin("Inspector", &show);
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
		Gui::End();

		Gui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0.0f, 0.0f });
		Gui::Begin("Scene");
		
		m_ViewportFocused = Gui::IsWindowFocused();
		m_ViewportHovered = Gui::IsWindowHovered();
		Application::Get().GetGuiLayer()->BlockEvents(!m_ViewportFocused || !m_ViewportHovered);

		ImVec2 scenePanelSize = Gui::GetContentRegionAvail();
		if (m_ViewportSize != *((Math::vec2*)&scenePanelSize) && scenePanelSize.x > 0 && scenePanelSize.y > 0)
		{
			m_ViewportSize = { scenePanelSize.x, scenePanelSize.y };
			m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);

			m_CameraController.OnResize(m_ViewportSize);
		}

		uint32_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
		Gui::Image((void*)textureID, { m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
		Gui::End();
		Gui::PopStyleVar();

		Gui::End();
	}

	void EditorLayer::OnEvent(Sparky::Event& e)
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

}