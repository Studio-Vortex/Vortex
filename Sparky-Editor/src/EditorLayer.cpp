#include "EditorLayer.h"

#include <Sparky/Scene/Components.h>
#include <Sparky/Scene/SceneSerializer.h>
#include <Sparky/Renderer/RenderCommand.h>
#include <Sparky/Utils/PlatformUtils.h>
#include <Sparky/Scripting/ScriptEngine.h>
#include <Sparky/Scripting/ScriptRegistry.h>
#include <Sparky/Audio/AudioEngine.h>

#include <ImGuizmo.h>

namespace Sparky {

	extern const std::filesystem::path g_AssetPath;

	EditorLayer::EditorLayer()
		: Layer("EditorLayer") { }

	void EditorLayer::OnAttach()
	{
		SP_PROFILE_FUNCTION();

		const auto& appProps = Application::Get().GetProperties();

		FramebufferProperties framebufferProps;
		framebufferProps.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth };
		framebufferProps.Width = 1600;
		framebufferProps.Height = 900;
		framebufferProps.Samples = appProps.SampleCount;

		m_Framebuffer = Framebuffer::Create(framebufferProps);

		m_PlayIcon = Texture2D::Create("Resources/Icons/PlayButton.png");
		m_PauseIcon = Texture2D::Create("Resources/Icons/PauseButton.png");
		m_StopIcon = Texture2D::Create("Resources/Icons/StopButton.png");
		m_SimulateIcon = Texture2D::Create("Resources/Icons/SimulateButton.png");
		m_StepIcon = Texture2D::Create("Resources/Icons/StepButton.png");

		m_LocalModeIcon = Texture2D::Create("Resources/Icons/Scene/LocalMode.png");
		m_WorldModeIcon = Texture2D::Create("Resources/Icons/Scene/WorldMode.png");
		m_TranslateToolIcon = Texture2D::Create("Resources/Icons/Scene/TranslateTool.png");
		m_RotateToolIcon = Texture2D::Create("Resources/Icons/Scene/RotateTool.png");
		m_ScaleToolIcon = Texture2D::Create("Resources/Icons/Scene/ScaleTool.png");

		m_EditorScene = CreateShared<Scene>();
		m_ActiveScene = m_EditorScene;

		m_ViewportSize = { appProps.WindowWidth, appProps.WindowHeight };

		auto commandLineArgs = appProps.CommandLineArgs;
		if (commandLineArgs.Count > 1)
		{
			auto sceneFilePath = commandLineArgs[1];
			OpenScene(std::filesystem::path(sceneFilePath));
		}
		else
			CreateNewScene(); // Start the editor off with a fresh scene

		m_EditorCamera = EditorCamera(m_EditorCameraFOV, 0.1778f, 0.1f, 1000.0f);
		RenderCommand::SetClearColor(m_EditorClearColor);
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
			m_MaterialViewerPanel.SetTexture(m_ScaleToolIcon);
		}

		// Render
		Renderer::ResetStats();
		Renderer2D::ResetStats();
		m_Framebuffer->Bind();
		RenderCommand::Clear();

		// Clear entityID attachment to -1
		m_Framebuffer->ClearAttachment(1, -1);

		const Math::vec2& mousePos = Input::GetMousePosition();

		// Update Scene
		switch (m_SceneState)
		{
			case SceneState::Edit:
			{
				// If the scene viewport is hovered or the mouse was moved moved since the last frame update the editor camera
				// this allows the user to manipulate the editor camera while they are holding the right mouse button even if the cursor is outside the scene viewport
				if (m_SceneViewportHovered || mousePos != m_MousePosLastFrame || Input::IsMouseButtonPressed(Mouse::ButtonRight))
					m_EditorCamera.OnUpdate(delta);

				float editorCameraFOV = m_EditorCamera.GetFOV();
				if (editorCameraFOV != m_EditorCameraFOVLastFrame)
					m_EditorCamera.SetFOV(m_EditorCameraFOV);

				m_ActiveScene->OnUpdateEditor(delta, m_EditorCamera);

				break;
			}
			case SceneState::Play:
			{
				if (const char* sceneToBeLoaded = ScriptRegistry::GetSceneToBeLoaded(); strlen(sceneToBeLoaded) != 0)
				{
					OpenScene(std::filesystem::path(std::format("Assets/Scenes/{}.sparky", sceneToBeLoaded)));
					OnScenePlay();
					ScriptRegistry::ResetSceneToBeLoaded();
				}

				bool scenePaused = m_ActiveScene->IsPaused();

				if (scenePaused)
					OnScenePause();

				if (!scenePaused && !m_AudioSourcesToResume.empty())
					OnSceneResume();

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
		
		static bool scenePanelOpen = true;

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
					if (m_SceneHierarchyPanel.GetSelectedEntity())
					{
						if (Gui::MenuItem("Move To Camera Position"))
						{
							Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
							TransformComponent& transform = selectedEntity.GetTransform();
							transform.Translation = m_EditorCamera.GetPosition();
							transform.Rotation = Math::vec3(-m_EditorCamera.GetPitch(), -m_EditorCamera.GetYaw(), transform.Rotation.z);
						}

						Gui::Separator();
					}

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
							m_SceneHierarchyPanel.SetEntityShouldBeRenamed(true);
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
				Gui::MenuItem("Maximize On Play", nullptr, &m_MaximizeOnPlay);
				Gui::Separator();

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
						m_EditorCamera.ResetCameraPositionToWorldOrigin();
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
				Gui::MenuItem("Asset Manager", nullptr, &m_AssetManagerPanel.IsOpen());
				Gui::Separator();
				Gui::MenuItem("Console", nullptr, &m_ConsolePanel.IsOpen());
				Gui::Separator();
				Gui::MenuItem("Content Browser", nullptr, &m_ContentBrowserPanel.IsOpen());
				Gui::Separator();
				Gui::MenuItem("Inspector", nullptr, &m_SceneHierarchyPanel.IsInspectorOpen());
				Gui::Separator();
				Gui::MenuItem("Material Viewer", nullptr, &m_MaterialViewerPanel.IsOpen());
				Gui::Separator();
				Gui::MenuItem("Performance", nullptr, &m_PerformancePanel.IsOpen());
				Gui::Separator();
				Gui::MenuItem("Scene", nullptr, &scenePanelOpen);
				Gui::Separator();
				Gui::MenuItem("Scene Hierarchy", nullptr, &m_SceneHierarchyPanel.IsOpen());
				Gui::Separator();
				Gui::MenuItem("Script Registry", nullptr, &m_ScriptRegistryPanel.IsOpen());
				Gui::Separator();
				Gui::MenuItem("Shader Editor", nullptr, &m_ShaderEditorPanel.IsOpen());
				Gui::Separator();
				Gui::MenuItem("Settings", nullptr, &m_SettingsPanel.IsOpen());

				Gui::EndMenu();
			}
			
			if (Gui::BeginMenu("Help"))
			{
				Gui::MenuItem("About", nullptr, &m_AboutPanel.IsOpen());

				Gui::EndMenu();
			}

			Gui::EndMenuBar();
		}

		// Render Panels if the scene isn't maximized
		if (!m_SceneViewportMaximized)
		{
			m_SceneHierarchyPanel.OnGuiRender(m_HoveredEntity);
			m_ContentBrowserPanel.OnGuiRender();
			m_ScriptRegistryPanel.OnGuiRender();
			m_MaterialViewerPanel.OnGuiRender();
			m_AssetManagerPanel.OnGuiRender();
			m_ShaderEditorPanel.OnGuiRender();
			m_SettingsPanel.OnGuiRender();
			m_ConsolePanel.OnGuiRender();
			m_PerformancePanel.OnGuiRender(m_ActiveScene->GetEntityCount());
			m_AboutPanel.OnGuiRender();
		}

		// Update C# Entity.OnGui()
		m_ActiveScene->OnUpdateEntityGui();

		Gui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0.0f, 0.0f });
		Gui::Begin("Scene", &scenePanelOpen, ImGuiWindowFlags_NoCollapse);
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
		Gui::Image(reinterpret_cast<void*>(textureID), ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 }, m_ActiveScene->IsPaused() ? ImVec4{ 1.0f, 1.0f, 1.0f, 0.5f } : ImVec4{ 1.0f, 1.0f, 1.0f, 1.0f });

		// Accept Items from the content browser
		if (Gui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				const wchar_t* path = (const wchar_t*)payload->Data;
				std::filesystem::path filePath = std::filesystem::path(path);

				if (filePath.extension().string() == ".png" || filePath.extension().string() == ".jpg" || filePath.extension().string() == ".tga")
				{
					std::filesystem::path texturePath = std::filesystem::path(g_AssetPath) / path;
					SharedRef<Texture2D> texture = Texture2D::Create(texturePath.string());
					if (texture->IsLoaded())
					{
						if (m_HoveredEntity && m_HoveredEntity.HasComponent<SpriteRendererComponent>())
							m_HoveredEntity.GetComponent<SpriteRendererComponent>().Texture = texture;

						if (m_HoveredEntity && m_HoveredEntity.HasComponent<MeshRendererComponent>())
							m_HoveredEntity.GetComponent<MeshRendererComponent>().Texture = texture;
					}
					else
						SP_CORE_WARN("Could not load texture - {}", texturePath.filename().string());
				}
				else if (filePath.extension().string() == ".obj")
				{
					std::filesystem::path modelPath = std::filesystem::path(g_AssetPath) / path;

					if (m_HoveredEntity && m_HoveredEntity.HasComponent<MeshRendererComponent>())
					{
						MeshRendererComponent& meshRenderer = m_HoveredEntity.GetComponent<MeshRendererComponent>();

						meshRenderer.Mesh = Model::Create(modelPath.string(), m_HoveredEntity);
						meshRenderer.Type = MeshRendererComponent::MeshType::Custom;
					}
				}
				else if (filePath.extension().string() == ".sparky")
					OpenScene(std::filesystem::path(g_AssetPath) / path);
			}

			Gui::EndDragDropTarget();
		}

		if (m_ShowSceneCreateEntityMenu)
		{
			Gui::OpenPopup("SceneCreateEntityMenu");
			m_ShowSceneCreateEntityMenu = false;
		}

		if (Gui::IsPopupOpen("SceneCreateEntityMenu"))
			Gui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 5.0f, 5.0f });

		if (Gui::BeginPopup("SceneCreateEntityMenu"))
		{
			m_SceneHierarchyPanel.DisplayCreateEntityMenu();

			Gui::PopStyleVar();
			Gui::EndPopup();
		}

		UI_Toolbar();

		// Render Gizmos
		Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();

		bool notInPlayMode = m_SceneState != SceneState::Play;
		bool currentGizmoToolIsValid = m_GizmoType != -1;
		bool altPressed = Input::IsKeyPressed(Key::LeftAlt) || Input::IsKeyPressed(Key::RightAlt);
		bool showGizmos = (selectedEntity && currentGizmoToolIsValid && notInPlayMode && !altPressed);

		bool sceneIsInDebugMode = selectedEntity && m_ActiveScene->IsInDebugMode();

		if (showGizmos || sceneIsInDebugMode)
		{
			ImGuizmo::Enable(m_GizmosEnabled);
			ImGuizmo::SetOrthographic(m_OrthographicGizmos);
			ImGuizmo::SetDrawlist();

			ImGuizmo::SetRect(m_ViewportBounds[0].x, m_ViewportBounds[0].y, m_ViewportBounds[1].x - m_ViewportBounds[0].x, m_ViewportBounds[1].y - m_ViewportBounds[0].y);

			// Editor camera
			const Math::mat4& cameraProjection = m_EditorCamera.GetProjection();
			Math::mat4 cameraView = m_EditorCamera.GetViewMatrix();

			// Entity transform
			TransformComponent& entityTransformComponent = selectedEntity.GetTransform();
			Math::mat4 entityTransform = entityTransformComponent.GetTransform();

			// Snapping
			bool controlPressed = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
			float snapValue = m_GizmoType == ImGuizmo::ROTATE ? m_RotationSnapValue : m_SnapValue;

			std::array<float, 3> snapValues{};
			snapValues.fill(snapValue);

			if (sceneIsInDebugMode)
			{
				Entity primaryCameraEntity = m_ActiveScene->GetPrimaryCameraEntity();

				if (primaryCameraEntity)
				{
					TransformComponent& primaryCameraTransformComponent = primaryCameraEntity.GetTransform();

					Math::quaternion orientation = Math::quaternion(Math::vec3(-primaryCameraTransformComponent.Rotation.x, -primaryCameraTransformComponent.Rotation.y, 0.0f));
					Math::vec3 cameraPosition = primaryCameraTransformComponent.Translation - Math::Rotate(orientation, Math::vec3(0.0f, 0.0f, -1.0f));
					Math::mat4 cameraViewMatrix = Math::Translate(cameraPosition) * Math::ToMat4(orientation);
					cameraViewMatrix = Math::Inverse(cameraViewMatrix);
					Math::mat4 cameraProjectionMatrix = primaryCameraEntity.GetComponent<CameraComponent>().Camera.GetProjection();

					ImGuizmo::Manipulate(
						Math::ValuePtr(cameraViewMatrix), Math::ValuePtr(cameraProjectionMatrix),
						static_cast<ImGuizmo::OPERATION>(m_GizmoType), static_cast<ImGuizmo::MODE>(m_TranslationMode), Math::ValuePtr(entityTransform),
						nullptr, (controlPressed && m_GizmoSnapEnabled) ? snapValues.data() : nullptr
					);

					if (m_DrawGizmoGrid)
						ImGuizmo::DrawGrid(Math::ValuePtr(cameraViewMatrix), Math::ValuePtr(cameraProjectionMatrix), Math::ValuePtr(entityTransform), m_GizmoGridSize);
				}
			}
			else
			{
				ImGuizmo::Manipulate(
					Math::ValuePtr(cameraView), Math::ValuePtr(cameraProjection),
					static_cast<ImGuizmo::OPERATION>(m_GizmoType), static_cast<ImGuizmo::MODE>(m_TranslationMode), Math::ValuePtr(entityTransform),
					nullptr, (controlPressed && m_GizmoSnapEnabled) ? snapValues.data() : nullptr
				);

				if (m_DrawGizmoGrid)
					ImGuizmo::DrawGrid(Math::ValuePtr(cameraView), Math::ValuePtr(cameraProjection), Math::ValuePtr(entityTransform), m_GizmoGridSize);
			}

			if (ImGuizmo::IsUsing())
			{
				Math::vec3 translation;
				Math::vec3 rotation;
				Math::vec3 scale;

				Math::DecomposeTransform(entityTransform, translation, rotation, scale);
				Math::vec3 deltaRotation = rotation - entityTransformComponent.Rotation;
				entityTransformComponent.Translation = translation;
				entityTransformComponent.Rotation += deltaRotation;
				entityTransformComponent.Scale = scale;
			}
		}

		Gui::End();
		Gui::PopStyleVar();

		Gui::End();
	}

	void EditorLayer::UI_Toolbar()
	{
		Gui::PushStyleColor(ImGuiCol_Button, { 0.0f, 0.0f, 0.0f, 0.0f });
		auto& colors = Gui::GetStyle().Colors;
		const auto& buttonActive = colors[ImGuiCol_ButtonActive];
		Gui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ buttonActive.x, buttonActive.y, buttonActive.z, 0.5f });
		const auto& buttonHovered = colors[ImGuiCol_ButtonHovered];
		Gui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ buttonHovered.x, buttonHovered.y, buttonHovered.z, 0.5f });

		auto DisplayTooltipFunc = [](const char* message) {
			Gui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 5.0f, 5.0f });
			Gui::BeginTooltip();
			Gui::Text(message);
			Gui::EndTooltip();
			Gui::PopStyleVar();
		};

		ImVec4 normalColor = { 1.0f, 1.0f, 1.0f, 0.0f };
		ImVec4 tintColor = { 0.7f, 0.7f, 0.7f, 1.0f };

		float size = Gui::GetTextLineHeightWithSpacing() * 1.25f;
		Gui::SetCursorPos({ size, 10.0f });
		if (Gui::ImageButton((void*)m_LocalModeIcon->GetRendererID(), ImVec2(size, size), { 0, 1 }, { 1, 0 }, -1, m_TranslationMode == 0 ? tintColor : normalColor))
			m_TranslationMode = static_cast<uint32_t>(ImGuizmo::MODE::LOCAL);
		else if (Gui::IsItemHovered())
			DisplayTooltipFunc("Local Mode");

		Gui::SameLine();
		if (Gui::ImageButton((void*)m_WorldModeIcon->GetRendererID(), ImVec2(size, size), { 0, 1 }, { 1, 0 }, -1, m_TranslationMode == 1 ? tintColor : normalColor))
			m_TranslationMode = static_cast<uint32_t>(ImGuizmo::MODE::WORLD);
		else if (Gui::IsItemHovered())
			DisplayTooltipFunc("World Mode");

		Gui::SetCursorPos({ size * 6, 10.0f });
		if (Gui::ImageButton((void*)m_TranslateToolIcon->GetRendererID(), ImVec2(size, size), { 0, 1 }, { 1, 0 }, -1, m_GizmoType == 0 ? tintColor : normalColor))
			OnTranslationToolSelected();
		else if (Gui::IsItemHovered())
			DisplayTooltipFunc("Translate Tool");

		Gui::SameLine();
		if (Gui::ImageButton((void*)m_RotateToolIcon->GetRendererID(), ImVec2(size, size), { 0, 1 }, { 1, 0 }, -1, m_GizmoType == 1 ? tintColor : normalColor))
			OnRotationToolSelected();
		else if (Gui::IsItemHovered())
			DisplayTooltipFunc("Rotate Tool");

		Gui::SameLine();
		if (Gui::ImageButton((void*)m_ScaleToolIcon->GetRendererID(), ImVec2(size, size), { 0, 1 }, { 1, 0 }, -1, m_GizmoType == 2 ? tintColor : normalColor))
			OnScaleToolSelected();
		else if (Gui::IsItemHovered())
			DisplayTooltipFunc("Scale Tool");

		bool hasPlayButton = m_SceneState != SceneState::Simulate;
		bool hasSimulateButton = m_SceneState != SceneState::Play;
		bool hasPauseButton = m_SceneState != SceneState::Edit;
		bool scenePaused = m_ActiveScene->IsPaused();

		Gui::SetCursorPos({ Gui::GetWindowWidth() * 0.5f - size * 0.5f, 10.0f });

		if (hasPlayButton)
		{
			SharedRef<Texture2D> icon = (hasSimulateButton) ? m_PlayIcon : m_StopIcon;
			if (Gui::ImageButton(reinterpret_cast<void*>(icon->GetRendererID()), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1)))
			{
				StopAudioSourcesToBeResumed();

				if (hasSimulateButton)
					OnScenePlay();
				else
					OnSceneStop();
			}
			else if (Gui::IsItemHovered())
				DisplayTooltipFunc((hasSimulateButton) ? "Play" : "Stop");

			Gui::SameLine();
		}

		if (hasSimulateButton)
		{
			SharedRef<Texture2D> icon = (hasPlayButton) ? m_SimulateIcon : m_StopIcon;
			if (Gui::ImageButton(reinterpret_cast<void*>(icon->GetRendererID()), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1)))
			{
				StopAudioSourcesToBeResumed();

				if (hasPlayButton)
					OnSceneSimulate();
				else
					OnSceneStop();
			}
			else if (Gui::IsItemHovered())
				DisplayTooltipFunc("Simulate Physics");

			Gui::SameLine();
		}

		if (hasPauseButton)
		{
			SharedRef<Texture2D> icon = m_PauseIcon;
			if (Gui::ImageButton(reinterpret_cast<void*>(icon->GetRendererID()), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1)))
			{
				bool paused = !scenePaused;

				if (paused)
					OnScenePause();
				else
					OnSceneResume();
			}
			else if (Gui::IsItemHovered())
				DisplayTooltipFunc("Pause Scene");

			if (scenePaused)
			{
				Gui::SameLine();

				SharedRef<Texture2D> icon = m_StepIcon;
				if (Gui::ImageButton(reinterpret_cast<void*>(icon->GetRendererID()), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1)))
					m_ActiveScene->Step(m_FrameStepCount);
				else if (Gui::IsItemHovered())
					DisplayTooltipFunc("Next Frame");
			}
		}

		Gui::PopStyleColor(3);
	}

	void EditorLayer::OnLaunchRuntime()
	{
		FileSystem::LaunchApplication("runtime\\Release\\Sparky-Runtime.exe", m_EditorScenePath.string().c_str());
	}

	void EditorLayer::OnOverlayRender()
	{
		if (m_SceneState == SceneState::Play)
		{
			Entity cameraEntity = m_ActiveScene->GetPrimaryCameraEntity();

			if (cameraEntity)
				Renderer2D::BeginScene(cameraEntity.GetComponent<CameraComponent>().Camera, cameraEntity.GetComponent<TransformComponent>().GetTransform());
		}
		else
			Renderer2D::BeginScene(m_EditorCamera);

		// Render Editor Grid
		if ((m_SceneState != SceneState::Play && m_DrawEditorGrid) || m_EditorDebugViewEnabled)
		{
			float axisLineLength = 1'000.0f;
			float gridLineLength = 750.0f;
			float gridWidth = 750.0f;
			float gridLength = 750.0f;

			float originalLineWidth = Renderer2D::GetLineWidth();

			// Render Axes
			if (m_DrawEditorAxes)
			{
				Renderer2D::SetLineWidth(5.0f);
				Renderer2D::DrawLine({ -axisLineLength, 0.0f + 0.02f, 0.0f }, { axisLineLength, 0.0f + 0.02f, 0.0f }, ColorToVec4(Color::Red));   // X Axis
				Renderer2D::DrawLine({ 0.0f, -axisLineLength + 0.02f, 0.0f }, { 0.0f, axisLineLength + 0.02f, 0.0f }, ColorToVec4(Color::Green)); // Y Axis
				Renderer2D::DrawLine({ 0.0f, 0.0f + 0.02f, -axisLineLength }, { 0.0f, 0.0f + 0.02f, axisLineLength }, ColorToVec4(Color::Blue));  // Z Axis
				Renderer2D::Flush();
				Renderer2D::SetLineWidth(originalLineWidth);
			}

			Math::vec4 gridColor = { 0.2f, 0.2f, 0.2f, 1.0f };

			// X Grid Lines
			for (int32_t x = -gridWidth; x <= (int32_t)gridWidth; x++)
			{
				// Skip the origin lines
				if (x == 0 && m_DrawEditorAxes)
					continue;

				Renderer2D::DrawLine({ x, 0, -gridLineLength }, { x, 0, gridLineLength }, gridColor);
			}
			
			// Z Grid Lines
			for (int32_t z = -gridLength; z <= (int32_t)gridLength; z++)
			{
				// Skip the origin lines
				if (z == 0 && m_DrawEditorAxes)
					continue;

				Renderer2D::DrawLine({ -gridLineLength, 0, z }, { gridLineLength, 0, z }, gridColor);
			}

			Renderer2D::Flush();
		}
		
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
						* Math::Translate(Math::vec3(bc2d.Offset, colliderDistance))
						* Math::Rotate(tc.Rotation.z, Math::vec3(0.0f, 0.0f, 1.0f))
						* Math::Scale(scale);

					Renderer2D::DrawRect(transform, m_Physics2DColliderColor);
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
						* Math::Translate(Math::vec3(cc2d.Offset, colliderDistance))
						* Math::Rotate(tc.Rotation.z, Math::vec3(0.0f, 0.0f, 1.0f))
						* Math::Scale(scale);

					Renderer2D::DrawCircle(transform, m_Physics2DColliderColor, Renderer2D::GetLineWidth() / 100.0f);
				}
			}
		}

		// Draw selected entity outline 
		if (Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity())
		{
			const auto& entityTransform = selectedEntity.GetComponent<TransformComponent>();

			if (selectedEntity.HasComponent<MeshRendererComponent>())
			{
				const auto& meshRenderer = selectedEntity.GetComponent<MeshRendererComponent>();

				Renderer::DrawCubeWireframe(entityTransform);
			}
			else if (selectedEntity.HasComponent<CameraComponent>())
			{
				const SceneCamera& sceneCamera = selectedEntity.GetComponent<CameraComponent>().Camera;
				if (sceneCamera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
				{
					// TODO: Draw Camera Frustum
					//Renderer::DrawFrustum(entityTransform, sceneCamera, ColorToVec4(Color::LightBlue));
				}

				Renderer2D::DrawRect(entityTransform.GetTransform(), ColorToVec4(Color::Orange));
			}
			else
			{
				//Orange
				Renderer2D::DrawRect(entityTransform.GetTransform(), ColorToVec4(Color::Orange));
			}
		}

		Renderer2D::EndScene();
	}

	void EditorLayer::OnEvent(Event& e)
	{
		m_EditorCamera.OnEvent(e);

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<KeyPressedEvent>(SP_BIND_CALLBACK(EditorLayer::OnKeyPressedEvent));
		dispatcher.Dispatch<MouseButtonPressedEvent>(SP_BIND_CALLBACK(EditorLayer::OnMouseButtonPressedEvent));
		dispatcher.Dispatch<MouseButtonReleasedEvent>(SP_BIND_CALLBACK(EditorLayer::OnMouseButtonReleasedEvent));
	}

	bool EditorLayer::OnKeyPressedEvent(KeyPressedEvent& e)
	{
		bool controlPressed = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		bool shiftPressed = Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift);
		bool rightMouseButtonPressed = Input::IsMouseButtonPressed(Mouse::ButtonRight);
		bool altPressed = Input::IsKeyPressed(Key::LeftAlt) || Input::IsKeyPressed(Key::RightAlt);
		Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();

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
			case Key::F1:
			{
				if (m_SceneState != SceneState::Play)
				{
					float editorCameraDistance = m_EditorCamera.GetDistance();

					if (editorCameraDistance <= 2.0f)
						m_EditorCamera.SetDistance(100.0f);
					else
						m_EditorCamera.SetDistance(1.0f);
				}

				break;
			}
			case Key::F2:
			{
				if (selectedEntity)
					m_SceneHierarchyPanel.SetEntityShouldBeRenamed(true);

				break;
			}
			case Key::F3:
			{
				if (m_SceneState == SceneState::Play)
				{
					m_ActiveScene->SetDebugMode(!m_EditorDebugViewEnabled);
					m_EditorDebugViewEnabled = !m_EditorDebugViewEnabled;
				}

				break;
			}

			case Key::Delete:
			{
				if (selectedEntity)
					m_SceneHierarchyPanel.SetEntityToBeDestroyed(true);

				break;
			}

			case Key::F:
			{
				if (selectedEntity && !ImGuizmo::IsUsing() && !rightMouseButtonPressed && !m_SceneHierarchyPanel.GetEntityShouldBeRenamed())
				{
					Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
					m_EditorCamera.MoveToPosition(selectedEntity.GetTransform().Translation);
				}

				break;
			}

			case Key::A:
			{
				if (controlPressed)
					m_ShowSceneCreateEntityMenu = true;

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
				if (!ImGuizmo::IsUsing() && !rightMouseButtonPressed && !m_SceneHierarchyPanel.GetEntityShouldBeRenamed())
					OnNoGizmoSelected();

				break;
			}
			case Key::W:
			{
				if (!ImGuizmo::IsUsing() && !rightMouseButtonPressed && !m_SceneHierarchyPanel.GetEntityShouldBeRenamed())
				{
					if (altPressed && selectedEntity)
					{
						TransformComponent& transformComponent = selectedEntity.GetTransform();
						transformComponent.Translation = Math::vec3(0.0f);
					}

					OnTranslationToolSelected();
				}

				break;
			}
			case Key::E:
			{
				if (!ImGuizmo::IsUsing() && !rightMouseButtonPressed && !m_SceneHierarchyPanel.GetEntityShouldBeRenamed())
				{
					if (altPressed && selectedEntity)
					{
						TransformComponent& transformComponent = selectedEntity.GetTransform();
						transformComponent.Rotation = Math::vec3(0.0f);
					}

					OnRotationToolSelected();
				}

				break;
			}
			case Key::R:
			{
				if (controlPressed)
					ScriptEngine::ReloadAssembly();

				else if (!ImGuizmo::IsUsing() && !rightMouseButtonPressed && !m_SceneHierarchyPanel.GetEntityShouldBeRenamed())
				{
					if (altPressed && selectedEntity)
					{
						TransformComponent& transformComponent = selectedEntity.GetTransform();
						transformComponent.Scale = Math::vec3(1.0f);
					}

					OnScaleToolSelected();
				}

				break;
			}
		}

		return false;
	}

	bool EditorLayer::OnMouseButtonPressedEvent(MouseButtonPressedEvent& e)
	{
		bool altPressed = Input::IsKeyPressed(Key::LeftAlt) || Input::IsKeyPressed(Key::RightAlt);
		bool rightMouseButtonPressed = Input::IsMouseButtonPressed(Mouse::ButtonRight);

		switch (e.GetMouseButton())
		{
			case Mouse::ButtonLeft:
			{
				if (m_SceneViewportHovered && !ImGuizmo::IsOver() && !altPressed && !rightMouseButtonPressed)
				{
					m_SceneHierarchyPanel.SetSelectedEntity(m_HoveredEntity);

					if (m_SceneHierarchyPanel.GetSelectedEntity() != Entity{})
						m_SceneHierarchyPanel.SetEntityShouldBeRenamed(false);
				}

				break;
			}

			case Mouse::ButtonRight:
			{
				if (m_SceneViewportHovered)
					Application::Get().GetWindow().ShowMouseCursor(false, true);

				break;
			}
		}

		return false;
	}

	bool EditorLayer::OnMouseButtonReleasedEvent(MouseButtonReleasedEvent& e)
	{
		switch (e.GetMouseButton())
		{
			case Mouse::ButtonRight:
			{
				Application::Get().GetWindow().ShowMouseCursor(true);

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

		// Starting Entities
		Entity startingCube = m_EditorScene->CreateEntity("Cube");
		startingCube.AddComponent<MeshRendererComponent>();

		Entity startingPointLight1 = m_EditorScene->CreateEntity("Point Light");
		startingPointLight1.AddComponent<LightSourceComponent>().Type = LightSourceComponent::LightType::Point;
		startingPointLight1.GetTransform().Translation = Math::vec3(-2.0f, 4.0f, 4.0f);

		Entity startingPointLight2 = m_EditorScene->CreateEntity("Point Light");
		startingPointLight2.AddComponent<LightSourceComponent>().Type = LightSourceComponent::LightType::Point;
		startingPointLight2.GetTransform().Translation = Math::vec3(2.0f, 4.0f, 4.0f);

		Entity startingSpotLight = m_EditorScene->CreateEntity("Spot Light");
		startingSpotLight.AddComponent<LightSourceComponent>().Type = LightSourceComponent::LightType::Spot;
		startingSpotLight.GetTransform().Translation = Math::vec3(-0.0f, 4.0f, 4.0f);

		Entity startingCamera = m_EditorScene->CreateEntity("Camera");
		SceneCamera& camera = startingCamera.AddComponent<CameraComponent>().Camera;
		TransformComponent& cameraTransform = startingCamera.GetTransform();
		cameraTransform.Translation = Math::vec3(-4.0f, 3.0f, 4.0f);
		cameraTransform.Rotation = Math::vec3(Math::Deg2Rad(-25.0f), Math::Deg2Rad(-45.0f), 0.0f);
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

		m_HoveredEntity = Entity{}; // Prevent an invalid entity from being used elsewhere in the editor

		if (path.extension().string() != ".sparky")
		{
			SP_WARN("Could not load {} - not a scene file", path.filename().string());
			return;
		}

		SharedRef<Scene> newScene = CreateShared<Scene>();
		SceneSerializer serializer(newScene);
		newScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);

		std::string name = std::format("{} Load Time", path.filename().string());
		InstrumentationTimer timer(name.c_str());

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
		auto view = m_ActiveScene->GetAllEntitiesWith<AudioSourceComponent>();

		// Stop audio sources that were playing before the play button was pressed
		for (auto& e : view)
		{
			Entity entity{ e, m_ActiveScene.get() };
			SharedRef<AudioSource> audioSource = entity.GetComponent<AudioSourceComponent>().Source;
			
			if (audioSource->IsPlaying())
				audioSource->Stop();
		}

		if (m_SceneState == SceneState::Simulate)
			OnSceneStop();

		// Disable the debug view when starting a scene
		m_EditorDebugViewEnabled = false;

		m_SceneState = SceneState::Play;

		if (m_MaximizeOnPlay)
			m_SceneViewportMaximized = true;

		m_ActiveScene = Scene::Copy(m_EditorScene);
		m_ActiveScene->OnRuntimeStart();

		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
	}

	void EditorLayer::OnScenePause()
	{
		if (m_SceneState == SceneState::Edit)
			return;

		if (m_SceneState == SceneState::Play)
		{
			auto view = m_ActiveScene->GetAllEntitiesWith<AudioSourceComponent>();

			// Pause all audio sources in the scene
			for (auto& e : view)
			{
				Entity entity{ e, m_ActiveScene.get() };
				SharedRef<AudioSource> audioSource = entity.GetComponent<AudioSourceComponent>().Source;
				if (audioSource->IsPlaying())
				{
					audioSource->Pause();
					m_AudioSourcesToResume.push_back(audioSource);
				}
			}
		}

		m_ActiveScene->SetPaused(true);
	}

	void EditorLayer::OnSceneResume()
	{
		if (m_SceneState == SceneState::Play)
		{
			if (!m_AudioSourcesToResume.empty())
			{
				for (auto& audioSource : m_AudioSourcesToResume)
					audioSource->Play();

				m_AudioSourcesToResume.clear();
			}
		}

		m_ActiveScene->SetPaused(false);
	}

	void EditorLayer::StopAudioSourcesToBeResumed()
	{
		for (auto& audioSource : m_AudioSourcesToResume)
			audioSource->Stop();

		m_AudioSourcesToResume.clear();
	}

	void EditorLayer::OnSceneStop()
	{
		SP_CORE_ASSERT(m_SceneState == SceneState::Play || m_SceneState == SceneState::Simulate, "Invalid scene state!");

		if (m_SceneState == SceneState::Play)
			m_ActiveScene->OnRuntimeStop();
		else if (m_SceneState == SceneState::Simulate)
			m_ActiveScene->OnSimulationStop();

		m_SceneState = SceneState::Edit;

		if (m_MaximizeOnPlay)
			m_SceneViewportMaximized = false;

		m_ActiveScene = m_EditorScene;

		m_HoveredEntity = Entity{};
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);

		// Reset the mouse cursor in case a script turned it off
		Application::Get().GetWindow().ShowMouseCursor(true);
	}

	void EditorLayer::RestartScene()
	{
		auto view = m_ActiveScene->GetAllEntitiesWith<AudioSourceComponent>();

		// Restart audio sources
		for (auto& e : view)
		{
			Entity entity{ e, m_ActiveScene.get() };
			SharedRef<AudioSource> audioSource = entity.GetComponent<AudioSourceComponent>().Source;
			if (audioSource->IsPlaying())
				audioSource->Restart();
		}

		OnScenePlay();
	}

	void EditorLayer::OnSceneSimulate()
	{
		if (m_SceneState == SceneState::Play)
			OnSceneStop();

		m_SceneState = SceneState::Simulate;

		if (m_MaximizeOnPlay)
			m_SceneViewportMaximized = true;

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
		m_SceneHierarchyPanel.SetEntityShouldBeRenamed(true);
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
			m_SceneHierarchyPanel.SetEntityShouldBeRenamed(true);
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
