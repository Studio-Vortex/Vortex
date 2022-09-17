#include "EditorLayer.h"

#include <Sparky/Scene/SceneSerializer.h>
#include <Sparky/Utils/PlatformUtils.h>

#include <ImGuizmo.h>

namespace Sparky {

	extern const std::filesystem::path g_AssetPath;

	EditorLayer::EditorLayer()
		: Layer("EditorLayer") { }

	void EditorLayer::OnAttach()
	{
		SP_PROFILE_FUNCTION();

		FramebufferProperties properties;
		properties.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth };
		properties.Width = 1600;
		properties.Height = 900;

		m_Framebuffer = Framebuffer::Create(properties);

		m_PlayIcon = Texture2D::Create("Resources/Icons/PlayButton.png");
		m_StopIcon = Texture2D::Create("Resources/Icons/StopButton.png");

		m_ActiveScene = CreateShared<Scene>();
		m_EditorCamera = EditorCamera(30.0f, 0.1778f, 0.1f, 1000.0f);

		m_SceneHierarchyPanel.SetContext(m_ActiveScene);

		CreateNewScene(); // Start the editor off with a fresh scene
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
			m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
			m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		}

		// Render
		Renderer2D::ResetStats();
		m_Framebuffer->Bind();
		RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f });
		RenderCommand::Clear();

		// Clear entityID attachment to -1
		m_Framebuffer->ClearAttachment(1, -1);

		// Update Scene
		switch (m_SceneState)
		{
			case Sparky::EditorLayer::SceneState::Edit:
			{
				// If the scene viewport is hovered or the mouse was moved moved since the last frame update the editor camera
				// this allows the user to manipulate the editor camera while they are holding the left mouse button even if the cursor is outside the scene viewport
				const Math::vec2& mousePos = Input::GetMousePosition();
				if (m_SceneViewportHovered || mousePos != m_MousePosLastFrame)
					m_EditorCamera.OnUpdate(delta);

				if (Input::IsKeyPressed(SP_KEY_ESCAPE))
					Application::Get().Close();

				m_ActiveScene->OnUpdateEditor(delta, m_EditorCamera);
				break;
			}
			case Sparky::EditorLayer::SceneState::Play:
			{
				m_ActiveScene->OnUpdateRuntime(delta);
				break;
			}
		}

		m_MousePosLastFrame = Input::GetMousePosition();

		auto [mx, my] = ImGui::GetMousePos();
		mx -= m_ViewportBounds[0].x;
		my -= m_ViewportBounds[0].y;
		glm::vec2 viewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];
		my = viewportSize.y - my;

		int mouseX = (int)mx;
		int mouseY = (int)my;
		
		if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x && mouseY < (int)viewportSize.y)
		{
			int pixelData = m_Framebuffer->ReadPixel(1, mouseX, mouseY);
			m_HoveredEntity = pixelData == -1 ? Entity() : Entity((entt::entity)pixelData, m_ActiveScene.get());
		}

		if (m_ShowPhysicsColliders)
			OnOverlayRender();

		m_Framebuffer->Unbind();
	}

	void EditorLayer::OnGuiRender()
	{
		SP_PROFILE_FUNCTION();

		static bool show = true;
		
		// Dockspace
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
			dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;

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
		float minWinSizeX = style.WindowMinSize.x;
		style.WindowMinSize.x = 370.0f;
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = Gui::GetID("MyDockSpace");
			Gui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		} // End Dockspace

		style.WindowMinSize.x = minWinSizeX;

		if (Gui::BeginMenuBar())
		{
			if (Gui::BeginMenu("File"))
			{
				if (Gui::MenuItem("New Scene", "Ctrl+N"))
					CreateNewScene();
				Gui::Separator();

				if (Gui::MenuItem("Open Scene...", "Ctrl+O"))
					OpenExistingScene();
				Gui::Separator();

				if (Gui::MenuItem("Save", "Ctrl+S"))
					SaveScene();
				Gui::Separator();

				if (Gui::MenuItem("Save As...", "Ctrl+Shift+S"))
					SaveSceneAs();
				Gui::Separator();

				if (Gui::MenuItem("Exit"))
					Application::Get().Close();

				Gui::EndMenu();
			}
			
			if (Gui::BeginMenu("Edit"))
			{
				if (m_SceneState == SceneState::Edit)
				{
					if (Gui::MenuItem("Play Scene", "Ctrl+P"))
						OnScenePlay();
				}
				else
				{
					if (Gui::MenuItem("Stop Scene", "Ctrl+P"))
						OnSceneStop();
					Gui::Separator();
					if (Gui::MenuItem("Restart Scene", "Ctrl+Shift+P"))
						RestartScene();
				}

				Gui::EndMenu();
			}

			if (Gui::BeginMenu("View"))
			{
				if (m_SceneViewportMaximized)
				{
					if (Gui::MenuItem("Minimize Scene", "Shift+F12"))
						m_SceneViewportMaximized = false;
				}
				else
				{
					if (Gui::MenuItem("Maximize Scene", "Shift+F12"))
						m_SceneViewportMaximized = true;
				}

				Gui::EndMenu();
			}

			if (Gui::BeginMenu("Tools"))
			{
				if (Gui::MenuItem("No Selection", "Q"))
					OnNoGizmoSelected();
				Gui::Separator();

				if (Gui::MenuItem("Translation Tool", "W"))
					OnTranslationToolSelected();
				Gui::Separator();

				if (Gui::MenuItem("Rotation Tool", "E"))
					OnRotationToolSelected();
				Gui::Separator();

				if (Gui::MenuItem("Scale Tool", "R"))
					OnScaleToolSelected();

				Gui::EndMenu();
			}

			if (Gui::BeginMenu("Window"))
			{
				if (Gui::MenuItem("Stats"))
					m_StatsPanel.ShowPanel();
				Gui::Separator();
				if (Gui::MenuItem("Settings"))
					m_SettingsPanel.ShowPanel();

				Gui::EndMenu();
			}
			
			if (Gui::BeginMenu("Help"))
			{
				if (Gui::MenuItem("About"))
					m_AboutPanel.ShowPanel();

				Gui::EndMenu();
			}

			Gui::EndMenuBar();
		}

		if (!m_SceneViewportMaximized)
		{
			m_SceneHierarchyPanel.OnGuiRender();
			m_ContentBrowserPanel.OnGuiRender();
			m_SettingsPanel.OnGuiRender(true);
			m_StatsPanel.OnGuiRender(m_HoveredEntity, true);
			m_AboutPanel.OnGuiRender();
		}

		Gui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0.0f, 0.0f });
		Gui::Begin("Scene");
		auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
		auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
		auto viewportOffset = ImGui::GetWindowPos();
		m_ViewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
		m_ViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };
		
		m_SceneViewportFocused = Gui::IsWindowFocused();
		m_SceneViewportHovered = Gui::IsWindowHovered();
		Application::Get().GetGuiLayer()->BlockEvents(!m_SceneViewportFocused && !m_SceneViewportHovered);

		ImVec2 scenePanelSize = Gui::GetContentRegionAvail();
		m_ViewportSize = { scenePanelSize.x, scenePanelSize.y };

		uint32_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
		Gui::Image(reinterpret_cast<void*>(textureID), ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

		// Accept a Scene from the content browser
		if (Gui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				const wchar_t* path = (const wchar_t*)payload->Data;
				OpenScene(std::filesystem::path(g_AssetPath) / path);
			}
			Gui::EndDragDropTarget();
		}

		// Render Gizmos
		Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
		if (selectedEntity && m_GizmoType != -1 && m_SceneState != SceneState::Play)
		{
			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();

			ImGuizmo::SetRect(m_ViewportBounds[0].x, m_ViewportBounds[0].y, m_ViewportBounds[1].x - m_ViewportBounds[0].x, m_ViewportBounds[1].y - m_ViewportBounds[0].y);

			// Camera
			//auto cameraEntity = m_ActiveScene->GetPrimaryCameraEntity();
			//const auto& camera = cameraEntity.GetComponent<CameraComponent>().Camera;
			//const Math::mat4& cameraProjection = camera.GetProjection();
			//Math::mat4 cameraView = Math::Inverse(cameraEntity.GetComponent<TransformComponent>().GetTransform());

			// Editor camera
			const Math::mat4& cameraProjection = m_EditorCamera.GetProjection();
			Math::mat4 cameraView = m_EditorCamera.GetViewMatrix();

			// Entity transform
			auto& tc = selectedEntity.GetComponent<TransformComponent>();
			Math::mat4 transform = tc.GetTransform();

			// Snapping
			bool snap = Input::IsKeyPressed(Key::LeftControl);
			// Snap to 45 degrees for rotation only, otherwise snap by half one unit
			float snapValue = m_GizmoType == ImGuizmo::ROTATE ? 45.0f : 0.5f;

			std::array<float, 3> snapValues{};
			snapValues.fill(snapValue);

			ImGuizmo::Manipulate(
				Math::ValuePtr(cameraView), Math::ValuePtr(cameraProjection),
				(ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::MODE::LOCAL, Math::ValuePtr(transform),
				nullptr, snap ? snapValues.data() : nullptr
			);

			if (ImGuizmo::IsUsing())
			{
				Math::vec3 translation, rotation, scale;
				Math::DecomposeTransform(transform, translation, rotation, scale);
				Math::vec3 deltaRotation = rotation - tc.Rotation;
				tc.Translation = translation;
				tc.Rotation += deltaRotation;
				tc.Scale = scale;
			}
		}

		Gui::End();
		Gui::PopStyleVar();

		UI_Toolbar();

		Gui::End();
	}

	void EditorLayer::UI_Toolbar()
	{
		Gui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0.0f, 2.0f });
		Gui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2{ 0.0f, 0.0f });
		Gui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.0f, 0.0f, 0.0f, 0.0f });
		auto& colors = Gui::GetStyle().Colors;
		const auto& buttonActive = colors[ImGuiCol_ButtonActive];
		Gui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ buttonActive.x, buttonActive.y, buttonActive.z, 0.5f });
		const auto& buttonHovered = colors[ImGuiCol_ButtonHovered];
		Gui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ buttonHovered.x, buttonHovered.y, buttonHovered.z, 0.5f });

		Gui::Begin("##toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

		SharedRef<Texture2D> icon = m_SceneState == SceneState::Edit ? m_PlayIcon : m_StopIcon;
		float size = Gui::GetWindowHeight() - 4.0f;
		Gui::SetCursorPosX((Gui::GetWindowContentRegionMax().x * 0.5f)  - (size * 0.5f));
		if (Gui::ImageButton(reinterpret_cast<void*>(icon->GetRendererID()), ImVec2{ size, size }, ImVec2{ 0, 0 }, ImVec2{ 1, 1 }, 0))
		{
			if (m_SceneState == SceneState::Edit)
				OnScenePlay();
			else if (m_SceneState == SceneState::Play)
				OnSceneStop();
		}

		Gui::PopStyleVar(2);
		Gui::PopStyleColor(3);

		Gui::End();
	}

	void EditorLayer::OnEvent(Event& e)
	{
		m_EditorCamera.OnEvent(e);

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<KeyPressedEvent>(SP_BIND_CALLBACK(EditorLayer::OnKeyPressedEvent));
		dispatcher.Dispatch<MouseButtonPressedEvent>(SP_BIND_CALLBACK(EditorLayer::OnMouseButtonPressedEvent));
	}

	bool EditorLayer::OnKeyPressedEvent(KeyPressedEvent& e)
	{
		bool controlPressed = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		bool shiftPressed = Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift);

		switch (e.GetKeyCode())
		{
			// File
			case Key::N:
			{
				if (controlPressed)
					CreateNewScene();

				break;
			}
			case Key::O:
			{
				if (controlPressed)
					OpenExistingScene();

				break;
			}
			case Key::S:
			{
				if (controlPressed)
				{
					if (shiftPressed)
						SaveSceneAs();
					else
						SaveScene();
				}

				break;
			}

			// Tools
			case Key::D:
			{
				if (controlPressed)
					DuplicateSelectedEntity();

				break;
			}

			case Key::Space:
			{
				static bool wireframe{};
				wireframe = !wireframe;

				if (wireframe)
					RenderCommand::SetWireframe(true);
				else
					RenderCommand::SetWireframe(false);

				break;
			}

			case Key::P:
			{
				if (controlPressed && shiftPressed)
				{
					if (m_SceneState == SceneState::Play)
						RestartScene();

					break;
				}

				if (controlPressed)
				{
					switch (m_SceneState)
					{
						case Sparky::EditorLayer::SceneState::Edit: OnScenePlay(); break;
						case Sparky::EditorLayer::SceneState::Play: OnSceneStop(); break;
					}

					break;
				}
			}

			case Key::F12:
			{
				if (shiftPressed)
					m_SceneViewportMaximized = !m_SceneViewportMaximized;

				break;
			}

			case Key::Q:
			{
				if (!ImGuizmo::IsUsing())
					OnNoGizmoSelected();

				break;
			}
			case Key::W:
			{
				if (!ImGuizmo::IsUsing())
					OnTranslationToolSelected();

				break;
			}
			case Key::E:
			{
				if (!ImGuizmo::IsUsing())
					OnRotationToolSelected();

				break;
			}
			case Key::R:
			{
				if (!ImGuizmo::IsUsing())
					OnScaleToolSelected();

				break;
			}
		}

		return false;
	}

	bool EditorLayer::OnMouseButtonPressedEvent(MouseButtonPressedEvent& e)
	{
		if (e.GetMouseButton() == Mouse::ButtonLeft)
		{
			if (m_SceneViewportHovered && !ImGuizmo::IsOver() && !Input::IsKeyPressed(Key::LeftAlt))
				m_SceneHierarchyPanel.SetSelectedEntity(m_HoveredEntity);
		}

		return false;
	}

	void EditorLayer::OnOverlayRender()
	{
		if (m_SceneState == SceneState::Play)
		{
			Entity cameraEntity = m_ActiveScene->GetPrimaryCameraEntity();
			Renderer2D::BeginScene(cameraEntity.GetComponent<CameraComponent>().Camera, cameraEntity.GetComponent<TransformComponent>().GetTransform());
		}
		else
		{
			Renderer2D::BeginScene(m_EditorCamera);
		}

		// Render Box Colliders
		{
			auto view = m_ActiveScene->GetAllEntitiesWith<TransformComponent, BoxCollider2DComponent>();
			for (auto entity : view)
			{
				auto [tc, bc2d] = view.get<TransformComponent, BoxCollider2DComponent>(entity);

				Math::vec3 translation = tc.Translation + Math::vec3(bc2d.Offset, 0.001f);
				Math::vec3 scale = tc.Scale * Math::vec3(bc2d.Size * 2.0f, 1.0f);

				Math::mat4 transform = Math::Translate(translation) * Math::Rotate(tc.Rotation.z, { 0.0f, 0.0f, 1.0f }) * Math::Scale(scale);

				Renderer2D::DrawRect(transform, ColorToVec4(Color::Green));
			}
		}

		// Render Circle Colliders
		{
			auto view = m_ActiveScene->GetAllEntitiesWith<TransformComponent, CircleCollider2DComponent>();
			for (auto entity : view)
			{
				auto [tc, cc2d] = view.get<TransformComponent, CircleCollider2DComponent>(entity);

				Math::vec3 translation = tc.Translation + Math::vec3(cc2d.Offset, 0.001f);
				Math::vec3 scale = tc.Scale * Math::vec3(cc2d.Radius * 2.0f);

				Math::mat4 transform = Math::Translate(translation) * Math::Scale(scale);

				Renderer2D::DrawCircle(transform, ColorToVec4(Color::Green), 0.01f);
			}
		}

		Renderer2D::EndScene();
	}

	void EditorLayer::CreateNewScene()
	{
		m_ActiveScene = CreateShared<Scene>();
		m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);

		m_EditorScenePath = std::filesystem::path(); // Reset the current scene path otherwise the previous scene will be overwritten
		m_EditorScene = m_ActiveScene; // Set the editors scene
	}

	void EditorLayer::OpenExistingScene()
	{
		std::string filepath = FileSystem::OpenFile("Sparky Scene (*.sparky)\0*.sparky\0");

		if (!filepath.empty())
			OpenScene(filepath);
	}

	void EditorLayer::OpenScene(const std::filesystem::path& path)
	{
		if (m_SceneState != SceneState::Edit)
			OnSceneStop();

		if (path.extension().string() != ".sparky")
		{
			SP_WARN("Could not load {} - not a scene file", path.filename().string());
			return;
		}

		SharedRef<Scene> newScene = CreateShared<Scene>();
		newScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		SceneSerializer serializer(newScene);
		if (serializer.Deserialize(path.string()))
		{
			m_EditorScene = newScene;
			m_SceneHierarchyPanel.SetContext(m_EditorScene);

			m_ActiveScene = m_EditorScene;
			m_EditorScenePath = path;
		}
	}

	void EditorLayer::SaveSceneAs()
	{
		std::string filepath = FileSystem::SaveFile("Sparky Scene (*.sparky)\0*.sparky\0");

		if (!filepath.empty())
		{
			m_EditorScenePath = filepath;
			
			SerializeScene(m_ActiveScene, m_EditorScenePath);
		}
	}

	void EditorLayer::SaveScene()
	{
		if (!m_EditorScenePath.empty())
			SerializeScene(m_ActiveScene, m_EditorScenePath);
		else
			SaveSceneAs();
	}

	void EditorLayer::SerializeScene(SharedRef<Scene> scene, const std::filesystem::path& path)
	{
		SceneSerializer serializer(scene);
		serializer.Serialize(path.string());
	}

	void EditorLayer::OnScenePlay()
	{
		if (!m_EditorScene)
			return;

		m_SceneState = SceneState::Play;

		m_ActiveScene = Scene::Copy(m_EditorScene);
		m_ActiveScene->OnRuntimeStart();

		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
	}

	void EditorLayer::OnSceneStop()
	{
		m_SceneState = SceneState::Edit;

		m_ActiveScene->OnRuntimeStop();
		m_ActiveScene = m_EditorScene;

		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
	}

	void EditorLayer::RestartScene()
	{
		OnScenePlay();
	}

	void EditorLayer::DuplicateSelectedEntity()
	{
		if (m_SceneState != SceneState::Edit)
			return;

		Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();

		if (selectedEntity)
		{
			Entity duplicatedEntity = m_ActiveScene->DuplicateEntity(selectedEntity);
			m_SceneHierarchyPanel.SetSelectedEntity(duplicatedEntity);
		}
	}

	void EditorLayer::OnNoGizmoSelected()
	{
		m_GizmoType = -1; // Invalid gizmo
	}

	void EditorLayer::OnTranslationToolSelected()
	{
		m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
	}

	void EditorLayer::OnRotationToolSelected()
	{
		m_GizmoType = ImGuizmo::OPERATION::ROTATE;
	}

	void EditorLayer::OnScaleToolSelected()
	{
		m_GizmoType = ImGuizmo::OPERATION::SCALE;
	}

}