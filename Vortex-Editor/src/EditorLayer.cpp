#include "EditorLayer.h"

#include <Vortex/Scene/SceneSerializer.h>
#include <Vortex/Project/ProjectSerializer.h>
#include <Vortex/Renderer/RenderCommand.h>
#include <Vortex/Scripting/ScriptEngine.h>
#include <Vortex/Scripting/ScriptRegistry.h>
#include <Vortex/Audio/AudioEngine.h>
#include <Vortex/Utils/PlatformUtils.h>
#include <Vortex/Scene/Components.h>

#include "EditorResources.h"

#include <ImGuizmo.h>

namespace Vortex {

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

		EditorResources::Init();

		m_EditorScene = Scene::Create();
		m_ActiveScene = m_EditorScene;

		m_ViewportSize = { appProps.WindowWidth, appProps.WindowHeight };

		auto commandLineArgs = appProps.CommandLineArgs;
		if (commandLineArgs.Count > 1)
		{
			auto projectFilepath = commandLineArgs[1];
			OpenProject(std::filesystem::path(projectFilepath));
		}
		else
		{
			// TODO: Prompt user to select Project Name and Directory
			//CreateNewProject();

			if (!OpenExistingProject())
			{
				Application::Get().Quit();
			}
		}

		m_EditorCamera = EditorCamera(Project::GetActive()->GetProperties().EditorProps.EditorCameraFOV, 0.1778f, 0.1f, 1000.0f);
	}

	void EditorLayer::OnDetach() { }

	void EditorLayer::OnUpdate(TimeStep delta)
	{
		SP_PROFILE_FUNCTION();

		SharedRef<Project> activeProject = Project::GetActive();
		const ProjectProperties& projectProps = activeProject->GetProperties();

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
					m_EditorCamera.SetFOV(projectProps.EditorProps.EditorCameraFOV);

				m_ActiveScene->OnUpdateEditor(delta, m_EditorCamera);

				break;
			}
			case SceneState::Play:
			{
				if (const char* sceneToBeLoaded = ScriptRegistry::GetSceneToBeLoaded(); strlen(sceneToBeLoaded) != 0)
				{
					auto scenePath = std::format("Assets/Scenes/{}.vortex", sceneToBeLoaded);
					OpenScene(Project::GetProjectDirectory() / scenePath);
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
				if (m_SceneViewportHovered || mousePos != m_MousePosLastFrame || Input::IsMouseButtonPressed(Mouse::ButtonRight))
					m_EditorCamera.OnUpdate(delta);

				float editorCameraFOV = m_EditorCamera.GetFOV();
				if (editorCameraFOV != m_EditorCameraFOVLastFrame)
					m_EditorCamera.SetFOV(projectProps.EditorProps.EditorCameraFOV);

				m_ActiveScene->OnUpdateSimulation(delta, m_EditorCamera);
				break;
			}
		}

		m_MousePosLastFrame = Input::GetMousePosition();
		m_EditorCameraFOVLastFrame = projectProps.EditorProps.EditorCameraFOV;

		auto [mx, my] = ImGui::GetMousePos();
		mx -= m_ViewportBounds[0].x;
		my -= m_ViewportBounds[0].y;
		Math::vec2 viewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];
		my = viewportSize.y - my;

		int mouseX = (int)mx;
		int mouseY = (int)my;
		
		if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x && mouseY < (int)viewportSize.y)
		{
			int pixelData = m_Framebuffer->ReadPixel(1, mouseX, mouseY);
			m_HoveredEntity = pixelData == -1 ? Entity() : Entity{ (entt::entity)pixelData, m_ActiveScene.get() };
		}

		ScriptRegistry::SetHoveredEntity(m_HoveredEntity);

		OnOverlayRender();

		m_Framebuffer->Unbind();
	}

	void EditorLayer::OnGuiRender()
	{
		SP_PROFILE_FUNCTION();

		SharedRef<Project> activeProject = Project::GetActive();
		ProjectProperties& projectProps = activeProject->GetProperties();
		
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
				if (Gui::MenuItem("New Project"))
					CreateNewProject();
				Gui::Separator();

				if (Gui::MenuItem("Open Project...", "Ctrl+O"))
					OpenExistingProject();
				Gui::Separator();

				if (Gui::MenuItem("Save Project"))
					SaveProject();
				Gui::Separator();

				if (Gui::MenuItem("New Scene", "Ctrl+N"))
					CreateNewScene();
				Gui::Separator();

				if (Gui::MenuItem("Open Scene..."))
					OpenExistingScene();
				Gui::Separator();

				if (Gui::MenuItem("Save", "Ctrl+S"))
					SaveScene();
				Gui::Separator();

				if (Gui::MenuItem("Save As...", "Ctrl+Shift+S"))
					SaveSceneAs();
				Gui::Separator();

				if (Gui::MenuItem("Exit"))
					Application::Get().Quit();

				Gui::EndMenu();
			}
			
			if (Gui::BeginMenu("Edit"))
			{
				if (inEditMode)
				{
					Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
					if (selectedEntity)
					{
						if (Gui::MenuItem("Move To Camera Position"))
						{
							Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
							TransformComponent& transform = selectedEntity.GetTransform();
							transform.Translation = m_EditorCamera.GetPosition();
							transform.SetRotationEuler(Math::vec3(-m_EditorCamera.GetPitch(), -m_EditorCamera.GetYaw(), transform.GetRotationEuler().z));
						}

						Gui::Separator();
					}

					if (Gui::MenuItem("Play Scene", "Ctrl+P"))
						OnScenePlay();
					Gui::Separator();

					if (Gui::MenuItem("Play Simulation", "Ctrl+X"))
						OnSceneSimulate();

					if (selectedEntity)
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
				Gui::MenuItem("Maximize On Play", nullptr, &projectProps.EditorProps.MaximizeOnPlay);
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
				Gui::MenuItem("Content Browser", nullptr, &m_ContentBrowserPanel->IsOpen());
				Gui::Separator();
				Gui::MenuItem("Inspector", nullptr, &m_SceneHierarchyPanel.IsInspectorOpen());
				Gui::Separator();
				Gui::MenuItem("Material Editor", nullptr, &m_MaterialEditorPanel.IsOpen());
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
				Gui::MenuItem("Build Settings", nullptr, &m_BuildSettingsPanel.IsOpen());
				Gui::Separator();
				Gui::MenuItem("Project Settings", nullptr, &m_ProjectSettingsPanel->IsOpen());

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
			m_ProjectSettingsPanel->OnGuiRender();
			m_SceneHierarchyPanel.OnGuiRender(m_HoveredEntity, m_EditorCamera);
			m_ContentBrowserPanel->OnGuiRender();
			m_ScriptRegistryPanel.OnGuiRender();
			m_MaterialEditorPanel.OnGuiRender(m_SceneHierarchyPanel.GetSelectedEntity());
			m_BuildSettingsPanel.OnGuiRender();
			m_AssetManagerPanel.OnGuiRender();
			m_ShaderEditorPanel.OnGuiRender();
			m_ConsolePanel.OnGuiRender();
			m_AboutPanel.OnGuiRender();
		}

		// Always render if open
		m_PerformancePanel.OnGuiRender(m_ActiveScene->GetEntityCount());

		// Update C# Entity.OnGui()
		m_ActiveScene->OnUpdateEntityGui();

		UI::ScopedStyle windowPadding(ImGuiStyleVar_WindowPadding, ImVec2{ 5.0f, 5.0f });
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
		Gui::Image(reinterpret_cast<void*>(textureID), ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

		// Accept Items from the content browser
		if (Gui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				const wchar_t* path = (const wchar_t*)payload->Data;
				std::filesystem::path filePath = std::filesystem::path(path);

				if (filePath.extension().string() == ".png" || filePath.extension().string() == ".jpg" || filePath.extension().string() == ".tga")
				{
					std::filesystem::path texturePath = filePath;
					SharedRef<Texture2D> texture = Texture2D::Create(texturePath.string());
					if (texture->IsLoaded())
					{
						if (m_HoveredEntity && m_HoveredEntity.HasComponent<SpriteRendererComponent>())
							m_HoveredEntity.GetComponent<SpriteRendererComponent>().Texture = texture;

						if (m_HoveredEntity && m_HoveredEntity.HasComponent<MeshRendererComponent>())
							m_HoveredEntity.GetComponent<MeshRendererComponent>().Mesh->GetMaterial()->SetAlbedoMap(texture);
					}
					else
						VX_CORE_WARN("Could not load texture - {}", texturePath.filename().string());
				}
				else if (filePath.extension().string() == ".obj")
				{
					std::filesystem::path modelPath = filePath;

					if (m_HoveredEntity && m_HoveredEntity.HasComponent<MeshRendererComponent>())
					{
						MeshRendererComponent& meshRenderer = m_HoveredEntity.GetComponent<MeshRendererComponent>();

						meshRenderer.Mesh = Model::Create(modelPath.string(), m_HoveredEntity.GetTransform(), (int)(entt::entity)m_HoveredEntity);
						meshRenderer.Type = MeshType::Custom;
					}
				}
				else if (filePath.extension().string() == ".vortex")
					OpenScene(filePath);
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
			m_SceneHierarchyPanel.DisplayCreateEntityMenu(m_EditorCamera);

			Gui::PopStyleVar();
			Gui::EndPopup();
		}

		UI_GizmosModeToolbar();
		UI_GizmosToolbar();
		UI_CentralToolbar();
		UI_SceneSettingsToolbar();

		// Render Gizmos
		Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();

		bool notInPlayMode = m_SceneState != SceneState::Play;
		bool currentGizmoToolIsValid = m_GizmoType != -1;
		bool altPressed = Input::IsKeyPressed(Key::LeftAlt) || Input::IsKeyPressed(Key::RightAlt);
		bool showGizmos = (selectedEntity && currentGizmoToolIsValid && notInPlayMode && !altPressed);

		if (showGizmos)
		{
			ImGuizmo::Enable(projectProps.GizmoProps.Enabled);
			ImGuizmo::SetOrthographic(projectProps.GizmoProps.IsOrthographic);
			ImGuizmo::SetDrawlist();

			ImGuizmo::SetRect(m_ViewportBounds[0].x, m_ViewportBounds[0].y, m_ViewportBounds[1].x - m_ViewportBounds[0].x, m_ViewportBounds[1].y - m_ViewportBounds[0].y);

			// Editor camera
			const Math::mat4& cameraProjection = m_EditorCamera.GetProjection();
			Math::mat4 cameraView = m_EditorCamera.GetViewMatrix();

			// Entity transform
			TransformComponent& entityTransform = selectedEntity.GetTransform();
			Math::mat4 transform = m_ActiveScene->GetWorldSpaceTransformMatrix(selectedEntity);

			// Snapping
			bool controlPressed = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
			float snapValue = m_GizmoType == ImGuizmo::ROTATE ? projectProps.GizmoProps.RotationSnapValue : projectProps.GizmoProps.SnapValue;
			std::array<float, 3> snapValues{};
			snapValues.fill(snapValue);

			ImGuizmo::Manipulate(
				Math::ValuePtr(cameraView),
				Math::ValuePtr(cameraProjection),
				static_cast<ImGuizmo::OPERATION>(m_GizmoType),
				static_cast<ImGuizmo::MODE>(m_TranslationMode),
				Math::ValuePtr(transform),
				nullptr,
				(controlPressed && projectProps.GizmoProps.SnapEnabled) ? snapValues.data() : nullptr
			);

			if (projectProps.GizmoProps.DrawGrid)
				ImGuizmo::DrawGrid(Math::ValuePtr(cameraView), Math::ValuePtr(cameraProjection), Math::ValuePtr(transform), projectProps.GizmoProps.GridSize);

			if (ImGuizmo::IsUsing())
			{
				Entity parent = m_ActiveScene->TryGetEntityWithUUID(selectedEntity.GetParentUUID());

				if (parent)
				{
					Math::mat4 parentTransform = m_ActiveScene->GetWorldSpaceTransformMatrix(parent);
					transform = Math::Inverse(parentTransform) * transform;
				}

				Math::vec3 translation, rotation, scale;
				Math::DecomposeTransform(transform, translation, rotation, scale);

				// Setting only the component of the transform we are modifying is much
				// more robust than just setting the entire tranform
				// all those floating point computations will be slightly off from the original
				switch (m_GizmoType)
				{
					case ImGuizmo::OPERATION::TRANSLATE:
					{
						entityTransform.Translation = translation;
						break;
					}
					case ImGuizmo::OPERATION::ROTATE:
					{
						// Do this in Euler in an attempt to preserve any full revolutions (> 360)
						Math::vec3 originalEulerRotation = entityTransform.GetRotationEuler();

						// Map original rotation to range [-180, 180] which is what ImGuizmo gives us
						originalEulerRotation.x = fmodf(originalEulerRotation.x + Math::PI, Math::TWO_PI) - Math::PI;
						originalEulerRotation.y = fmodf(originalEulerRotation.y + Math::PI, Math::TWO_PI) - Math::PI;
						originalEulerRotation.z = fmodf(originalEulerRotation.z + Math::PI, Math::TWO_PI) - Math::PI;

						Math::vec3 deltaRotationEuler = rotation - originalEulerRotation;

						// Try to avoid drift due numeric precision
						if (fabs(deltaRotationEuler.x) < 0.001) deltaRotationEuler.x = 0.0f;
						if (fabs(deltaRotationEuler.y) < 0.001) deltaRotationEuler.y = 0.0f;
						if (fabs(deltaRotationEuler.z) < 0.001) deltaRotationEuler.z = 0.0f;

						entityTransform.SetRotationEuler(entityTransform.GetRotationEuler() += deltaRotationEuler);
						break;
					}
					case ImGuizmo::OPERATION::SCALE:
					{
						entityTransform.Scale = scale;
						break;
					}
				}
			}
		}

		Gui::End();

		Gui::End();
	}

	void EditorLayer::UI_GizmosModeToolbar()
	{
		UI::ScopedStyle disableSpacing(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		UI::ScopedStyle disableWindowBorder(ImGuiStyleVar_WindowBorderSize, 0.0f);
		UI::ScopedStyle windowRounding(ImGuiStyleVar_WindowRounding, 4.0f);
		UI::ScopedStyle disablePadding(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		UI::ScopedColor buttonBackground(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

		ImVec4 normalColor = { 1.0f, 1.0f, 1.0f, 0.0f };
		ImVec4 tintColor = { 0.7f, 0.7f, 0.7f, 1.0f };

		const float buttonSize = 18.0f + 5.0f;
		const float edgeOffset = 4.0f;
		const float windowHeight = 32.0f; // annoying limitation of ImGui, window can't be smaller than 32 pixels
		const float numberOfButtons = 2.0f;
		const float backgroundWidth = edgeOffset * 6.0f + buttonSize * numberOfButtons + edgeOffset * (numberOfButtons - 1.0f) * 2.0f;

		ImGui::SetNextWindowPos(ImVec2(m_ViewportBounds[0].x + 14, m_ViewportBounds[0].y + edgeOffset));
		Gui::SetNextWindowSize(ImVec2(backgroundWidth, windowHeight));
		Gui::SetNextWindowBgAlpha(0.0f);
		Gui::Begin("##viewport_gizmos_mode", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking);

		// A hack to make icon panel appear smaller than minimum allowed by ImGui size
		// Filling the background for the desired 26px height
		const float desiredHeight = 26.0f + 5.0f;
		ImRect background = UI::RectExpanded(Gui::GetCurrentWindow()->Rect(), 0.0f, -(windowHeight - desiredHeight) / 2.0f);
		Gui::GetWindowDrawList()->AddRectFilled(background.Min, background.Max, IM_COL32(15, 15, 15, 127), 4.0f);

		Gui::BeginVertical("##viewport_gizmos_modeV", { backgroundWidth, Gui::GetContentRegionAvail().y });
		Gui::Spring();
		Gui::BeginHorizontal("##viewport_gizmos_modeH", { backgroundWidth, Gui::GetContentRegionAvail().y });
		Gui::Spring();

		if (Gui::ImageButton((void*)EditorResources::LocalModeIcon->GetRendererID(), ImVec2(buttonSize, buttonSize), { 0, 1 }, { 1, 0 }, -1, m_TranslationMode == 0 ? tintColor : normalColor))
			m_TranslationMode = static_cast<uint32_t>(ImGuizmo::MODE::LOCAL);
		UI::SetTooltip("Local Mode");

		if (Gui::ImageButton((void*)EditorResources::WorldModeIcon->GetRendererID(), ImVec2(buttonSize, buttonSize), { 0, 1 }, { 1, 0 }, -1, m_TranslationMode == 1 ? tintColor : normalColor))
			m_TranslationMode = static_cast<uint32_t>(ImGuizmo::MODE::WORLD);
		UI::SetTooltip("World Mode");

		Gui::Spring();
		Gui::EndHorizontal();
		Gui::Spring();
		Gui::EndVertical();

		Gui::End();
	}

	void EditorLayer::UI_GizmosToolbar()
	{
		UI::ScopedStyle disableSpacing(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		UI::ScopedStyle disableWindowBorder(ImGuiStyleVar_WindowBorderSize, 0.0f);
		UI::ScopedStyle windowRounding(ImGuiStyleVar_WindowRounding, 4.0f);
		UI::ScopedStyle disablePadding(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		UI::ScopedColor buttonBackground(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

		ImVec4 normalColor = { 1.0f, 1.0f, 1.0f, 0.0f };
		ImVec4 tintColor = { 0.7f, 0.7f, 0.7f, 1.0f };

		const float buttonSize = 18.0f + 5.0f;
		const float edgeOffset = 4.0f;
		const float windowHeight = 32.0f; // annoying limitation of ImGui, window can't be smaller than 32 pixels
		const float numberOfButtons = 4.0f;
		const float backgroundWidth = edgeOffset * 6.0f + buttonSize * numberOfButtons + edgeOffset * (numberOfButtons - 1.0f) * 2.0f;

		ImGui::SetNextWindowPos(ImVec2(m_ViewportBounds[0].x + 128, m_ViewportBounds[0].y + edgeOffset));
		Gui::SetNextWindowSize(ImVec2(backgroundWidth, windowHeight));
		Gui::SetNextWindowBgAlpha(0.0f);
		Gui::Begin("##viewport_gizmos_toolbar", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking);

		// A hack to make icon panel appear smaller than minimum allowed by ImGui size
		// Filling the background for the desired 26px height
		const float desiredHeight = 26.0f + 5.0f;
		ImRect background = UI::RectExpanded(Gui::GetCurrentWindow()->Rect(), 0.0f, -(windowHeight - desiredHeight) / 2.0f);
		Gui::GetWindowDrawList()->AddRectFilled(background.Min, background.Max, IM_COL32(15, 15, 15, 127), 4.0f);

		Gui::BeginVertical("##viewport_gizmos_toolbarV", { backgroundWidth, Gui::GetContentRegionAvail().y });
		Gui::Spring();
		Gui::BeginHorizontal("##viewport_gizmos_toolbarH", { backgroundWidth, Gui::GetContentRegionAvail().y });
		Gui::Spring();

		if (Gui::ImageButton((void*)EditorResources::SelectToolIcon->GetRendererID(), ImVec2(buttonSize, buttonSize), { 0, 1 }, { 1, 0 }, -1, m_GizmoType == -1 ? tintColor : normalColor))
			OnNoGizmoSelected();
		UI::SetTooltip("Select Tool");

		if (Gui::ImageButton((void*)EditorResources::TranslateToolIcon->GetRendererID(), ImVec2(buttonSize, buttonSize), { 0, 1 }, { 1, 0 }, -1, m_GizmoType == 0 ? tintColor : normalColor))
			OnTranslationToolSelected();
		UI::SetTooltip("Translate Tool");

		if (Gui::ImageButton((void*)EditorResources::RotateToolIcon->GetRendererID(), ImVec2(buttonSize, buttonSize), { 0, 1 }, { 1, 0 }, -1, m_GizmoType == 1 ? tintColor : normalColor))
			OnRotationToolSelected();
		UI::SetTooltip("Rotate Tool");

		if (Gui::ImageButton((void*)EditorResources::ScaleToolIcon->GetRendererID(), ImVec2(buttonSize, buttonSize), { 0, 1 }, { 1, 0 }, -1, m_GizmoType == 2 ? tintColor : normalColor))
			OnScaleToolSelected();
		UI::SetTooltip("Scale Tool");

		Gui::Spring();
		Gui::EndHorizontal();
		Gui::Spring();
		Gui::EndVertical();

		Gui::End();
	}

	void EditorLayer::UI_CentralToolbar()
	{
		SharedRef<Project> activeProject = Project::GetActive();
		const ProjectProperties& projectProps = activeProject->GetProperties();

		UI::ScopedStyle disableSpacing(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		UI::ScopedStyle disableWindowBorder(ImGuiStyleVar_WindowBorderSize, 0.0f);
		UI::ScopedStyle windowRounding(ImGuiStyleVar_WindowRounding, 4.0f);
		UI::ScopedStyle disablePadding(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		UI::ScopedColor buttonBackground(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

		const float buttonSize = 18.0f + 5.0f;
		const float edgeOffset = 4.0f;
		const float windowHeight = 32.0f; // annoying limitation of ImGui, window can't be smaller than 32 pixels
		const float numberOfButtons = 3.0f;
		const float backgroundWidth = edgeOffset * 6.0f + buttonSize * numberOfButtons + edgeOffset * (numberOfButtons - 1.0f) * 2.0f;

		float toolbarX = (m_ViewportBounds[0].x + m_ViewportBounds[1].x) / 2.0f;
		Gui::SetNextWindowPos(ImVec2(toolbarX - (backgroundWidth / 2.0f), m_ViewportBounds[0].y + edgeOffset));
		Gui::SetNextWindowSize(ImVec2(backgroundWidth, windowHeight));
		Gui::SetNextWindowBgAlpha(0.0f);
		Gui::Begin("##viewport_central_toolbar", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking);

		// A hack to make icon panel appear smaller than minimum allowed by ImGui size
		// Filling the background for the desired 26px height
		const float desiredHeight = 26.0f + 5.0f;
		ImRect background = UI::RectExpanded(Gui::GetCurrentWindow()->Rect(), 0.0f, -(windowHeight - desiredHeight) / 2.0f);
		Gui::GetWindowDrawList()->AddRectFilled(background.Min, background.Max, IM_COL32(15, 15, 15, 127), 4.0f);

		Gui::BeginVertical("##viewport_central_toolbarV", { backgroundWidth, Gui::GetContentRegionAvail().y });
		Gui::Spring();
		Gui::BeginHorizontal("##viewport_central_toolbarH", { backgroundWidth, Gui::GetContentRegionAvail().y });
		Gui::Spring();

		bool hasPlayButton = m_SceneState != SceneState::Simulate;
		bool hasSimulateButton = m_SceneState != SceneState::Play;
		bool hasPauseButton = m_SceneState != SceneState::Edit;
		bool scenePaused = m_ActiveScene->IsPaused();

		if (hasPlayButton)
		{
			SharedRef<Texture2D> icon = (hasSimulateButton) ? EditorResources::PlayIcon : EditorResources::StopIcon;
			if (Gui::ImageButton(reinterpret_cast<void*>(icon->GetRendererID()), ImVec2(buttonSize, buttonSize), ImVec2(0, 0), ImVec2(1, 1)))
			{
				if (hasSimulateButton)
					OnScenePlay();
				else
					OnSceneStop();
			}

			UI::SetTooltip((hasSimulateButton) ? "Play" : "Stop");
		}

		if (hasSimulateButton)
		{
			SharedRef<Texture2D> icon = (hasPlayButton) ? EditorResources::SimulateIcon : EditorResources::StopIcon;
			if (Gui::ImageButton(reinterpret_cast<void*>(icon->GetRendererID()), ImVec2(buttonSize, buttonSize), ImVec2(0, 0), ImVec2(1, 1)))
			{
				if (hasPlayButton)
					OnSceneSimulate();
				else
					OnSceneStop();
			}

			UI::SetTooltip(hasPlayButton ? "Simulate Physics" : "Stop");
		}

		if (hasPauseButton)
		{
			SharedRef<Texture2D> icon = EditorResources::PauseIcon;
			if (Gui::ImageButton(reinterpret_cast<void*>(icon->GetRendererID()), ImVec2(buttonSize, buttonSize), ImVec2(0, 0), ImVec2(1, 1)))
			{
				bool paused = !scenePaused;

				if (paused)
					OnScenePause();
				else
					OnSceneResume();
			}

			UI::SetTooltip("Pause Scene");

			if (scenePaused)
			{
				SharedRef<Texture2D> icon = EditorResources::StepIcon;
				if (Gui::ImageButton(reinterpret_cast<void*>(icon->GetRendererID()), ImVec2(buttonSize, buttonSize), ImVec2(0, 0), ImVec2(1, 1)))
					m_ActiveScene->Step(projectProps.EditorProps.FrameStepCount);

				UI::SetTooltip("Next Frame");
			}
		}

		Gui::Spring();
		Gui::EndHorizontal();
		Gui::Spring();
		Gui::EndVertical();

		Gui::End();
	}

	void EditorLayer::UI_SceneSettingsToolbar()
	{
		SharedRef<Project> activeProject = Project::GetActive();
		ProjectProperties& projectProps = activeProject->GetProperties();

		UI::ScopedStyle disableSpacing(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		UI::ScopedStyle disableWindowBorder(ImGuiStyleVar_WindowBorderSize, 0.0f);
		UI::ScopedStyle windowRounding(ImGuiStyleVar_WindowRounding, 4.0f);
		UI::ScopedStyle disablePadding(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		UI::ScopedColor buttonBackground(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

		ImVec4 normalColor = { 1.0f, 1.0f, 1.0f, 0.0f };
		ImVec4 tintColor = { 0.7f, 0.7f, 0.7f, 1.0f };

		const float buttonSize = 18.0f + 5.0f;
		const float edgeOffset = 4.0f;
		const float windowHeight = 32.0f; // annoying limitation of ImGui, window can't be smaller than 32 pixels
		const float numberOfButtons = 6.0f;
		const float backgroundWidth = edgeOffset * 6.0f + buttonSize * numberOfButtons + edgeOffset * (numberOfButtons - 1.0f) * 2.0f;

		Gui::SetNextWindowPos(ImVec2(m_ViewportBounds[1].x - backgroundWidth - 14, m_ViewportBounds[0].y + edgeOffset));
		Gui::SetNextWindowSize(ImVec2(backgroundWidth, windowHeight));
		Gui::SetNextWindowBgAlpha(0.0f);
		Gui::Begin("##scene_settings_toolbar", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking);

		// A hack to make icon panel appear smaller than minimum allowed by ImGui size
		// Filling the background for the desired 26px height
		const float desiredHeight = 26.0f + 5.0f;
		ImRect background = UI::RectExpanded(Gui::GetCurrentWindow()->Rect(), 0.0f, -(windowHeight - desiredHeight) / 2.0f);
		Gui::GetWindowDrawList()->AddRectFilled(background.Min, background.Max, IM_COL32(15, 15, 15, 127), 4.0f);

		Gui::BeginVertical("##scene_settings_toolbarV", { backgroundWidth, Gui::GetContentRegionAvail().y });
		Gui::Spring();
		Gui::BeginHorizontal("##scene_settings_toolbarH", { backgroundWidth, Gui::GetContentRegionAvail().y });
		Gui::Spring();

		if (Gui::ImageButton((void*)EditorResources::MaximizeOnPlayIcon->GetRendererID(), ImVec2(buttonSize, buttonSize), { 0, 1 }, { 1, 0 }, -1, projectProps.EditorProps.MaximizeOnPlay ? tintColor : normalColor))
			projectProps.EditorProps.MaximizeOnPlay = !projectProps.EditorProps.MaximizeOnPlay;
		UI::SetTooltip("Maximize On Play");

		if (Gui::ImageButton((void*)EditorResources::ShowGridIcon->GetRendererID(), ImVec2(buttonSize, buttonSize), { 0, 1 }, { 1, 0 }, -1, normalColor))
			projectProps.EditorProps.DrawEditorGrid = !projectProps.EditorProps.DrawEditorGrid;
		UI::SetTooltip(projectProps.EditorProps.DrawEditorGrid ? "Hide Grid" : "Show Grid");

		if (Gui::ImageButton((void*)EditorResources::DisplayPhysicsCollidersIcon->GetRendererID(), ImVec2(buttonSize, buttonSize), { 0, 1 }, { 1, 0 }, -1, projectProps.PhysicsProps.ShowColliders ? tintColor : normalColor))
			projectProps.PhysicsProps.ShowColliders = !projectProps.PhysicsProps.ShowColliders;
		UI::SetTooltip(projectProps.PhysicsProps.ShowColliders ? "Hide Colliders" : "Show Colliders");

		if (Gui::ImageButton((void*)EditorResources::DisplaySceneIconsIcon->GetRendererID(), ImVec2(buttonSize, buttonSize), { 0, 1 }, { 1, 0 }, -1, normalColor))
			projectProps.RendererProps.DisplaySceneIconsInEditor = !projectProps.RendererProps.DisplaySceneIconsInEditor;
		UI::SetTooltip(projectProps.RendererProps.DisplaySceneIconsInEditor ? "Hide Scene Icons" : "Show Scene Icons");

		bool isIn2DView = m_EditorCamera.IsIn2DView();
		if (Gui::ImageButton((void*)EditorResources::TwoDViewIcon->GetRendererID(), ImVec2(buttonSize, buttonSize), { 0, 1 }, { 1, 0 }, -1, isIn2DView ? tintColor : normalColor))
			m_EditorCamera.LockTo2DView(!isIn2DView);
		UI::SetTooltip("2D View");

		bool isInTopDownView = m_EditorCamera.IsInTopDownView();
		if (Gui::ImageButton((void*)EditorResources::TopDownViewIcon->GetRendererID(), ImVec2(buttonSize, buttonSize), { 0, 1 }, { 1, 0 }, -1, isInTopDownView ? tintColor : normalColor))
			m_EditorCamera.LockToTopDownView(!isInTopDownView);
		UI::SetTooltip("Top Down View");

		Gui::Spring();
		Gui::EndHorizontal();
		Gui::Spring();
		Gui::EndVertical();

		Gui::End();
	}

	void EditorLayer::OnLaunchRuntime(const std::filesystem::path& path)
	{
		Project::SaveActive(path);
		std::string runtimePath = Application::Get().GetRuntimeBinaryPath();
		FileSystem::LaunchApplication(runtimePath.c_str(), path.string().c_str());
	}

	void EditorLayer::OnOverlayRender()
	{
		SharedRef<Project> activeProject = Project::GetActive();
		const ProjectProperties& projectProps = activeProject->GetProperties();

		if (m_SceneState == SceneState::Play)
		{
			Entity cameraEntity = m_ActiveScene->GetPrimaryCameraEntity();

			if (cameraEntity)
				Renderer2D::BeginScene(cameraEntity.GetComponent<CameraComponent>().Camera, cameraEntity.GetComponent<TransformComponent>().GetTransform());
		}
		else
		{
			Renderer2D::BeginScene(m_EditorCamera);
		}

		// Render Editor Grid
		if ((m_SceneState != SceneState::Play && projectProps.EditorProps.DrawEditorGrid) || m_EditorDebugViewEnabled)
		{
			float axisLineLength = 1'000.0f;
			float gridLineLength = 750.0f;
			float gridWidth = 750.0f;
			float gridLength = 750.0f;

			float originalLineWidth = Renderer2D::GetLineWidth();

			// Render Axes
			if (projectProps.EditorProps.DrawEditorAxes)
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
				if (x == 0 && projectProps.EditorProps.DrawEditorAxes)
					continue;

				Renderer2D::DrawLine({ x, 0, -gridLineLength }, { x, 0, gridLineLength }, gridColor);
			}
			
			// Z Grid Lines
			for (int32_t z = -gridLength; z <= (int32_t)gridLength; z++)
			{
				// Skip the origin lines
				if (z == 0 && projectProps.EditorProps.DrawEditorAxes)
					continue;

				Renderer2D::DrawLine({ -gridLineLength, 0, z }, { gridLineLength, 0, z }, gridColor);
			}

			Renderer2D::Flush();
		}
		
		if (projectProps.PhysicsProps.ShowColliders)
		{
			// Render 3D Colliders
			{
				{
					auto view = m_ActiveScene->GetAllEntitiesWith<TransformComponent, BoxColliderComponent>();
					for (auto e : view)
					{
						auto [tc, bc] = view.get<TransformComponent, BoxColliderComponent>(e);
						Entity entity{ e, m_ActiveScene.get() };

						Math::AABB aabb = {
							- Math::vec3(0.5f),
							+ Math::vec3(0.5f)
						};

						Math::mat4 transform = m_ActiveScene->GetWorldSpaceTransformMatrix(entity)
							* Math::Translate(bc.Offset)
							* Math::Scale(bc.HalfSize * 2.0f);

						Renderer2D::DrawAABB(aabb, transform, projectProps.PhysicsProps.Physics3DColliderColor);
					}
				}
			}

			// Render 2D Colliders
			{
				float colliderDistance = 0.005f; // Editor camera will be looking at the origin of the world on the first frame
				if (m_EditorCamera.GetPosition().z < 0) // Show colliders on the side that the editor camera facing
					colliderDistance = -colliderDistance;

				{
					auto view = m_ActiveScene->GetAllEntitiesWith<TransformComponent, BoxCollider2DComponent>();
					for (auto e : view)
					{
						auto [tc, bc2d] = view.get<TransformComponent, BoxCollider2DComponent>(e);
						Entity entity{ e, m_ActiveScene.get() };

						Math::vec3 scale = Math::vec3(bc2d.Size * 2.0f, 1.0f);

						Math::mat4 transform = m_ActiveScene->GetWorldSpaceTransformMatrix(entity)
							* Math::Translate(Math::vec3(bc2d.Offset, colliderDistance))
							* Math::Scale(scale);

						Renderer2D::DrawRect(transform, projectProps.PhysicsProps.Physics2DColliderColor);
					}
				}

				{
					auto view = m_ActiveScene->GetAllEntitiesWith<TransformComponent, CircleCollider2DComponent>();
					for (auto e : view)
					{
						auto [tc, cc2d] = view.get<TransformComponent, CircleCollider2DComponent>(e);
						Entity entity{ e, m_ActiveScene.get() };

						Math::vec3 scale = Math::vec3(cc2d.Radius * 2.0f);

						Math::mat4 transform = m_ActiveScene->GetWorldSpaceTransformMatrix(entity)
							* Math::Translate(Math::vec3(cc2d.Offset, colliderDistance))
							* Math::Scale(scale);

						Renderer2D::DrawCircle(transform, projectProps.PhysicsProps.Physics3DColliderColor, Renderer2D::GetLineWidth() / 100.0f);
					}
				}
			}
		}

		// Draw selected entity outline 
		if (Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity())
		{
			const auto& entityTransform = selectedEntity.GetTransform();

			if (selectedEntity.HasComponent<MeshRendererComponent>())
			{
				const auto& meshRenderer = selectedEntity.GetComponent<MeshRendererComponent>();

				Math::mat4 transform = m_ActiveScene->GetWorldSpaceTransformMatrix(selectedEntity);

				Math::AABB aabb = {
					- Math::vec3(0.5f),
					+ Math::vec3(0.5f)
				};

				Renderer2D::DrawAABB(aabb, transform, ColorToVec4(Color::Orange));
			}
			else if (selectedEntity.HasComponent<TextMeshComponent>())
			{
				const auto& textMesh = selectedEntity.GetComponent<TextMeshComponent>();

				const TransformComponent& worldSpaceTransform = m_ActiveScene->GetWorldSpaceTransform(selectedEntity);
				Math::mat4 transform = worldSpaceTransform.GetTransform();

				Renderer2D::DrawRect(transform, ColorToVec4(Color::Orange));
			}
			else if (selectedEntity.HasComponent<CameraComponent>())
			{
				const SceneCamera& sceneCamera = selectedEntity.GetComponent<CameraComponent>().Camera;
				if (sceneCamera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
				{
					// TODO: Draw Camera Frustum
					//Renderer::DrawFrustum(entityTransform, sceneCamera, ColorToVec4(Color::LightBlue));
				}

				Renderer2D::DrawRect(m_ActiveScene->GetWorldSpaceTransformMatrix(selectedEntity), ColorToVec4(Color::Orange));
			}
			else
			{
				//Orange
				Renderer2D::DrawRect(m_ActiveScene->GetWorldSpaceTransformMatrix(selectedEntity), ColorToVec4(Color::Orange));
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
					OpenExistingProject();

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
			case Key::F9:
			{
				if (m_SceneState == SceneState::Play)
				{
					Application::Get().GetWindow().ShowMouseCursor(true, true);
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
						transformComponent.SetRotationEuler(Math::vec3(0.0f));
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
				if (m_SceneViewportHovered && m_SceneState != SceneState::Play)
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
				if (m_SceneState != SceneState::Play)
					Application::Get().GetWindow().ShowMouseCursor(true);

				break;
			}
		}

		return false;
	}

	void EditorLayer::CreateNewProject()
	{
		Project::New();
	}

	bool EditorLayer::OpenExistingProject()
	{
		std::string filepath = FileSystem::OpenFileDialog("Vortex Project (*.vxproject)\0*.vxproject\0");

		if (filepath.empty())
			return false;

		OpenProject(filepath);
		return true;
	}

	void EditorLayer::OpenProject(const std::filesystem::path& path)
	{
		if (m_SceneState != SceneState::Edit)
			OnSceneStop();

		m_HoveredEntity = Entity{};

		if (path.extension().string() != ".vxproject")
		{
			VX_WARN("Could not load {} - not a project file", path.filename().string());
			return;
		}

		if (Project::Load(path))
		{
			std::string projectName = std::format("{} Project Load Time", path.filename().string());
			InstrumentationTimer timer(projectName.c_str());

			ScriptEngine::Init();

			auto startScenePath = Project::GetAssetFileSystemPath(Project::GetActive()->GetProperties().General.StartScene);
			OpenScene(startScenePath.string());

			m_ProjectSettingsPanel = CreateShared<ProjectSettingsPanel>(Project::GetActive());
			m_ContentBrowserPanel = CreateShared<ContentBrowserPanel>();
			m_BuildSettingsPanel.SetContext(SP_BIND_CALLBACK(EditorLayer::OnLaunchRuntime));

			TagComponent::ResetAddedMarkers();
		}
	}

	void EditorLayer::SaveProject()
	{
		const auto& projectProps = Project::GetActive()->GetProperties();
		auto projectFilename = std::format("{}.vxproject", projectProps.General.Name);
		const auto& projectPath = Project::GetProjectDirectory() / std::filesystem::path(projectFilename);
		Project::SaveActive(projectPath);
	}

	void EditorLayer::CreateNewScene()
	{
		if (m_SceneState != SceneState::Edit)
			return;

		m_ActiveScene = Scene::Create();
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);

		m_EditorScenePath = std::filesystem::path(); // Reset the current scene path otherwise the previous scene will be overwritten
		m_EditorScene = m_ActiveScene; // Set the editors scene

		Scene::CreateDefaultEntities(m_ActiveScene);
	}

	void EditorLayer::OpenExistingScene()
	{
		std::string filepath = FileSystem::OpenFileDialog("Vortex Scene (*.vortex)\0*.vortex\0");

		if (!filepath.empty())
			OpenScene(filepath);
	}

	void EditorLayer::OpenScene(const std::filesystem::path& path)
	{
		if (m_SceneState != SceneState::Edit)
			OnSceneStop();

		m_HoveredEntity = Entity{}; // Prevent an invalid entity from being used elsewhere in the editor

		std::string sceneFilename = path.filename().string();

		if (path.extension().string() != ".vortex")
		{
			VX_WARN("Could not load {} - not a scene file", sceneFilename);
			return;
		}

		SharedRef<Scene> newScene = CreateShared<Scene>();
		SceneSerializer serializer(newScene);
		newScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);

		// Default clear color
		RenderCommand::SetClearColor(Math::vec3((38.0f / 255.0f), (44.0f / 255.0f), (60.0f / 255.0f)));

		std::string timerName = std::format("{} Scene Load Time", sceneFilename);
		InstrumentationTimer timer(timerName.c_str());

		if (serializer.Deserialize(path.string()))
		{
			m_EditorScene = newScene;
			m_SceneHierarchyPanel.SetContext(m_EditorScene);

			m_ActiveScene = m_EditorScene;
			m_EditorScenePath = path;
			std::string sceneName = sceneFilename.substr(0, sceneFilename.find('.'));
			ScriptRegistry::SetActiveSceneName(sceneName);
		}
	}

	void EditorLayer::SaveSceneAs()
	{
		std::string filepath = FileSystem::SaveFileDialog("Vortex Scene (*.vortex)\0*.vortex\0");

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

		m_ActiveScene->SortEntities();
	}

	void EditorLayer::SerializeScene(SharedRef<Scene> scene, const std::filesystem::path& path)
	{
		SceneSerializer serializer(scene);
		serializer.Serialize(path.string());
	}

	void EditorLayer::OnScenePlay()
	{
		StopAudioSources();

		if (m_SceneState == SceneState::Simulate)
			OnSceneStop();

		// Disable the debug view when starting a scene
		m_EditorDebugViewEnabled = false;

		m_SceneState = SceneState::Play;

		if (Project::GetActive()->GetProperties().EditorProps.MaximizeOnPlay)
			m_SceneViewportMaximized = true;

		m_ActiveScene = Scene::Copy(m_EditorScene);
		m_ActiveScene->OnRuntimeStart();

		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
		ScriptRegistry::SetSceneStartTime(Time::GetTime());

		OnNoGizmoSelected();
	}

	void EditorLayer::OnScenePause()
	{
		if (m_SceneState == SceneState::Edit)
			return;

		PauseAudioSources();

		m_ActiveScene->SetPaused(true);
	}

	void EditorLayer::OnSceneResume()
	{
		ResumeAudioSources();

		m_ActiveScene->SetPaused(false);
	}

	void EditorLayer::OnSceneStop()
	{
		VX_CORE_ASSERT(m_SceneState == SceneState::Play || m_SceneState == SceneState::Simulate, "Invalid scene state!");

		if (m_SceneState == SceneState::Play)
			m_ActiveScene->OnRuntimeStop();
		else if (m_SceneState == SceneState::Simulate)
			m_ActiveScene->OnPhysicsSimulationStop();

		m_SceneState = SceneState::Edit;

		if (Project::GetActive()->GetProperties().EditorProps.MaximizeOnPlay)
			m_SceneViewportMaximized = false;

		m_ActiveScene = m_EditorScene;

		m_HoveredEntity = Entity{};
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);

		// Reset the mouse cursor in case a script turned it off
		Application::Get().GetWindow().ShowMouseCursor(true);
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

		if (Project::GetActive()->GetProperties().EditorProps.MaximizeOnPlay)
			m_SceneViewportMaximized = true;

		m_ActiveScene = Scene::Copy(m_EditorScene);
		m_ActiveScene->OnPhysicsSimulationStart();

		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
	}

	void EditorLayer::RestartSceneSimulation()
	{
		OnSceneSimulate();
	}

	void EditorLayer::PauseAudioSources()
	{
		if (m_SceneState == SceneState::Play)
		{
			auto view = m_ActiveScene->GetAllEntitiesWith<AudioSourceComponent>();

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
	}

	void EditorLayer::ResumeAudioSources()
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
	}

	void EditorLayer::StopAudioSources()
	{
		auto view = m_ActiveScene->GetAllEntitiesWith<AudioSourceComponent>();

		for (auto& e : view)
		{
			Entity entity{ e, m_ActiveScene.get() };
			SharedRef<AudioSource> audioSource = entity.GetComponent<AudioSourceComponent>().Source;

			if (audioSource->IsPlaying())
				audioSource->Stop();
		}
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
		if (m_SceneState == SceneState::Edit)
			m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
	}

	void EditorLayer::OnRotationToolSelected()
	{
		if (m_SceneState == SceneState::Edit)
			m_GizmoType = ImGuizmo::OPERATION::ROTATE;
	}

	void EditorLayer::OnScaleToolSelected()
	{
		if (m_SceneState == SceneState::Edit)
			m_GizmoType = ImGuizmo::OPERATION::SCALE;
	}

}
