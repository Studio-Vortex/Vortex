#include "EditorLayer.h"

#include <Sparky/Scene/SceneSerializer.h>
#include <Sparky/Scripting/ScriptEngine.h>
#include <Sparky/Utils/PlatformUtils.h>
#include <Sparky/Renderer/RenderCommand.h>

#include <ImGuizmo.h>

namespace Sparky {

	extern const std::filesystem::path g_AssetPath;

	EditorLayer::EditorLayer()
		: Layer("EditorLayer") { }

	void EditorLayer::OnAttach()
	{
		SP_PROFILE_FUNCTION();

		FramebufferProperties framebufferProps;
		framebufferProps.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth };
		framebufferProps.Width = 1600;
		framebufferProps.Height = 900;

		m_Framebuffer = Framebuffer::Create(framebufferProps);

		m_PlayIcon = Texture2D::Create("Resources/Icons/PlayButton.png");
		m_StopIcon = Texture2D::Create("Resources/Icons/StopButton.png");
		m_SimulateIcon = Texture2D::Create("Resources/Icons/SimulateButton.png");

		m_EditorScene = CreateShared<Scene>();
		m_ActiveScene = m_EditorScene;

		const auto& appProps = Application::Get().GetProperties();
		m_ViewportSize = { appProps.WindowWidth, appProps.WindowHeight };

		auto commandLineArgs = appProps.CommandLineArgs;
		if (commandLineArgs.Count > 1)
		{
			auto sceneFilePath = commandLineArgs[1];
			OpenScene(std::filesystem::path(sceneFilePath));
		}
		else
		{
			CreateNewScene(); // Start the editor off with a fresh scene
		}

		m_EditorCamera = EditorCamera(m_EditorCameraFOV, 0.1778f, 0.1f, 1000.0f);
	}

	void EditorLayer::OnDetach() { }

	void EditorLayer::OnUpdate(TimeStep delta)
	{
		SP_PROFILE_FUNCTION();

		m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);

		// Resize
		if (FramebufferProperties props = m_Framebuffer->GetProperties();
			m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f && // zero sized framebuffer is invalid
			(props.Width != m_ViewportSize.x || props.Height != m_ViewportSize.y))
		{
			m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
		}

		// Render
		Renderer2D::ResetStats();
		m_Framebuffer->Bind();
		RenderCommand::Clear();

		// Clear entityID attachment to -1
		m_Framebuffer->ClearAttachment(1, -1);

		// Update Scene
		switch (m_SceneState)
		{
			case SceneState::Edit:
			{
				const Math::vec2& mousePos = Input::GetMousePosition();
				// If the scene viewport is hovered or the mouse was moved moved since the last frame update the editor camera
				// this allows the user to manipulate the editor camera while they are holding the left mouse button even if the cursor is outside the scene viewport
				if (m_SceneViewportHovered || mousePos != m_MousePosLastFrame)
					m_EditorCamera.OnUpdate(delta);

				float editorCameraFOV = m_EditorCamera.GetFOV();
				if (editorCameraFOV != m_EditorCameraFOVLastFrame)
					m_EditorCamera.SetFOV(m_EditorCameraFOV);

				m_ActiveScene->OnUpdateEditor(delta, m_EditorCamera);
				break;
			}
			case SceneState::Play:
			{
				m_ActiveScene->OnUpdateRuntime(delta);
				break;
			}
			case SceneState::Simulate:
			{
				const Math::vec2& mousePos = Input::GetMousePosition();
				if (m_SceneViewportHovered || mousePos != m_MousePosLastFrame)
					m_EditorCamera.OnUpdate(delta);

				float editorCameraFOV = m_EditorCamera.GetFOV();
				if (editorCameraFOV != m_EditorCameraFOVLastFrame)
					m_EditorCamera.SetFOV(m_EditorCameraFOV);

				m_ActiveScene->OnUpdateSimulation(delta, m_EditorCamera);
				break;
			}
		}

		m_MousePosLastFrame = Input::GetMousePosition();
		m_EditorCameraFOVLastFrame = m_EditorCameraFOV;

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
			m_HoveredEntity = pixelData == -1 ? Entity() : Entity{ (entt::entity)pixelData, m_ActiveScene.get() };
		}

		OnOverlayRender();

		m_Framebuffer->Unbind();
	}

	void EditorLayer::OnGuiRender()
	{
		SP_PROFILE_FUNCTION();
		
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
		Gui::Begin("Engine Dockspace", &dockspaceOpen, window_flags);
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
			bool inEditMode = m_SceneState == SceneState::Edit;
			bool inPlayMode = m_SceneState == SceneState::Play;
			bool inSimulateMode = m_SceneState == SceneState::Simulate;

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
				if (inEditMode)
				{
					if (Gui::MenuItem("Play Scene", "Ctrl+P"))
						OnScenePlay();
					Gui::Separator();

					if (Gui::MenuItem("Play Simulation", "Ctrl+X"))
						OnSceneSimulate();
					Gui::Separator();

					if (Gui::MenuItem("Add Empty Entity", "Ctrl+A"))
						AddEmptyEntity();
					if (Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity())
					{
						Gui::Separator();

						if (Gui::MenuItem("Rename Entity", "F2"))
							m_SceneHierarchyPanel.SetEntityToBeRenamed(true);
						Gui::Separator();

						if (Gui::MenuItem("Duplicate Entity", "Ctrl+D"))
							DuplicateSelectedEntity();
						Gui::Separator();

						if (Gui::MenuItem("Delete Entity", "Del"))
							m_SceneHierarchyPanel.SetEntityToBeDestroyed(true);
					}
				}
				else
				{
					if (inPlayMode)
					{
						if (Gui::MenuItem("Stop Scene", "Ctrl+P"))
							OnSceneStop();
						Gui::Separator();

						if (Gui::MenuItem("Restart Scene", "Ctrl+Shift+P"))
							RestartScene();
					}
					else if (inSimulateMode)
					{
						if (Gui::MenuItem("Stop Simulation", "Ctrl+X"))
							OnSceneStop();
						Gui::Separator();

						if (Gui::MenuItem("Restart Simulation", "Ctrl+Shift+X"))
							RestartSceneSimulation();
					}
				}

				Gui::EndMenu();
			}

			if (Gui::BeginMenu("Script"))
			{
				if (inEditMode)
				{
					if (Gui::MenuItem("Reload Mono Assembly", "Ctrl+R"))
						ScriptEngine::ReloadAssembly();
				}

				Gui::EndMenu();
			}

			if (Gui::BeginMenu("View"))
			{
				if (m_SceneViewportMaximized)
				{
					if (Gui::MenuItem("Minimize Scene", "Ctrl+Space"))
						m_SceneViewportMaximized = false;
				}
				else
				{
					if (Gui::MenuItem("Maximize Scene", "Ctrl+Space"))
						m_SceneViewportMaximized = true;
				}

				if (inEditMode)
					Gui::Separator();

				if (inEditMode)
				{
					if (Gui::MenuItem("Center Editor Camera"))
						m_EditorCamera.ResetPositionToWorldOrigin();
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
				if (Gui::MenuItem("Console"))
					m_ConsolePanel.ShowPanel();
				Gui::Separator();

				if (Gui::MenuItem("Performance"))
					m_PerformancePanel.ShowPanel();
				Gui::Separator();

				if (Gui::MenuItem("Shader Editor"))
					m_ShaderEditorPanel.ShowPanel();
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

		// Render Panels if the scene isn't maximized
		if (!m_SceneViewportMaximized)
		{
			m_SceneHierarchyPanel.OnGuiRender(m_HoveredEntity);
			m_ContentBrowserPanel.OnGuiRender();
			m_ShaderEditorPanel.OnGuiRender();
			m_SettingsPanel.OnGuiRender();
			m_ConsolePanel.OnGuiRender();
			m_PerformancePanel.OnGuiRender();
			m_AboutPanel.OnGuiRender();
		}

		Gui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0.0f, 0.0f });
		bool open = true;
		Gui::Begin("Scene", &open, ImGuiWindowFlags_NoCollapse);
		auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
		auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
		auto viewportOffset = ImGui::GetWindowPos();
		m_ViewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
		m_ViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };
		
		m_SceneViewportFocused = Gui::IsWindowFocused();
		m_SceneViewportHovered = Gui::IsWindowHovered();
		Application::Get().GetGuiLayer()->BlockEvents(!m_SceneViewportHovered);

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
				std::filesystem::path filePath = std::filesystem::path(path);

				if (filePath.extension().string() == ".png" || filePath.extension().string() == ".jpg")
				{
					std::filesystem::path texturePath = std::filesystem::path(g_AssetPath) / path;
					SharedRef<Texture2D> texture = Texture2D::Create(texturePath.string());
					if (texture->IsLoaded())
					{
						if (m_HoveredEntity && m_HoveredEntity.HasComponent<SpriteRendererComponent>())
							m_HoveredEntity.GetComponent<SpriteRendererComponent>().Texture = texture;
					}
					else
						SP_WARN("Could not load texture {}", texturePath.filename().string());
				}
				else
					OpenScene(std::filesystem::path(g_AssetPath) / path);
			}

			Gui::EndDragDropTarget();
		}

		// Render Gizmos
		bool isInEditMode = m_SceneState != SceneState::Play;
		Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
		if (selectedEntity && m_GizmoType != -1 && isInEditMode && !Input::IsKeyPressed(Key::LeftAlt))
		{
			ImGuizmo::Enable(m_GizmosEnabled);
			ImGuizmo::SetOrthographic(m_OrthographicGizmos);
			ImGuizmo::SetDrawlist();

			ImGuizmo::SetRect(m_ViewportBounds[0].x, m_ViewportBounds[0].y, m_ViewportBounds[1].x - m_ViewportBounds[0].x, m_ViewportBounds[1].y - m_ViewportBounds[0].y);

			// Editor camera
			const Math::mat4& cameraProjection = m_EditorCamera.GetProjection();
			Math::mat4 cameraView = m_EditorCamera.GetViewMatrix();

			// Entity transform
			auto& transformComponent = selectedEntity.GetComponent<TransformComponent>();
			Math::mat4 transform = transformComponent.GetTransform();

			// Snapping
			bool controlPressed = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
			float snapValue = m_GizmoType == ImGuizmo::ROTATE ? m_RotationSnapValue : m_SnapValue;

			std::array<float, 3> snapValues{};
			snapValues.fill(snapValue);

			ImGuizmo::Manipulate(
				Math::ValuePtr(cameraView), Math::ValuePtr(cameraProjection),
				(ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::MODE::LOCAL, Math::ValuePtr(transform),
				nullptr, (controlPressed && m_GizmoSnapEnabled) ? snapValues.data() : nullptr
			);

			if (m_DrawGizmoGrid)
				ImGuizmo::DrawGrid(Math::ValuePtr(cameraView), Math::ValuePtr(cameraProjection), Math::ValuePtr(transform), m_GizmoGridSize);

			if (ImGuizmo::IsUsing())
			{
				Math::vec3 translation;
				Math::vec3 rotation;
				Math::vec3 scale;

				Math::DecomposeTransform(transform, translation, rotation, scale);
				Math::vec3 deltaRotation = rotation - transformComponent.Rotation;
				transformComponent.Translation = translation;
				transformComponent.Rotation += deltaRotation;
				transformComponent.Scale = scale;
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

		Gui::Begin("Toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

		bool toolbarEnabled = (bool)m_ActiveScene;

		ImVec4 tintColor = ImVec4(1, 1, 1, 1);
		if (!toolbarEnabled)
			tintColor.w = 0.5f;

		float size = Gui::GetWindowHeight() - 4.0f;

		{
			SharedRef<Texture2D> icon = (m_SceneState == SceneState::Edit || m_SceneState == SceneState::Simulate) ? m_PlayIcon : m_StopIcon;
			Gui::SetCursorPosX((Gui::GetWindowContentRegionMax().x * 0.5f) - (size * 0.5f));
			if (Gui::ImageButton(reinterpret_cast<void*>(icon->GetRendererID()), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0, 0, 0, 0), tintColor) && toolbarEnabled)
			{
				if (m_SceneState == SceneState::Edit || m_SceneState == SceneState::Simulate)
					OnScenePlay();
				else if (m_SceneState == SceneState::Play)
					OnSceneStop();
			}
		}

		Gui::SameLine();

		{
			SharedRef<Texture2D> icon = (m_SceneState == SceneState::Edit || m_SceneState == SceneState::Play) ? m_SimulateIcon : m_StopIcon;
			if (Gui::ImageButton(reinterpret_cast<void*>(icon->GetRendererID()), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0, 0, 0, 0), tintColor) && toolbarEnabled)
			{
				if (m_SceneState == SceneState::Edit || m_SceneState == SceneState::Play)
					OnSceneSimulate();
				else if (m_SceneState == SceneState::Simulate)
					OnSceneStop();
			}
		}

		Gui::PopStyleVar(2);
		Gui::PopStyleColor(3);

		Gui::End();
	}

	void EditorLayer::OnLaunchRuntime()
	{
		FileSystem::LaunchApplication("game\\Debug\\Sparky-Runtime.exe", m_EditorScenePath.string().c_str());
	}

	void EditorLayer::OnOverlayRender()
	{
		if (m_SceneState == SceneState::Play)
		{
			Entity cameraEntity = m_ActiveScene->GetPrimaryCameraEntity();

			if (!cameraEntity) // No entity with a camera component was found so don't try to render
				return;

			Renderer2D::BeginScene(cameraEntity.GetComponent<CameraComponent>().Camera, cameraEntity.GetComponent<TransformComponent>().GetTransform());
		}
		else
			Renderer2D::BeginScene(m_EditorCamera);

		if (m_ShowPhysicsColliders)
		{
			float colliderDistance = 0.005f; // Editor camera will be looking at the origin of the world on the first frame
			if (m_EditorCamera.GetPosition().z < 0) // Show colliders on the side that the editor camera facing
				colliderDistance = -colliderDistance;

			// Render Box Colliders
			{
				auto view = m_ActiveScene->GetAllEntitiesWith<TransformComponent, BoxCollider2DComponent>();
				for (auto entity : view)
				{
					auto [tc, bc2d] = view.get<TransformComponent, BoxCollider2DComponent>(entity);

					Math::vec3 scale = tc.Scale * Math::vec3(bc2d.Size * 2.0f, 1.0f);

					Math::mat4 transform = Math::Translate(tc.Translation)
						* Math::Rotate(tc.Rotation.z, Math::vec3(0.0f, 0.0f, 1.0f))
						* Math::Translate(Math::vec3(bc2d.Offset, colliderDistance))
						* Math::Scale(scale);

					Renderer2D::DrawRect(transform, m_PhysicsColliderColor);
				}
			}

			// Render Circle Colliders
			{
				auto view = m_ActiveScene->GetAllEntitiesWith<TransformComponent, CircleCollider2DComponent>();
				for (auto entity : view)
				{
					auto [tc, cc2d] = view.get<TransformComponent, CircleCollider2DComponent>(entity);

					glm::vec3 scale = tc.Scale * glm::vec3(cc2d.Radius * 2.0f);

					Math::mat4 transform = Math::Translate(tc.Translation)
						* Math::Rotate(tc.Rotation.z, Math::vec3(0.0f, 0.0f, 1.0f))
						* Math::Translate(Math::vec3(cc2d.Offset, colliderDistance))
						* Math::Scale(Math::vec3(scale.x, scale.x, scale.z));

					Renderer2D::DrawCircle(transform, m_PhysicsColliderColor, Renderer2D::GetLineWidth() / 100.0f);
				}
			}
		}

		// Draw selected entity outline 
		if (Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity()) {
			const auto& transform = selectedEntity.GetComponent<TransformComponent>();

			//Orange
			Renderer2D::DrawRect(transform.GetTransform(), ColorToVec4(Color::Orange));
		}

		Renderer2D::EndScene();
	}

	void EditorLayer::OnEvent(Event& e)
	{
		m_EditorCamera.OnEvent(e);

		// Set cursor position
		Math::vec2 mousePosition = Input::GetMousePosition();
		Math::vec2 windowSize = Application::Get().GetWindow().GetSize();

		if (m_MousePosLastFrame.x == 0)
			Application::Get().GetWindow().SetCursorPosition(windowSize.x, m_MousePosLastFrame.y);
		if (m_MousePosLastFrame.x == windowSize.x - 1.0f)
			Application::Get().GetWindow().SetCursorPosition(0, m_MousePosLastFrame.y);

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
				if (controlPressed && m_SceneState == SceneState::Edit)
					CreateNewScene();

				break;
			}
			case Key::O:
			{
				if (controlPressed && m_SceneState == SceneState::Edit)
					OpenExistingScene();

				break;
			}
			case Key::S:
			{
				if (controlPressed && m_SceneState == SceneState::Edit)
				{
					if (shiftPressed)
						SaveSceneAs();
					else
						SaveScene();
				}

				break;
			}

			// Tools
			case Key::F2:
			{
				if (Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity())
					m_SceneHierarchyPanel.SetEntityToBeRenamed(true);

				break;
			}

			case Key::Delete:
			{
				if (Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity())
					m_SceneHierarchyPanel.SetEntityToBeDestroyed(true);

				break;
			}

			case Key::A:
			{
				if (controlPressed && !shiftPressed) // Ctrl+Shift+A opens the add component popup in inspector panel
					AddEmptyEntity();

				break;
			}
			case Key::D:
			{
				if (controlPressed)
					DuplicateSelectedEntity();

				break;
			}

			case Key::P:
			{
				if (controlPressed && shiftPressed && m_SceneState == SceneState::Play)
				{
					RestartScene();

					break;
				}

				if (controlPressed)
				{
					if (m_SceneState == SceneState::Edit || m_SceneState == SceneState::Simulate)
						OnScenePlay();
					else if (m_SceneState == SceneState::Play)
						OnSceneStop();

					break;
				}
			}
			case Key::X:
			{
				if (controlPressed && shiftPressed)
				{
					if (m_SceneState == SceneState::Simulate)
						RestartSceneSimulation();

					break;
				}

				if (controlPressed)
				{
					if (m_SceneState == SceneState::Edit || m_SceneState == SceneState::Play)
						OnSceneSimulate();
					else if (m_SceneState == SceneState::Simulate)
						OnSceneStop();

					break;
				}
			}

			case Key::Space:
			{
				if (controlPressed)
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
				if (controlPressed)
					ScriptEngine::ReloadAssembly();
				else if (!ImGuizmo::IsUsing())
					OnScaleToolSelected();

				break;
			}
		}

		return false;
	}

	bool EditorLayer::OnMouseButtonPressedEvent(MouseButtonPressedEvent& e)
	{
		switch (e.GetMouseButton())
		{
			case Mouse::ButtonLeft:
			{
				if (m_SceneViewportHovered && !ImGuizmo::IsOver() && !Input::IsKeyPressed(Key::LeftAlt))
				{
					// TODO: Fix renaming when pressing F2 and selecting another entity
					// Currently this will rename the clicked entity to whatever the previous entity's name was
					if (m_SceneHierarchyPanel.GetSelectedEntity() != Entity{})
						m_SceneHierarchyPanel.SetEntityToBeRenamed(false);

					m_SceneHierarchyPanel.SetSelectedEntity(m_HoveredEntity);
				}

				break;
			}
		}

		return false;
	}

	void EditorLayer::CreateNewScene()
	{
		if (m_SceneState != SceneState::Edit)
			return;

		m_ActiveScene = CreateShared<Scene>();
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
		std::string name = std::format("{} Load Time", path.filename().string());
		InstrumentationTimer timer(name.c_str());

		if (m_SceneState != SceneState::Edit)
			OnSceneStop();

		m_HoveredEntity = Entity{}; // Prevent an invalid entity from being used elsewhere in the editor

		if (path.extension().string() != ".sparky")
		{
			SP_WARN("Could not load {} - not a scene file", path.filename().string());
			return;
		}

		SharedRef<Scene> newScene = CreateShared<Scene>();
		SceneSerializer serializer(newScene);
		newScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);

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
		if (m_SceneState == SceneState::Simulate)
			OnSceneStop();

		m_SceneState = SceneState::Play;

		m_ActiveScene = Scene::Copy(m_EditorScene);
		m_ActiveScene->OnRuntimeStart();

		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
	}

	void EditorLayer::OnSceneStop()
	{
		SP_CORE_ASSERT(m_SceneState == SceneState::Play || m_SceneState == SceneState::Simulate, "Invalid scene state!");

		if (m_SceneState == SceneState::Play)
			m_ActiveScene->OnRuntimeStop();
		else if (m_SceneState == SceneState::Simulate)
			m_ActiveScene->OnSimulationStop();

		m_SceneState = SceneState::Edit;

		m_ActiveScene = m_EditorScene;

		m_HoveredEntity = Entity{};
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
	}

	void EditorLayer::RestartScene()
	{
		OnScenePlay();
	}

	void EditorLayer::OnSceneSimulate()
	{
		if (m_SceneState == SceneState::Play)
			OnSceneStop();

		m_SceneState = SceneState::Simulate;

		m_ActiveScene = Scene::Copy(m_EditorScene);
		m_ActiveScene->OnSimulationStart();

		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
	}

	void EditorLayer::RestartSceneSimulation()
	{
		OnSceneSimulate();
	}

	void EditorLayer::AddEmptyEntity()
	{
		if (m_SceneState != SceneState::Edit)
			return;

		Entity newEntity = m_ActiveScene->CreateEntity();
		m_SceneHierarchyPanel.SetSelectedEntity(newEntity);
		m_SceneHierarchyPanel.SetEntityToBeRenamed(true);
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
			m_SceneHierarchyPanel.SetEntityToBeRenamed(true);
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
