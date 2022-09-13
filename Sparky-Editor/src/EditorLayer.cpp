#include "EditorLayer.h"

namespace Sparky {

	EditorLayer::EditorLayer() :
		Layer("EditorLayer"), m_CameraController(1280.0f / 720.0f, true) { }

	void EditorLayer::OnAttach()
	{
		SP_PROFILE_FUNCTION();

		FramebufferProperties properties;
		properties.Width = 1280.0f;
		properties.Height = 720.0f;

		m_Framebuffer = Framebuffer::Create(properties);

		m_ActiveScene = CreateShared<Scene>();

		m_SquareEntity = m_ActiveScene->CreateEntity("Green Square");
		m_SquareEntity.AddComponent<SpriteComponent>(ColorToVec4(Color::Green));

		m_CameraEntity = m_ActiveScene->CreateEntity("Camera A");
		m_CameraEntity.AddComponent<CameraComponent>();

		m_SecondCamera = m_ActiveScene->CreateEntity("Camera B");
		auto& cc = m_SecondCamera.AddComponent<CameraComponent>();
		cc.Primary = false;

		auto redSquare = m_ActiveScene->CreateEntity("Red Square");
		redSquare.AddComponent<SpriteComponent>(ColorToVec4(Color::LightRed));

		class CameraController : public ScriptableEntity
		{
		public:
			void OnUpdate(TimeStep delta)
			{
				auto& translation = GetComponent<TransformComponent>().Translation;
				float speed = 5.0f;

				if (Input::IsKeyPressed(Key::W))
					translation.y += speed * delta;
				if (Input::IsKeyPressed(Key::A))
					translation.x -= speed * delta;
				if (Input::IsKeyPressed(Key::S))
					translation.y -= speed * delta;
				if (Input::IsKeyPressed(Key::D))
					translation.x += speed * delta;
			}
		};

		m_SecondCamera.AddComponent<NativeScriptComponent>().Bind<CameraController>();
		m_CameraEntity.AddComponent<NativeScriptComponent>().Bind<CameraController>();

		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
	}

	void EditorLayer::OnDetach() { }

	void EditorLayer::OnUpdate(TimeStep delta)
	{
		SP_PROFILE_FUNCTION();

		// Resize
		if (FramebufferProperties props = m_Framebuffer->GetProperties();
			m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f && // zero sized framebuffer is invalid
			(props.Width != m_ViewportSize.x || props.Height != m_ViewportSize.y))
		{
			m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			m_CameraController.OnResize(m_ViewportSize);

			m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		}

		// Update
		if (m_ViewportFocused)
			m_CameraController.OnUpdate(delta);

		if (Input::IsKeyPressed(SP_KEY_ESCAPE))
			Application::Get().Close();

		// Render
		Renderer2D::ResetStats();
		m_Framebuffer->Bind();
		RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f });
		RenderCommand::Clear();

		// Update Scene
		m_ActiveScene->OnUpdate(delta);

		m_Framebuffer->Unbind();
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
		ImGuiStyle& style = Gui::GetStyle();
		float minWinSize = style.WindowMinSize.x;
		style.WindowMinSize.x = 370.0f;
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = Gui::GetID("MyDockSpace");
			Gui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		style.WindowMinSize.x = minWinSize;

		if (Gui::BeginMenuBar())
		{
			if (Gui::BeginMenu("File"))
			{
				if (Gui::MenuItem("Exit"))
					Application::Get().Close();
				Gui::EndMenu();
			}

			Gui::EndMenuBar();
		}

		m_SceneHierarchyPanel.OnGuiRender();

		auto stats = Renderer2D::GetStats();

		Gui::Begin("Stats", &show);
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
		m_ViewportSize = { scenePanelSize.x, scenePanelSize.y };

		uint32_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
		Gui::Image((void*)textureID, { m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
		Gui::End();
		Gui::PopStyleVar();

		Gui::End();
	}

	void EditorLayer::OnEvent(Event& e)
	{
		m_CameraController.OnEvent(e);

		if (e.GetEventType() == EventType::KeyPressed)
		{
			static bool wireframe{};

			auto& event = (KeyPressedEvent&)e;

			if (event.GetKeyCode() == Key::Space)
			{
				wireframe = !wireframe;

				if (wireframe)
					RenderCommand::SetWireframe(true);
				else
					RenderCommand::SetWireframe(false);
			}
		}
	}

}