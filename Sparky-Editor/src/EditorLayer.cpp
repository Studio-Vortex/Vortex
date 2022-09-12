#include "EditorLayer.h"

namespace Sparky {

	EditorLayer::EditorLayer() :
		Layer("EditorLayer"), m_CameraController(1280.0f / 720.0f, true),
		m_SquareColor(ColorToVec4(Color::LightBlue)), m_GridColor(ColorToVec4(Color::White)),
		m_GridScale(5) { }

	void EditorLayer::OnAttach()
	{
		SP_PROFILE_FUNCTION();

		FramebufferProperties properties;
		properties.Width = 1280.0f;
		properties.Height = 720.0f;

		m_Framebuffer = Framebuffer::Create(properties);

		m_GridTexture = Texture2D::Create("assets/textures/Checkerboard.png");

		m_ActiveScene = CreateShared<Scene>();

		m_SquareEntity = m_ActiveScene->CreateEntity("Square");
		m_SquareEntity.AddComponent<Sprite2DComponent>(ColorToVec4(Color::Purple));

		m_CameraEntity = m_ActiveScene->CreateEntity("Camera Entity");
		m_CameraEntity.AddComponent<CameraComponent>();

		m_SecondCamera = m_ActiveScene->CreateEntity("Clip-Space Camera");
		auto& cc = m_SecondCamera.AddComponent<CameraComponent>();
		cc.Primary = false;

		class CameraController : public ScriptableEntity
		{
		public:
			void OnCreate() { }

			void OnUpdate(TimeStep delta)
			{
				auto& transform = GetComponent<TransformComponent>().Transform;
				float speed = 5.0f;

				if (Input::IsKeyPressed(Key::W))
					transform[3][1] += speed * delta;
				if (Input::IsKeyPressed(Key::A))
					transform[3][0] -= speed * delta;
				if (Input::IsKeyPressed(Key::S))
					transform[3][1] -= speed * delta;
				if (Input::IsKeyPressed(Key::D))
					transform[3][0] += speed * delta;
			}

			void OnDestroy() { }
		};

		m_CameraEntity.AddComponent<NativeScriptComponent>().Bind<CameraController>();
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
					Application::Get().Close();
				Gui::EndMenu();
			}

			Gui::EndMenuBar();
		}

		Gui::Begin("Inspector", &show);
		Gui::ColorEdit4("Grid Color", Math::ValuePtr(m_GridColor));
		Gui::SliderFloat("Grid Scale", &m_GridScale, 1, 20, "%.2f");
		Gui::SliderFloat3("Quad Position", Math::ValuePtr(m_RotatedQuadPos), -5.0f, 5.0f, "%.2f");
		Gui::SliderFloat("Quad Rotation Speed", &m_RotatedQuadRotationSpeed, -150.0f, 150.0f, "%.2f");

		if (m_SquareEntity)
		{
			Gui::Separator();
			auto& tag = m_SquareEntity.GetComponent<TagComponent>().Tag;
			Gui::Text("%s", tag.c_str());
			auto& sprite = m_SquareEntity.GetComponent<Sprite2DComponent>();
			Gui::ColorEdit4("Square Color", Math::ValuePtr(sprite.SpriteColor));
			Gui::Separator();
		}

		if (Gui::Checkbox("Primary Camera", &m_PrimaryCamera))
		{
			m_CameraEntity.GetComponent<CameraComponent>().Primary = m_PrimaryCamera;
			m_SecondCamera.GetComponent<CameraComponent>().Primary = !m_PrimaryCamera;
		}

		{
			auto& camera = m_SecondCamera.GetComponent<CameraComponent>().Camera;
			static float size = camera.GetOrthographicSize();
			if (Gui::DragFloat("Second Camera Ortho Size", &size, 1.0f, 1.0f, 50.0f))
				camera.SetOrthographicSize(size);
		}

		Gui::DragFloat3("Camera Transform", Math::ValuePtr(m_CameraEntity.GetComponent<TransformComponent>().Transform[3]));

		auto stats = Renderer2D::GetStats();

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

			if (event.GetKeyCode() == SP_KEY_SPACE)
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