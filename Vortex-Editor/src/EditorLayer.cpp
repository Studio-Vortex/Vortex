#include "EditorLayer.h"

#include <Vortex/Serialization/SceneSerializer.h>
#include <Vortex/Project/ProjectLoader.h>

#include <Vortex/Scripting/ScriptEngine.h>
#include <Vortex/Scripting/ScriptRegistry.h>

#include <Vortex/Editor/EditorCamera.h>
#include <Vortex/Editor/EditorResources.h>
#include <Vortex/Editor/SelectionManager.h>

#include <Vortex/Gui/Colors.h>

#include <ImGuizmo.h>

#include <Vortex/Editor/ConsolePanel.h>

#include "Panels/PhysicsMaterialEditorPanel.h"
#include "Panels/PhysicsStatisticsPanel.h"
#include "Panels/ProjectSettingsPanel.h"
#include "Panels/NetworkManagerPanel.h"
#include "Panels/SceneHierarchyPanel.h"
#include "Panels/ContentBrowserPanel.h"
#include "Panels/ScriptRegistryPanel.h"
#include "Panels/MaterialEditorPanel.h"
#include "Panels/SceneRendererPanel.h"
#include "Panels/AssetRegistryPanel.h"
#include "Panels/BuildSettingsPanel.h"
#include "Panels/SystemManagerPanel.h"
#include "Panels/ShaderEditorPanel.h"
#include "Panels/PerformancePanel.h"
#include "Panels/SubModulesPanel.h"
#include "Panels/ECSDebugPanel.h"
#include "Panels/AboutPanel.h"

namespace Vortex {

	EditorLayer::EditorLayer()
		: Layer("EditorLayer") { }

	void EditorLayer::OnAttach()
	{
		VX_PROFILE_FUNCTION();

		const auto& appProps = Application::Get().GetProperties();

		FramebufferProperties framebufferProps;
		
		framebufferProps.Attachments = {
			ImageFormat::RGBA16F,
			ImageFormat::RED_INTEGER,
			ImageFormat::RGBA16F,
			ImageFormat::Depth
		};

		framebufferProps.Width = 1600;
		framebufferProps.Height = 900;
		framebufferProps.Samples = appProps.SampleCount;

		m_Framebuffer = Framebuffer::Create(framebufferProps);
		m_SecondViewportFramebuffer = Framebuffer::Create(framebufferProps);

		EditorResources::Init();

		m_EditorScene = nullptr;
		m_ActiveScene = m_EditorScene;

		m_ViewportSize = { appProps.WindowWidth, appProps.WindowHeight };
		m_SecondViewportSize = { appProps.WindowWidth, appProps.WindowHeight };

		m_PanelManager = PanelManager::Create();

		m_PanelManager->AddPanel<PhysicsMaterialEditorPanel>();
		m_PanelManager->AddPanel<PhysicsStatisticsPanel>();
		m_PanelManager->AddPanel<NetworkManagerPanel>();
		m_PanelManager->AddPanel<SceneHierarchyPanel>()->IsOpen = true;
		m_PanelManager->AddPanel<ScriptRegistryPanel>();
		m_PanelManager->AddPanel<MaterialEditorPanel>()->IsOpen = true;
		m_PanelManager->AddPanel<SceneRendererPanel>()->IsOpen = true;
		m_PanelManager->AddPanel<AssetRegistryPanel>();
		m_PanelManager->AddPanel<BuildSettingsPanel>(
			VX_BIND_CALLBACK(BuildAndRunProject),
			VX_BIND_CALLBACK(BuildProject)
		);
		m_PanelManager->AddPanel<SystemManagerPanel>();
		m_PanelManager->AddPanel<ShaderEditorPanel>();
		m_PanelManager->AddPanel<PerformancePanel>();
		m_PanelManager->AddPanel<SubModulesPanel>();
		m_PanelManager->AddPanel<ECSDebugPanel>();
		m_PanelManager->AddPanel<ConsolePanel>()->IsOpen = true;
		m_PanelManager->AddPanel<AboutPanel>();

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
				Application::Get().Close();
			}
		}

		SharedReference<Project> activeProject = Project::GetActive();
		const ProjectProperties& projectProps = activeProject->GetProperties();

		m_EditorCamera = new EditorCamera(
			projectProps.EditorProps.EditorCameraFOV,
			m_ViewportSize.x,
			m_ViewportSize.y,
			0.1f,
			1000.0f
		);

		m_SecondEditorCamera = new EditorCamera(
			projectProps.EditorProps.EditorCameraFOV,
			m_SecondViewportSize.x,
			m_SecondViewportSize.y,
			0.1f,
			1000.0f
		);

		m_PanelManager->OnEditorAttach();
		m_PanelManager->SetSceneContext(m_ActiveScene);
		m_PanelManager->SetProjectContext(activeProject);
	}

	void EditorLayer::OnDetach()
	{
		VX_PROFILE_FUNCTION();

		m_PanelManager->OnEditorDetach();
		m_PanelManager->SetSceneContext(nullptr);
		m_PanelManager->SetProjectContext(nullptr);

		delete m_EditorCamera;
		delete m_SecondEditorCamera;

		EditorResources::Shutdown();
	}

	void EditorLayer::OnUpdate(TimeStep delta)
	{
		VX_PROFILE_FUNCTION();

		Renderer::ResetRenderTime();
		RenderTime& renderTime = Renderer::GetRenderTime();

		// Shadow pass
		if (Entity skyLightEntity = m_ActiveScene->GetSkyLightEntity())
		{
			const auto& lsc = skyLightEntity.GetComponent<LightSourceComponent>();
			if (lsc.CastShadows)
			{
				InstrumentationTimer timer("Shadow Pass");
				Renderer::RenderToDepthMap(m_ActiveScene);
				renderTime.ShadowMapRenderTime += timer.ElapsedMS();
			}
		}

		m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);

		ResizeTargetFramebuffersIfNeeded();

		// Bind Render Target and Clear Attachments
		Renderer::ResetStats();
		Renderer2D::ResetStats();
		m_Framebuffer->Bind();
		RenderCommand::Clear();

		// Clear entityID attachment to -1
		m_Framebuffer->ClearAttachment(1, -1);

		m_EditorCamera->SetActive(m_AllowViewportCameraEvents);
		m_EditorCamera->OnUpdate(delta);

		m_SecondEditorCamera->SetActive(m_AllowSecondViewportCameraEvents);
		m_SecondEditorCamera->OnUpdate(delta);

		// Update Scene
		switch (m_SceneState)
		{
			case SceneState::Edit:
			{
				m_ActiveScene->OnUpdateEditor(delta, m_EditorCamera);
				break;
			}
			case SceneState::Play:
			{
				const bool scenePaused = m_ActiveScene->IsPaused();

				if (scenePaused)
					OnScenePause();
				else
					OnSceneResume();

				m_ActiveScene->OnUpdateRuntime(delta);

				break;
			}
			case SceneState::Simulate:
			{
				m_ActiveScene->OnUpdateSimulation(delta, m_EditorCamera);
				break;
			}
		}

		// Scene Viewport Entity Selection
		{
			auto [mx, my] = Gui::GetMousePos();
			mx -= m_ViewportBounds[0].x;
			my -= m_ViewportBounds[0].y;
			Math::vec2 viewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];
			my = viewportSize.y - my;

			int mouseX = (int)mx;
			int mouseY = (int)my;

			if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x && mouseY < (int)viewportSize.y)
			{
				int pixelData = m_Framebuffer->ReadPixel(1, mouseX, mouseY);
				m_HoveredEntity = pixelData == -1 ? Entity() : Entity{ (entt::entity)pixelData, m_ActiveScene.Raw() };
				ScriptRegistry::SetHoveredEntity(m_HoveredEntity);
			}
		}

		OnOverlayRender(m_EditorCamera, false);

		m_Framebuffer->Unbind();

		// Bloom pass
		{
			InstrumentationTimer timer = InstrumentationTimer("Bloom Pass");
			Math::vec3 cameraPos = m_EditorCamera->GetPosition();
			if (m_SceneState == SceneState::Play)
			{
				if (Entity primaryCamera = m_ActiveScene->GetPrimaryCameraEntity())
					cameraPos = primaryCamera.GetTransform().Translation;
			}

			PostProcessProperties postProcessProps{};
			postProcessProps.TargetFramebuffer = m_Framebuffer;

			postProcessProps.CameraPosition = cameraPos;
			postProcessProps.ViewportSize = Viewport{ 0, 0, (uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y };
			PostProcessStage stages[] = { PostProcessStage::Bloom };
			postProcessProps.Stages = stages;
			postProcessProps.StageCount = VX_ARRAYCOUNT(stages);
			Renderer::BeginPostProcessingStages(postProcessProps);
			renderTime.BloomPassRenderTime += timer.ElapsedMS();
		}

		if (m_ShowSecondViewport)
		{
			// Second Viewport
			m_SecondViewportFramebuffer->Bind();
			RenderCommand::Clear();

			// Clear entityID attachment to -1
			m_SecondViewportFramebuffer->ClearAttachment(1, -1);

			SceneRenderPacket renderPacket{};
			renderPacket.MainCamera = m_SecondEditorCamera;
			renderPacket.MainCameraViewMatrix = m_SecondEditorCamera->GetViewMatrix();
			renderPacket.MainCameraProjectionMatrix = m_SecondEditorCamera->GetProjectionMatrix();
			renderPacket.TargetFramebuffer = m_SecondViewportFramebuffer;
			renderPacket.Scene = m_ActiveScene.Raw();
			renderPacket.EditorScene = true;
			m_SecondViewportRenderer.RenderScene(renderPacket);

			// Scene Viewport Entity Selection
			{
				auto [mx, my] = Gui::GetMousePos();
				mx -= m_SecondViewportBounds[0].x;
				my -= m_SecondViewportBounds[0].y;
				Math::vec2 viewportSize = m_SecondViewportBounds[1] - m_SecondViewportBounds[0];
				my = viewportSize.y - my;

				int mouseX = (int)mx;
				int mouseY = (int)my;

				if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x && mouseY < (int)viewportSize.y)
				{
					int pixelData = m_SecondViewportFramebuffer->ReadPixel(1, mouseX, mouseY);
					m_HoveredEntity = pixelData == -1 ? Entity() : Entity{ (entt::entity)pixelData, m_ActiveScene.Raw() };
					ScriptRegistry::SetHoveredEntity(m_HoveredEntity);
				}
			}

			OnOverlayRender(m_SecondEditorCamera, true);

			m_SecondViewportFramebuffer->Unbind();
		}

		const bool altDown = Input::IsKeyDown(KeyCode::LeftAlt) || Input::IsKeyDown(KeyCode::RightAlt);
		const bool leftMouseButtonDown = Input::IsMouseButtonDown(MouseButton::Left);
		const bool rightMouseButtonDown = Input::IsMouseButtonDown(MouseButton::Right);
		const bool middleMouseButtonDown = Input::IsMouseButtonDown(MouseButton::Middle);

		if (((altDown && (leftMouseButtonDown || middleMouseButtonDown)) || rightMouseButtonDown))
		{
			if (!m_StartedClickInViewport && m_SceneViewportFocused && m_SceneViewportHovered)
				m_StartedClickInViewport = true;
			else if (!m_StartedClickInSecondViewport && m_SecondViewportFocused && m_SecondViewportHovered)
				m_StartedClickInSecondViewport = true;
		}

		if (!rightMouseButtonDown && !(altDown && (leftMouseButtonDown || (middleMouseButtonDown))))
		{
			m_StartedClickInViewport = m_StartedClickInSecondViewport = false;
		}

		const bool pendingTransisiton = ScriptRegistry::HasPendingTransitionQueued();
		if (m_SceneState == SceneState::Play && pendingTransisiton)
		{
			QueueSceneTransition();
		}
	}

	void EditorLayer::OnEvent(Event& e)
	{
		if (m_AllowViewportCameraEvents)
		{
			m_EditorCamera->OnEvent(e);
		}
		else if (m_AllowSecondViewportCameraEvents)
		{
			m_SecondEditorCamera->OnEvent(e);
		}

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowDragDropEvent>(VX_BIND_CALLBACK(EditorLayer::OnWindowDragDropEvent));
		dispatcher.Dispatch<KeyPressedEvent>(VX_BIND_CALLBACK(EditorLayer::OnKeyPressedEvent));
		dispatcher.Dispatch<MouseButtonPressedEvent>(VX_BIND_CALLBACK(EditorLayer::OnMouseButtonPressedEvent));
		dispatcher.Dispatch<WindowCloseEvent>(VX_BIND_CALLBACK(EditorLayer::OnWindowCloseEvent));
	}

	void EditorLayer::OnGuiRender()
	{
		VX_PROFILE_FUNCTION();

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
		{
			dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
		}

		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
		{
			window_flags |= ImGuiWindowFlags_NoBackground;
		}
		if (!opt_padding)
		{
			Gui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		}

		Gui::Begin("Engine Dockspace", &dockspaceOpen, window_flags);
		if (!opt_padding)
		{
			Gui::PopStyleVar();
		}

		if (opt_fullscreen)
		{
			Gui::PopStyleVar(2);
		}

		ImGuiIO& io = Gui::GetIO();
		ImGuiStyle& style = Gui::GetStyle();
		float minWinSizeX = style.WindowMinSize.x;
		style.WindowMinSize.x = 370.0f;
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = Gui::GetID("Engine Dockspace");
			Gui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		} // End Dockspace

		style.WindowMinSize.x = minWinSizeX;

		if ((Gui::IsMouseClicked(ImGuiMouseButton_Left) || Gui::IsMouseClicked(ImGuiMouseButton_Right)) && (!m_StartedClickInViewport || !m_StartedClickInSecondViewport))
		{
			ImGuiWindow* hoveredWindow = GImGui->HoveredWindow;
			if (hoveredWindow)
			{
				std::string name = hoveredWindow->Name;
				if (name == "Scene" || name == "Second Viewport")
				{
					Gui::FocusWindow(hoveredWindow);
				}
			}
		}

		OnMainMenuBarRender();

		// Render Panels if the scene isn't maximized
		if (!m_SceneViewportMaximized)
		{
			m_PanelManager->OnGuiRender<PhysicsMaterialEditorPanel>();
			m_PanelManager->OnGuiRender<PhysicsStatisticsPanel>();
			m_PanelManager->OnGuiRender<ProjectSettingsPanel>();
			m_PanelManager->OnGuiRender<NetworkManagerPanel>();
			m_PanelManager->GetPanel<SceneHierarchyPanel>()->OnGuiRender(m_HoveredEntity, m_EditorCamera);
			m_PanelManager->OnGuiRender<ContentBrowserPanel>();
			m_PanelManager->OnGuiRender<ScriptRegistryPanel>();
			m_PanelManager->OnGuiRender<MaterialEditorPanel>();
			m_PanelManager->OnGuiRender<SceneRendererPanel>();
			m_PanelManager->OnGuiRender<AssetRegistryPanel>();
			m_PanelManager->OnGuiRender<BuildSettingsPanel>();
			m_PanelManager->OnGuiRender<SystemManagerPanel>();
			m_PanelManager->OnGuiRender<ShaderEditorPanel>();
			m_PanelManager->OnGuiRender<SubModulesPanel>();
			m_PanelManager->OnGuiRender<ECSDebugPanel>();
			m_PanelManager->OnGuiRender<ConsolePanel>();
			m_PanelManager->OnGuiRender<AboutPanel>();
		}

		// Always render if open
		m_PanelManager->OnGuiRender<PerformancePanel>();

		// Update Engine Systems Gui
		SystemManager::OnGuiRender();

		// Update C# Entity.OnGui()
		m_ActiveScene->OnUpdateEntityGui();

		if (m_ShowScenePanel)
		{
			OnScenePanelRender();
		}
		if (m_ShowSecondViewport)
		{
			OnSecondViewportPanelRender();
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
			EditorCamera* camera = m_SceneViewportHovered ? m_EditorCamera : m_SecondEditorCamera;
			m_PanelManager->GetPanel<SceneHierarchyPanel>()->DisplayCreateEntityMenu(camera);

			Gui::PopStyleVar();
			Gui::EndPopup();
		}

		Gui::End();
	}

	void EditorLayer::OnMainMenuBarRender()
	{
		VX_PROFILE_FUNCTION();

		ImGuiIO& io = Gui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];
		auto largeFont = io.Fonts->Fonts[1];

		SharedReference<Project> activeProject = Project::GetActive();
		ProjectProperties& projectProps = activeProject->GetProperties();

		if (Gui::BeginMenuBar())
		{
			bool inEditMode = m_SceneState == SceneState::Edit;
			bool inPlayMode = m_SceneState == SceneState::Play;
			bool inSimulateMode = m_SceneState == SceneState::Simulate;

			if (Gui::BeginMenu("File"))
			{
				if (Gui::MenuItem("New Project"))
				{
					CreateNewProject();
					Gui::CloseCurrentPopup();
				}
				UI::Draw::Underline();

				if (Gui::MenuItem("Open Project...", "Ctrl+O"))
				{
					OpenExistingProject();
					Gui::CloseCurrentPopup();
				}
				UI::Draw::Underline();

				if (Gui::MenuItem("Save Project"))
				{
					SaveProject();
					Gui::CloseCurrentPopup();
				}
				UI::Draw::Underline();

				if (Gui::MenuItem("New Scene", "Ctrl+N"))
				{
					CreateNewScene();
					Gui::CloseCurrentPopup();
				}
				UI::Draw::Underline();

				if (Gui::MenuItem("Open Scene..."))
				{
					OpenExistingScene();
					Gui::CloseCurrentPopup();
				}
				UI::Draw::Underline();

				if (Gui::MenuItem("Save", "Ctrl+S"))
				{
					SaveScene();
					Gui::CloseCurrentPopup();
				}
				UI::Draw::Underline();

				if (Gui::MenuItem("Save As...", "Ctrl+Shift+S"))
				{
					SaveSceneAs();
					Gui::CloseCurrentPopup();
				}
				UI::Draw::Underline();

				if (Gui::MenuItem("Exit", "Alt+F4"))
				{
					Application::Get().Close();
					Gui::CloseCurrentPopup();
				}

				Gui::EndMenu();
			}

			if (Gui::BeginMenu("Edit"))
			{
				if (inEditMode)
				{
					Entity selectedEntity = SelectionManager::GetSelectedEntity();

					if (selectedEntity)
					{
						if (Gui::MenuItem("Move To Camera Position"))
						{
							TransformComponent& transform = selectedEntity.GetTransform();
							transform.Translation = m_EditorCamera->GetPosition();
							transform.SetRotationEuler(Math::vec3(-m_EditorCamera->GetPitch(), -m_EditorCamera->GetYaw(), transform.GetRotationEuler().z));
							Gui::CloseCurrentPopup();
						}

						UI::Draw::Underline();
					}

					if (Gui::MenuItem("Play Scene", "Ctrl+P"))
					{
						OnScenePlay();
						Gui::CloseCurrentPopup();
					}
					UI::Draw::Underline();

					if (Gui::MenuItem("Play Simulation", "Ctrl+X"))
					{
						OnSceneSimulate();
						Gui::CloseCurrentPopup();
					}

					if (selectedEntity)
					{
						UI::Draw::Underline();

						if (Gui::MenuItem("Rename Entity", "F2"))
						{
							m_PanelManager->GetPanel<SceneHierarchyPanel>()->EditSelectedEntityName(true);
							Gui::CloseCurrentPopup();
						}
						UI::Draw::Underline();

						if (Gui::MenuItem("Duplicate Entity", "Ctrl+D"))
						{
							DuplicateSelectedEntity();
							Gui::CloseCurrentPopup();
						}
						UI::Draw::Underline();

						if (Gui::MenuItem("Delete Entity", "Del"))
						{
							SelectionManager::DeselectEntity();
							m_ActiveScene->SubmitToDestroyEntity(selectedEntity);
							Gui::CloseCurrentPopup();
						}
					}
				}
				else
				{
					if (inPlayMode)
					{
						if (Gui::MenuItem("Stop Scene", "Ctrl+P"))
						{
							OnSceneStop();
							Gui::CloseCurrentPopup();
						}
						UI::Draw::Underline();

						if (Gui::MenuItem("Restart Scene", "Ctrl+Shift+P"))
						{
							RestartScene();
							Gui::CloseCurrentPopup();
						}
					}
					else if (inSimulateMode)
					{
						if (Gui::MenuItem("Stop Simulation", "Ctrl+X"))
						{
							OnSceneStop();
							Gui::CloseCurrentPopup();
						}
						UI::Draw::Underline();

						if (Gui::MenuItem("Restart Simulation", "Ctrl+Shift+X"))
						{
							RestartSceneSimulation();
							Gui::CloseCurrentPopup();
						}
					}
				}

				Gui::EndMenu();
			}

			if (Gui::BeginMenu("View"))
			{
				Gui::MenuItem("Maximize On Play", nullptr, &projectProps.EditorProps.MaximizeOnPlay);
				UI::Draw::Underline();

				if (m_SceneViewportMaximized)
				{
					if (Gui::MenuItem("Minimize Scene", "Ctrl+Space"))
					{
						m_SceneViewportMaximized = false;
						Gui::CloseCurrentPopup();
					}
				}
				else
				{
					if (Gui::MenuItem("Maximize Scene", "Ctrl+Space"))
					{
						m_SceneViewportMaximized = true;
						Gui::CloseCurrentPopup();
					}
				}

				Gui::EndMenu();
			}

			if (Gui::BeginMenu("Tools"))
			{
				if (Gui::MenuItem("No Selection", "Q"))
				{
					OnNoGizmoSelected();
					Gui::CloseCurrentPopup();
				}
				UI::Draw::Underline();

				if (Gui::MenuItem("Translation Tool", "W"))
				{
					OnTranslationToolSelected();
					Gui::CloseCurrentPopup();
				}
				UI::Draw::Underline();

				if (Gui::MenuItem("Rotation Tool", "E"))
				{
					OnRotationToolSelected();
					Gui::CloseCurrentPopup();
				}
				UI::Draw::Underline();

				if (Gui::MenuItem("Scale Tool", "R"))
				{
					OnScaleToolSelected();
					Gui::CloseCurrentPopup();
				}

				Gui::EndMenu();
			}

			if (Gui::BeginMenu("Script"))
			{
				auto projectSolutionFilename = std::filesystem::path(activeProject->GetName());
				projectSolutionFilename.replace_extension(".sln");
				std::filesystem::path scriptsFolder = std::filesystem::path("Projects") / activeProject->GetName() / "Assets\\Scripts";
				std::filesystem::path solutionPath = scriptsFolder / projectSolutionFilename;

				if (Gui::MenuItem("Create Script"))
				{
					m_OpenCreateScriptPopup = true;
					Gui::CloseCurrentPopup();
				}
				UI::Draw::Underline();

				if (Gui::MenuItem("Open Visual Studio Solution"))
				{
					FileSystem::SetWorkingDirectory(scriptsFolder);
					Platform::LaunchProcess(projectSolutionFilename.string().c_str(), "");
					FileSystem::SetWorkingDirectory(Application::Get().GetProperties().WorkingDirectory);
					Gui::CloseCurrentPopup();
				}
				UI::Draw::Underline();

				if (Gui::MenuItem("Rebuild C# Assembly"))
				{
					FileSystem::SetWorkingDirectory("Resources/HelperScripts");
					Platform::LaunchProcess("BuildSolution.bat", ("..\\..\\" / solutionPath).string().c_str());
					FileSystem::SetWorkingDirectory(Application::Get().GetProperties().WorkingDirectory);
					Gui::CloseCurrentPopup();
				}

				Gui::EndMenu();
			}

			if (Gui::BeginMenu("Build"))
			{
				if (Gui::MenuItem("Build"))
				{
					BuildProject();
					Gui::CloseCurrentPopup();
				}
				UI::Draw::Underline();

				if (Gui::MenuItem("Build and Run", "Ctrl+B"))
				{
					BuildAndRunProject();
					Gui::CloseCurrentPopup();
				}
				UI::Draw::Underline();

				m_PanelManager->MainMenuBarItem<BuildSettingsPanel>("Ctrl+Shift+B");

				Gui::EndMenu();
			}
			
			if (Gui::BeginMenu("Window"))
			{
				m_PanelManager->MainMenuBarItem<ConsolePanel>();
				UI::Draw::Underline();
				m_PanelManager->MainMenuBarItem<ContentBrowserPanel>();
				UI::Draw::Underline();
				Gui::MenuItem("Inspector", nullptr, &m_PanelManager->GetPanel<SceneHierarchyPanel>()->IsInspectorOpen());
				UI::Draw::Underline();
				m_PanelManager->MainMenuBarItem<MaterialEditorPanel>();
				UI::Draw::Underline();
				m_PanelManager->MainMenuBarItem<PhysicsMaterialEditorPanel>();
				UI::Draw::Underline();
				Gui::MenuItem("Scene", nullptr, &m_ShowScenePanel);
				UI::Draw::Underline();
				m_PanelManager->MainMenuBarItem<SceneHierarchyPanel>();
				UI::Draw::Underline();
				m_PanelManager->MainMenuBarItem<SceneRendererPanel>();
				UI::Draw::Underline();
				Gui::MenuItem("Second Viewport", nullptr, &m_ShowSecondViewport);
				UI::Draw::Underline();
				m_PanelManager->MainMenuBarItem<ShaderEditorPanel>();
				UI::Draw::Underline();
				m_PanelManager->MainMenuBarItem<ProjectSettingsPanel>();
				UI::Draw::Underline();

				if (Gui::BeginMenu("Debug"))
				{
					m_PanelManager->MainMenuBarItem<AssetRegistryPanel>();
					UI::Draw::Underline();
					m_PanelManager->MainMenuBarItem<ECSDebugPanel>();
					UI::Draw::Underline();
					m_PanelManager->MainMenuBarItem<NetworkManagerPanel>();
					UI::Draw::Underline();
					m_PanelManager->MainMenuBarItem<PerformancePanel>();
					UI::Draw::Underline();
					m_PanelManager->MainMenuBarItem<PhysicsStatisticsPanel>();
					UI::Draw::Underline();
					m_PanelManager->MainMenuBarItem<ScriptRegistryPanel>();
					UI::Draw::Underline();
					m_PanelManager->MainMenuBarItem<SubModulesPanel>();
					UI::Draw::Underline();
					m_PanelManager->MainMenuBarItem<SystemManagerPanel>();

					Gui::EndMenu();
				}

				Gui::EndMenu();
			}

			if (Gui::BeginMenu("Help"))
			{
				m_PanelManager->MainMenuBarItem<AboutPanel>();

				Gui::EndMenu();
			}

			Gui::EndMenuBar();
		}
	}

	void EditorLayer::OnScenePanelRender()
	{
		VX_PROFILE_FUNCTION();

		UI::ScopedStyle windowPadding(ImGuiStyleVar_WindowPadding, ImVec2{ 0.0f, 0.0f });
		Gui::Begin("Scene", &m_ShowScenePanel, ImGuiWindowFlags_NoCollapse);
		auto viewportMinRegion = Gui::GetWindowContentRegionMin();
		auto viewportMaxRegion = Gui::GetWindowContentRegionMax();
		auto viewportOffset = Gui::GetWindowPos();
		m_ViewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
		m_ViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

		ImVec2 minBound = { m_ViewportBounds[0].x, m_ViewportBounds[0].y };
		ImVec2 maxBound = { m_ViewportBounds[1].x, m_ViewportBounds[1].y };

		m_AllowViewportCameraEvents = (ImGui::IsMouseHoveringRect(minBound, maxBound) && m_SceneViewportFocused) || m_StartedClickInViewport;

		m_SceneViewportFocused = Gui::IsWindowFocused();
		m_SceneViewportHovered = Gui::IsWindowHovered();
		Application::Get().GetGuiLayer()->BlockEvents(!m_SceneViewportHovered && !m_SecondViewportFocused);

		ImVec2 scenePanelSize = Gui::GetContentRegionAvail();
		m_ViewportSize = { scenePanelSize.x, scenePanelSize.y };

		uint32_t sceneTextureID = m_Framebuffer->GetColorAttachmentRendererID();
		UI::ImageEx(sceneTextureID, ImVec2{ m_ViewportSize.x, m_ViewportSize.y });

		UIHandleAssetDrop();
		UIOnPopupRender();

		if (Gui::IsItemVisible())
		{
			UI_GizmosToolbar();
			UI_CentralToolbar();
			UI_SceneSettingsToolbar();
		}

		if (m_SceneViewportHovered || !m_ShowSecondViewport)
		{
			OnGizmosRender(m_EditorCamera, m_ViewportBounds, false);
		}

		Gui::End();
	}

	void EditorLayer::UIHandleAssetDrop()
	{
		// Accept Items from the content browser
		if (Gui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				const wchar_t* path = (const wchar_t*)payload->Data;
				std::filesystem::path filepath = std::filesystem::path(path);

				AssetType assetType = Project::GetEditorAssetManager()->GetAssetTypeFromFilepath(filepath);

				if (assetType == AssetType::None)
				{
					VX_CONSOLE_LOG_ERROR("Could not load asset with AssetType of none!");
					Gui::EndDragDropTarget();
					return;
				}

				switch (assetType)
				{
					case AssetType::MeshAsset:
					{
						break;
					}
					case AssetType::FontAsset:
					{
						break;
					}
					case AssetType::AudioAsset:
					{
						break;
					}
					case AssetType::SceneAsset:
					{
						OpenScene(filepath);

						break;
					}
					case AssetType::PrefabAsset:
					{
						break;
					}
					case AssetType::ScriptAsset:
					{
						break;
					}
					case AssetType::TextureAsset:
					{
						if (!m_HoveredEntity.HasAny<SpriteRendererComponent, StaticMeshRendererComponent>())
							break;

						std::filesystem::path textureFilepath = filepath;

						AssetHandle textureHandle = Project::GetEditorAssetManager()->GetAssetHandleFromFilepath(textureFilepath);

						if (AssetManager::IsHandleValid(textureHandle))
						{
							SharedReference<StaticMesh> staticMesh = AssetManager::GetAsset<StaticMesh>(textureHandle);

							if (m_HoveredEntity && m_HoveredEntity.HasComponent<SpriteRendererComponent>())
							{
								m_HoveredEntity.GetComponent<SpriteRendererComponent>().Texture = textureHandle;
							}
							else if (m_HoveredEntity && m_HoveredEntity.HasComponent<StaticMeshRendererComponent>())
							{
								auto& staticMeshRendererComponent = m_HoveredEntity.GetComponent<StaticMeshRendererComponent>();
								AssetHandle staticMeshHandle = staticMeshRendererComponent.StaticMesh;

								auto& materialTable = staticMeshRendererComponent.Materials;

								if (AssetManager::IsHandleValid(staticMeshHandle))
								{
									std::string filename = textureFilepath.filename().string();
									// TODO this should be dynamic
									AssetHandle materialHandle = materialTable->GetMaterial(0);

									if (AssetManager::IsHandleValid(materialHandle))
									{
										SharedReference<Material> material = AssetManager::GetAsset<Material>(materialHandle);

										if (material)
										{
											if (filename.find("albedo") != std::string::npos || filename.find("diffuse") != std::string::npos || filename.find("base_color") != std::string::npos)
												material->SetAlbedoMap(textureHandle);
											if (filename.find("normal") != std::string::npos)
												material->SetNormalMap(textureHandle);
											if (filename.find("metallic") != std::string::npos || filename.find("specular") != std::string::npos)
												material->SetMetallicMap(textureHandle);
											if (filename.find("roughness") != std::string::npos)
												material->SetRoughnessMap(textureHandle);
											if (filename.find("emissive") != std::string::npos || filename.find("emission") != std::string::npos)
												material->SetEmissionMap(textureHandle);
											if (filename.find("height") != std::string::npos || filename.find("displacement") != std::string::npos)
												material->SetParallaxOcclusionMap(textureHandle);
											if (filename.find("ao") != std::string::npos)
												material->SetAmbientOcclusionMap(textureHandle);
										}
									}
								}
							}
						}
						else
						{
							VX_CONSOLE_LOG_WARN("Could not load texture - {}", textureFilepath.filename().string());
						}

						break;
					}
					case AssetType::MaterialAsset:
					{
						if (!m_HoveredEntity.HasAny<MeshRendererComponent, StaticMeshRendererComponent>())
							break;

						std::filesystem::path materialFilepath = filepath;

						AssetHandle materialHandle = Project::GetEditorAssetManager()->GetAssetHandleFromFilepath(materialFilepath);

						if (AssetManager::IsHandleValid(materialHandle))
						{
							SharedReference<Material> material = AssetManager::GetAsset<Material>(materialHandle);

							if (material)
							{
								// TODO we should be able to set an actual index here eventually instead of being hardcoded

								if (m_HoveredEntity.HasComponent<MeshRendererComponent>())
								{
									auto& meshRendererComponent = m_HoveredEntity.GetComponent<MeshRendererComponent>();
									auto& materialTable = meshRendererComponent.Materials;
									
									materialTable->SetMaterial(0, materialHandle);
								}
								else if (m_HoveredEntity.HasComponent<StaticMeshRendererComponent>())
								{
									auto& staticMeshRendererComponent = m_HoveredEntity.GetComponent<StaticMeshRendererComponent>();
									auto& materialTable = staticMeshRendererComponent.Materials;
									
									materialTable->SetMaterial(0, materialHandle);
								}

								material->SetName(FileSystem::RemoveFileExtension(materialFilepath));
							}
						}
						else
						{
							VX_CONSOLE_LOG_WARN("Could not load material - {}", materialFilepath.filename().string());
						}

						break;
					}
					case AssetType::AnimatorAsset:
					{
						break;
					}
					case AssetType::AnimationAsset:
					{
						break;
					}
					case AssetType::StaticMeshAsset:
					{
						std::filesystem::path modelPath = filepath;

						if (m_HoveredEntity && m_HoveredEntity.HasComponent<StaticMeshRendererComponent>())
						{
							m_OpenMeshImportPopup = true;
							m_MeshFilepath = modelPath.string();
							m_MeshEntityToEdit = m_HoveredEntity;
						}

						break;
					}
					case AssetType::EnvironmentAsset:
					{
						break;
					}
					case AssetType::PhysicsMaterialAsset:
					{
						break;
					}
				}
			}

			Gui::EndDragDropTarget();
		}
	}

	void EditorLayer::UIOnPopupRender()
	{
		OnCreateScriptPopupRender();
		OnMeshImportPopupRender();
	}

	void EditorLayer::OnGizmosRender(EditorCamera* editorCamera, Math::vec2 viewportBounds[2], bool allowInPlayMode)
	{
		VX_PROFILE_FUNCTION();

		SharedReference<Project> activeProject = Project::GetActive();
		const ProjectProperties& projectProps = activeProject->GetProperties();

		// Render Gizmos
		Entity selectedEntity = SelectionManager::GetSelectedEntity();

		const bool notInPlayMode = m_SceneState != SceneState::Play;
		const bool validGizmoTool = m_GizmoType != -1;
		const bool altDown = Input::IsKeyDown(KeyCode::LeftAlt) || Input::IsKeyDown(KeyCode::RightAlt);
		const bool rightMouseButtonDown = Input::IsMouseButtonDown(MouseButton::Right);
		bool showGizmos;

		if (allowInPlayMode)
			showGizmos = (selectedEntity && validGizmoTool && !altDown && !rightMouseButtonDown);
		else
			showGizmos = (selectedEntity && validGizmoTool && !altDown && !rightMouseButtonDown && notInPlayMode);

		if (showGizmos)
		{
			ImGuizmo::Enable(projectProps.GizmoProps.Enabled);
			ImGuizmo::SetOrthographic(projectProps.GizmoProps.IsOrthographic);
			ImGuizmo::SetDrawlist();

			ImGuizmo::SetRect(viewportBounds[0].x, viewportBounds[0].y, viewportBounds[1].x - viewportBounds[0].x, viewportBounds[1].y - viewportBounds[0].y);

			// Editor camera
			const Math::mat4& cameraProjection = editorCamera->GetProjectionMatrix();
			Math::mat4 cameraView = editorCamera->GetViewMatrix();

			// Entity transform
			TransformComponent& entityTransform = selectedEntity.GetTransform();
			Math::mat4 transform = m_ActiveScene->GetWorldSpaceTransformMatrix(selectedEntity);

			// Snapping
			const bool controlDown = Input::IsKeyDown(KeyCode::LeftControl) || Input::IsKeyDown(KeyCode::RightControl);
			const bool snapEnabled = projectProps.GizmoProps.SnapEnabled && controlDown;
			
			const float snapValue = m_GizmoType == ImGuizmo::ROTATE ? projectProps.GizmoProps.RotationSnapValue : projectProps.GizmoProps.SnapValue;
			std::array<float, 3> snapValues{};
			snapValues.fill(snapValue);

			ImGuizmo::Manipulate(
				Math::ValuePtr(cameraView),
				Math::ValuePtr(cameraProjection),
				(ImGuizmo::OPERATION)m_GizmoType,
				(ImGuizmo::MODE)m_TranslationMode,
				Math::ValuePtr(transform),
				nullptr,
				snapEnabled ? snapValues.data() : nullptr
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
	}

	void EditorLayer::OnSecondViewportPanelRender()
	{
		VX_PROFILE_FUNCTION();

		UI::ScopedStyle windowPadding(ImGuiStyleVar_WindowPadding, ImVec2{ 0.0f, 0.0f });
		Gui::Begin("Second Viewport", &m_ShowSecondViewport, ImGuiWindowFlags_NoCollapse);
		auto viewportMinRegion = Gui::GetWindowContentRegionMin();
		auto viewportMaxRegion = Gui::GetWindowContentRegionMax();
		auto viewportOffset = Gui::GetWindowPos();
		m_SecondViewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
		m_SecondViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

		ImVec2 minBound = { m_SecondViewportBounds[0].x, m_SecondViewportBounds[0].y };
		ImVec2 maxBound = { m_SecondViewportBounds[1].x, m_SecondViewportBounds[1].y };

		m_AllowSecondViewportCameraEvents = (ImGui::IsMouseHoveringRect(minBound, maxBound) && m_SecondViewportFocused) || m_StartedClickInSecondViewport;

		m_SecondViewportFocused = Gui::IsWindowFocused();
		m_SecondViewportHovered = Gui::IsWindowHovered();
		Application::Get().GetGuiLayer()->BlockEvents(!m_SecondViewportHovered && !m_SceneViewportFocused);

		ImVec2 scenePanelSize = Gui::GetContentRegionAvail();
		m_SecondViewportSize = { scenePanelSize.x, scenePanelSize.y };

		uint32_t sceneTextureID = m_SecondViewportFramebuffer->GetColorAttachmentRendererID();
		UI::ImageEx(sceneTextureID, ImVec2{ m_SecondViewportSize.x, m_SecondViewportSize.y });

		if (m_SecondViewportHovered)
		{
			OnGizmosRender(m_SecondEditorCamera, m_SecondViewportBounds, true);
		}

		Gui::End();
	}

	void EditorLayer::ResizeTargetFramebuffersIfNeeded()
	{
		VX_PROFILE_FUNCTION();

		// Resize
		if (FramebufferProperties props = m_Framebuffer->GetProperties();
			m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f && // zero sized framebuffer is invalid
			(props.Width != m_ViewportSize.x || props.Height != m_ViewportSize.y))
		{
			m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			m_EditorCamera->SetViewportSize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		}

		if (m_ShowSecondViewport)
		{
			if (FramebufferProperties props = m_SecondViewportFramebuffer->GetProperties();
				m_SecondViewportSize.x > 0.0f && m_SecondViewportSize.y > 0.0f && // zero sized framebuffer is invalid
				(props.Width != m_SecondViewportSize.x || props.Height != m_SecondViewportSize.y))
			{
				m_SecondViewportFramebuffer->Resize((uint32_t)m_SecondViewportSize.x, (uint32_t)m_SecondViewportSize.y);
				m_SecondEditorCamera->SetViewportSize((uint32_t)m_SecondViewportSize.x, (uint32_t)m_SecondViewportSize.y);
			}
		}
	}

	void EditorLayer::UI_GizmosToolbar()
	{
		VX_PROFILE_FUNCTION();

		UI::ScopedStyle disableSpacing(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		UI::ScopedStyle disableWindowBorder(ImGuiStyleVar_WindowBorderSize, 0.0f);
		UI::ScopedStyle windowRounding(ImGuiStyleVar_WindowRounding, 4.0f);
		UI::ScopedStyle disablePadding(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		UI::ScopedColor buttonBackground(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

		const ImVec4 normalColor = { 1.0f, 1.0f, 1.0f, 0.0f };
		const ImVec4 bgColor = { 0.7f, 0.7f, 0.7f, 1.0f };
		const ImVec4 tintColor = { 1.0f, 1.0f, 1.0f, 1.0f };

		const float buttonSize = 18.0f;
		const float edgeOffset = 4.0f;
		const float windowHeight = 32.0f; // annoying limitation of ImGui, window can't be smaller than 32 pixels
		const float numberOfButtons = 4.0f;
		const float backgroundWidth = edgeOffset * 6.0f + buttonSize * numberOfButtons + edgeOffset * (numberOfButtons - 1.0f) * 2.0f;
		const ImVec2 textureSize = { buttonSize, buttonSize };

		Gui::SetNextWindowPos(ImVec2(m_ViewportBounds[0].x + 14, m_ViewportBounds[0].y + edgeOffset));
		Gui::SetNextWindowSize(ImVec2(backgroundWidth, windowHeight));
		Gui::SetNextWindowBgAlpha(0.0f);
		Gui::Begin("Gizmos Toolbar", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking);

		const float desiredHeight = 26.0f;
		ImRect background = UI::RectExpanded(Gui::GetCurrentWindow()->Rect(), 0.0f, -(windowHeight - desiredHeight) / 2.0f);
		Gui::GetWindowDrawList()->AddRectFilled(background.Min, background.Max, IM_COL32(15, 15, 15, 127), 4.0f);

		Gui::BeginVertical("##viewportGizmosToolbarV", { backgroundWidth, Gui::GetContentRegionAvail().y });
		Gui::Spring();
		Gui::BeginHorizontal("##viewportGizmosToolbarH", { backgroundWidth, Gui::GetContentRegionAvail().y });
		Gui::Spring();

		if (UI::ImageButtonEx(EditorResources::SelectToolIcon, textureSize, m_GizmoType == -1 ? bgColor : normalColor, tintColor))
			OnNoGizmoSelected();
		UI::SetTooltip("Select Tool");

		if (UI::ImageButtonEx(EditorResources::TranslateToolIcon, textureSize, m_GizmoType == 0 ? bgColor : normalColor, tintColor))
			OnTranslationToolSelected();
		UI::SetTooltip("Translate Tool");

		if (UI::ImageButtonEx(EditorResources::RotateToolIcon, textureSize, m_GizmoType == 1 ? bgColor : normalColor, tintColor))
			OnRotationToolSelected();
		UI::SetTooltip("Rotate Tool");

		if (UI::ImageButtonEx(EditorResources::ScaleToolIcon, textureSize, m_GizmoType == 2 ? bgColor : normalColor, tintColor))
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
		VX_PROFILE_FUNCTION();

		SharedReference<Project> activeProject = Project::GetActive();
		const ProjectProperties& projectProps = activeProject->GetProperties();

		UI::ScopedStyle disableSpacing(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		UI::ScopedStyle disableWindowBorder(ImGuiStyleVar_WindowBorderSize, 0.0f);
		UI::ScopedStyle windowRounding(ImGuiStyleVar_WindowRounding, 4.0f);
		UI::ScopedStyle disablePadding(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		UI::ScopedColor buttonBackground(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

		const ImVec4 normalColor = { 1.0f, 1.0f, 1.0f, 0.0f };
		const ImVec4 bgColor = { 0.7f, 0.7f, 0.7f, 1.0f };
		const ImVec4 tintColor = { 1.0f, 1.0f, 1.0f, 1.0f };

		const float buttonSize = 18.0f;
		const float edgeOffset = 4.0f;
		const float windowHeight = 32.0f; // annoying limitation of ImGui, window can't be smaller than 32 pixels
		const float numberOfButtons = 3.0f;
		const float backgroundWidth = edgeOffset * 6.0f + buttonSize * numberOfButtons + edgeOffset * (numberOfButtons - 1.0f) * 2.0f;
		const ImVec2 textureSize = { buttonSize, buttonSize };

		float toolbarX = (m_ViewportBounds[0].x + m_ViewportBounds[1].x) / 2.0f;
		Gui::SetNextWindowPos(ImVec2(toolbarX - (backgroundWidth / 2.0f), m_ViewportBounds[0].y + edgeOffset));
		Gui::SetNextWindowSize(ImVec2(backgroundWidth, windowHeight));
		Gui::SetNextWindowBgAlpha(0.0f);
		Gui::Begin("Central Toolbar", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking);

		const float desiredHeight = 26.0f;
		ImRect background = UI::RectExpanded(Gui::GetCurrentWindow()->Rect(), 0.0f, -(windowHeight - desiredHeight) / 2.0f);
		Gui::GetWindowDrawList()->AddRectFilled(background.Min, background.Max, IM_COL32(15, 15, 15, 127), 4.0f);

		Gui::BeginVertical("##viewportCentralToolbarV", { backgroundWidth, Gui::GetContentRegionAvail().y });
		Gui::Spring();
		Gui::BeginHorizontal("##viewportCentralToolbarH", { backgroundWidth, Gui::GetContentRegionAvail().y });
		Gui::Spring();

		bool hasPlayButton = m_SceneState != SceneState::Simulate;
		bool hasSimulateButton = m_SceneState != SceneState::Play;
		bool hasPauseButton = m_SceneState != SceneState::Edit;
		bool scenePaused = m_ActiveScene->IsPaused();

		if (hasPlayButton)
		{
			SharedReference<Texture2D> icon = (hasSimulateButton) ? EditorResources::PlayIcon : EditorResources::StopIcon;
			if (UI::ImageButtonEx(icon, textureSize, normalColor, tintColor))
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
			SharedReference<Texture2D> icon = (hasPlayButton) ? EditorResources::SimulateIcon : EditorResources::StopIcon;
			if (UI::ImageButtonEx(icon, textureSize, normalColor, tintColor))
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
			SharedReference<Texture2D> icon = EditorResources::PauseIcon;
			if (UI::ImageButtonEx(icon, textureSize, normalColor, tintColor))
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
				SharedReference<Texture2D> icon = EditorResources::StepIcon;
				if (UI::ImageButtonEx(icon, textureSize, normalColor, tintColor))
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
		VX_PROFILE_FUNCTION();

		UI::PushID();

		ImGuiIO& io = Gui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];
		auto largeFont = io.Fonts->Fonts[1];

		SharedReference<Project> activeProject = Project::GetActive();
		ProjectProperties& projectProps = activeProject->GetProperties();

		UI::ScopedStyle disableSpacing(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		UI::ScopedStyle disableWindowBorder(ImGuiStyleVar_WindowBorderSize, 0.0f);
		UI::ScopedStyle windowRounding(ImGuiStyleVar_WindowRounding, 4.0f);
		UI::ScopedStyle disablePadding(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

		const float buttonSize = 18.0f;
		const float edgeOffset = 2.0f;
		const float windowHeight = 32.0f; // annoying limitation of ImGui, window can't be smaller than 32 pixels
		const float numberOfButtons = 1.0f;
		const float backgroundWidth = edgeOffset * 6.0f + buttonSize * numberOfButtons + edgeOffset * (numberOfButtons - 1.0f) * 2.0f;

		ImVec2 textureSize = { buttonSize, buttonSize };

		const ImVec4 normalColor = { 1.0f, 1.0f, 1.0f, 0.0f };
		const ImVec4 bgColor = { 0.7f, 0.7f, 0.7f, 1.0f };
		const ImVec4 tintColor = { 1.0f, 1.0f, 1.0f, 1.0f };

		Gui::SetNextWindowPos(ImVec2(m_ViewportBounds[1].x - backgroundWidth - 14, m_ViewportBounds[0].y + edgeOffset));
		Gui::SetNextWindowSize(ImVec2(backgroundWidth, windowHeight));
		Gui::SetNextWindowBgAlpha(0.0f);
		Gui::Begin("Viewport Settings", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking);

		const float desiredHeight = 26.0f;
		ImRect background = UI::RectExpanded(Gui::GetCurrentWindow()->Rect(), 0.0f, -(windowHeight - desiredHeight) / 2.0f);
		Gui::GetWindowDrawList()->AddRectFilled(background.Min, background.Max, IM_COL32(15, 15, 15, 127), 4.0f);

		bool openSettingsPopup = false;

		Gui::BeginVertical("##viewportSettingsV", { backgroundWidth, Gui::GetContentRegionAvail().y });
		Gui::Spring();
		Gui::BeginHorizontal("##viewportSettingsH", { backgroundWidth, Gui::GetContentRegionAvail().y });
		Gui::Spring();
		{
			UI::ScopedStyle enableSpacing(ImGuiStyleVar_ItemSpacing, ImVec2(edgeOffset * 2.0f, 0));

			if (UI::ImageButtonEx(EditorResources::SettingsIcon, textureSize, normalColor, tintColor))
				openSettingsPopup = true;
			UI::SetTooltip("Viewport Settings");
		}
		Gui::Spring();
		Gui::EndHorizontal();
		Gui::Spring();
		Gui::EndVertical();

		// Draw the settings popup
		{
			int32_t sectionIdx = 0;
			static float popupWidth = 300.0f;

			UI::ScopedStyle itemSpacing(ImGuiStyleVar_ItemSpacing, ImVec2(0, 5.5f));
			UI::ScopedStyle windowPadding(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));
			UI::ScopedStyle windowRounding(ImGuiStyleVar_PopupRounding, 4.0f);
			UI::ScopedStyle cellPadding(ImGuiStyleVar_CellPadding, ImVec2(0.0f, 5.5f));

			if (openSettingsPopup)
				Gui::OpenPopup("ViewportSettingsPanel");

			float columnWidth = 165.0f;
			Gui::SetNextWindowSize({ popupWidth, 375.0f });
			Gui::SetNextWindowPos({ (m_ViewportBounds[1].x - popupWidth) - 17, m_ViewportBounds[0].y + edgeOffset + windowHeight });
			if (Gui::BeginPopup("ViewportSettingsPanel", ImGuiWindowFlags_NoMove))
			{
				{
					Gui::PushFont(boldFont);
					Gui::Text("Display");
					Gui::PopFont();
					UI::Draw::Underline();
					UI::BeginPropertyGrid(columnWidth);

					if (UI::ImageButton("Maximize On Play", EditorResources::MaximizeOnPlayIcon, textureSize, projectProps.EditorProps.MaximizeOnPlay ? bgColor : normalColor, tintColor))
						projectProps.EditorProps.MaximizeOnPlay = !projectProps.EditorProps.MaximizeOnPlay;

					if (UI::ImageButton(projectProps.EditorProps.DrawEditorGrid ? "Hide Grid" : "Show Grid", EditorResources::ShowGridIcon, textureSize, projectProps.EditorProps.DrawEditorGrid ? normalColor : bgColor, tintColor))
						projectProps.EditorProps.DrawEditorGrid = !projectProps.EditorProps.DrawEditorGrid;

					if (UI::ImageButton(projectProps.PhysicsProps.ShowColliders ? "Hide Colliders" : "Show Colliders", EditorResources::PhysicsCollidersIcon, textureSize, projectProps.PhysicsProps.ShowColliders ? bgColor : normalColor, tintColor))
						projectProps.PhysicsProps.ShowColliders = !projectProps.PhysicsProps.ShowColliders;

					if (UI::ImageButton(projectProps.EditorProps.ShowBoundingBoxes ? "Hide Bounding Boxes" : "Show Bounding Boxes", EditorResources::BoundingBoxesIcon, textureSize, projectProps.EditorProps.ShowBoundingBoxes ? bgColor : normalColor, tintColor))
						projectProps.EditorProps.ShowBoundingBoxes = !projectProps.EditorProps.ShowBoundingBoxes;

					static const char* selectionModes[] = { "Entity", "Submesh" };
					uint32_t currentSelectionMode = (uint32_t)m_SelectionMode;
					if (UI::PropertyDropdown("Selection Mode", selectionModes, VX_ARRAYCOUNT(selectionModes), currentSelectionMode))
						m_SelectionMode = (SelectionMode)currentSelectionMode;

					UI::Property("Selected Entity Outline", m_ShowSelectedEntityOutline);
					UI::Property("Selected Entity Collider", m_ShowSelectedEntityCollider);

					if (UI::ImageButton(projectProps.EditorProps.MuteAudioSources ? "Unmute Audio" : "Mute Audio", EditorResources::MuteAudioSourcesIcons, textureSize, projectProps.EditorProps.MuteAudioSources ? bgColor : normalColor, tintColor))
						projectProps.EditorProps.MuteAudioSources = !projectProps.EditorProps.MuteAudioSources;

					UI::EndPropertyGrid();
				}

				{
					Gui::PushFont(boldFont);
					Gui::Text("Gizmos");
					Gui::PopFont();
					UI::Draw::Underline();
					UI::BeginPropertyGrid(columnWidth);

					UI::Property("Gimzo Size", projectProps.GizmoProps.GizmoSize, 0.05f, 0.05f);

					if (UI::ImageButton("Local Mode", EditorResources::LocalModeIcon, textureSize, m_TranslationMode == 0 ? bgColor : normalColor, tintColor))
						m_TranslationMode = (uint32_t)ImGuizmo::MODE::LOCAL;

					if (UI::ImageButton("World Mode", EditorResources::WorldModeIcon, textureSize, m_TranslationMode == 1 ? bgColor : normalColor, tintColor))
						m_TranslationMode = static_cast<uint32_t>(ImGuizmo::MODE::WORLD);

					if (UI::ImageButton(projectProps.RendererProps.DisplaySceneIconsInEditor ? "Hide Gizmos" : "Show Gizmos", EditorResources::DisplaySceneIconsIcon, textureSize, projectProps.RendererProps.DisplaySceneIconsInEditor ? normalColor : bgColor, tintColor))
						projectProps.RendererProps.DisplaySceneIconsInEditor = !projectProps.RendererProps.DisplaySceneIconsInEditor;

					UI::EndPropertyGrid();
				}

				{
					Gui::PushFont(boldFont);
					Gui::Text("Editor Camera");
					Gui::PopFont();
					UI::Draw::Underline();
					UI::BeginPropertyGrid(columnWidth);

					float degFOV = Math::Rad2Deg(m_EditorCamera->m_VerticalFOV);
					if (UI::Property("Field of View", degFOV))
					{
						m_EditorCamera->SetVerticalFOV(degFOV);
						projectProps.EditorProps.EditorCameraFOV = degFOV;
					}

					UI::Property("Camera Speed", m_EditorCamera->m_NormalSpeed, 0.001f, 0.0002f, 0.5f, "%.4f");

					bool isUsing2DView = m_EditorCamera->IsUsing2DView();
					if (UI::ImageButton("2D View", EditorResources::TwoDViewIcon, textureSize, isUsing2DView ? bgColor : normalColor, tintColor))
					{
						m_EditorCamera->SetUse2DView(!isUsing2DView);
						m_EditorCamera->SetUseTopDownView(false);
					}

					bool isUsingTopDownView = m_EditorCamera->IsUsingTopDownView();
					if (UI::ImageButton("Top Down View", EditorResources::TopDownViewIcon, textureSize, isUsingTopDownView ? bgColor : normalColor, tintColor))
					{
						m_EditorCamera->SetUseTopDownView(!isUsingTopDownView);
						m_EditorCamera->SetUse2DView(false);
					}

					UI::EndPropertyGrid();
				}
				
				Gui::EndPopup();
			}
		}

		Gui::End();

		UI::PopID();
	}

	void EditorLayer::OnLaunchRuntime(const std::filesystem::path& filepath)
	{
		SaveProject();

		std::string runtimeApplicationPath = Application::Get().GetRuntimeBinaryPath();
		Platform::LaunchProcess(runtimeApplicationPath.c_str(), filepath.string().c_str());
	}

	void EditorLayer::OnOverlayRender(EditorCamera* editorCamera, bool renderInPlayMode)
	{
		VX_PROFILE_FUNCTION();

		SharedReference<Project> activeProject = Project::GetActive();
		const ProjectProperties& projectProps = activeProject->GetProperties();

		if (m_SceneState == SceneState::Play && !renderInPlayMode)
		{
			Entity cameraEntity = m_ActiveScene->GetPrimaryCameraEntity();

			if (cameraEntity)
			{
				Math::mat4 transform = m_ActiveScene->GetWorldSpaceTransformMatrix(cameraEntity);
				SceneCamera& camera = cameraEntity.GetComponent<CameraComponent>().Camera;
				Math::mat4 view = Math::Inverse(transform);
				Renderer2D::BeginScene(camera, view);
			}
		}
		else
		{
			Renderer2D::BeginScene(editorCamera);
		}

		// Render Editor Grid
		if (m_SceneState != SceneState::Play && projectProps.EditorProps.DrawEditorGrid)
		{
			OverlayRenderGrid(projectProps.EditorProps.DrawEditorAxes);
		}
		
		const auto colliderColor = projectProps.PhysicsProps.Physics3DColliderColor;
		const auto spriteColliderColor = projectProps.PhysicsProps.Physics2DColliderColor;
		const auto boundingBoxColor = ColorToVec4(Color::Orange);
		const auto outlineColor = ColorToVec4(Color::Orange);

		// Render Physics Colliders
		if (projectProps.PhysicsProps.ShowColliders)
		{
			OverlayRenderMeshColliders(colliderColor);
			OverlayRenderSpriteColliders(editorCamera, spriteColliderColor);
		}

		// Render Bounding Boxes
		if (projectProps.EditorProps.ShowBoundingBoxes)
		{
			OverlayRenderMeshBoundingBoxes(boundingBoxColor);
			OverlayRenderSpriteBoundingBoxes(boundingBoxColor);
		}

		// Render Visible Mesh Colliders
		{
			std::vector<Entity> entities;

			auto boxColliderView = m_ActiveScene->GetAllEntitiesWith<BoxColliderComponent>();
			auto sphereColliderView = m_ActiveScene->GetAllEntitiesWith<SphereColliderComponent>();

			for (const auto e : boxColliderView)
			{
				Entity entity = { e, m_ActiveScene.Raw() };
				const BoxColliderComponent& boxCollider = entity.GetComponent<BoxColliderComponent>();
				if (boxCollider.Visible)
					entities.emplace_back(e, m_ActiveScene.Raw());
			}
			for (const auto e : sphereColliderView)
			{
				Entity entity = { e, m_ActiveScene.Raw() };
				const SphereColliderComponent& sphereCollider = entity.GetComponent<SphereColliderComponent>();
				if (sphereCollider.Visible)
					entities.emplace_back(e, m_ActiveScene.Raw());
			}

			for (auto& entity : entities)
			{
				auto transform = m_ActiveScene->GetWorldSpaceTransformMatrix(entity);
				OverlayRenderMeshCollider(entity, transform, colliderColor);
			}
		}

		// Render Visible 2D Colliders
		{
			std::vector<Entity> entities;

			auto boxColliderView = m_ActiveScene->GetAllEntitiesWith<BoxCollider2DComponent>();
			auto circleColliderView = m_ActiveScene->GetAllEntitiesWith<CircleCollider2DComponent>();

			for (const auto e : boxColliderView)
			{
				Entity entity = { e, m_ActiveScene.Raw() };
				const BoxCollider2DComponent& boxCollider = entity.GetComponent<BoxCollider2DComponent>();
				if (boxCollider.Visible)
					entities.emplace_back(e, m_ActiveScene.Raw());
			}
			for (const auto e : circleColliderView)
			{
				Entity entity = { e, m_ActiveScene.Raw() };
				const CircleCollider2DComponent& circleCollider = entity.GetComponent<CircleCollider2DComponent>();
				if (circleCollider.Visible)
					entities.emplace_back(e, m_ActiveScene.Raw());
			}

			for (auto& entity : entities)
			{
				auto transform = m_ActiveScene->GetWorldSpaceTransformMatrix(entity);
				OverlayRenderSpriteCollider(editorCamera, entity, transform, colliderColor);
			}
		}

		// Draw selected entity outline + colliders
		{
			if (Entity selectedEntity = SelectionManager::GetSelectedEntity(); selectedEntity)
			{
				Math::mat4 transform = m_ActiveScene->GetWorldSpaceTransformMatrix(selectedEntity);

				if (m_ShowSelectedEntityCollider)
				{
					OverlayRenderMeshCollider(selectedEntity, transform, colliderColor);
					OverlayRenderSpriteCollider(editorCamera, selectedEntity, transform, spriteColliderColor);
				}

				if (m_ShowSelectedEntityOutline)
				{
					Math::mat4 scaledTransform = transform * Math::Scale(Math::vec3(1.001f));

					if (selectedEntity.HasAny<MeshRendererComponent, StaticMeshRendererComponent>())
						OverlayRenderMeshOutline(selectedEntity, scaledTransform, outlineColor);

					if (selectedEntity.HasAny<SpriteRendererComponent, CircleRendererComponent>())
						OverlayRenderSpriteOutline(selectedEntity, scaledTransform, outlineColor);

					if (selectedEntity.HasComponent<TextMeshComponent>())
					{
						const auto& textMesh = selectedEntity.GetComponent<TextMeshComponent>();

						const TransformComponent& worldSpaceTransform = m_ActiveScene->GetWorldSpaceTransform(selectedEntity);
						Math::mat4 transform = worldSpaceTransform.GetTransform();

						Renderer2D::DrawRect(transform, outlineColor);
					}

					if (selectedEntity.HasComponent<CameraComponent>())
					{
						const SceneCamera& sceneCamera = selectedEntity.GetComponent<CameraComponent>().Camera;

						switch (sceneCamera.GetProjectionType())
						{
							case SceneCamera::ProjectionType::Perspective:
							{
								const TransformComponent& worldSpaceTransform = m_ActiveScene->GetWorldSpaceTransform(selectedEntity);
								auto corners = GetFrustumCornersWorldSpace(worldSpaceTransform, sceneCamera);

								Renderer::DrawFrustum(corners, ColorToVec4(Color::LightBlue));
								break;
							}
							case SceneCamera::ProjectionType::Orthographic:
							{
								auto scaled = transform * Math::Scale({ sceneCamera.GetOrthographicSize() * 1.6f, sceneCamera.GetOrthographicSize() * 0.9f, 1.0f });
								Renderer2D::DrawRect(scaled, outlineColor);
								break;
							}
						}
					}

					if (selectedEntity.HasComponent<LightSourceComponent>())
					{
						const LightSourceComponent& lightSourceComponent = selectedEntity.GetComponent<LightSourceComponent>();

						const TransformComponent& worldSpaceTransform = m_ActiveScene->GetWorldSpaceTransform(selectedEntity);
						Math::vec3 translation = worldSpaceTransform.Translation;
						Math::vec4 color = { lightSourceComponent.Radiance, 1.0f };

						switch (lightSourceComponent.Type)
						{
							case LightType::Directional:
							{
								Math::vec3 midpoint = Math::Midpoint(translation, Math::vec3(0.0f));

								Math::quaternion rotation = worldSpaceTransform.GetRotation();

								Math::vec3 left = Math::Rotate(rotation, Math::vec3(-1.0f, 0.0f, 0.0f)) * 0.5f;
								Math::vec3 right = Math::Rotate(rotation, Math::vec3(1.0f, 0.0f, 0.0f)) * 0.5f;
								Math::vec3 up = Math::Rotate(rotation, Math::vec3(0.0f, 1.0f, 0.0f)) * 0.5f;
								Math::vec3 down = Math::Rotate(rotation, Math::vec3(0.0f, -1.0f, 0.0f)) * 0.5f;

								Renderer2D::DrawLine(translation, midpoint, color);
								Renderer2D::DrawLine(translation + left, midpoint + left, color);
								Renderer2D::DrawLine(translation + right, midpoint + right, color);
								Renderer2D::DrawLine(translation + up, midpoint + up, color);
								Renderer2D::DrawLine(translation + down, midpoint + down, color);

								break;
							}
							case LightType::Point:
							{
								const float radius = lightSourceComponent.Intensity * 0.5f;

								Renderer2D::DrawCircle(translation, { 0.0f, 0.0f, 0.0f }, radius, color);
								Renderer2D::DrawCircle(translation, { Math::Deg2Rad(90.0f), 0.0f, 0.0f }, radius, color);
								Renderer2D::DrawCircle(translation, { 0.0f, Math::Deg2Rad(90.0f), 0.0f }, radius, color);

								break;
							}
							case LightType::Spot:
							{
								break;
							}
						}
					}
				}
			}
		}

		Renderer2D::EndScene();
	}

	void EditorLayer::OnCreateScriptPopupRender()
	{
		if (m_OpenCreateScriptPopup)
		{
			Gui::OpenPopup("Create Script");
			m_OpenCreateScriptPopup = false;
		}

		if (UI::ShowMessageBox("Create Script", { 500, 220 }))
		{
			UI::Draw::Underline();
			Gui::Spacing();

			ImVec2 buttonSize(Gui::GetFontSize() * 13.27f, 0.0f);

			Gui::TextCentered("Enter script name and choose a programming language", 40.0f);

			Gui::Spacing();
			UI::Draw::Underline();

			UI::BeginPropertyGrid();

			static std::string className = "Untitled";
			UI::Property("Class Name", className);

			enum class ScriptingLanguage
			{
				CSharp,
				Cpp,
			};

			static ScriptingLanguage currentScriptingLanguage = ScriptingLanguage::CSharp;
			const char* scriptingLanguages[] = { "C#", "C++" };
			UI::PropertyDropdown("Language", scriptingLanguages, VX_ARRAYCOUNT(scriptingLanguages), currentScriptingLanguage);
			
			UI::EndPropertyGrid();

			UI::ShiftCursorY(20.0f);
			UI::ShiftCursorX(7.0f);

			auto resetPopup = []() {
				currentScriptingLanguage = ScriptingLanguage::CSharp;
				className = "Untitiled";
			};

			if (Gui::Button("Create", buttonSize))
			{
				switch (currentScriptingLanguage)
				{
					case ScriptingLanguage::CSharp:
					{
						std::string scriptName = className + ".cs";

						break;
					}
					case ScriptingLanguage::Cpp:
					{
						break;
					}
				}

				resetPopup();
				Gui::CloseCurrentPopup();
			}

			Gui::SameLine();

			if (Gui::Button("Cancel", buttonSize))
			{
				resetPopup();
				Gui::CloseCurrentPopup();
			}

			Gui::EndPopup();
		}
	}

	void EditorLayer::OnMeshImportPopupRender()
	{
		if (m_OpenMeshImportPopup)
		{
			Gui::OpenPopup("Mesh Import Options");
			m_OpenMeshImportPopup = false;
		}

		if (UI::ShowMessageBox("Mesh Import Options", { 500, 285 }))
		{
			UI::Draw::Underline();
			Gui::Spacing();

			ImVec2 buttonSize(Gui::GetFontSize() * 13.27f, 0.0f);

			Gui::TextCentered("A mesh asset must be generated from this mesh file. (i.e. .fbx)", 40.0f);
			Gui::TextCentered("Import options can be selected below", 60.0f);

			Gui::Spacing();
			UI::Draw::Underline();

			UI::ShiftCursorX(-15.0f);

			UI::DrawVec3Controls("Translation", m_ModelImportOptions.MeshTransformation.Translation);
			Math::vec3 rotationEuler = m_ModelImportOptions.MeshTransformation.GetRotationEuler();
			UI::DrawVec3Controls("Rotation", rotationEuler, 0.0f, 100.0f, [&]()
			{
				m_ModelImportOptions.MeshTransformation.SetRotationEuler(rotationEuler);
			});
			UI::DrawVec3Controls("Scale", m_ModelImportOptions.MeshTransformation.Scale);

			UI::ShiftCursorY(20.0f);

			UI::BeginPropertyGrid();

			std::string assetDir = Project::GetAssetDirectory().string();
			size_t assetDirPos = m_MeshFilepath.find(assetDir);
			std::string filepath = m_MeshFilepath.substr(assetDirPos + assetDir.size() + 1);
			UI::Property("Filepath", filepath, true);

			UI::EndPropertyGrid();

			UI::ShiftCursorY(10.0f);
			UI::ShiftCursorX(7.0f);

			if (Gui::Button("Import", buttonSize))
			{
				if (m_MeshEntityToEdit.HasComponent<MeshRendererComponent>())
				{
					MeshRendererComponent& meshRenderer = m_MeshEntityToEdit.GetComponent<MeshRendererComponent>();
					meshRenderer.Mesh = Project::GetEditorAssetManager()->GetAssetHandleFromFilepath(m_MeshFilepath);
				}
				else if (m_MeshEntityToEdit.HasComponent<StaticMeshRendererComponent>())
				{
					StaticMeshRendererComponent& staticMeshRenderer = m_MeshEntityToEdit.GetComponent<StaticMeshRendererComponent>();
					staticMeshRenderer.StaticMesh = Project::GetEditorAssetManager()->GetAssetHandleFromFilepath(m_MeshFilepath);

					if (AssetManager::IsHandleValid(staticMeshRenderer.StaticMesh))
					{
						staticMeshRenderer.Type = MeshType::Custom;

						SharedReference<StaticMesh> staticMesh = AssetManager::GetAsset<StaticMesh>(staticMeshRenderer.StaticMesh);
						if (staticMesh)
						{
							staticMeshRenderer.Materials->Clear();
							staticMesh->LoadMaterialTable(staticMeshRenderer.Materials);
						}
					}
				}

				m_MeshFilepath = "";
				m_ModelImportOptions = MeshImportOptions();

				Gui::CloseCurrentPopup();
			}

			Gui::SameLine();

			if (Gui::Button("Cancel", buttonSize))
			{
				m_MeshFilepath = "";
				m_ModelImportOptions = MeshImportOptions();

				Gui::CloseCurrentPopup();
			}

			Gui::EndPopup();
		}
	}

	void EditorLayer::OverlayRenderMeshBoundingBox(Entity entity, const Math::mat4& transform, const Math::vec4& boundingBoxColor)
	{
		if (entity.HasComponent<MeshRendererComponent>())
		{
			const auto& meshRendererComponent = entity.GetComponent<MeshRendererComponent>();
			const auto& scaledTransform = transform * Math::Scale(Math::vec3(1.001f));

			if (!meshRendererComponent.Visible)
				return;

			AssetHandle meshHandle = meshRendererComponent.Mesh;
			if (!AssetManager::IsHandleValid(meshHandle))
				return;

			SharedReference<Mesh> mesh = AssetManager::GetAsset<Mesh>(meshHandle);
			if (!mesh)
				return;

			const auto& submesh = mesh->GetSubmesh();

			Renderer2D::DrawAABB(submesh.GetBoundingBox(), scaledTransform, boundingBoxColor);
		}

		if (entity.HasComponent<StaticMeshRendererComponent>())
		{
			const auto& staticMeshRendererComponent = entity.GetComponent<StaticMeshRendererComponent>();
			const auto& transform = m_ActiveScene->GetWorldSpaceTransformMatrix(entity) * Math::Scale(Math::vec3(1.001f));

			if (!staticMeshRendererComponent.Visible)
				return;

			AssetHandle staticMeshHandle = staticMeshRendererComponent.StaticMesh;
			if (!AssetManager::IsHandleValid(staticMeshHandle))
				return;

			SharedReference<StaticMesh> staticMesh = AssetManager::GetAsset<StaticMesh>(staticMeshHandle);
			if (!staticMesh)
				return;

			const auto& submeshes = staticMesh->GetSubmeshes();

			for (const auto& [submeshIndex, submesh] : submeshes)
			{
				Renderer2D::DrawAABB(submesh.GetBoundingBox(), transform, boundingBoxColor);
			}
		}
	}

	void EditorLayer::OverlayRenderMeshBoundingBoxes(const Math::vec4& boundingBoxColor)
	{
		std::vector<Entity> entities;

		auto meshRendererView = m_ActiveScene->GetAllEntitiesWith<MeshRendererComponent>();
		auto staticMeshRendererView = m_ActiveScene->GetAllEntitiesWith<StaticMeshRendererComponent>();

		for (const auto e : meshRendererView)
			entities.emplace_back(e, m_ActiveScene.Raw());
		for (const auto e : staticMeshRendererView)
			entities.emplace_back(e, m_ActiveScene.Raw());

		for (auto& entity : entities)
		{
			if (!entity.IsActive())
				continue;

			auto transform = m_ActiveScene->GetWorldSpaceTransformMatrix(entity);
			OverlayRenderMeshBoundingBox(entity, transform, boundingBoxColor);
		}
	}

	void EditorLayer::OverlayRenderMeshCollider(Entity entity, const Math::mat4& transform, const Math::vec4& colliderColor)
	{
		if (entity.HasComponent<BoxColliderComponent>())
		{
			const auto& bc = entity.GetComponent<BoxColliderComponent>();

			const Math::AABB aabb = {
				-Math::vec3(0.503f),
				+Math::vec3(0.503f)
			};

			Math::mat4 transform = m_ActiveScene->GetWorldSpaceTransformMatrix(entity)
				* Math::Translate(bc.Offset)
				* Math::Scale(bc.HalfSize * 2.0f);

			Renderer2D::DrawAABB(aabb, transform, colliderColor);
		}

		if (entity.HasComponent<SphereColliderComponent>())
		{
			const auto& sc = entity.GetComponent<SphereColliderComponent>();
			auto transform = m_ActiveScene->GetWorldSpaceTransform(entity);
			Math::vec3 translation = transform.Translation + sc.Offset;
			Math::vec3 scale = transform.Scale;

			const float largestComponent = Math::Max(scale.x, Math::Max(scale.y, scale.z));
			const float radius = (largestComponent * sc.Radius) * 1.005f;

			Renderer2D::DrawCircle(translation, { 0.0f, 0.0f, 0.0f }, radius, colliderColor);
			Renderer2D::DrawCircle(translation, { Math::Deg2Rad(90.0f), 0.0f, 0.0f }, radius, colliderColor);
			Renderer2D::DrawCircle(translation, { Math::Deg2Rad(-45.0f), 0.0f, 0.0f }, radius, colliderColor);
			Renderer2D::DrawCircle(translation, { Math::Deg2Rad(45.0f), 0.0f, 0.0f }, radius, colliderColor);
			Renderer2D::DrawCircle(translation, { 0.0f, Math::Deg2Rad(90.0f), 0.0f }, radius, colliderColor);
			Renderer2D::DrawCircle(translation, { 0.0f, Math::Deg2Rad(-45.0f), 0.0f }, radius, colliderColor);
			Renderer2D::DrawCircle(translation, { 0.0f, Math::Deg2Rad(45.0f), 0.0f }, radius, colliderColor);
		}
	}

	void EditorLayer::OverlayRenderMeshColliders(const Math::vec4& colliderColor)
	{
		std::vector<Entity> entities;

		auto boxColliderView = m_ActiveScene->GetAllEntitiesWith<BoxColliderComponent>();
		auto sphereColliderView = m_ActiveScene->GetAllEntitiesWith<SphereColliderComponent>();

		for (const auto e : boxColliderView)
			entities.emplace_back(e, m_ActiveScene.Raw());
		for (const auto e : sphereColliderView)
			entities.emplace_back(e, m_ActiveScene.Raw());

		for (auto& entity : entities)
		{
			auto transform = m_ActiveScene->GetWorldSpaceTransformMatrix(entity);
			OverlayRenderMeshCollider(entity, transform, colliderColor);
		}
	}

	void EditorLayer::OverlayRenderMeshOutline(Entity entity, const Math::mat4& transform, const Math::vec4& outlineColor)
	{
		if (!entity.IsActive())
			return;

		if (entity.HasComponent<MeshRendererComponent>())
		{
			const auto& meshRendererComponent = entity.GetComponent<MeshRendererComponent>();

			AssetHandle meshHandle = meshRendererComponent.Mesh;
			if (AssetManager::IsHandleValid(meshHandle))
				return;

			SharedReference<Mesh> mesh = AssetManager::GetAsset<Mesh>(meshHandle);
			if (!mesh)
				return;

			switch (m_SelectionMode)
			{
				case SelectionMode::Entity:
					Renderer2D::DrawAABB(mesh->GetBoundingBox(), transform, outlineColor);
					break;
				case SelectionMode::Submesh:
					const auto& submesh = mesh->GetSubmesh();
					Renderer2D::DrawAABB(submesh.GetBoundingBox(), transform, outlineColor);
					break;
			}
		}

		if (entity.HasComponent<StaticMeshRendererComponent>())
		{
			const auto& staticMeshRendererComponent = entity.GetComponent<StaticMeshRendererComponent>();

			AssetHandle staticMeshHandle = staticMeshRendererComponent.StaticMesh;
			if (!AssetManager::IsHandleValid(staticMeshHandle))
				return;

			SharedReference<StaticMesh> staticMesh = AssetManager::GetAsset<StaticMesh>(staticMeshHandle);
			if (!staticMesh)
				return;

			switch (m_SelectionMode)
			{
				case SelectionMode::Entity:
					Renderer2D::DrawAABB(staticMesh->GetBoundingBox(), transform, outlineColor);
					break;
				case SelectionMode::Submesh:
					const auto& submeshes = staticMesh->GetSubmeshes();
					for (const auto& [submeshIndex, submesh] : submeshes)
					{
						Renderer2D::DrawAABB(submesh.GetBoundingBox(), transform, outlineColor);
					}
					break;
			}
		}
	}

	void EditorLayer::OverlayRenderSpriteCollider(EditorCamera* editorCamera, Entity entity, const Math::mat4& transform, const Math::vec4& colliderColor)
	{
		float colliderDistance = 0.005f; // Editor camera will be looking at the origin of the world on the first frame
		if (editorCamera->GetPosition().z < 0) // Show colliders on the side that the editor camera facing
			colliderDistance = -colliderDistance;

		if (entity.HasComponent<BoxCollider2DComponent>())
		{
			const auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();

			Math::vec3 scale = Math::vec3(bc2d.Size * 2.0f, 1.0f);

			Math::mat4 transform = m_ActiveScene->GetWorldSpaceTransformMatrix(entity)
				* Math::Translate(Math::vec3(bc2d.Offset, colliderDistance))
				* Math::Scale(scale);

			Renderer2D::DrawRect(transform, colliderColor);
		}

		if (entity.HasComponent<CircleCollider2DComponent>())
		{
			const auto& cc2d = entity.GetComponent<CircleCollider2DComponent>();

			Math::vec3 scale = Math::vec3(cc2d.Radius * 2.0f);

			Math::mat4 transform = m_ActiveScene->GetWorldSpaceTransformMatrix(entity)
				* Math::Translate(Math::vec3(cc2d.Offset, colliderDistance))
				* Math::Scale(scale);

			Renderer2D::DrawCircle(transform, colliderColor, Renderer2D::GetLineWidth() / 100.0f);
		}
	}

	void EditorLayer::OverlayRenderSpriteColliders(EditorCamera* editorCamera, const Math::vec4& colliderColor)
	{
		std::vector<Entity> entities;

		auto spriteRendererView = m_ActiveScene->GetAllEntitiesWith<SpriteRendererComponent>();
		auto circleRendererView = m_ActiveScene->GetAllEntitiesWith<CircleRendererComponent>();

		for (const auto e : spriteRendererView)
			entities.emplace_back(e, m_ActiveScene.Raw());
		for (const auto e : circleRendererView)
			entities.emplace_back(e, m_ActiveScene.Raw());

		for (auto& entity : entities)
		{
			auto transform = m_ActiveScene->GetWorldSpaceTransformMatrix(entity);
			OverlayRenderSpriteCollider(editorCamera, entity, transform, colliderColor);
		}
	}

	void EditorLayer::OverlayRenderSpriteBoundingBoxes(const Math::vec4& boundingBoxColor)
	{
		std::vector<Entity> entities;

		auto spriteRendererView = m_ActiveScene->GetAllEntitiesWith<SpriteRendererComponent>();
		auto circleRendererView = m_ActiveScene->GetAllEntitiesWith<CircleRendererComponent>();

		for (const auto e : spriteRendererView)
			entities.emplace_back(e, m_ActiveScene.Raw());
		for (const auto e : circleRendererView)
			entities.emplace_back(e, m_ActiveScene.Raw());

		for (auto& entity : entities)
		{
			auto transform = m_ActiveScene->GetWorldSpaceTransformMatrix(entity);
			OverlayRenderSpriteOutline(entity, transform, boundingBoxColor);
		}
	}

	void EditorLayer::OverlayRenderSpriteOutline(Entity entity, const Math::mat4& transform, const Math::vec4& outlineColor)
	{
		if (entity.HasComponent<SpriteRendererComponent>())
		{
			if (!entity.IsActive())
				return;

			const auto& spriteRenderer = entity.GetComponent<SpriteRendererComponent>();
			if (!spriteRenderer.Visible)
				return;

			Renderer2D::DrawRect(transform, outlineColor);
		}

		if (entity.HasComponent<CircleRendererComponent>())
		{
			if (!entity.IsActive())
				return;

			const auto& circleRenderer = entity.GetComponent<CircleRendererComponent>();
			if (!circleRenderer.Visible)
				return;

			Math::mat4 scaledTransform = transform * Math::Scale(Math::vec3(0.505f));

			Renderer2D::DrawCircle(scaledTransform, outlineColor);
		}
	}

	void EditorLayer::OverlayRenderGrid(bool drawAxis)
	{
		static constexpr float axisLineLength = 1'000.0f;
		static constexpr float gridLineLength = 750.0f;
		static constexpr float gridWidth = 750.0f;
		static constexpr float gridLength = 750.0f;

		float originalLineWidth = Renderer2D::GetLineWidth();

		// Render Axes
		if (drawAxis)
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
			if (x == 0 && drawAxis)
				continue;

			Renderer2D::DrawLine({ x, 0, -gridLineLength }, { x, 0, gridLineLength }, gridColor);
		}

		// Z Grid Lines
		for (int32_t z = -gridLength; z <= (int32_t)gridLength; z++)
		{
			// Skip the origin lines
			if (z == 0 && drawAxis)
				continue;

			Renderer2D::DrawLine({ -gridLineLength, 0, z }, { gridLineLength, 0, z }, gridColor);
		}

		Renderer2D::Flush();
	}

    bool EditorLayer::OnWindowDragDropEvent(WindowDragDropEvent& e)
    {
		for (const auto& path : e.GetPaths())
		{
			Project::GetEditorAssetManager()->ImportAsset(path);
		}

        return true;
    }

    bool EditorLayer::OnKeyPressedEvent(KeyPressedEvent& e)
	{
		auto sceneHierarchyPanel = m_PanelManager->GetPanel<SceneHierarchyPanel>();
		if (sceneHierarchyPanel->IsEditingEntityName())
			return false;

		if (ImGuizmo::IsUsing())
			return false;

		if (Input::IsMouseButtonDown(MouseButton::Right))
			return false;

		const bool altDown = Input::IsKeyDown(KeyCode::LeftAlt) || Input::IsKeyDown(KeyCode::RightAlt);
		const bool shiftDown = Input::IsKeyDown(KeyCode::LeftShift) || Input::IsKeyDown(KeyCode::RightShift);
		const bool controlDown = Input::IsKeyDown(KeyCode::LeftControl) || Input::IsKeyDown(KeyCode::RightControl);

		Entity selectedEntity = SelectionManager::GetSelectedEntity();

		switch (e.GetKeyCode())
		{
			// File
			case KeyCode::N:
			{
				if (controlDown && m_SceneState == SceneState::Edit)
					CreateNewScene();

				break;
			}
			case KeyCode::O:
			{
				if (controlDown && m_SceneState == SceneState::Edit)
					OpenExistingProject();

				break;
			}
			case KeyCode::S:
			{
				if (controlDown && m_SceneState == SceneState::Edit)
				{
					if (shiftDown)
						SaveSceneAs();
					else
						SaveScene();
				}

				break;
			}

			case KeyCode::Q:
			{
				OnNoGizmoSelected();

				break;
			}
			case KeyCode::W:
			{
				if (altDown && selectedEntity)
				{
					TransformComponent& transformComponent = selectedEntity.GetTransform();
					transformComponent.Translation = Math::vec3(0.0f);
				}

				OnTranslationToolSelected();

				break;
			}
			case KeyCode::E:
			{
				if (altDown && selectedEntity)
				{
					TransformComponent& transformComponent = selectedEntity.GetTransform();
					transformComponent.SetRotationEuler(Math::vec3(0.0f));
				}

				OnRotationToolSelected();

				break;
			}
			case KeyCode::R:
			{
				if (altDown && selectedEntity)
				{
					TransformComponent& transformComponent = selectedEntity.GetTransform();
					transformComponent.Scale = Math::vec3(1.0f);
				}

				OnScaleToolSelected();

				break;
			}

			case KeyCode::F:
			{
				if (selectedEntity)
				{
					if (m_AllowViewportCameraEvents && m_SceneViewportHovered)
					{
						m_EditorCamera->Focus(m_ActiveScene->GetWorldSpaceTransform(selectedEntity).Translation);
						m_EditorCamera->SetDistance(10);
					}
					else if (m_AllowSecondViewportCameraEvents && m_SecondViewportHovered)
					{
						m_SecondEditorCamera->Focus(m_ActiveScene->GetWorldSpaceTransform(selectedEntity).Translation);
						m_SecondEditorCamera->SetDistance(10);
					}
				}

				break;
			}
			case KeyCode::G:
			{
				if (m_SceneState == SceneState::Edit)
				{
					SharedReference<Project> activeProject = Project::GetActive();
					ProjectProperties& projectProps = activeProject->GetProperties();
					projectProps.EditorProps.DrawEditorGrid = !projectProps.EditorProps.DrawEditorGrid;
				}

				break;
			}

			case KeyCode::A:
			{
				if (controlDown)
					m_ShowSceneCreateEntityMenu = true;

				break;
			}
			case KeyCode::B:
			{
				if (controlDown && m_SceneState == SceneState::Edit)
				{
					if (shiftDown)
					{
						auto buildSettingsPanel = m_PanelManager->GetPanel<BuildSettingsPanel>();
						buildSettingsPanel->IsOpen = !buildSettingsPanel->IsOpen;
					}
					else
					{
						BuildAndRunProject();
					}
				}

				break;
			}
			case KeyCode::D:
			{
				if (controlDown)
					DuplicateSelectedEntity();

				break;
			}

			case KeyCode::P:
			{
				if (controlDown && shiftDown && m_SceneState == SceneState::Play)
				{
					RestartScene();

					break;
				}

				if (controlDown)
				{
					if (m_SceneState == SceneState::Edit || m_SceneState == SceneState::Simulate)
						OnScenePlay();
					else if (m_SceneState == SceneState::Play)
						OnSceneStop();

					break;
				}

				break;
			}
			case KeyCode::X:
			{
				if (controlDown && shiftDown)
				{
					if (m_SceneState == SceneState::Simulate)
						RestartSceneSimulation();

					break;
				}

				if (controlDown)
				{
					if (m_SceneState == SceneState::Edit || m_SceneState == SceneState::Play)
						OnSceneSimulate();
					else if (m_SceneState == SceneState::Simulate)
						OnSceneStop();

					break;
				}

				break;
			}

			// Tools
			case KeyCode::F2:
			{
				if (selectedEntity)
				{
					m_PanelManager->GetPanel<SceneHierarchyPanel>()->EditSelectedEntityName(true);
				}

				break;
			}
			case KeyCode::F11:
			{
				Window& window = Application::Get().GetWindow();
				window.SetMaximized(!window.IsMaximized());

				if (!window.IsMaximized())
				{
					window.CenterWindow();
				}

				break;
			}

			case KeyCode::Delete:
			{
				if (selectedEntity)
					m_ActiveScene->SubmitToDestroyEntity(selectedEntity);

				break;
			}

			case KeyCode::Space:
			{
				if (controlDown)
					m_SceneViewportMaximized = !m_SceneViewportMaximized;

				break;
			}
		}

		return false;
	}

	bool EditorLayer::OnMouseButtonPressedEvent(MouseButtonPressedEvent& e)
	{
		if (!m_SceneViewportHovered && !m_SecondViewportHovered)
			return false;

		if (ImGuizmo::IsOver())
			return false;

		if (e.GetMouseButton() != MouseButton::Left)
			return false;

		const bool altPressed = Input::IsKeyDown(KeyCode::LeftAlt) || Input::IsKeyDown(KeyCode::RightAlt);

		if (altPressed)
			return false;

		// TODO we should handle this in a better way
		if (m_HoveredEntity && m_HoveredEntity.HasAny<SpriteRendererComponent, CircleRendererComponent, TextMeshComponent, CameraComponent, LightSourceComponent, AudioSourceComponent>())
		{
			SelectionManager::SetSelectedEntity(m_HoveredEntity);
			return false;
		}

		std::vector<SelectionData> selectionData;

		auto [mouseX, mouseY] = GetMouseViewportSpace(m_SceneViewportHovered);
		if (mouseX > -1.0f && mouseX < 1.0f && mouseY > -1.0f && mouseY < 1.0f)
		{
			const auto& camera = m_SceneViewportHovered ? m_EditorCamera : m_SecondEditorCamera;
			auto [origin, direction] = CastRay(camera, mouseX, mouseY);

			auto meshView = m_ActiveScene->GetAllEntitiesWith<MeshRendererComponent>();
			for (const auto e : meshView)
			{
				Entity entity{ e, m_ActiveScene.Raw() };

				Math::mat4 transform = m_ActiveScene->GetWorldSpaceTransformMatrix(entity);
			}

			auto staticMeshView = m_ActiveScene->GetAllEntitiesWith<StaticMeshRendererComponent>();
			for (const auto e : staticMeshView)
			{
				Entity entity{ e, m_ActiveScene.Raw() };

				const TransformComponent& worldSpaceTransform = m_ActiveScene->GetWorldSpaceTransform(entity);
				
				const auto& staticMeshRenderer = entity.GetComponent<StaticMeshRendererComponent>();
				if (!AssetManager::IsHandleValid(staticMeshRenderer.StaticMesh))
					continue;

				auto staticMesh = AssetManager::GetAsset<StaticMesh>(staticMeshRenderer.StaticMesh);
				if (!staticMesh)
					continue;

				const auto& submeshes = staticMesh->GetSubmeshes();
				uint32_t submeshIndex = 0;

				while (staticMesh->HasSubmesh(submeshIndex))
				{
					const auto& submesh = submeshes.at(submeshIndex++);

					Math::mat4 transform = m_ActiveScene->GetWorldSpaceTransformMatrix(entity);
					Math::Ray ray{
						Math::Inverse(transform) * Math::vec4(origin, 1.0f),
						Math::Inverse((Math::mat3(transform))) * direction
					};

					const Math::AABB& aabb = submesh.GetBoundingBox();

					float t;
					const bool intersects = ray.IntersectsAABB(aabb, t);
					if (intersects)
					{
						float distance = Math::Distance(camera->GetPosition(), worldSpaceTransform.Translation);
						selectionData.emplace_back(SelectionData{ entity.GetUUID(), distance });
						break;
					}
				}
			}

			if (selectionData.empty())
			{
				SelectionManager::DeselectEntity();
				return false;
			}

			const bool anyViewportHovered = (m_SceneViewportHovered && m_SceneState != SceneState::Play) || m_SecondViewportHovered;
			if (anyViewportHovered)
			{
				SelectionData selectedData = selectionData[0];

				float closest = selectedData.Distance;
				for (const auto& data : selectionData)
				{
					if (data.Distance < closest)
					{
						closest = data.Distance;
						selectedData.SelectedUUID = data.SelectedUUID;
						selectedData.Distance = data.Distance;
					}
				}

				Entity selected = m_ActiveScene->TryGetEntityWithUUID(selectedData.SelectedUUID);
				SelectionManager::SetSelectedEntity(selected);

				if (SelectionManager::GetSelectedEntity() != Entity{})
				{
					m_PanelManager->GetPanel<SceneHierarchyPanel>()->EditSelectedEntityName(false);
				}
			}
		}

		return false;
	}

	bool EditorLayer::OnWindowCloseEvent(WindowCloseEvent& e)
	{
		CloseProject();

		return false;
	}

	void EditorLayer::CreateNewProject()
	{
		Project::New();
	}

	bool EditorLayer::OpenExistingProject()
	{
		std::string filepath = FileDialogue::OpenFileDialog("Vortex Project (*.vxproject)\0*.vxproject\0");

		if (filepath.empty())
			return false;

		OpenProject(filepath);
		return true;
	}

	bool EditorLayer::OpenProject(const std::filesystem::path& filepath)
	{
		VX_PROFILE_FUNCTION();

		if (Project::GetActive())
			CloseProject();

		m_HoveredEntity = Entity{};

		const bool success = ProjectLoader::LoadEditorProject(filepath);
		if (!success)
		{
			VX_CORE_FATAL("Failed to open project: '{}'", filepath.string());
			return false;
		}
		
		SharedReference<EditorAssetManager> editorAssetManager = Project::GetEditorAssetManager();
		std::filesystem::path startScenePath = Project::GetActive()->GetProperties().General.StartScene;
		const AssetMetadata& sceneMetadata = editorAssetManager->GetMetadata(startScenePath);

		auto relativePath = editorAssetManager->GetFileSystemPath(sceneMetadata);
		OpenScene(relativePath);

		SharedReference<Project> activeProject = Project::GetActive();
		m_PanelManager->AddPanel<ProjectSettingsPanel>(activeProject);
		m_PanelManager->AddPanel<ContentBrowserPanel>(Project::GetAssetDirectory())->IsOpen = true;
		
		return true;
	}

	void EditorLayer::SaveProject()
	{
		SaveScene();

		if (m_CaptureFramebufferToDiskOnSave)
		{
			CaptureFramebufferImageToDisk();
		}

		ProjectLoader::SaveActiveEditorProject();
	}

	void EditorLayer::CloseProject()
	{
		if (m_ActiveScene->IsRunning())
			OnSceneStop();

		ScriptEngine::Shutdown();
	}

	void EditorLayer::BuildProject()
	{
		// TODO build asset pack here
	}

	void EditorLayer::BuildAndRunProject()
	{
		BuildProject();

		OnLaunchRuntime(Project::GetProjectFilepath());
	}

	void EditorLayer::CreateNewScene()
	{
		if (m_SceneState != SceneState::Edit)
			return;

		m_EditorScenePath = ""; // No scene on disk yet

		m_ActiveScene = Scene::Create(m_Framebuffer);

		SetSceneContext(m_ActiveScene);
		ResetEditorCameras();

		m_EditorScene = m_ActiveScene;

		Scene::Create3DSampleScene(m_ActiveScene);

		// We didn't actually serialize yet
		std::filesystem::path memorySceneFilepath = Project::GetAssetDirectory() / "Scenes" / "UntitledScene.vortex";

		SetWindowTitle(FileSystem::RemoveFileExtension(memorySceneFilepath.filename()));
	}

	void EditorLayer::OpenExistingScene()
	{
		std::string filepath = FileDialogue::OpenFileDialog("Vortex Scene (*.vortex)\0*.vortex\0");

		if (!filepath.empty())
		{
			OpenScene(filepath);
		}
	}

	void EditorLayer::OpenScene(const std::filesystem::path& filepath)
	{
		if (m_SceneState != SceneState::Edit)
			OnSceneStop();

		m_HoveredEntity = Entity{}; // Prevent an invalid entity from being used elsewhere in the editor

		std::string sceneFilename = FileSystem::RemoveFileExtension(filepath.filename());

		if (filepath.extension() != ".vortex")
		{
			VX_CORE_WARN("Could not load {} - not a scene file", sceneFilename);
			return;
		}

		SharedReference<Scene> newScene = Scene::Create(m_Framebuffer);
		SceneSerializer serializer(newScene);
		newScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);

		std::string timerName = std::format("{} Scene Load Time", sceneFilename);
		InstrumentationTimer timer(timerName.c_str());

		if (serializer.Deserialize(filepath.string()))
		{
			m_EditorScene = newScene;
			SetSceneContext(m_EditorScene);

			ResetEditorCameras();

			m_ActiveScene = m_EditorScene;
			m_EditorScenePath = filepath;

			SetWindowTitle(sceneFilename);

			m_ActiveScene->SetDebugName(sceneFilename);

			const BuildIndexMap& buildIndices = Scene::GetScenesInBuild();

			for (const auto& [buildIndex, sceneFilepath] : buildIndices)
			{
				if (sceneFilepath.find(sceneFilename) == std::string::npos)
					continue;
				
				Scene::SetActiveSceneBuildIndex(buildIndex);

				break;
			}
		}
	}

	void EditorLayer::SaveSceneAs()
	{
		std::string filepath = FileDialogue::SaveFileDialog("Vortex Scene (*.vortex)\0*.vortex\0");

		if (!filepath.empty())
		{
			ReplaceSceneFileExtensionIfNeeded(filepath);

			m_EditorScenePath = filepath;
			
			SerializeScene(m_ActiveScene, m_EditorScenePath);

			SetWindowTitle(FileSystem::RemoveFileExtension(m_EditorScenePath.filename()));
		}
	}

	void EditorLayer::SaveScene()
	{
		m_ActiveScene->SortEntities();

		if (!m_EditorScenePath.empty())
			SerializeScene(m_ActiveScene, m_EditorScenePath);
		else
			SaveSceneAs();
	}

	void EditorLayer::SerializeScene(SharedReference<Scene>& scene, const std::filesystem::path& filepath)
	{
		SceneSerializer serializer(scene);
		serializer.Serialize(filepath.string());
	}

	void EditorLayer::OnScenePlay()
	{
		SharedReference<Project> activeProject = Project::GetActive();
		ProjectProperties projectProps = activeProject->GetProperties();

		if (m_SceneState == SceneState::Simulate)
			OnSceneStop();

		m_SceneState = SceneState::Play;

		if (projectProps.ScriptingProps.ReloadAssemblyOnPlay)
			ScriptEngine::ReloadAssembly();

		if (projectProps.EditorProps.MaximizeOnPlay)
			m_SceneViewportMaximized = true;

		auto consolePanel = m_PanelManager->GetPanel<ConsolePanel>();
		if (consolePanel->ClearOnPlay())
			consolePanel->ClearMessages();

		m_ActiveScene = Scene::Copy(m_EditorScene);
		m_ActiveScene->OnRuntimeStart(projectProps.EditorProps.MuteAudioSources);

		SetSceneContext(m_ActiveScene);

		ScriptRegistry::SetSceneStartTime(Time::GetTime());

		OnNoGizmoSelected();
	}

	void EditorLayer::OnScenePause()
	{
		VX_CORE_ASSERT(m_ActiveScene->IsRunning(), "Scene must be running!");
		if (m_SceneState == SceneState::Edit)
			return;

		m_ActiveScene->SetPaused(true);
	}

	void EditorLayer::OnSceneResume()
	{
		VX_CORE_ASSERT(m_ActiveScene->IsRunning(), "Scene must be running!");
		if (m_SceneState == SceneState::Edit)
			return;

		m_ActiveScene->SetPaused(false);
	}

	void EditorLayer::OnSceneStop()
	{
		VX_CORE_ASSERT(m_SceneState != SceneState::Edit, "Invalid scene state!");

		SharedReference<Project> activeProject = Project::GetActive();
		ProjectProperties& projectProps = activeProject->GetProperties();

		if (m_SceneState == SceneState::Play)
			m_ActiveScene->OnRuntimeStop();
		else if (m_SceneState == SceneState::Simulate)
			m_ActiveScene->OnPhysicsSimulationStop();

		m_SceneState = SceneState::Edit;

		if (projectProps.EditorProps.MaximizeOnPlay)
			m_SceneViewportMaximized = false;

		m_HoveredEntity = Entity{};

		m_ActiveScene = m_EditorScene;
		SetSceneContext(m_ActiveScene);

		// Reset the mouse cursor in case a script turned it off
		Input::SetCursorMode(CursorMode::Normal);

		if (m_TransitionedFromStartScene)
		{
			OpenScene(m_StartScenePath);
			m_TransitionedFromStartScene = false;
		}
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

		SetSceneContext(m_ActiveScene);
	}

	void EditorLayer::RestartSceneSimulation()
	{
		OnSceneSimulate();
	}

	void EditorLayer::QueueSceneTransition()
	{
		VX_PROFILE_FUNCTION();
		VX_CORE_ASSERT(m_ActiveScene->IsRunning(), "Scene must be running to queue transition");

		Application::Get().SubmitToMainThreadQueue([=]()
		{
			m_StartScenePath = m_EditorScenePath;

			const BuildIndexMap& buildIndices = Scene::GetScenesInBuild();
			const uint32_t nextBuildIndex = ScriptRegistry::GetNextBuildIndex();

			if (buildIndices.find(nextBuildIndex) == buildIndices.end())
			{
				VX_CONSOLE_LOG_ERROR("Trying to load Scene with invalid Build Index: {}", nextBuildIndex);
				return;
			}

			std::filesystem::path scenePath = buildIndices.at(nextBuildIndex);
			std::filesystem::path assetDirectory = Project::GetAssetDirectory();
			std::filesystem::path nextSceneFilepath = assetDirectory / scenePath;

			OpenScene(nextSceneFilepath);
			OnScenePlay();

			ScriptRegistry::ResetBuildIndex();

			m_TransitionedFromStartScene = true;
		});
	}

	void EditorLayer::SetWindowTitle(const std::string& sceneName)
	{
		std::string projectName = Project::GetActive()->GetName();
		std::string platformName = Platform::GetName();
		std::string graphicsAPI = RendererAPI::GetAPIInfo().Name;

		Application& application = Application::Get();
		Window& window = application.GetWindow();

		const static std::string originalTitle = window.GetTitle();
		std::string newTitle = fmt::format("{0} - {1} - {2} - {3} - <{4}>",
			projectName,
			sceneName,
			platformName,
			originalTitle,
			graphicsAPI
		);
		window.SetTitle(newTitle);
	}

	void EditorLayer::DuplicateSelectedEntity()
	{
		if (m_SceneState != SceneState::Edit)
			return;

		Entity selectedEntity = SelectionManager::GetSelectedEntity();

		if (!selectedEntity)
			return;

		Entity duplicatedEntity = m_ActiveScene->DuplicateEntity(selectedEntity);
		SelectionManager::SetSelectedEntity(duplicatedEntity);
		
		m_PanelManager->GetPanel<SceneHierarchyPanel>()->EditSelectedEntityName(true);
	}

	void EditorLayer::SetSceneContext(SharedReference<Scene>& scene)
	{
		m_PanelManager->SetSceneContext(scene);
	}

	void EditorLayer::ResetEditorCameras()
	{
		if (m_EditorCamera)
		{
			m_EditorCamera->Focus({ 0, 0, 0 });
			m_EditorCamera->SetDistance(10);
		}
		if (m_SecondEditorCamera)
		{
			m_EditorCamera->Focus({ 0, 0, 0 });
			m_EditorCamera->SetDistance(10);
		}
	}

	void EditorLayer::CaptureFramebufferImageToDisk()
	{
		const std::string sceneImagePath = Project::GetProjectDirectory().string() + "/" + Project::GetActive()->GetName() + ".png";
		
		if (FileSystem::Exists(sceneImagePath))
		{
			FileSystem::Remove(sceneImagePath);
		}

		const uint32_t nrChannels = 3;
		uint32_t stride = nrChannels * (uint32_t)m_ViewportSize.x;

		// make sure alignment is 4 bytes
		stride += (stride % 4) ? (4 - stride % 4) : 0;

		const uint32_t bufferSize = stride * (uint32_t)m_ViewportSize.y;
		Buffer buffer(bufferSize);
		m_Framebuffer->ReadAttachmentToBuffer(0, buffer.As<char>());

		TextureProperties imageProps;
		imageProps.Filepath = sceneImagePath;
		imageProps.Width = (uint32_t)m_ViewportSize.x;
		imageProps.Height = (uint32_t)m_ViewportSize.y;
		imageProps.Channels = nrChannels;
		imageProps.Buffer = buffer.As<const void>();
		imageProps.Stride = stride;

		SharedReference<Texture2D> sceneTexture = Texture2D::Create(imageProps);
		sceneTexture->SaveToFile();
	}

	void EditorLayer::ReplaceSceneFileExtensionIfNeeded(std::string& filepath)
	{
		std::filesystem::path copy = filepath;
		
		if (copy.extension() != ".vortex" || copy.extension().empty())
		{
			FileSystem::ReplaceExtension(copy, ".vortex");
			filepath = copy.string();
		}
	}

	std::vector<Math::vec4> EditorLayer::GetFrustumCornersWorldSpace(const TransformComponent& transform, const SceneCamera& sceneCamera)
	{
		Math::mat4 proj = sceneCamera.GetProjectionMatrix();
		Math::mat4 view = Math::Inverse(transform.GetTransform());

		Math::mat4 inv = Math::Inverse(proj * view);

		std::vector<Math::vec4> frustumCorners;
		for (uint32_t x = 0; x < 2; x++)
		{
			for (uint32_t y = 0; y < 2; y++)
			{
				for (uint32_t z = 0; z < 2; z++)
				{
					const Math::vec4 pt = inv * Math::vec4(
						2.0f * x - 1.0f,
						2.0f * y - 1.0f,
						2.0f * z - 1.0f,
						1.0f
					);

					frustumCorners.push_back(pt / pt.w);
				}
			}
		}

		return frustumCorners;
	}

	std::pair<float, float> EditorLayer::GetMouseViewportSpace(bool mainViewport)
	{
		auto [mx, my] = Gui::GetMousePos();
		const auto& viewportBounds = mainViewport ? m_ViewportBounds : m_SecondViewportBounds;
		mx -= viewportBounds[0].x;
		my -= viewportBounds[0].y;
		auto viewportWidth = viewportBounds[1].x - viewportBounds[0].x;
		auto viewportHeight = viewportBounds[1].y - viewportBounds[0].y;

		return { (mx / viewportWidth) * 2.0f - 1.0f, ((my / viewportHeight) * 2.0f - 1.0f) * -1.0f };
	}

	std::pair<Math::vec3, Math::vec3> EditorLayer::CastRay(EditorCamera* editorCamera, float mx, float my)
	{
		Math::vec4 mouseClipPos = { mx, my, -1.0f, 1.0f };

		auto inverseProj = Math::Inverse(editorCamera->GetProjectionMatrix());
		auto inverseView = Math::Inverse(Math::mat3(editorCamera->GetViewMatrix()));

		Math::vec4 ray = inverseProj * mouseClipPos;
		Math::vec3 rayPos = editorCamera->GetPosition();
		Math::vec3 rayDir = inverseView * Math::vec3(ray);

		return { rayPos, rayDir };
	}

	void EditorLayer::OnNoGizmoSelected()
	{
		m_GizmoType = -1; // Invalid gizmo
	}

	void EditorLayer::OnTranslationToolSelected()
	{
		if (m_SceneState == SceneState::Play && !m_SecondViewportHovered)
			return;

		m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
	}

	void EditorLayer::OnRotationToolSelected()
	{
		if (m_SceneState == SceneState::Play && !m_SecondViewportHovered)
			return;

		m_GizmoType = ImGuizmo::OPERATION::ROTATE;
	}

	void EditorLayer::OnScaleToolSelected()
	{
		if (m_SceneState == SceneState::Play && !m_SecondViewportHovered)
			return;

		m_GizmoType = ImGuizmo::OPERATION::SCALE;
	}

}
