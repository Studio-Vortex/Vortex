#include "EditorLayer.h"

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
#include "Panels/AudioMixerPanel.h"
#include "Panels/SubModulesPanel.h"
#include "Panels/ECSDebugPanel.h"
#include "Panels/AboutPanel.h"

#include <Vortex/Serialization/SceneSerializer.h>
#include <Vortex/Project/ProjectLoader.h>

#include <Vortex/Scripting/ScriptEngine.h>
#include <Vortex/Scripting/ScriptRegistry.h>

#include <Vortex/Editor/EditorCamera.h>
#include <Vortex/Editor/EditorResources.h>
#include <Vortex/Editor/SelectionManager.h>

#include <Vortex/Editor/ConsolePanel.h>

#include <Vortex/Gui/Colors.h>

#include <ImGuizmo.h>

namespace Vortex {

#define VX_EDITOR_MAX_CAMERAS 2

	EditorLayer::EditorLayer()
		: Layer("EditorLayer") { }

	void EditorLayer::OnAttach()
	{
		VX_PROFILE_FUNCTION();

		const Application& app = Application::Get();
		const ApplicationProperties& applicationProperties = app.GetProperties();

		m_RuntimeAppFilepath = app.GetRuntimeBinaryPath();

		FramebufferProperties framebufferProps;

		framebufferProps.Attachments = {
			ImageFormat::RGBA16F,
			ImageFormat::RED_INTEGER,
			ImageFormat::RGBA16F,
			ImageFormat::Depth
		};

		framebufferProps.Width = 1600;
		framebufferProps.Height = 900;
		framebufferProps.Samples = applicationProperties.SampleCount;
		m_Framebuffer = Framebuffer::Create(framebufferProps);
		m_SecondViewportFramebuffer = Framebuffer::Create(framebufferProps);

		EditorResources::Init();

		m_EditorScene = nullptr;
		m_ActiveScene = m_EditorScene;

		m_ViewportPanelSize = { (float)applicationProperties.WindowWidth, (float)applicationProperties.WindowHeight };
		m_SecondViewportPanelSize = { (float)applicationProperties.WindowWidth, (float)applicationProperties.WindowHeight };

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
		m_PanelManager->AddPanel<AudioMixerPanel>();
		m_PanelManager->AddPanel<SubModulesPanel>();
		m_PanelManager->AddPanel<ECSDebugPanel>();
		m_PanelManager->AddPanel<ConsolePanel>()->IsOpen = true;
		m_PanelManager->AddPanel<AboutPanel>();

		const ApplicationCommandLineArgs& arguments = applicationProperties.CommandLineArgs;
		if (arguments.Count > 1)
		{
			const char* arg1 = arguments[1];
			const Fs::Path projectFilepath = Fs::Path(arg1);
			const bool success = OpenProject(projectFilepath);
			VX_CORE_ASSERT(success, "Failed to open project!");
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

		VX_CORE_ASSERT(Project::GetActive(), "Failed to open project!");
		VX_CORE_ASSERT(m_ActiveScene, "failed to open a scene!");

		SharedReference<Project> project = Project::GetActive();
		const ProjectProperties& properties = project->GetProperties();

		EditorCameraProperties editorCameraParams;
		editorCameraParams.Translation = properties.EditorProps.EditorCameraProps.Translation;
		editorCameraParams.Width = m_ViewportPanelSize.x;
		editorCameraParams.Height = m_ViewportPanelSize.y;
		editorCameraParams.FOV = Math::Deg2Rad(properties.EditorProps.EditorCameraProps.FOVdegrees);
		editorCameraParams.NearClip = 0.01f;
		editorCameraParams.FarClip = 1000.0f;
		m_EditorCamera = new EditorCamera(editorCameraParams);

		editorCameraParams.Translation = Math::vec3(-5, 5, 5);
		editorCameraParams.Width = m_SecondViewportPanelSize.x;
		editorCameraParams.Height = m_SecondViewportPanelSize.y;
		editorCameraParams.FOV = Math::Deg2Rad(45.0f);
		editorCameraParams.NearClip = 0.01f;
		editorCameraParams.FarClip = 1000.0f;
		m_SecondEditorCamera = new EditorCamera(editorCameraParams);

		m_PanelManager->OnEditorAttach();
		m_PanelManager->SetSceneContext(m_ActiveScene);
		m_PanelManager->SetProjectContext(project);
	}

	void EditorLayer::OnDetach()
	{
		VX_PROFILE_FUNCTION();

		m_PanelManager->OnEditorDestroy();
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
		if (Actor skyLightActor = m_ActiveScene->GetSkyLightActor())
		{
			const LightSourceComponent& lsc = skyLightActor.GetComponent<LightSourceComponent>();
			if (lsc.Visible && lsc.CastShadows)
			{
				InstrumentationTimer timer("Shadow Pass");
				Renderer::RenderToDepthMap(m_ActiveScene);
				renderTime.ShadowMapRenderTime += timer.ElapsedMS();
			}
		}

		m_ActiveScene->OnViewportResize((uint32_t)m_ViewportPanelSize.x, (uint32_t)m_ViewportPanelSize.y);

		ResizeTargetFramebuffersIfNeeded();

		// Bind Render Target and Clear Attachments
		Renderer::ResetStats();
		Renderer2D::ResetStats();
		m_Framebuffer->Bind();
		RenderCommand::Clear();

		// Clear entityID attachment to -1
		m_Framebuffer->ClearAttachment(1, -1);

		if (!InPlaySceneState())
		{
			m_EditorCamera->SetActive(m_AllowViewportCameraEvents && !m_AllowSecondViewportCameraEvents);
			m_EditorCamera->OnUpdate(delta);
			m_AllowViewportCameraEvents = false;
		}

		if (m_SecondViewportPanelOpen)
		{
			m_SecondEditorCamera->SetActive(m_AllowSecondViewportCameraEvents && !m_AllowViewportCameraEvents);
			m_SecondEditorCamera->OnUpdate(delta);
			m_AllowSecondViewportCameraEvents = false;
		}

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
				{
					OnScenePause();
				}
				else
				{
					OnSceneResume();
				}

				m_ActiveScene->OnUpdateRuntime(delta);

				break;
			}
			case SceneState::Simulate:
			{
				m_ActiveScene->OnUpdateSimulation(delta, m_EditorCamera);
				break;
			}
		}

		// Scene Viewport Actor Selection
		{
			auto [mx, my] = Gui::GetMousePos();

			mx -= m_ViewportBounds.MinBound.x;
			my -= m_ViewportBounds.MinBound.y;
			const Math::vec2 viewportSize = m_ViewportBounds.CalculateViewportSize();
			my = viewportSize.y - my;

			const int mouseX = (int)mx;
			const int mouseY = (int)my;

			if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x && mouseY < (int)viewportSize.y)
			{
				const int pixelData = m_Framebuffer->ReadPixel(1, mouseX, mouseY);
				m_HoveredActor = pixelData == -1 ? Actor() : Actor{ (entt::entity)pixelData, m_ActiveScene.Raw() };
			}
		}

		if (!InPlaySceneState())
		{
			OnOverlayRender(m_EditorCamera);
		}

		m_Framebuffer->Unbind();

		// Bloom pass
		{
			InstrumentationTimer timer = InstrumentationTimer("Bloom Pass");
			Math::vec3 cameraPos = m_EditorCamera->GetPosition();
			if (InPlaySceneState())
			{
				if (Actor primaryCamera = m_ActiveScene->GetPrimaryCameraActor())
				{
					cameraPos = primaryCamera.GetTransform().Translation;
				}
			}

			PostProcessProperties postProcessProps{};
			postProcessProps.TargetFramebuffer = m_Framebuffer;
			postProcessProps.CameraPosition = cameraPos;
			postProcessProps.ViewportInfo = Viewport{ 0, 0, (uint32_t)m_ViewportPanelSize.x, (uint32_t)m_ViewportPanelSize.y };
			PostProcessStage stages[] = { PostProcessStage::Bloom };
			postProcessProps.Stages = stages;
			postProcessProps.StageCount = VX_ARRAYSIZE(stages);
			Renderer::BeginPostProcessingStages(postProcessProps);
			renderTime.BloomPassRenderTime += timer.ElapsedMS();
		}

		if (m_SecondViewportPanelOpen)
		{
			// Second Viewport
			m_SecondViewportFramebuffer->Bind();
			RenderCommand::Clear();

			// Clear entityID attachment to -1
			m_SecondViewportFramebuffer->ClearAttachment(1, -1);

			SceneRenderPacket renderPacket{};
			renderPacket.PrimaryCamera = m_SecondEditorCamera;
			renderPacket.PrimaryCameraViewMatrix = m_SecondEditorCamera->GetViewMatrix();
			renderPacket.PrimaryCameraProjectionMatrix = m_SecondEditorCamera->GetProjectionMatrix();
			renderPacket.TargetFramebuffer = m_SecondViewportFramebuffer;
			renderPacket.Scene = m_ActiveScene.Raw();
			renderPacket.IsEditorScene = true;
			m_SecondViewportRenderer.RenderScene(renderPacket);

			// Scene Viewport Actor Selection
			{
				auto [mx, my] = Gui::GetMousePos();

				mx -= m_SecondViewportBounds.MinBound.x;
				my -= m_SecondViewportBounds.MinBound.y;
				const Math::vec2 viewportSize = m_SecondViewportBounds.CalculateViewportSize();
				my = viewportSize.y - my;

				const int mouseX = (int)mx;
				const int mouseY = (int)my;

				if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x && mouseY < (int)viewportSize.y)
				{
					const int pixelData = m_SecondViewportFramebuffer->ReadPixel(1, mouseX, mouseY);
					m_HoveredActor = pixelData == -1 ? Actor() : Actor{ (entt::entity)pixelData, m_ActiveScene.Raw() };
				}
			}

			OnOverlayRender(m_SecondEditorCamera);

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

		if (ScriptRegistry::TransitionQueued())
		{
			QueueSceneTransition(ScriptRegistry::GetNextSceneByName());
		}
	}

	void EditorLayer::OnEvent(Event& e)
	{
		// only one camera can process events at a time
		if (!InPlaySceneState() && m_AllowViewportCameraEvents)
		{
			m_EditorCamera->OnEvent(e);
		}
		else if (m_SecondViewportPanelOpen && m_AllowSecondViewportCameraEvents)
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

		if ((Gui::IsMouseClicked(ImGuiMouseButton_Left) || Gui::IsMouseClicked(ImGuiMouseButton_Right))
			&& (!m_StartedClickInViewport || !m_StartedClickInSecondViewport))
		{
			ImGuiWindow* hoveredWindow = GImGui->HoveredWindow;
			if (hoveredWindow)
			{
				const std::string name = hoveredWindow->Name;
				if (name == "Scene" || name == "Second Viewport")
				{
					Gui::FocusWindow(hoveredWindow);
				}
			}
		}

		OnMainMenuBarRender();

		// Render Panels if the scene isn't maximized
		// we also need to wait until the project is open before rendering panels
		const bool projectOpen = Project::GetActive();
		if (projectOpen && !m_SceneViewportMaximized)
		{
			OnPanelsRender();
		}

		// Always render if open
		m_PanelManager->OnGuiRender<PerformancePanel>();

		// Render C# Actor.OnGui()
		m_ActiveScene->InvokeActorOnGuiRender();

		if (m_SceneViewportPanelOpen)
		{
			OnScenePanelRender();
		}

		if (m_SecondViewportPanelOpen)
		{
			OnSecondViewportPanelRender();
		}

		if (m_ShowViewportCreateActorMenu)
		{
			Gui::OpenPopup("ViewportCreateActorMenu");
			m_ShowViewportCreateActorMenu = false;
		}

		if (Gui::IsPopupOpen("ViewportCreateActorMenu"))
		{
			Gui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 5.0f, 5.0f });
		}

		if (Gui::BeginPopup("ViewportCreateActorMenu", ImGuiWindowFlags_NoMove))
		{
			EditorCamera* camera = GetCurrentEditorCamera();
			if (camera)
			{
				m_PanelManager->GetPanel<SceneHierarchyPanel>()->DisplayCreateActorMenu(camera);
			}

			Gui::PopStyleVar();
			Gui::EndPopup();
		}

		Gui::End();
	}

	void EditorLayer::OnPanelsRender()
	{
		m_PanelManager->OnGuiRender<PhysicsMaterialEditorPanel>();
		m_PanelManager->OnGuiRender<PhysicsStatisticsPanel>();
		m_PanelManager->OnGuiRender<ProjectSettingsPanel>();
		m_PanelManager->OnGuiRender<NetworkManagerPanel>();
		m_PanelManager->GetPanel<SceneHierarchyPanel>()->OnGuiRender(m_HoveredActor, m_EditorCamera);
		m_PanelManager->OnGuiRender<ContentBrowserPanel>();
		m_PanelManager->OnGuiRender<ScriptRegistryPanel>();
		m_PanelManager->OnGuiRender<MaterialEditorPanel>();
		m_PanelManager->OnGuiRender<SceneRendererPanel>();
		m_PanelManager->OnGuiRender<AssetRegistryPanel>();
		m_PanelManager->OnGuiRender<BuildSettingsPanel>();
		m_PanelManager->OnGuiRender<SystemManagerPanel>();
		m_PanelManager->OnGuiRender<ShaderEditorPanel>();
		m_PanelManager->OnGuiRender<AudioMixerPanel>();
		m_PanelManager->OnGuiRender<SubModulesPanel>();
		m_PanelManager->OnGuiRender<ECSDebugPanel>();
		m_PanelManager->OnGuiRender<ConsolePanel>();
		m_PanelManager->OnGuiRender<AboutPanel>();
	}

	void EditorLayer::OnMainMenuBarRender()
	{
		VX_PROFILE_FUNCTION();

		SharedReference<Project> project = Project::GetActive();
		ProjectProperties& properties = project->GetProperties();

		auto separator = []() {
			UI::Draw::Underline();
			Gui::Spacing();
		};

		if (Gui::BeginMenuBar())
		{
			if (Gui::BeginMenu("File"))
			{
				if (Gui::MenuItem("New Project"))
				{
					CreateNewProject();
					Gui::CloseCurrentPopup();
				}
				separator();

				if (Gui::MenuItem("Open Project...", "Ctrl+O"))
				{
					OpenExistingProject();
					Gui::CloseCurrentPopup();
				}
				separator();

				if (Gui::MenuItem("Save Project"))
				{
					SaveProject();
					Gui::CloseCurrentPopup();
				}
				separator();

				if (Gui::MenuItem("New Scene", "Ctrl+N"))
				{
					CreateNewScene();
					Gui::CloseCurrentPopup();
				}
				separator();

				if (Gui::MenuItem("Open Scene..."))
				{
					OpenExistingScene();
					Gui::CloseCurrentPopup();
				}
				separator();

				if (Gui::MenuItem("Save", "Ctrl+S"))
				{
					SaveScene();
					Gui::CloseCurrentPopup();
				}
				separator();

				if (Gui::MenuItem("Save As...", "Ctrl+Shift+S"))
				{
					SaveSceneAs();
					Gui::CloseCurrentPopup();
				}
				separator();

				if (Gui::MenuItem("Exit", "Alt+F4"))
				{
					Application::Get().Close();
					Gui::CloseCurrentPopup();
				}

				Gui::EndMenu();
			}

			if (Gui::BeginMenu("Edit"))
			{
				if (InEditSceneState())
				{
					if (Gui::MenuItem("Play Scene", "Ctrl+P"))
					{
						OnScenePlay();
						Gui::CloseCurrentPopup();
					}
					separator();

					if (Gui::MenuItem("Play Simulation", "Ctrl+X"))
					{
						OnSceneSimulate();
						Gui::CloseCurrentPopup();
					}

					if (Actor selectedActor = SelectionManager::GetSelectedActor())
					{
						separator();

						if (Gui::MenuItem("Rename Actor", "F2"))
						{
							m_PanelManager->GetPanel<SceneHierarchyPanel>()->FocusOnActorName(true);
							Gui::CloseCurrentPopup();
						}
						separator();

						if (Gui::MenuItem("Duplicate Actor", "Ctrl+D"))
						{
							DuplicateSelectedActor();
							Gui::CloseCurrentPopup();
						}
						separator();

						if (Gui::MenuItem("Delete Actor", "Del"))
						{
							m_ActiveScene->SubmitToDestroyActor(selectedActor);
							Gui::CloseCurrentPopup();
						}
						separator();

						if (Gui::MenuItem("Deselect Actor"))
						{
							SelectionManager::DeselectActor();
							Gui::CloseCurrentPopup();
						}
						separator();

						if (Gui::MenuItem("Move To Camera Position"))
						{
							TransformComponent& transform = selectedActor.GetTransform();
							transform.Translation = m_EditorCamera->GetPosition();
							transform.SetRotationEuler(Math::vec3(-m_EditorCamera->GetPitch(), -m_EditorCamera->GetYaw(), transform.GetRotationEuler().z));
							Gui::CloseCurrentPopup();
						}
						separator();

						if (Gui::MenuItem("Reset Translation", "Alt+W"))
						{
							TransformComponent& transformComponent = selectedActor.GetTransform();
							transformComponent.Translation = Math::vec3(0.0f);
							Gui::CloseCurrentPopup();
						}
						separator();

						if (Gui::MenuItem("Reset Rotation", "Alt+E"))
						{
							TransformComponent& transformComponent = selectedActor.GetTransform();
							Math::quaternion identity(1.0f, 0.0f, 0.0f, 0.0f);
							transformComponent.SetRotation(identity);
							Gui::CloseCurrentPopup();
						}
						separator();

						if (Gui::MenuItem("Reset Scale", "Alt+R"))
						{
							TransformComponent& transformComponent = selectedActor.GetTransform();
							transformComponent.Scale = Math::vec3(1.0f);
							Gui::CloseCurrentPopup();
						}
					}
				}
				else
				{
					if (InPlaySceneState())
					{
						if (Gui::MenuItem("Stop Scene", "Ctrl+P"))
						{
							OnSceneStop();
							Gui::CloseCurrentPopup();
						}
						separator();

						if (Gui::MenuItem("Restart Scene", "Ctrl+Shift+P"))
						{
							RestartScene();
							Gui::CloseCurrentPopup();
						}
					}
					else if (InSimulateSceneState())
					{
						if (Gui::MenuItem("Stop Simulation", "Ctrl+X"))
						{
							OnSceneStop();
							Gui::CloseCurrentPopup();
						}
						separator();

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
				if (Gui::MenuItem("Maximize On Play", nullptr, &properties.EditorProps.MaximizeOnPlay))
				{
					Gui::CloseCurrentPopup();
				}
				separator();

				std::string itemLabel = m_SceneViewportMaximized ? "Minimize Scene" : "Maximize Scene";
				if (Gui::MenuItem(itemLabel.c_str(), "Ctrl+Space"))
				{
					ToggleSceneViewportMaximized();
					Gui::CloseCurrentPopup();
				}

				Gui::EndMenu();
			}

			if (Gui::BeginMenu("Actor"))
			{
				EditorCamera* camera = GetCurrentEditorCamera();
				if (camera)
				{
					m_PanelManager->GetPanel<SceneHierarchyPanel>()->DisplayCreateActorMenu(camera);
				}

				Gui::EndMenu();
			}

			if (Gui::BeginMenu("Tools"))
			{
				if (Gui::MenuItem("Select Tool", "Q"))
				{
					OnSelectGizmoToolSelected();
					Gui::CloseCurrentPopup();
				}
				separator();

				if (Gui::MenuItem("Translation Tool", "W"))
				{
					OnTranslationGizmoToolSelected();
					Gui::CloseCurrentPopup();
				}
				separator();

				if (Gui::MenuItem("Rotation Tool", "E"))
				{
					OnRotationGizmoToolSelected();
					Gui::CloseCurrentPopup();
				}
				separator();

				if (Gui::MenuItem("Scale Tool", "R"))
				{
					OnScaleToolGizmoSelected();
					Gui::CloseCurrentPopup();
				}

				Gui::EndMenu();
			}

			if (Gui::BeginMenu("Script"))
			{
				Fs::Path projectSolutionFilename = Fs::Path(project->GetName());
				projectSolutionFilename.replace_extension(".sln");
				Fs::Path scriptsFolder = Fs::Path("Projects") / project->GetName() / "Assets\\Scripts";
				Fs::Path solutionPath = scriptsFolder / projectSolutionFilename;

				if (Gui::MenuItem("Create Script"))
				{
					m_CreateScriptPopupOpen = true;
					Gui::CloseCurrentPopup();
				}
				separator();

				if (Gui::MenuItem("Open Visual Studio Solution"))
				{
					FileSystem::SetWorkingDirectory(scriptsFolder);
					Platform::LaunchProcess(projectSolutionFilename.string().c_str(), "");
					FileSystem::SetWorkingDirectory(Application::Get().GetProperties().WorkingDirectory);
					Gui::CloseCurrentPopup();
				}
				separator();

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
				separator();

				if (Gui::MenuItem("Build and Run", "Ctrl+B"))
				{
					BuildAndRunProject();
					Gui::CloseCurrentPopup();
				}
				separator();

				m_PanelManager->MenuBarItem<BuildSettingsPanel>("Ctrl+Shift+B");

				Gui::EndMenu();
			}

			if (Gui::BeginMenu("Window"))
			{
				m_PanelManager->MenuBarItem<AudioMixerPanel>();
				separator();
				m_PanelManager->MenuBarItem<ConsolePanel>();
				separator();
				m_PanelManager->MenuBarItem<ContentBrowserPanel>();
				separator();
				Gui::MenuItem("Inspector", nullptr, &m_PanelManager->GetPanel<SceneHierarchyPanel>()->IsInspectorOpen());
				separator();
				m_PanelManager->MenuBarItem<MaterialEditorPanel>();
				separator();
				m_PanelManager->MenuBarItem<PhysicsMaterialEditorPanel>();
				separator();
				Gui::MenuItem("Scene", nullptr, &m_SceneViewportPanelOpen);
				separator();
				m_PanelManager->MenuBarItem<SceneHierarchyPanel>();
				separator();
				m_PanelManager->MenuBarItem<SceneRendererPanel>();
				separator();
				Gui::MenuItem("Second Viewport", nullptr, &m_SecondViewportPanelOpen);
				separator();
				m_PanelManager->MenuBarItem<ShaderEditorPanel>();
				separator();
				m_PanelManager->MenuBarItem<ProjectSettingsPanel>();
				separator();

				if (Gui::BeginMenu("Debug"))
				{
					m_PanelManager->MenuBarItem<AssetRegistryPanel>();
					separator();
					m_PanelManager->MenuBarItem<ECSDebugPanel>();
					separator();
					m_PanelManager->MenuBarItem<NetworkManagerPanel>();
					separator();
					m_PanelManager->MenuBarItem<PerformancePanel>();
					separator();
					m_PanelManager->MenuBarItem<PhysicsStatisticsPanel>();
					separator();
					m_PanelManager->MenuBarItem<ScriptRegistryPanel>();
					separator();
					m_PanelManager->MenuBarItem<SubModulesPanel>();
					separator();
					m_PanelManager->MenuBarItem<SystemManagerPanel>();

					Gui::EndMenu();
				}

				Gui::EndMenu();
			}

			if (Gui::BeginMenu("Help"))
			{
				m_PanelManager->MenuBarItem<AboutPanel>();

				Gui::EndMenu();
			}

			Gui::EndMenuBar();
		}
	}

	void EditorLayer::OnScenePanelRender()
	{
		VX_PROFILE_FUNCTION();

		ImVec4 borderColor = { 0, 0, 0, 0 };
		const bool timeLeft = m_SceneViewportBorderFadeTimer > 0.0f;
		const bool inEditMode = InEditSceneState();
		const bool fading = timeLeft && !inEditMode;
		const TimeStep delta = Time::GetDeltaTime();

		UI::ScopedStyle windowPadding(ImGuiStyleVar_WindowPadding, ImVec2{ m_SceneViewportBorderSize, m_SceneViewportBorderSize });
		if (fading)
		{
			Gui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, m_SceneViewportBorderSize);
		}

		const ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
		Gui::Begin("Scene", &m_SceneViewportPanelOpen, flags);

		const ImVec2 viewportMinRegion = Gui::GetWindowContentRegionMin();
		const ImVec2 viewportMaxRegion = Gui::GetWindowContentRegionMax();
		const ImVec2 viewportOffset = Gui::GetWindowPos();

		m_ViewportBounds.MinBound = Math::vec2(viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y);
		m_ViewportBounds.MaxBound = Math::vec2(viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y);

		m_ActiveScene->SetViewportBounds(m_ViewportBounds);

		const ImVec2 minBound = { m_ViewportBounds.MinBound.x, m_ViewportBounds.MinBound.y };
		const ImVec2 maxBound = { m_ViewportBounds.MaxBound.x, m_ViewportBounds.MaxBound.y };

		m_AllowViewportCameraEvents = (ImGui::IsMouseHoveringRect(minBound, maxBound) && m_SceneViewportFocused) || m_StartedClickInViewport;

		m_SceneViewportFocused = Gui::IsWindowFocused();
		m_SceneViewportHovered = Gui::IsWindowHovered();
		Application::Get().GetGuiLayer()->BlockEvents(!m_SceneViewportHovered && !m_SecondViewportFocused);

		// Scene Viewport Border
		if (fading)
		{
			m_SceneViewportBorderFadeTimer -= delta;
			const float mapped = Math::Min(1.0f, m_SceneViewportBorderFadeTimer * 0.5f + 0.5f);
			const Math::vec4& viewportBorderColor = m_SceneViewportBorderFadeColor;
			const Math::vec4 lvalue = viewportBorderColor * mapped;
			borderColor = *(ImVec4*)&(lvalue);
		}
		
		const ImVec2 scenePanelSize = Gui::GetContentRegionAvail();
		m_ViewportPanelSize = { scenePanelSize.x, scenePanelSize.y };

		const uint32_t sceneTextureID = m_Framebuffer->GetColorAttachmentRendererID();
		UI::ImageEx(sceneTextureID, ImVec2{ m_ViewportPanelSize.x, m_ViewportPanelSize.y }, { 1, 1, 1, 1 }, borderColor);

		if (inEditMode)
		{
			UIHandleAssetDrop();
			UIOnPopupRender();
		}

		const bool scenePanelVisible = Gui::IsItemVisible();
		const bool normalCursorMode = Input::GetCursorMode() == CursorMode::Normal;
		const bool inPlayMode = InPlaySceneState();
		const bool displayToolbars = scenePanelVisible && !(!normalCursorMode && inPlayMode);
		
		if (displayToolbars)
		{
			UITransformationModeToolbar();
			UIGizmosToolbar();
			UICentralToolbar();
			UIViewportSettingsToolbar();
		}

		if ((m_SceneViewportHovered || !m_SecondViewportPanelOpen) && !InPlaySceneState())
		{
			OnGizmosRender(m_EditorCamera, m_ViewportBounds);
		}

		if (fading)
		{
			Gui::PopStyleVar();
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
				const Fs::Path filepath = Fs::Path(path);

				const AssetType assetType = Project::GetEditorAssetManager()->GetAssetTypeFromFilepath(filepath);

				if (assetType == AssetType::None) {
					VX_CONSOLE_LOG_ERROR("[Editor] Could not load asset with AssetType of none!");
					Gui::EndDragDropTarget();
					return;
				}

				switch (assetType)
				{
					case AssetType::FontAsset: {
						break;
					}
					case AssetType::AudioAsset: {
						break;
					}
					case AssetType::SceneAsset: {
						const AssetMetadata& metadata = Project::GetEditorAssetManager()->GetMetadata(filepath);
						if (!AssetManager::IsHandleValid(metadata.Handle)) {
							VX_CONSOLE_LOG_ERROR("[Editor] Failed to open scene, metadata was invalid!");
							break;
						}

						OpenScene(metadata);

						break;
					}
					case AssetType::PrefabAsset: {
						AssetHandle prefabHandle = Project::GetEditorAssetManager()->GetAssetHandleFromFilepath(filepath);
						if (!AssetManager::IsHandleValid(prefabHandle)) {
							VX_CONSOLE_LOG_ERROR("[Editor] Failed to instantiate prefab, asset handle was invalid!");
							break;
						}

						EditorCamera* editorCamera = GetCurrentEditorCamera();
						if (editorCamera) {
							const Math::vec3 translation = editorCamera->GetFocalPoint() + editorCamera->GetForwardDirection();
							SharedReference<Prefab> prefab = AssetManager::GetAsset<Prefab>(prefabHandle);
							Actor prefabInstance = m_ActiveScene->Instantiate(prefab, &translation);
							SelectionManager::SetSelectedActor(prefabInstance);
							m_PanelManager->GetPanel<SceneHierarchyPanel>()->FocusOnActorName(true);
						}

						break;
					}
					case AssetType::ScriptAsset: {
						if (!m_HoveredActor)
							m_HoveredActor = GetHoveredMeshActorFromRaycast();
						if (!m_HoveredActor)
							break;

						ScriptComponent& scriptComponent = m_HoveredActor.AddOrReplaceComponent<ScriptComponent>();
						const std::unordered_map<std::string, SharedReference<ScriptClass>> scriptClasses = ScriptEngine::GetScriptClasses();

						const std::string droppedClassName = Project::GetEditorAssetManager()->GetRelativePath(filepath).string();

						for (const auto& [className, instance] : scriptClasses)
						{
							if (className.find(droppedClassName) != std::string::npos)
							{
								scriptComponent.ClassName = className;
							}
						}

						if (scriptComponent.ClassName.empty())
						{
							VX_CONSOLE_LOG_ERROR("[Editor] Failed to locate class name for script '{}', the class and script name must match!", droppedClassName);
						}

						break;
					}
					case AssetType::TextureAsset: {
						if (!m_HoveredActor)
							m_HoveredActor = GetHoveredMeshActorFromRaycast();
						if (!m_HoveredActor || !m_HoveredActor.HasAny<SpriteRendererComponent, StaticMeshRendererComponent, MeshRendererComponent>())
							break;

						AssetHandle textureHandle = Project::GetEditorAssetManager()->GetAssetHandleFromFilepath(filepath);
						if (!AssetManager::IsHandleValid(textureHandle))
						{
							VX_CONSOLE_LOG_WARN("[Editor] Failed to load texture, asset handle was invalid!");
							break;
						}

						if (m_HoveredActor.HasComponent<SpriteRendererComponent>())
						{
							m_HoveredActor.GetComponent<SpriteRendererComponent>().Texture = textureHandle;
						}
						else if (m_HoveredActor.HasComponent<StaticMeshRendererComponent>())
						{
							StaticMeshRendererComponent& staticMeshRendererComponent = m_HoveredActor.GetComponent<StaticMeshRendererComponent>();
							AssetHandle staticMeshHandle = staticMeshRendererComponent.StaticMesh;

							SharedReference<MaterialTable> materialTable = staticMeshRendererComponent.Materials;

							if (!AssetManager::IsHandleValid(staticMeshHandle)) {
								VX_CONSOLE_LOG_ERROR("[Editor] Failed to set material texture, static mesh asset handle was invalid!");
								break;
							}

							const std::string filename = filepath.filename().string();

							// TODO this should be dynamic
							AssetHandle materialHandle = materialTable->GetMaterial(0);
							if (!AssetManager::IsHandleValid(materialHandle)) {
								VX_CONSOLE_LOG_ERROR("[Editor] Failed to set material texture, asset handle was invalid!");
								break;
							}

							SharedReference<Material> material = AssetManager::GetAsset<Material>(materialHandle);
							if (!material) {
								break;
							}

							if (filename.find("albedo") != std::string::npos || filename.find("diffuse") != std::string::npos || filename.find("base_color") != std::string::npos)
								material->SetTexture("u_AlbedoMap", textureHandle);
							else if (filename.find("normal") != std::string::npos)
								material->SetTexture("u_NormalMap", textureHandle);
							else if (filename.find("metallic") != std::string::npos || filename.find("specular") != std::string::npos)
								material->SetTexture("u_MetallicMap", textureHandle);
							else if (filename.find("roughness") != std::string::npos)
								material->SetTexture("u_RoughnessMap", textureHandle);
							else if (filename.find("emissive") != std::string::npos || filename.find("emission") != std::string::npos)
								material->SetTexture("u_EmissionMap", textureHandle);
							else if (filename.find("height") != std::string::npos || filename.find("displacement") != std::string::npos)
								material->SetTexture("u_ParallaxOcclusionMap", textureHandle);
							else if (filename.find("ao") != std::string::npos)
								material->SetTexture("u_AmbientOcclusionMap", textureHandle);
							else
								material->SetTexture("u_AlbedoMap", textureHandle);
						}

						break;
					}
					case AssetType::MaterialAsset: {
						m_HoveredActor = GetHoveredMeshActorFromRaycast();
						if (!m_HoveredActor || !m_HoveredActor.HasAny<MeshRendererComponent, StaticMeshRendererComponent>())
							break;

						AssetHandle materialHandle = Project::GetEditorAssetManager()->GetAssetHandleFromFilepath(filepath);
						if (!AssetManager::IsHandleValid(materialHandle)) {
							VX_CONSOLE_LOG_WARN("[Editor] Failed to load material, asset handle was invalid!");
							break;
						}

						SharedReference<Material> material = AssetManager::GetAsset<Material>(materialHandle);
						if (!material) {
							break;
						}

						// TODO we should be able to set an actual index here eventually instead of being hardcoded

						if (m_HoveredActor.HasComponent<MeshRendererComponent>())
						{
							MeshRendererComponent& meshRendererComponent = m_HoveredActor.GetComponent<MeshRendererComponent>();
							SharedReference<MaterialTable> materialTable = meshRendererComponent.Materials;

							materialTable->SetMaterial(0, materialHandle);
						}
						else if (m_HoveredActor.HasComponent<StaticMeshRendererComponent>())
						{
							StaticMeshRendererComponent& staticMeshRendererComponent = m_HoveredActor.GetComponent<StaticMeshRendererComponent>();
							SharedReference<MaterialTable>& materialTable = staticMeshRendererComponent.Materials;

							materialTable->SetMaterial(0, materialHandle);
						}

						material->SetName(FileSystem::RemoveFileExtension(filepath));

						break;
					}
					case AssetType::AnimatorAsset: {
						break;
					}
					case AssetType::AnimationAsset: {
						break;
					}
					case AssetType::MeshAsset: // Fallthrough
					case AssetType::StaticMeshAsset: {
						m_HoveredActor = GetHoveredMeshActorFromRaycast();
						if (!m_HoveredActor || !m_HoveredActor.HasAny<MeshRendererComponent, StaticMeshRendererComponent>())
							break;

						const Fs::Path modelPath = filepath;

						m_MeshImportPopupOpen = true;
						m_MeshImportPopupData.MeshFilepath = modelPath.string();
						m_MeshImportPopupData.MeshActorToEdit = m_HoveredActor;

						break;
					}
					case AssetType::EnvironmentAsset: {
						AssetHandle environmentHandle = Project::GetEditorAssetManager()->GetAssetHandleFromFilepath(filepath);
						if (!AssetManager::IsHandleValid(environmentHandle))
							break;

						Actor environmentActor = m_ActiveScene->GetEnvironmentActor();
						// TODO should we create one if one doesn't exist?
						if (!environmentActor) {
							VX_CONSOLE_LOG_ERROR("[Editor] Failed to load environment map, no actor with a skybox component was found in the scene!");
							break;
						}

						SkyboxComponent& skyboxComponent = environmentActor.GetComponent<SkyboxComponent>();
						skyboxComponent.Skybox = environmentHandle;

						break;
					}
					case AssetType::PhysicsMaterialAsset: {
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

	void EditorLayer::OnGizmosRender(EditorCamera* editorCamera, const ViewportBounds& viewportBounds)
	{
		VX_PROFILE_FUNCTION();

		Actor selectedActor = SelectionManager::GetSelectedActor();
		if (!selectedActor)
		{
			return;
		}

		const bool validGizmoTool = m_GizmoType != -1;
		const bool altDown = Input::IsKeyDown(KeyCode::LeftAlt) || Input::IsKeyDown(KeyCode::RightAlt);
		const bool rightMouseButtonDown = Input::IsMouseButtonDown(MouseButton::Right);
		const bool showGizmos = (selectedActor && validGizmoTool && !altDown && !rightMouseButtonDown);

		if (showGizmos)
		{
			SharedReference<Project> project = Project::GetActive();
			const ProjectProperties& properties = project->GetProperties();

			ImGuizmo::Enable(properties.GizmoProps.Enabled);
			ImGuizmo::SetOrthographic(properties.GizmoProps.IsOrthographic);
			ImGuizmo::SetDrawlist();

			ImGuizmo::SetRect(viewportBounds.MinBound.x, viewportBounds.MinBound.y, viewportBounds.MaxBound.x - viewportBounds.MinBound.x, viewportBounds.MaxBound.y - viewportBounds.MinBound.y);

			const Math::mat4& cameraProjection = editorCamera->GetProjectionMatrix();
			const Math::mat4& cameraView = editorCamera->GetViewMatrix();

			TransformComponent& actorTransform = selectedActor.GetTransform();
			Math::mat4 transform = m_ActiveScene->GetWorldSpaceTransformMatrix(selectedActor);

			// Snapping
			const bool controlDown = Input::IsKeyDown(KeyCode::LeftControl) || Input::IsKeyDown(KeyCode::RightControl);
			const bool snapEnabled = properties.GizmoProps.SnapEnabled && controlDown;

			const float snapValue = m_GizmoType == ImGuizmo::ROTATE ? properties.GizmoProps.RotationSnapValue : properties.GizmoProps.SnapValue;
			std::array<float, 3> snapValues{};
			snapValues.fill(snapValue);

			ImGuizmo::Manipulate(
				Math::ValuePtr(cameraView),
				Math::ValuePtr(cameraProjection),
				(ImGuizmo::OPERATION)m_GizmoType,
				(ImGuizmo::MODE)m_TransformationMode,
				Math::ValuePtr(transform),
				nullptr,
				snapEnabled ? snapValues.data() : nullptr
			);

			if (properties.GizmoProps.DrawGrid)
			{
				ImGuizmo::DrawGrid(Math::ValuePtr(cameraView), Math::ValuePtr(cameraProjection), Math::ValuePtr(transform), properties.GizmoProps.GridSize);
			}

			if (ImGuizmo::IsUsing())
			{
				if (Actor parent = selectedActor.GetParent())
				{
					const Math::mat4 parentTransform = m_ActiveScene->GetWorldSpaceTransformMatrix(parent);
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
						actorTransform.Translation = translation;
						break;
					}
					case ImGuizmo::OPERATION::ROTATE:
					{
						// Do this in Euler in an attempt to preserve any full revolutions (> 360)
						Math::vec3 originalEulerRotation = actorTransform.GetRotationEuler();

						// Map original rotation to range [-180, 180] which is what ImGuizmo gives us
						originalEulerRotation.x = fmodf(originalEulerRotation.x + Math::PI, Math::TWO_PI) - Math::PI;
						originalEulerRotation.y = fmodf(originalEulerRotation.y + Math::PI, Math::TWO_PI) - Math::PI;
						originalEulerRotation.z = fmodf(originalEulerRotation.z + Math::PI, Math::TWO_PI) - Math::PI;

						Math::vec3 deltaRotationEuler = rotation - originalEulerRotation;

						// Try to avoid drift due numeric precision
						if (fabs(deltaRotationEuler.x) < 0.001) deltaRotationEuler.x = 0.0f;
						if (fabs(deltaRotationEuler.y) < 0.001) deltaRotationEuler.y = 0.0f;
						if (fabs(deltaRotationEuler.z) < 0.001) deltaRotationEuler.z = 0.0f;

						actorTransform.SetRotationEuler(actorTransform.GetRotationEuler() += deltaRotationEuler);
						break;
					}
					case ImGuizmo::OPERATION::SCALE:
					{
						actorTransform.Scale = scale;
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
		Gui::Begin("Second Viewport", &m_SecondViewportPanelOpen, ImGuiWindowFlags_NoCollapse);

		const ImVec2 viewportMinRegion = Gui::GetWindowContentRegionMin();
		const ImVec2 viewportMaxRegion = Gui::GetWindowContentRegionMax();
		const ImVec2 viewportOffset = Gui::GetWindowPos();

		m_SecondViewportBounds.MinBound = Math::vec2(viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y);
		m_SecondViewportBounds.MaxBound = Math::vec2(viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y);

		const ImVec2 minBound = { m_SecondViewportBounds.MinBound.x, m_SecondViewportBounds.MinBound.y };
		const ImVec2 maxBound = { m_SecondViewportBounds.MaxBound.x, m_SecondViewportBounds.MaxBound.y };

		m_AllowSecondViewportCameraEvents = (ImGui::IsMouseHoveringRect(minBound, maxBound) && m_SecondViewportFocused) || m_StartedClickInSecondViewport;

		m_SecondViewportFocused = Gui::IsWindowFocused();
		m_SecondViewportHovered = Gui::IsWindowHovered();
		Application::Get().GetGuiLayer()->BlockEvents(!m_SecondViewportHovered && !m_SceneViewportFocused);

		const ImVec2 scenePanelSize = Gui::GetContentRegionAvail();
		m_SecondViewportPanelSize = { scenePanelSize.x, scenePanelSize.y };

		const uint32_t sceneTextureID = m_SecondViewportFramebuffer->GetColorAttachmentRendererID();
		UI::ImageEx(sceneTextureID, ImVec2{ m_SecondViewportPanelSize.x, m_SecondViewportPanelSize.y });

		if (m_SecondViewportHovered)
		{
			OnGizmosRender(m_SecondEditorCamera, m_SecondViewportBounds);
		}

		Gui::End();
	}

	void EditorLayer::ResizeTargetFramebuffersIfNeeded()
	{
		VX_PROFILE_FUNCTION();

		// Resize
		if (m_SceneViewportPanelOpen)
		{
			if (FramebufferProperties props = m_Framebuffer->GetProperties();
				m_ViewportPanelSize.x > 0.0f && m_ViewportPanelSize.y > 0.0f && // zero sized framebuffer is invalid
				(props.Width != m_ViewportPanelSize.x || props.Height != m_ViewportPanelSize.y))
			{
				m_Framebuffer->Resize((uint32_t)m_ViewportPanelSize.x, (uint32_t)m_ViewportPanelSize.y);
				m_EditorCamera->SetViewportSize((uint32_t)m_ViewportPanelSize.x, (uint32_t)m_ViewportPanelSize.y);
			}

			if (m_EditorCamera->IsDirty())
			{
				m_EditorCamera->SetViewportSize((uint32_t)m_ViewportPanelSize.x, (uint32_t)m_ViewportPanelSize.y);
			}
		}

		if (m_SecondViewportPanelOpen)
		{
			if (FramebufferProperties props = m_SecondViewportFramebuffer->GetProperties();
				m_SecondViewportPanelSize.x > 0.0f && m_SecondViewportPanelSize.y > 0.0f && // zero sized framebuffer is invalid
				(props.Width != m_SecondViewportPanelSize.x || props.Height != m_SecondViewportPanelSize.y))
			{
				m_SecondViewportFramebuffer->Resize((uint32_t)m_SecondViewportPanelSize.x, (uint32_t)m_SecondViewportPanelSize.y);
				m_SecondEditorCamera->SetViewportSize((uint32_t)m_SecondViewportPanelSize.x, (uint32_t)m_SecondViewportPanelSize.y);
			}

			if (m_SecondEditorCamera->IsDirty())
			{
				m_SecondEditorCamera->SetViewportSize((uint32_t)m_SecondViewportPanelSize.x, (uint32_t)m_SecondViewportPanelSize.y);
			}
		}
	}

	void EditorLayer::UITransformationModeToolbar()
	{
		const UI::ScopedStyle disableSpacing(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		const UI::ScopedStyle disableWindowBorder(ImGuiStyleVar_WindowBorderSize, 0.0f);
		const UI::ScopedStyle windowRounding(ImGuiStyleVar_WindowRounding, 4.0f);
		const UI::ScopedStyle disablePadding(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		const UI::ScopedColor buttonBackground(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

		const ImVec4 normalColor = { 1.0f, 1.0f, 1.0f, 0.0f };
		const ImVec4 bgColor = { 0.7f, 0.7f, 0.7f, 1.0f };
		const ImVec4 tintColor = { 1.0f, 1.0f, 1.0f, 1.0f };

		const float buttonSize = 18.0f;
		const float edgeOffset = 4.0f;
		const float windowHeight = 32.0f; // annoying limitation of ImGui, window can't be smaller than 32 pixels
		const float numberOfButtons = 2.0f;
		const float backgroundWidth = edgeOffset * 6.0f + buttonSize * numberOfButtons + edgeOffset * (numberOfButtons - 1.0f) * 2.0f;
		const ImVec2 textureSize = { buttonSize, buttonSize };

		Gui::SetNextWindowPos(ImVec2(m_ViewportBounds.MinBound.x + 14, m_ViewportBounds.MinBound.y + edgeOffset));
		Gui::SetNextWindowSize(ImVec2(backgroundWidth, windowHeight));
		Gui::SetNextWindowBgAlpha(0.0f);
		Gui::Begin("Transformation Toolbar", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking);

		const float desiredHeight = 26.0f;
		ImRect background = UI::RectExpanded(Gui::GetCurrentWindow()->Rect(), 0.0f, -(windowHeight - desiredHeight) / 2.0f);
		Gui::GetWindowDrawList()->AddRectFilled(background.Min, background.Max, IM_COL32(15, 15, 15, 127), 4.0f);

		Gui::BeginVertical("##viewportTransformationModeToolbarV", { backgroundWidth, Gui::GetContentRegionAvail().y });
		Gui::Spring();
		Gui::BeginHorizontal("##viewportTransformationModeToolbarH", { backgroundWidth, Gui::GetContentRegionAvail().y });
		Gui::Spring();
		
		uint32_t mode = m_TransformationMode;
		SharedReference<Texture2D> transformationModeIcon = nullptr;
		std::string tooltip = "";
		if (mode == (uint32_t)ImGuizmo::MODE::LOCAL)
		{
			transformationModeIcon = EditorResources::WorldModeIcon;
			tooltip = "World";
		}
		else if (mode == (uint32_t)ImGuizmo::MODE::WORLD)
		{
			transformationModeIcon = EditorResources::LocalModeIcon;
			tooltip = "Local";
		}

		if (transformationModeIcon)
		{
			if (UI::ImageButtonEx(transformationModeIcon, textureSize, normalColor, tintColor))
			{
				m_TransformationMode++;
				if (m_TransformationMode == 2) // invalid
					m_TransformationMode = 0;
			}
			UI::SetTooltip(tooltip.c_str());
		}

		const std::string label = m_EditorCamera->IsPerspective() ? "2D" : "3D";
		if (Gui::Button(label.c_str(), textureSize))
		{
			switch (m_EditorCamera->GetProjectionType())
			{
				case Camera::ProjectionType::Perspective:  m_EditorCamera->SetProjectionType(Camera::ProjectionType::Orthographic); break;
				case Camera::ProjectionType::Orthographic: m_EditorCamera->SetProjectionType(Camera::ProjectionType::Perspective); break;
			}

			Gui::SetWindowFocus("Scene");
		}

		Gui::Spring();
		Gui::EndHorizontal();
		Gui::Spring();
		Gui::EndVertical();

		Gui::End();
	}

	void EditorLayer::UIGizmosToolbar()
	{
		const UI::ScopedStyle disableSpacing(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		const UI::ScopedStyle disableWindowBorder(ImGuiStyleVar_WindowBorderSize, 0.0f);
		const UI::ScopedStyle windowRounding(ImGuiStyleVar_WindowRounding, 4.0f);
		const UI::ScopedStyle disablePadding(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		const UI::ScopedColor buttonBackground(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

		const ImVec4 normalColor = { 1.0f, 1.0f, 1.0f, 0.0f };
		const ImVec4 bgColor = { 0.7f, 0.7f, 0.7f, 1.0f };
		const ImVec4 tintColor = { 1.0f, 1.0f, 1.0f, 1.0f };

		const float buttonSize = 18.0f;
		const float edgeOffset = 4.0f;
		const float windowHeight = 32.0f; // annoying limitation of ImGui, window can't be smaller than 32 pixels
		const float numberOfButtons = 4.0f;
		const float backgroundWidth = edgeOffset * 6.0f + buttonSize * numberOfButtons + edgeOffset * (numberOfButtons - 1.0f) * 2.0f;
		const ImVec2 textureSize = { buttonSize, buttonSize };

		Gui::SetNextWindowPos(ImVec2(m_ViewportBounds.MinBound.x + 150, m_ViewportBounds.MinBound.y + edgeOffset));
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
		{
			OnSelectGizmoToolSelected();
		}
		UI::SetTooltip("Select Tool");

		if (UI::ImageButtonEx(EditorResources::TranslateToolIcon, textureSize, m_GizmoType == 0 ? bgColor : normalColor, tintColor))
		{
			OnTranslationGizmoToolSelected();
		}
		UI::SetTooltip("Translate Tool");

		if (UI::ImageButtonEx(EditorResources::RotateToolIcon, textureSize, m_GizmoType == 1 ? bgColor : normalColor, tintColor))
		{
			OnRotationGizmoToolSelected();
		}
		UI::SetTooltip("Rotate Tool");

		if (UI::ImageButtonEx(EditorResources::ScaleToolIcon, textureSize, m_GizmoType == 2 ? bgColor : normalColor, tintColor))
		{
			OnScaleToolGizmoSelected();
		}
		UI::SetTooltip("Scale Tool");

		Gui::Spring();
		Gui::EndHorizontal();
		Gui::Spring();
		Gui::EndVertical();

		Gui::End();
	}

	void EditorLayer::UICentralToolbar()
	{
		SharedReference<Project> project = Project::GetActive();
		const ProjectProperties& properties = project->GetProperties();

		const UI::ScopedStyle disableSpacing(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		const UI::ScopedStyle disableWindowBorder(ImGuiStyleVar_WindowBorderSize, 0.0f);
		const UI::ScopedStyle windowRounding(ImGuiStyleVar_WindowRounding, 4.0f);
		const UI::ScopedStyle disablePadding(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		const UI::ScopedColor buttonBackground(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

		const ImVec4 normalColor = { 1.0f, 1.0f, 1.0f, 0.0f };
		const ImVec4 bgColor = { 0.7f, 0.7f, 0.7f, 1.0f };
		const ImVec4 tintColor = { 1.0f, 1.0f, 1.0f, 1.0f };

		const float buttonSize = 18.0f;
		const float edgeOffset = 4.0f;
		const float windowHeight = 32.0f; // annoying limitation of ImGui, window can't be smaller than 32 pixels
		const float numberOfButtons = 3.0f;
		const float backgroundWidth = edgeOffset * 6.0f + buttonSize * numberOfButtons + edgeOffset * (numberOfButtons - 1.0f) * 2.0f;
		const ImVec2 textureSize = { buttonSize, buttonSize };

		const float toolbarX = (m_ViewportBounds.MinBound.x + m_ViewportBounds.MaxBound.x) / 2.0f;
		Gui::SetNextWindowPos(ImVec2(toolbarX - (backgroundWidth / 2.0f), m_ViewportBounds.MinBound.y + edgeOffset));
		Gui::SetNextWindowSize(ImVec2(backgroundWidth, windowHeight));
		Gui::SetNextWindowBgAlpha(0.0f);
		Gui::Begin("Central Toolbar", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking);

		const const float desiredHeight = 26.0f;
		const ImRect background = UI::RectExpanded(Gui::GetCurrentWindow()->Rect(), 0.0f, -(windowHeight - desiredHeight) / 2.0f);
		Gui::GetWindowDrawList()->AddRectFilled(background.Min, background.Max, IM_COL32(15, 15, 15, 127), 4.0f);

		Gui::BeginVertical("##viewportCentralToolbarV", { backgroundWidth, Gui::GetContentRegionAvail().y });
		Gui::Spring();
		Gui::BeginHorizontal("##viewportCentralToolbarH", { backgroundWidth, Gui::GetContentRegionAvail().y });
		Gui::Spring();

		const bool hasPlayButton = !InSimulateSceneState();
		const bool hasSimulateButton = !InPlaySceneState();
		const bool hasPauseButton = !InEditSceneState();
		const bool scenePaused = m_ActiveScene->IsPaused();

		if (hasPlayButton)
		{
			SharedReference<Texture2D> icon = (hasSimulateButton) ? EditorResources::PlayIcon : EditorResources::StopIcon;
			if (UI::ImageButtonEx(icon, textureSize, normalColor, tintColor))
			{
				if (hasSimulateButton)
				{
					OnScenePlay();
				}
				else
				{
					OnSceneStop();
				}

				Gui::SetWindowFocus("Scene");
			}

			UI::SetTooltip(hasSimulateButton ? "Play" : "Stop");
		}

		if (hasSimulateButton)
		{
			SharedReference<Texture2D> icon = (hasPlayButton) ? EditorResources::SimulateIcon : EditorResources::StopIcon;
			if (UI::ImageButtonEx(icon, textureSize, normalColor, tintColor))
			{
				if (hasPlayButton)
				{
					OnSceneSimulate();
				}
				else
				{
					OnSceneStop();
				}

				Gui::SetWindowFocus("Scene");
			}

			UI::SetTooltip(hasPlayButton ? "Simulate Physics" : "Stop");
		}

		if (hasPauseButton)
		{
			SharedReference<Texture2D> icon = scenePaused ? EditorResources::PlayIcon : EditorResources::PauseIcon;
			if (UI::ImageButtonEx(icon, textureSize, normalColor, tintColor))
			{
				const bool isPaused = !scenePaused;

				if (isPaused)
				{
					OnScenePause();
				}
				else
				{
					OnSceneResume();
				}

				Gui::SetWindowFocus("Scene");
			}

			UI::SetTooltip(scenePaused ? "Resume Scene" : "Pause Scene");

			if (scenePaused)
			{
				SharedReference<Texture2D> icon = EditorResources::StepIcon;
				if (UI::ImageButtonEx(icon, textureSize, normalColor, tintColor))
				{
					m_ActiveScene->Step(properties.EditorProps.FrameStepCount);
					Gui::SetWindowFocus("Scene");
				}

				UI::SetTooltip("Next Frame");
			}
		}

		Gui::Spring();
		Gui::EndHorizontal();
		Gui::Spring();
		Gui::EndVertical();

		Gui::End();
	}

	void EditorLayer::UIViewportSettingsToolbar()
	{
		UI::PushID();

		SharedReference<Project> project = Project::GetActive();
		ProjectProperties& properties = project->GetProperties();

		const UI::ScopedStyle disableSpacing(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		const UI::ScopedStyle disableWindowBorder(ImGuiStyleVar_WindowBorderSize, 0.0f);
		const UI::ScopedStyle windowRounding(ImGuiStyleVar_WindowRounding, 4.0f);
		const UI::ScopedStyle disablePadding(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

		const float buttonSize = 18.0f;
		const float edgeOffset = 4.0f;
		const float windowHeight = 32.0f; // annoying limitation of ImGui, window can't be smaller than 32 pixels
		const float numberOfButtons = 1.0f;
		const float backgroundWidth = edgeOffset * 6.0f + buttonSize * numberOfButtons + edgeOffset * (numberOfButtons - 1.0f) * 2.0f;

		const ImVec2 textureSize = { buttonSize, buttonSize };

		const ImVec4 normalColor = { 1.0f, 1.0f, 1.0f, 0.0f };
		const ImVec4 bgColor = { 0.7f, 0.7f, 0.7f, 1.0f };
		const ImVec4 tintColor = { 1.0f, 1.0f, 1.0f, 1.0f };

		Gui::SetNextWindowPos(ImVec2(m_ViewportBounds.MaxBound.x - backgroundWidth - 14, m_ViewportBounds.MinBound.y + edgeOffset));
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
			{
				openSettingsPopup = true;
			}
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
			{
				Gui::OpenPopup("ViewportSettingsPanel");
			}

			auto drawHeading = [](auto label) {
				UI::PushFont("Bold");
				Gui::Text(label);
				UI::PopFont();
				UI::Draw::Underline();
			};

			const float columnWidth = 165.0f;
			Gui::SetNextWindowSize({ popupWidth, 375.0f });
			Gui::SetNextWindowPos({ (m_ViewportBounds.MaxBound.x - popupWidth) - 17, m_ViewportBounds.MinBound.y + edgeOffset + windowHeight });
			const ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar;
			if (Gui::BeginPopup("ViewportSettingsPanel", windowFlags))
			{
				UI::PushFont("Huge");
				Gui::Text("Viewport Settings");
				UI::PopFont();
				UI::Draw::Underline();

				UI::ShiftCursorY(10.0f);

				{
					drawHeading("Tools");
					UI::BeginPropertyGrid(columnWidth);

					SharedReference<PerformancePanel> performancePanel = m_PanelManager->GetPanel<PerformancePanel>();
					UI::Property("Show Stats", performancePanel->IsOpen);

					UI::EndPropertyGrid();
				}

				{
					drawHeading("Display");
					UI::BeginPropertyGrid(columnWidth);

					if (UI::ImageButton("Maximize On Play", EditorResources::MaximizeOnPlayIcon, textureSize, properties.EditorProps.MaximizeOnPlay ? bgColor : normalColor, tintColor))
					{
						properties.EditorProps.MaximizeOnPlay = !properties.EditorProps.MaximizeOnPlay;
					}

					if (UI::ImageButton(properties.EditorProps.DrawEditorGrid ? "Hide Grid" : "Show Grid", EditorResources::ShowGridIcon, textureSize, properties.EditorProps.DrawEditorGrid ? normalColor : bgColor, tintColor))
					{
						ToggleGrid();
					}

					if (UI::ImageButton(properties.PhysicsProps.ShowColliders ? "Hide Colliders" : "Show Colliders", EditorResources::PhysicsCollidersIcon, textureSize, properties.PhysicsProps.ShowColliders ? bgColor : normalColor, tintColor))
					{
						properties.PhysicsProps.ShowColliders = !properties.PhysicsProps.ShowColliders;
					}

					if (UI::ImageButton(properties.EditorProps.ShowBoundingBoxes ? "Hide Bounding Boxes" : "Show Bounding Boxes", EditorResources::BoundingBoxesIcon, textureSize, properties.EditorProps.ShowBoundingBoxes ? bgColor : normalColor, tintColor))
					{
						properties.EditorProps.ShowBoundingBoxes = !properties.EditorProps.ShowBoundingBoxes;
					}

					static const char* selectionModes[] = { "Actor", "Submesh" };
					uint32_t currentSelectionMode = (uint32_t)m_SelectionMode;
					if (UI::PropertyDropdown("Selection Mode", selectionModes, VX_ARRAYSIZE(selectionModes), currentSelectionMode))
					{
						m_SelectionMode = (SelectionMode)currentSelectionMode;
					}

					UI::Property("Selected Actor Outline", m_ShowSelectedActorOutline);

					if (UI::ImageButton(properties.EditorProps.MuteAudioSources ? "Unmute Audio" : "Mute Audio", EditorResources::MuteAudioSourcesIcons, textureSize, properties.EditorProps.MuteAudioSources ? bgColor : normalColor, tintColor))
					{
						properties.EditorProps.MuteAudioSources = !properties.EditorProps.MuteAudioSources;
					}

					UI::EndPropertyGrid();
				}

				{
					drawHeading("Gizmos");
					UI::BeginPropertyGrid(columnWidth);

					uint32_t mode = m_TransformationMode;
					SharedReference<Texture2D> transformationModeIcon = nullptr;
					std::string tooltip = "";
					if (mode == (uint32_t)ImGuizmo::MODE::LOCAL)
					{
						transformationModeIcon = EditorResources::WorldModeIcon;
						tooltip = "World";
					}
					else if (mode == (uint32_t)ImGuizmo::MODE::WORLD)
					{
						transformationModeIcon = EditorResources::LocalModeIcon;
						tooltip = "Local";
					}

					if (transformationModeIcon)
					{
						if (UI::ImageButton("Transformation Mode", transformationModeIcon, textureSize, normalColor, tintColor))
						{
							m_TransformationMode++;
							if (m_TransformationMode == 2) // invalid
								m_TransformationMode = 0;
						}
						UI::SetTooltip(tooltip.c_str());
					}

					UI::Property("Enable Snapping", properties.GizmoProps.SnapEnabled);
					UI::Property("Snap Value", properties.GizmoProps.SnapValue, 0.05f, FLT_MIN, FLT_MAX);
					UI::Property("Rotation Snap Value", properties.GizmoProps.RotationSnapValue, 1.0f, FLT_MIN, FLT_MAX);

					UI::Property("Gimzo Size", properties.GizmoProps.GizmoSize, 0.05f, FLT_MIN, FLT_MAX);

					if (UI::ImageButton(properties.RendererProps.DisplaySceneIconsInEditor ? "Hide Gizmos" : "Show Gizmos", EditorResources::DisplaySceneIconsIcon, textureSize, properties.RendererProps.DisplaySceneIconsInEditor ? normalColor : bgColor, tintColor))
					{
						properties.RendererProps.DisplaySceneIconsInEditor = !properties.RendererProps.DisplaySceneIconsInEditor;
					}

					UI::EndPropertyGrid();
				}

				{
					drawHeading("Editor Camera");
					UI::BeginPropertyGrid(columnWidth);

					float FOVdegrees = Math::Rad2Deg(m_EditorCamera->m_PerspectiveFOV);
					if (UI::Property("Field of View", FOVdegrees))
					{
						m_EditorCamera->SetPerspectiveFOV(Math::Deg2Rad(FOVdegrees));
						properties.EditorProps.EditorCameraProps.FOVdegrees = FOVdegrees;
					}

					if (UI::Property("Camera Speed", m_EditorCamera->m_Speed, 0.001f, EditorCamera::MIN_SPEED, EditorCamera::MAX_SPEED, "%.4f"))
					{
						properties.EditorProps.EditorCameraProps.Speed = m_EditorCamera->m_Speed;
					}

					UI::EndPropertyGrid();
				}

				Gui::EndPopup();
			}
		}

		Gui::End();

		UI::PopID();
	}

	void EditorLayer::LaunchRuntimeApp()
	{
		const std::string projectPath = Project::GetProjectFilepath().string();
		const std::string runtimeAppPath = m_RuntimeAppFilepath.string();

		Platform::LaunchProcess(runtimeAppPath.c_str(), projectPath.c_str());
	}

	void EditorLayer::OnOverlayRender(EditorCamera* editorCamera)
	{
		VX_PROFILE_FUNCTION();

		SharedReference<Project> project = Project::GetActive();
		const ProjectProperties& properties = project->GetProperties();

		Renderer2D::BeginScene(editorCamera);

		// Render Editor Grid
		if (properties.EditorProps.DrawEditorGrid)
		{
			OverlayRenderGrid(editorCamera, properties.EditorProps.DrawEditorAxes);
		}

		const Math::vec4& colliderColor = properties.PhysicsProps.Physics3DColliderColor;
		const Math::vec4& spriteColliderColor = properties.PhysicsProps.Physics2DColliderColor;
		const Math::vec4 boundingBoxColor = ColorToVec4(Color::Orange);
		const Math::vec4& outlineColor = boundingBoxColor;

		// Render Physics Colliders
		if (properties.PhysicsProps.ShowColliders)
		{
			OverlayRenderMeshColliders(colliderColor);
			OverlayRenderSpriteColliders(editorCamera, spriteColliderColor);
		}

		// Render Bounding Boxes
		if (properties.EditorProps.ShowBoundingBoxes)
		{
			OverlayRenderMeshBoundingBoxes(boundingBoxColor);
			OverlayRenderSpriteBoundingBoxes(boundingBoxColor);
		}

		// Render Visible Mesh Colliders
		{
			std::vector<Actor> entities;

			auto boxColliderView = m_ActiveScene->GetAllActorsWith<BoxColliderComponent>();
			auto sphereColliderView = m_ActiveScene->GetAllActorsWith<SphereColliderComponent>();

			for (const auto e : boxColliderView)
			{
				Actor actor = { e, m_ActiveScene.Raw() };
				if (!actor.IsActive())
					continue;
				const BoxColliderComponent& boxCollider = actor.GetComponent<BoxColliderComponent>();
				if (boxCollider.Visible)
					entities.emplace_back(e, m_ActiveScene.Raw());
			}
			for (const auto e : sphereColliderView)
			{
				Actor actor = { e, m_ActiveScene.Raw() };
				if (!actor.IsActive())
					continue;
				const SphereColliderComponent& sphereCollider = actor.GetComponent<SphereColliderComponent>();
				if (sphereCollider.Visible)
					entities.emplace_back(e, m_ActiveScene.Raw());
			}

			for (Actor actor : entities)
			{
				const Math::mat4 transform = m_ActiveScene->GetWorldSpaceTransformMatrix(actor);
				OverlayRenderMeshCollider(actor, transform, colliderColor);
			}
		}

		// Render Visible 2D Colliders
		{
			std::vector<Actor> entities;

			auto boxColliderView = m_ActiveScene->GetAllActorsWith<BoxCollider2DComponent>();
			auto circleColliderView = m_ActiveScene->GetAllActorsWith<CircleCollider2DComponent>();

			for (const auto e : boxColliderView)
			{
				Actor actor = { e, m_ActiveScene.Raw() };
				if (!actor.IsActive())
					continue;
				const BoxCollider2DComponent& boxCollider = actor.GetComponent<BoxCollider2DComponent>();
				if (boxCollider.Visible)
					entities.emplace_back(e, m_ActiveScene.Raw());
			}
			for (const auto e : circleColliderView)
			{
				Actor actor = { e, m_ActiveScene.Raw() };
				if (!actor.IsActive())
					continue;
				const CircleCollider2DComponent& circleCollider = actor.GetComponent<CircleCollider2DComponent>();
				if (circleCollider.Visible)
					entities.emplace_back(e, m_ActiveScene.Raw());
			}

			for (Actor actor : entities)
			{
				const Math::mat4 transform = m_ActiveScene->GetWorldSpaceTransformMatrix(actor);
				OverlayRenderSpriteCollider(editorCamera, actor, transform, colliderColor);
			}
		}

		// Draw selected actor outline
		if (m_ShowSelectedActorOutline)
		{
			Actor selectedActor = SelectionManager::GetSelectedActor();
			if (selectedActor && selectedActor.IsActive())
			{
				OverlayRenderSelectedActorOutline(outlineColor);
			}
		}

		Renderer2D::EndScene();
	}

	void EditorLayer::OverlayRenderSelectedActorOutline(const Math::vec4& outlineColor)
	{
		Actor selectedActor = SelectionManager::GetSelectedActor();

		const Math::mat4 transform = m_ActiveScene->GetWorldSpaceTransformMatrix(selectedActor);

		if (selectedActor.HasAny<MeshRendererComponent, StaticMeshRendererComponent>())
		{
			const Math::mat4 scaledTransform = transform * Math::Scale(Math::vec3(1.001f));
			OverlayRenderMeshOutline(selectedActor, scaledTransform, outlineColor);
		}

		if (selectedActor.HasAny<SpriteRendererComponent, CircleRendererComponent>())
		{
			const Math::mat4 scaledTransform = transform * Math::Scale(Math::vec3(1.001f));
			OverlayRenderSpriteOutline(selectedActor, scaledTransform, outlineColor);
		}

		if (selectedActor.HasComponent<TextMeshComponent>())
		{
			const TextMeshComponent& textMeshComponent = selectedActor.GetComponent<TextMeshComponent>();

			if (textMeshComponent.Visible)
			{
				// TODO calculate the text size and scale transform
				Renderer2D::DrawRect(transform, outlineColor);
			}
		}

		if (selectedActor.HasComponent<CameraComponent>())
		{
			const SceneCamera& sceneCamera = selectedActor.GetComponent<CameraComponent>().Camera;

			switch (sceneCamera.GetProjectionType())
			{
				case Camera::ProjectionType::Perspective:
				{
					const Camera* camera = &sceneCamera;
					const Math::mat4 view = Math::Inverse(transform);
					std::vector<Math::vec4> corners = GetCameraFrustumCornersWorldSpace(camera, view);

					const Math::vec4 frustumColor = ColorToVec4(Color::LightBlue);

					Renderer::DrawFrustum(corners, frustumColor);
					break;
				}
				case Camera::ProjectionType::Orthographic:
				{
					const Math::mat4 scaled = transform * Math::Scale({ sceneCamera.GetOrthographicSize() * 1.6f, sceneCamera.GetOrthographicSize() * 0.9f, 1.0f });
					Renderer2D::DrawRect(scaled, outlineColor);
					break;
				}
			}
		}

		if (selectedActor.HasComponent<LightSourceComponent>())
		{
			const LightSourceComponent& lightSourceComponent = selectedActor.GetComponent<LightSourceComponent>();

			if (lightSourceComponent.Visible)
			{
				const TransformComponent& worldSpaceTransform = m_ActiveScene->GetWorldSpaceTransform(selectedActor);
				const Math::vec3 translation = worldSpaceTransform.Translation;
				const Math::vec4 color = { lightSourceComponent.Radiance, 1.0f };

				switch (lightSourceComponent.Type)
				{
					case LightType::Directional:
					{
						const Math::vec3 midpoint = Math::Midpoint(translation, Math::vec3(0.0f));

						const Math::quaternion rotation = worldSpaceTransform.GetRotation();

						const Math::vec3 left = Math::Rotate(rotation, Math::vec3(-1.0f, 0.0f, 0.0f)) * 0.5f;
						const Math::vec3 right = Math::Rotate(rotation, Math::vec3(1.0f, 0.0f, 0.0f)) * 0.5f;
						const Math::vec3 up = Math::Rotate(rotation, Math::vec3(0.0f, 1.0f, 0.0f)) * 0.5f;
						const Math::vec3 down = Math::Rotate(rotation, Math::vec3(0.0f, -1.0f, 0.0f)) * 0.5f;

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

	void EditorLayer::OnCreateScriptPopupRender()
	{
		const std::string popupName = "Create Script";
		if (m_CreateScriptPopupOpen)
		{
			Gui::OpenPopup(popupName.c_str());
		}

		if (UI::ShowMessageBox(popupName.c_str(), &m_CreateScriptPopupOpen, { 500, 220 }))
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
			UI::PropertyDropdown("Language", scriptingLanguages, VX_ARRAYSIZE(scriptingLanguages), currentScriptingLanguage);

			UI::EndPropertyGrid();

			UI::ShiftCursorY(20.0f);
			UI::ShiftCursorX(7.0f);

			auto resetPopup = [&]() {
				currentScriptingLanguage = ScriptingLanguage::CSharp;
				className = "Untitiled";
				m_CreateScriptPopupOpen = false;
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
		const std::string popupName = "Import Mesh";
		if (m_MeshImportPopupOpen)
		{
			Gui::OpenPopup(popupName.c_str());
		}

		if (UI::ShowMessageBox(popupName.c_str(), &m_MeshImportPopupOpen, { 500, 285 }))
		{
			UI::Draw::Underline();
			Gui::Spacing();

			ImVec2 buttonSize(Gui::GetFontSize() * 13.27f, 0.0f);

			Gui::TextCentered("A mesh asset must be generated from this mesh file. (i.e. .fbx)", 40.0f);
			Gui::TextCentered("Import options can be selected below", 60.0f);

			Gui::Spacing();
			UI::Draw::Underline();

			UI::ShiftCursorX(-15.0f);

			UI::DrawVec3Controls("Translation", m_MeshImportPopupData.ModelImportOptions.MeshTransformation.Translation);
			Math::vec3 rotationEuler = m_MeshImportPopupData.ModelImportOptions.MeshTransformation.GetRotationEuler();
			UI::DrawVec3Controls("Rotation", rotationEuler, 0.0f, 100.0f, 0.0f, 0.0f, [&]()
			{
				m_MeshImportPopupData.ModelImportOptions.MeshTransformation.SetRotationEuler(rotationEuler);
			});
			UI::DrawVec3Controls("Scale", m_MeshImportPopupData.ModelImportOptions.MeshTransformation.Scale, 1.0f, 100.0f, FLT_MIN);

			UI::ShiftCursorY(20.0f);

			UI::BeginPropertyGrid();

			std::string assetDir = Project::GetAssetDirectory().string();
			size_t assetDirPos = m_MeshImportPopupData.MeshFilepath.find(assetDir);
			std::string filepath = m_MeshImportPopupData.MeshFilepath;
			if (assetDirPos != std::string::npos)
				filepath = m_MeshImportPopupData.MeshFilepath.substr(assetDirPos + assetDir.size() + 1);
			UI::Property("Filepath", filepath, true);

			UI::EndPropertyGrid();

			UI::ShiftCursorY(10.0f);
			UI::ShiftCursorX(7.0f);

			auto resetPopup = [&]() {
				m_MeshImportPopupOpen = false;
				m_MeshImportPopupData.MeshFilepath = "";
				m_MeshImportPopupData.ModelImportOptions = MeshImportOptions();
			};

			if (Gui::Button("Import", buttonSize))
			{
				if (m_MeshImportPopupData.MeshActorToEdit.HasComponent<MeshRendererComponent>())
				{
					MeshRendererComponent& meshRenderer = m_MeshImportPopupData.MeshActorToEdit.GetComponent<MeshRendererComponent>();
					meshRenderer.Mesh = Project::GetEditorAssetManager()->GetAssetHandleFromFilepath(m_MeshImportPopupData.MeshFilepath);
				}
				else if (m_MeshImportPopupData.MeshActorToEdit.HasComponent<StaticMeshRendererComponent>())
				{
					StaticMeshRendererComponent& staticMeshRenderer = m_MeshImportPopupData.MeshActorToEdit.GetComponent<StaticMeshRendererComponent>();
					staticMeshRenderer.StaticMesh = Project::GetEditorAssetManager()->GetAssetHandleFromFilepath(m_MeshImportPopupData.MeshFilepath);

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

	void EditorLayer::OverlayRenderMeshBoundingBox(Actor actor, const Math::mat4& transform, const Math::vec4& boundingBoxColor)
	{
		if (actor.HasComponent<MeshRendererComponent>())
		{
			const MeshRendererComponent& meshRendererComponent = actor.GetComponent<MeshRendererComponent>();
			const Math::mat4& scaledTransform = transform * Math::Scale(Math::vec3(1.001f));

			if (!meshRendererComponent.Visible)
				return;

			AssetHandle meshHandle = meshRendererComponent.Mesh;
			if (!AssetManager::IsHandleValid(meshHandle))
				return;

			SharedReference<Mesh> mesh = AssetManager::GetAsset<Mesh>(meshHandle);
			if (!mesh)
				return;

			const Submesh& submesh = mesh->GetSubmesh();

			Renderer2D::DrawAABB(submesh.GetBoundingBox(), scaledTransform, boundingBoxColor);
		}

		if (actor.HasComponent<StaticMeshRendererComponent>())
		{
			const StaticMeshRendererComponent& staticMeshRendererComponent = actor.GetComponent<StaticMeshRendererComponent>();
			const Math::mat4& transform = m_ActiveScene->GetWorldSpaceTransformMatrix(actor) * Math::Scale(Math::vec3(1.001f));

			if (!staticMeshRendererComponent.Visible)
				return;

			AssetHandle staticMeshHandle = staticMeshRendererComponent.StaticMesh;
			if (!AssetManager::IsHandleValid(staticMeshHandle))
				return;

			SharedReference<StaticMesh> staticMesh = AssetManager::GetAsset<StaticMesh>(staticMeshHandle);
			if (!staticMesh)
				return;

			const std::unordered_map<uint32_t, StaticSubmesh>& submeshes = staticMesh->GetSubmeshes();

			for (const auto& [submeshIndex, submesh] : submeshes)
			{
				Renderer2D::DrawAABB(submesh.GetBoundingBox(), transform, boundingBoxColor);
			}
		}
	}

	void EditorLayer::OverlayRenderMeshBoundingBoxes(const Math::vec4& boundingBoxColor)
	{
		std::vector<Actor> actors;

		auto meshRendererView = m_ActiveScene->GetAllActorsWith<MeshRendererComponent>();
		auto staticMeshRendererView = m_ActiveScene->GetAllActorsWith<StaticMeshRendererComponent>();

		for (const auto e : meshRendererView)
			actors.emplace_back(e, m_ActiveScene.Raw());
		for (const auto e : staticMeshRendererView)
			actors.emplace_back(e, m_ActiveScene.Raw());

		for (Actor actor : actors)
		{
			if (!actor.IsActive())
				continue;

			const Math::mat4 transform = m_ActiveScene->GetWorldSpaceTransformMatrix(actor);
			OverlayRenderMeshBoundingBox(actor, transform, boundingBoxColor);
		}
	}

	void EditorLayer::OverlayRenderMeshCollider(Actor actor, const Math::mat4& transform, const Math::vec4& colliderColor)
	{
		if (actor.HasComponent<BoxColliderComponent>())
		{
			const BoxColliderComponent& bc = actor.GetComponent<BoxColliderComponent>();

			const Math::AABB aabb = {
				-Math::vec3(0.503f),
				+Math::vec3(0.503f)
			};

			Math::mat4 transform = m_ActiveScene->GetWorldSpaceTransformMatrix(actor)
				* Math::Translate(bc.Offset)
				* Math::Scale(bc.HalfSize * 2.0f);

			Renderer2D::DrawAABB(aabb, transform, colliderColor);
		}

		if (actor.HasComponent<SphereColliderComponent>())
		{
			const SphereColliderComponent& sc = actor.GetComponent<SphereColliderComponent>();
			const TransformComponent transform = m_ActiveScene->GetWorldSpaceTransform(actor);
			const Math::vec3 translation = transform.Translation + sc.Offset;
			const Math::vec3 scale = transform.Scale;

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
		std::vector<Actor> entities;

		auto boxColliderView = m_ActiveScene->GetAllActorsWith<BoxColliderComponent>();
		auto sphereColliderView = m_ActiveScene->GetAllActorsWith<SphereColliderComponent>();

		for (const auto e : boxColliderView)
			entities.emplace_back(e, m_ActiveScene.Raw());
		for (const auto e : sphereColliderView)
			entities.emplace_back(e, m_ActiveScene.Raw());

		for (Actor actor : entities)
		{
			const Math::mat4 transform = m_ActiveScene->GetWorldSpaceTransformMatrix(actor);
			OverlayRenderMeshCollider(actor, transform, colliderColor);
		}
	}

	void EditorLayer::OverlayRenderMeshOutline(Actor actor, const Math::mat4& transform, const Math::vec4& outlineColor)
	{
		if (!actor.IsActive())
			return;

		if (actor.HasComponent<MeshRendererComponent>())
		{
			const MeshRendererComponent& meshRendererComponent = actor.GetComponent<MeshRendererComponent>();
			if (!meshRendererComponent.Visible)
				return;

			AssetHandle meshHandle = meshRendererComponent.Mesh;
			if (AssetManager::IsHandleValid(meshHandle))
				return;

			SharedReference<Mesh> mesh = AssetManager::GetAsset<Mesh>(meshHandle);
			if (!mesh)
				return;

			switch (m_SelectionMode)
			{
				case SelectionMode::Actor:
				{
					Renderer2D::DrawAABB(mesh->GetBoundingBox(), transform, outlineColor);
					break;
				}
				case SelectionMode::Submesh:
				{
					const Submesh& submesh = mesh->GetSubmesh();
					Renderer2D::DrawAABB(submesh.GetBoundingBox(), transform, outlineColor);
					break;
				}
			}
		}

		if (actor.HasComponent<StaticMeshRendererComponent>())
		{
			const StaticMeshRendererComponent& staticMeshRendererComponent = actor.GetComponent<StaticMeshRendererComponent>();
			if (!staticMeshRendererComponent.Visible)
				return;

			AssetHandle staticMeshHandle = staticMeshRendererComponent.StaticMesh;
			if (!AssetManager::IsHandleValid(staticMeshHandle))
				return;

			SharedReference<StaticMesh> staticMesh = AssetManager::GetAsset<StaticMesh>(staticMeshHandle);
			if (!staticMesh)
				return;

			switch (m_SelectionMode)
			{
				case SelectionMode::Actor:
				{
					Renderer2D::DrawAABB(staticMesh->GetBoundingBox(), transform, outlineColor);
					break;
				}
				case SelectionMode::Submesh:
				{
					const std::unordered_map<uint32_t, StaticSubmesh>& submeshes = staticMesh->GetSubmeshes();
					for (const auto& [submeshIndex, submesh] : submeshes)
					{
						Renderer2D::DrawAABB(submesh.GetBoundingBox(), transform, outlineColor);
					}
					break;
				}
			}
		}
	}

	void EditorLayer::OverlayRenderSpriteCollider(EditorCamera* editorCamera, Actor actor, const Math::mat4& transform, const Math::vec4& colliderColor)
	{
		if (!actor)
			return;

		float colliderDistance = 0.005f; // Editor camera will be looking at the origin of the world on the first frame
		if (editorCamera->GetPosition().z < 0) // Show colliders on the side that the editor camera facing
			colliderDistance = -colliderDistance;

		if (actor.HasComponent<BoxCollider2DComponent>())
		{
			const BoxCollider2DComponent& bc2d = actor.GetComponent<BoxCollider2DComponent>();

			Math::vec3 scale = Math::vec3(bc2d.Size * 2.0f, 1.0f);

			Math::mat4 transform = m_ActiveScene->GetWorldSpaceTransformMatrix(actor)
				* Math::Translate(Math::vec3(bc2d.Offset, colliderDistance))
				* Math::Scale(scale);

			Renderer2D::DrawRect(transform, colliderColor);
		}

		if (actor.HasComponent<CircleCollider2DComponent>())
		{
			const CircleCollider2DComponent& cc2d = actor.GetComponent<CircleCollider2DComponent>();

			Math::vec3 scale = Math::vec3(cc2d.Radius * 2.0f);

			Math::mat4 transform = m_ActiveScene->GetWorldSpaceTransformMatrix(actor)
				* Math::Translate(Math::vec3(cc2d.Offset, colliderDistance))
				* Math::Scale(scale);

			Renderer2D::DrawCircle(transform, colliderColor, Renderer2D::GetLineWidth() / 100.0f);
		}
	}

	void EditorLayer::OverlayRenderSpriteColliders(EditorCamera* editorCamera, const Math::vec4& colliderColor)
	{
		std::vector<Actor> entities;

		auto spriteRendererView = m_ActiveScene->GetAllActorsWith<SpriteRendererComponent>();
		auto circleRendererView = m_ActiveScene->GetAllActorsWith<CircleRendererComponent>();

		for (const auto e : spriteRendererView)
			entities.emplace_back(e, m_ActiveScene.Raw());
		for (const auto e : circleRendererView)
			entities.emplace_back(e, m_ActiveScene.Raw());

		for (Actor actor : entities)
		{
			const Math::mat4 transform = m_ActiveScene->GetWorldSpaceTransformMatrix(actor);
			OverlayRenderSpriteCollider(editorCamera, actor, transform, colliderColor);
		}
	}

	void EditorLayer::OverlayRenderSpriteBoundingBoxes(const Math::vec4& boundingBoxColor)
	{
		std::vector<Actor> entities;

		auto spriteRendererView = m_ActiveScene->GetAllActorsWith<SpriteRendererComponent>();
		auto circleRendererView = m_ActiveScene->GetAllActorsWith<CircleRendererComponent>();

		for (const auto e : spriteRendererView)
			entities.emplace_back(e, m_ActiveScene.Raw());
		for (const auto e : circleRendererView)
			entities.emplace_back(e, m_ActiveScene.Raw());

		for (Actor actor : entities)
		{
			const Math::mat4 transform = m_ActiveScene->GetWorldSpaceTransformMatrix(actor);
			OverlayRenderSpriteOutline(actor, transform, boundingBoxColor);
		}
	}

	void EditorLayer::OverlayRenderSpriteOutline(Actor actor, const Math::mat4& transform, const Math::vec4& outlineColor)
	{
		if (actor.HasComponent<SpriteRendererComponent>())
		{
			if (!actor.IsActive())
				return;

			const SpriteRendererComponent& spriteRenderer = actor.GetComponent<SpriteRendererComponent>();
			if (!spriteRenderer.Visible)
				return;

			Renderer2D::DrawRect(transform, outlineColor);
		}

		if (actor.HasComponent<CircleRendererComponent>())
		{
			if (!actor.IsActive())
				return;

			const CircleRendererComponent& circleRenderer = actor.GetComponent<CircleRendererComponent>();
			if (!circleRenderer.Visible)
				return;

			Math::mat4 scaledTransform = transform * Math::Scale(Math::vec3(0.505f));

			Renderer2D::DrawCircle(scaledTransform, outlineColor);
		}
	}

	void EditorLayer::OverlayRenderGrid(EditorCamera* editorCamera, bool drawAxes)
	{
		const float axisLineLength = 1'000.0f;
		const float gridLineLength = 750.0f;
		const int32_t gridWidth = 750;
		const int32_t gridDepth = 750;
		const int32_t gridHeight = 750;
		const float originalLineWidth = Renderer2D::GetLineWidth();
		const float axisLineWidth = 5.0f;

		const Math::vec4 xAxisColor = ColorToVec4(Color::Red);
		const Math::vec4 yAxisColor = ColorToVec4(Color::Green);
		const Math::vec4 zAxisColor = ColorToVec4(Color::Blue);

		const Math::vec4 gridColor = { 0.2f, 0.2f, 0.2f, 1.0f };

		// Render Axes
		if (drawAxes)
		{
			Renderer2D::DrawLine({ -axisLineLength, 0.0f + 0.02f, 0.0f }, { axisLineLength, 0.0f + 0.02f, 0.0f }, xAxisColor);
			Renderer2D::DrawLine({ 0.0f, -axisLineLength + 0.02f, 0.0f }, { 0.0f, axisLineLength + 0.02f, 0.0f }, yAxisColor);
			
			if (editorCamera->IsPerspective())
			{
				Renderer2D::DrawLine({ 0.0f, 0.0f + 0.02f, -axisLineLength }, { 0.0f, 0.0f + 0.02f, axisLineLength }, zAxisColor);
			}

			Renderer2D::SetLineWidth(axisLineWidth);
			Renderer2D::Flush();
			Renderer2D::SetLineWidth(originalLineWidth);
		}

		if (editorCamera->IsOrthographic())
		{
			// X grid lines
			for (int32_t x = -gridWidth; x <= gridWidth; x++)
			{
				// Skip the origin line
				if (drawAxes && x == 0)
					continue;

				Renderer2D::DrawLine({ x, -gridLineLength, 0 }, { x, gridLineLength, 0 }, gridColor);
			}

			// Y grid lines
			for (int32_t y = -gridHeight; y <= gridHeight; y++)
			{
				// Skip the origin line
				if (drawAxes && y == 0)
					continue;

				Renderer2D::DrawLine({ -gridLineLength, y, 0 }, { gridLineLength, y, 0 }, gridColor);
			}
		}
		else if (editorCamera->IsPerspective())
		{
			// X grid lines
			for (int32_t x = -gridWidth; x <= gridWidth; x++)
			{
				// Skip the origin line
				if (drawAxes && x == 0)
					continue;

				Renderer2D::DrawLine({ x, 0, -gridLineLength }, { x, 0, gridLineLength }, gridColor);
			}

			// Z grid lines
			for (int32_t z = -gridDepth; z <= gridDepth; z++)
			{
				// Skip the origin line
				if (drawAxes && z == 0)
					continue;

				Renderer2D::DrawLine({ -gridLineLength, 0, z }, { gridLineLength, 0, z }, gridColor);
			}
		}

		Renderer2D::Flush();
	}

	bool EditorLayer::OnWindowDragDropEvent(WindowDragDropEvent& e)
	{
		for (const Fs::Path& path : e.GetPaths())
		{
			Project::GetEditorAssetManager()->ImportAsset(path);
		}

		return true;
	}

	bool EditorLayer::OnKeyPressedEvent(KeyPressedEvent& e)
	{
		SharedReference<SceneHierarchyPanel> sceneHierarchyPanel = m_PanelManager->GetPanel<SceneHierarchyPanel>();
		if (sceneHierarchyPanel->IsFocusedOnActorName())
		{
			return false;
		}

		if (ImGuizmo::IsUsing())
		{
			return false;
		}

		if (Input::IsMouseButtonDown(MouseButton::Right))
		{
			return false;
		}

		const bool altDown = Input::IsKeyDown(KeyCode::LeftAlt) || Input::IsKeyDown(KeyCode::RightAlt);
		const bool shiftDown = Input::IsKeyDown(KeyCode::LeftShift) || Input::IsKeyDown(KeyCode::RightShift);
		const bool controlDown = Input::IsKeyDown(KeyCode::LeftControl) || Input::IsKeyDown(KeyCode::RightControl);

		Actor selectedActor = SelectionManager::GetSelectedActor();

		switch (e.GetKeyCode())
		{
			// File
			case KeyCode::N:
			{
				if (controlDown && InEditSceneState())
				{
					CreateNewScene();
				}

				break;
			}
			case KeyCode::O:
			{
				if (controlDown && InEditSceneState())
				{
					OpenExistingProject();
				}

				break;
			}
			case KeyCode::S:
			{
				if (controlDown && InEditSceneState())
				{
					if (shiftDown)
					{
						SaveSceneAs();
					}
					else
					{
						SaveScene();
					}
				}

				break;
			}

			case KeyCode::Q:
			{
				OnSelectGizmoToolSelected();

				break;
			}
			case KeyCode::W:
			{
				if (altDown && selectedActor)
				{
					TransformComponent& transformComponent = selectedActor.GetTransform();
					transformComponent.Translation = Math::vec3(0.0f);
				}

				OnTranslationGizmoToolSelected();

				break;
			}
			case KeyCode::E:
			{
				if (altDown && selectedActor)
				{
					TransformComponent& transformComponent = selectedActor.GetTransform();
					const Math::quaternion identity(1.0f, 0.0f, 0.0f, 0.0f);
					transformComponent.SetRotation(identity);
				}

				OnRotationGizmoToolSelected();

				break;
			}
			case KeyCode::R:
			{
				if (altDown && selectedActor)
				{
					TransformComponent& transformComponent = selectedActor.GetTransform();
					transformComponent.Scale = Math::vec3(1.0f);
				}

				OnScaleToolGizmoSelected();

				break;
			}

			case KeyCode::F:
			{
				if (selectedActor)
				{
					const Math::vec3& translation = m_ActiveScene->GetWorldSpaceTransform(selectedActor).Translation;
					const float distance = 10.0f;

					EditorCamera* camera = GetCurrentEditorCamera();
					if (camera == nullptr)
					{
						break;
					}

					camera->Focus(translation);
					camera->SetDistance(distance);
				}

				break;
			}
			case KeyCode::G:
			{
				if (InEditSceneState() || m_SecondViewportHovered)
				{
					ToggleGrid();
				}

				break;
			}

			case KeyCode::A:
			{
				if (controlDown)
				{
					m_ShowViewportCreateActorMenu = true;
				}

				break;
			}
			case KeyCode::B:
			{
				if (controlDown && InEditSceneState())
				{
					if (shiftDown)
					{
						m_PanelManager->TogglePanelOpen<BuildSettingsPanel>();
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
				{
					if (m_SceneViewportHovered && !InEditSceneState())
					{
						break;
					}

					DuplicateSelectedActor();
				}

				break;
			}

			case KeyCode::P:
			{
				if (controlDown && shiftDown && InPlaySceneState())
				{
					RestartScene();

					break;
				}

				if (controlDown)
				{
					if (!InPlaySceneState())
					{
						OnScenePlay();
					}
					else
					{
						OnSceneStop();
					}
				}

				break;
			}
			case KeyCode::X:
			{
				if (controlDown && shiftDown)
				{
					if (InSimulateSceneState())
					{
						RestartSceneSimulation();
					}

					break;
				}

				if (controlDown)
				{
					if (!InSimulateSceneState())
					{
						OnSceneSimulate();
					}
					else
					{
						OnSceneStop();
					}
				}

				break;
			}

			// Tools
			case KeyCode::F2:
			{
				if (selectedActor)
				{
					m_PanelManager->GetPanel<SceneHierarchyPanel>()->FocusOnActorName(true);
				}

				break;
			}
			case KeyCode::F11:
			{
				Window& window = Application::Get().GetWindow();
				window.SetMaximized(!window.IsMaximized());

				break;
			}

			case KeyCode::Delete:
			{
				if (selectedActor)
				{
					m_ActiveScene->SubmitToDestroyActor(selectedActor);
				}

				break;
			}

			case KeyCode::Space:
			{
				if (controlDown)
				{
					ToggleSceneViewportMaximized();
				}

				break;
			}
		}

		return false;
	}

	bool EditorLayer::OnMouseButtonPressedEvent(MouseButtonPressedEvent& e)
	{
		const bool noViewportsHovered = !m_SceneViewportHovered && !m_SecondViewportHovered;
		if (noViewportsHovered)
		{
			return false;
		}

		if (InPlaySceneState() && m_SceneViewportHovered)
		{
			return false;
		}

		if (m_GizmoType != -1 && ImGuizmo::IsOver())
		{
			return false;
		}

		if (e.GetMouseButton() != MouseButton::Left)
		{
			return false;
		}

		const bool altPressed = Input::IsKeyDown(KeyCode::LeftAlt) || Input::IsKeyDown(KeyCode::RightAlt);

		if (altPressed)
		{
			return false;
		}

		// TODO we should handle this in a better way
		if (m_HoveredActor)
		{
			if (m_HoveredActor.HasAny<
				SpriteRendererComponent,
				CircleRendererComponent,
				ParticleEmitterComponent,
				TextMeshComponent,
				ButtonComponent,
				CameraComponent,
				LightSourceComponent,
				AudioSourceComponent>())
			{
				SelectionManager::SetSelectedActor(m_HoveredActor);
				m_PanelManager->GetPanel<SceneHierarchyPanel>()->FocusOnActorName(false);
				return false;
			}
		}

		if (Actor hoveredMesh = GetHoveredMeshActorFromRaycast())
		{
			SelectionManager::SetSelectedActor(hoveredMesh);
			m_PanelManager->GetPanel<SceneHierarchyPanel>()->FocusOnActorName(false);
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
		const std::string filepath = FileDialogue::OpenFileDialog("Vortex Project (*.vxproject)\0*.vxproject\0");
		if (filepath.empty()) {
			return false;
		}

		return OpenProject(filepath);
	}

	bool EditorLayer::OpenProject(const Fs::Path& filepath)
	{
		VX_PROFILE_FUNCTION();

		if (Project::GetActive()) {
			CloseProject();
		}

		m_HoveredActor = Actor{};
		SelectionManager::DeselectActor();

		if (!ProjectLoader::LoadEditorProject(filepath)) {
			return false;
		}

		SharedReference<Project> project = Project::GetActive();
		const ProjectProperties& properties = project->GetProperties();

		const Fs::Path startScenePath = properties.General.StartScene;
		const AssetMetadata& metadata = Project::GetEditorAssetManager()->GetMetadata(startScenePath);
		if (!AssetManager::IsHandleValid(metadata.Handle)) {
			return false;
		}

		OpenScene(metadata);

		m_PanelManager->AddPanel<ProjectSettingsPanel>();
		m_PanelManager->AddPanel<ContentBrowserPanel>(Project::GetAssetDirectory())->IsOpen = true;

		return true;
	}

	void EditorLayer::SaveProject()
	{
		SaveScene();

		if (m_CaptureSceneViewportFramebufferToDiskOnSave) {
			CaptureSceneViewportFramebufferImageToDisk();
		}

		if (!ProjectLoader::SaveActiveEditorProject()) {
			return;
		}
	}

	void EditorLayer::CloseProject()
	{
		if (m_ActiveScene->IsRunning()) {
			OnSceneStop();
		}

		ScriptEngine::Shutdown();
	}

	void EditorLayer::BuildProject()
	{
		SaveProject();

		// TODO build asset pack here
	}

	void EditorLayer::BuildAndRunProject()
	{
		BuildProject();

		LaunchRuntimeApp();
	}

	void EditorLayer::CreateNewScene()
	{
		if (!InEditSceneState()) {
			return;
		}

		m_EditorSceneMetadata = {}; // No scene on disk yet

		m_EditorScene = Scene::Create(m_Framebuffer);
		SetSceneContext(m_EditorScene);

		m_ActiveScene = m_EditorScene;

		ResetEditorCameras();

		// TODO are we going to store the project type in the project?
		ProjectType type = ProjectType::e3D;
		Scene::CreateSampleScene(type, m_ActiveScene);

		SetWindowTitle("UntitledScene");
	}

	void EditorLayer::OpenExistingScene()
	{
		const std::string filepath = FileDialogue::OpenFileDialog("Vortex Scene (*.vortex)\0*.vortex\0");
		if (filepath.empty()) {
			return;
		}

		const AssetMetadata& metadata = Project::GetEditorAssetManager()->GetMetadata(filepath);
		if (!AssetManager::IsHandleValid(metadata.Handle)) {
			VX_CONSOLE_LOG_ERROR("[Editor] Failed to open scene: {}", filepath);
			return;
		}

		OpenScene(metadata);
	}

	void EditorLayer::OpenScene(const AssetMetadata& metadata)
	{
		if (!InEditSceneState()) {
			OnSceneStop();
		}

		if (metadata.Type != AssetType::SceneAsset) {
			VX_CONSOLE_LOG_WARN("[Editor] Failed to open scene, metadata was invalid!");
			return;
		}

		m_HoveredActor = Actor{}; // Prevent an invalid actor from being used elsewhere in the editor
		SelectionManager::DeselectActor();

		const Fs::Path fullPath = Project::GetAssetDirectory() / metadata.Filepath;
		const std::string filename = FileSystem::RemoveFileExtension(fullPath);

		SharedReference<Scene> newScene = Scene::Create(m_Framebuffer);
		SceneSerializer serializer(newScene);

		if (!serializer.Deserialize(fullPath.string())) {
			VX_CONSOLE_LOG_ERROR("[Editor] Failed to deserialize scene - {}", filename);
			return;
		}

		newScene->OnViewportResize((uint32_t)m_ViewportPanelSize.x, (uint32_t)m_ViewportPanelSize.y);

		m_EditorScene = newScene;
		SetSceneContext(m_EditorScene);

		ResetEditorCameras();

		m_ActiveScene = m_EditorScene;
		m_EditorSceneMetadata = metadata;

		SetWindowTitle(filename);

		m_ActiveScene->SetName(filename);
	}

	void EditorLayer::SaveSceneAs()
	{
		std::string filepath = FileDialogue::SaveFileDialog("Vortex Scene (*.vortex)\0*.vortex\0");

		if (!filepath.empty())
		{
			ReplaceSceneFileExtensionIfNeeded(filepath);

			const AssetMetadata& metadata = Project::GetEditorAssetManager()->GetMetadata(filepath);
			if (!AssetManager::IsHandleValid(metadata.Handle)) {
				VX_CONSOLE_LOG_WARN("[Editor] Failed to save scene, metadata was invalid!");
				return;
			}

			m_EditorSceneMetadata = metadata;

			SerializeScene();

			const Fs::Path fullPath = Project::GetAssetDirectory() / metadata.Filepath;
			const std::string filename = FileSystem::RemoveFileExtension(fullPath.filename());
			SetWindowTitle(filename);
		}
	}

	void EditorLayer::SaveScene()
	{
		m_ActiveScene->SortActors();

		if (AssetManager::IsHandleValid(m_EditorSceneMetadata.Handle))
		{
			SerializeScene();
		}
		else
		{
			SaveSceneAs();
		}

		SharedReference<Project> project = Project::GetActive();
		ProjectProperties& properties = project->GetProperties();
		properties.EditorProps.EditorCameraProps.Translation = m_EditorCamera->GetPosition();
	}

	void EditorLayer::SerializeScene()
	{
		VX_CORE_VERIFY(Project::GetActive());
		VX_CORE_VERIFY(InEditSceneState());

		if (!AssetManager::IsHandleValid(m_EditorSceneMetadata.Handle)) {
			VX_CONSOLE_LOG_ERROR("[Editor] Failed to serialize scene, metadata was invalid!");
			return;
		}

		const Fs::Path fullPath = Project::GetAssetDirectory() / m_EditorSceneMetadata.Filepath;

		SceneSerializer serializer(m_ActiveScene);
		serializer.Serialize(fullPath.string());
	}

	void EditorLayer::OnScenePlay()
	{
		if (!InEditSceneState()) {
			OnSceneStop();
		}

		SwitchSceneState(SceneState::Play);

		SharedReference<Project> project = Project::GetActive();
		ProjectProperties properties = project->GetProperties();

		if (properties.ScriptingProps.ReloadAssemblyOnPlay)
		{
			ScriptEngine::ReloadAssembly();
			// NOTE: this doesn't work, the assembly still doesn't get reloaded
			using namespace std::chrono_literals;
			std::this_thread::sleep_for(750ms);
		}

		m_SceneViewportMaximized = properties.EditorProps.MaximizeOnPlay;

		SharedReference<ConsolePanel> consolePanel = m_PanelManager->GetPanel<ConsolePanel>();
		if (consolePanel->ClearOnPlay())
		{
			consolePanel->ClearMessages();
		}

		// Make a copy of the editors scene
		m_ActiveScene = Scene::Copy(m_EditorScene);

		// we should let the user know
		Actor primaryCamera = m_ActiveScene->GetPrimaryCameraActor();
		if (!primaryCamera)
		{
			VX_CONSOLE_LOG_ERROR("[Scene Renderer] Scene cannot render without a primary camera! Attach a camera component to an actor and enable 'Primary'");
		}

		ScriptRegistry::SetSceneStartTime(Time::GetTime());

		m_ActiveScene->OnRuntimeStart(properties.EditorProps.MuteAudioSources);

		SetSceneContext(m_ActiveScene);

		OnSelectGizmoToolSelected();

		m_SceneViewportBorderFadeTimer = m_SceneViewportBorderFadeLengthInSeconds;
		m_SceneViewportBorderFadeColor = m_SceneViewportOnPlayBorderColor;
	}

	void EditorLayer::OnScenePause()
	{
		VX_CORE_ASSERT(m_ActiveScene->IsRunning(), "Scene must be running!");

		if (InEditSceneState()) {
			return;
		}

		if (InPlaySceneState() && m_ActiveScene->IsPaused()) {
			return;
		}

		m_ActiveScene->SetPaused(true);
	}

	void EditorLayer::OnSceneResume()
	{
		VX_CORE_ASSERT(m_ActiveScene->IsRunning(), "Scene must be running!");

		if (InEditSceneState()) {
			return;
		}

		if (InPlaySceneState() && !m_ActiveScene->IsPaused()) {
			return;
		}

		m_ActiveScene->SetPaused(false);

		m_SceneViewportBorderFadeTimer = m_SceneViewportBorderFadeLengthInSeconds;

		if (InPlaySceneState())
			m_SceneViewportBorderFadeColor = m_SceneViewportOnPlayBorderColor;
		else if (InSimulateSceneState())
			m_SceneViewportBorderFadeColor = m_SceneViewportOnSimulateBorderColor;
	}

	void EditorLayer::OnSceneStop()
	{
		VX_CORE_ASSERT(!InEditSceneState(), "Invalid scene state!");

		SharedReference<Project> project = Project::GetActive();
		ProjectProperties& properties = project->GetProperties();

		if (InPlaySceneState())
		{
			// Invoke Actor.OnApplicationQuit
			m_ActiveScene->GetAllActorsWith<ScriptComponent>().each([=](auto actorID, auto& sc)
			{
				Actor actor{ actorID, m_ActiveScene.Raw() };

				actor.CallMethod(ScriptMethod::OnApplicationQuit);
			});

			m_ActiveScene->OnRuntimeStop();
		}
		else if (InSimulateSceneState())
		{
			m_ActiveScene->OnPhysicsSimulationStop();
		}

		SwitchSceneState(SceneState::Edit);

		if (properties.EditorProps.MaximizeOnPlay) {
			// we need to minimize the scene viewport
			ToggleSceneViewportMaximized();
		}

		m_HoveredActor = Actor{};

		m_ActiveScene = m_EditorScene;
		SetSceneContext(m_ActiveScene);

		// Reset the mouse cursor in case a script turned it off
		Input::SetCursorMode(CursorMode::Normal);

		if (m_TransitionedFromStartScene) {
			OpenScene(m_StartSceneMetadata);
			m_TransitionedFromStartScene = false;
		}
	}

	void EditorLayer::RestartScene()
	{
		VX_CORE_ASSERT(InPlaySceneState(), "active scene must be in play state!");

		UUID selectedUUID = 0;
		if (Actor selected = SelectionManager::GetSelectedActor()) {
			selectedUUID = selected.GetUUID();
		}

		OnScenePlay();

		// try to select the actor again in the new copy of the scene
		if (selectedUUID == 0) {
			return;
		}

		if (Actor selected = m_ActiveScene->TryGetActorWithUUID(selectedUUID)) {
			SelectionManager::SetSelectedActor(selected);
		}
	}

	void EditorLayer::OnSceneSimulate()
	{
		if (!InEditSceneState()) {
			OnSceneStop();
		}

		SwitchSceneState(SceneState::Simulate);

		SharedReference<Project> project = Project::GetActive();
		const ProjectProperties& properties = project->GetProperties();

		m_SceneViewportMaximized = properties.EditorProps.MaximizeOnPlay;

		m_ActiveScene = Scene::Copy(m_EditorScene);
		m_ActiveScene->OnPhysicsSimulationStart();

		SetSceneContext(m_ActiveScene);

		m_SceneViewportBorderFadeTimer = m_SceneViewportBorderFadeLengthInSeconds;
		m_SceneViewportBorderFadeColor = m_SceneViewportOnSimulateBorderColor;
	}

	void EditorLayer::RestartSceneSimulation()
	{
		VX_CORE_ASSERT(InSimulateSceneState(), "active scene must be in simulate state!");

		OnSceneSimulate();
	}

	void EditorLayer::QueueSceneTransition(const std::string& sceneName)
	{
		VX_PROFILE_FUNCTION();

		VX_CORE_ASSERT(m_ActiveScene->IsRunning(), "active scene must be running to queue transition!");

		auto fn = [=]() {
			if (!m_TransitionedFromStartScene) {
				m_StartSceneMetadata = m_EditorSceneMetadata;
			}

			bool opened = false;

			std::unordered_set<AssetHandle> scenes = Project::GetEditorAssetManager()->GetAllAssetsWithType(AssetType::SceneAsset);

			for (AssetHandle handle : scenes)
			{
				if (!AssetManager::IsHandleValid(handle))
					continue;

				SharedReference<Scene> scene = AssetManager::GetAsset<Scene>(handle);
				const std::string& entry = scene->GetName();
				if (String::FastCompare(sceneName, entry) == 0)
					continue;

				const AssetMetadata& metadata = Project::GetEditorAssetManager()->GetMetadata(handle);
				if (!AssetManager::IsHandleValid(metadata.Handle))
					continue;

				// play the scene
				OpenScene(metadata);
				OnScenePlay();
				opened = true;
			}

			if (!opened) {
				VX_CONSOLE_LOG_ERROR("[Editor] Failed to queue scene transition, failed to locate scene with name: {}", sceneName);
				return;
			}

			if (!m_TransitionedFromStartScene) {
				m_TransitionedFromStartScene = true;
			}
		};

		Application::Get().GetPreUpdateFunctionQueue().queue(fn);
	}
	
	void EditorLayer::SetWindowTitle(const std::string& sceneName)
	{
		const std::string projectName = Project::GetActive()->GetName();
		const std::string platformName = Platform::GetName();
		const std::string graphicsAPI = RendererAPI::GetAPIInfo().Name;

		Application& application = Application::Get();
		Window& window = application.GetWindow();

		const static std::string originalTitle = window.GetTitle();
		std::string newTitle = fmt::format(
			"{0} - {1} - {2} - {3} - <{4}>",
			projectName,
			sceneName,
			platformName,
			originalTitle,
			graphicsAPI
		);

		window.SetTitle(newTitle);
	}

	void EditorLayer::DuplicateSelectedActor()
	{
		Actor selectedActor = SelectionManager::GetSelectedActor();

		if (!selectedActor)
		{
			return;
		}

		Actor duplicate = m_ActiveScene->DuplicateActor(selectedActor);
		SelectionManager::SetSelectedActor(duplicate);

		m_PanelManager->GetPanel<SceneHierarchyPanel>()->FocusOnActorName(true);
	}

	void EditorLayer::SetSceneContext(SharedReference<Scene>& scene)
	{
		m_PanelManager->SetSceneContext(scene);
	}

	void EditorLayer::ResetEditorCameras()
	{
		EditorCamera* cameras[VX_EDITOR_MAX_CAMERAS] = { m_EditorCamera, m_SecondEditorCamera };

		const Math::vec3 origin = Math::vec3(0.0f);
		const float distance = 10.0f;

		for (size_t i = 0; i < VX_EDITOR_MAX_CAMERAS; i++)
		{
			EditorCamera* camera = cameras[i];
			if (camera == nullptr)
				continue;

			camera->Focus(origin);
			camera->SetDistance(distance);
		}
	}

	void EditorLayer::CaptureSceneViewportFramebufferImageToDisk()
	{
		const std::string sceneImagePath = Project::GetProjectDirectory().string() + "/" + Project::GetActive()->GetName() + ".png";

		if (FileSystem::Exists(sceneImagePath))
		{
			FileSystem::Remove(sceneImagePath);
		}

		const uint32_t nrChannels = 3;
		uint32_t stride = nrChannels * (uint32_t)m_ViewportPanelSize.x;

		// make sure alignment is 4 bytes
		stride += (stride % 4) ? (4 - stride % 4) : 0;

		const uint32_t bufferSize = stride * (uint32_t)m_ViewportPanelSize.y;
		Buffer buffer(bufferSize);

		// TODO if the scene viewport isn't maximized we need to read
		// at an offset into the framebuffer, offset by the viewport bounds?
		m_Framebuffer->ReadAttachmentToBuffer(0, buffer.As<char>());

		TextureProperties imageProps;
		imageProps.Filepath = sceneImagePath;
		imageProps.Width = (uint32_t)m_ViewportPanelSize.x;
		imageProps.Height = (uint32_t)m_ViewportPanelSize.y;
		imageProps.Channels = nrChannels;
		imageProps.Buffer = buffer.As<const void>();
		imageProps.Stride = stride;

		SharedReference<Texture2D> sceneTexture = Texture2D::Create(imageProps);
		sceneTexture->SaveToFile();
	}

	void EditorLayer::ReplaceSceneFileExtensionIfNeeded(std::string& filepath)
	{
		Fs::Path path = filepath;

		const std::string sceneExtension = Project::GetEditorAssetManager()->GetExtensionFromAssetType(AssetType::SceneAsset);

		if (path.extension() != sceneExtension || path.extension().empty())
		{
			FileSystem::ReplaceExtension(path, sceneExtension);
			filepath = path.string();
		}
	}

	void EditorLayer::ToggleGrid() const
	{
		SharedReference<Project> project = Project::GetActive();
		ProjectProperties& properties = project->GetProperties();
		properties.EditorProps.DrawEditorGrid = !properties.EditorProps.DrawEditorGrid;
	}

	void EditorLayer::ToggleSceneViewportMaximized() const
	{
		m_SceneViewportMaximized = !m_SceneViewportMaximized;
	}

	void EditorLayer::SwitchSceneState(SceneState state)
	{
		m_SceneState = state;
	}

	bool EditorLayer::InEditSceneState() const
	{
		return m_SceneState == SceneState::Edit;
	}

	bool EditorLayer::InPlaySceneState() const
	{
		return m_SceneState == SceneState::Play;
	}

	bool EditorLayer::InSimulateSceneState() const
	{
		return m_SceneState == SceneState::Simulate;
	}

	EditorCamera* EditorLayer::GetCurrentEditorCamera() const
	{
		if (m_SceneViewportHovered)
			return m_EditorCamera;

		if (m_SecondViewportHovered)
			return m_SecondEditorCamera;

		const bool noViewportHovered = !m_SceneViewportHovered && !m_SecondViewportHovered;
		if (noViewportHovered)
		{
			const bool bothViewportsOpen = m_SceneViewportPanelOpen && m_SecondViewportPanelOpen;
			if (bothViewportsOpen)
			{
				return m_SecondEditorCamera;
			}
			else
			{
				if (m_SecondViewportPanelOpen)
					return m_SecondEditorCamera;
				else if (m_SceneViewportPanelOpen)
					return m_EditorCamera;
			}
		}

		return nullptr;
	}

	std::vector<Math::vec4> EditorLayer::GetCameraFrustumCornersWorldSpace(const Camera* camera, const Math::mat4& view)
	{
		Math::mat4 proj = camera->GetProjectionMatrix();

		Math::mat4 inverseViewProj = Math::Inverse(proj * view);

		std::vector<Math::vec4> frustumCorners;
		for (uint32_t x = 0; x < 2; x++)
		{
			for (uint32_t y = 0; y < 2; y++)
			{
				for (uint32_t z = 0; z < 2; z++)
				{
					const Math::vec4 pt = inverseViewProj * Math::vec4(
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

	std::pair<float, float> EditorLayer::GetEditorCameraMouseViewportSpace(bool mainViewport)
	{
		auto [mx, my] = Gui::GetMousePos();

		const ViewportBounds viewportBounds = mainViewport ? m_ViewportBounds : m_SecondViewportBounds;

		mx -= viewportBounds.MinBound.x;
		my -= viewportBounds.MinBound.y;
		const Math::vec2 viewportSize = viewportBounds.CalculateViewportSize();

		return { (mx / viewportSize.x) * 2.0f - 1.0f, ((my / viewportSize.y) * 2.0f - 1.0f) * -1.0f };
	}

	Math::Ray EditorLayer::Raycast(EditorCamera* editorCamera, float mx, float my)
	{
		const Math::vec4 mouseClipPos = { mx, my, -1.0f, 1.0f };

		const Math::mat4 inverseProj = Math::Inverse(editorCamera->GetProjectionMatrix());
		const Math::mat3 inverseView = Math::Inverse(Math::mat3(editorCamera->GetViewMatrix()));

		const Math::vec4 ray = inverseProj * mouseClipPos;
		const Math::vec3 rayPos = editorCamera->GetPosition();
		const Math::vec3 rayDir = inverseView * Math::vec3(ray);

		return Math::Ray(rayPos, rayDir);
	}

	Actor EditorLayer::GetHoveredMeshActorFromRaycast()
	{
		std::vector<SelectionData> selectionData;

		const auto [mouseX, mouseY] = GetEditorCameraMouseViewportSpace(m_SceneViewportHovered);
		if (mouseX > -1.0f && mouseX < 1.0f && mouseY > -1.0f && mouseY < 1.0f)
		{
			EditorCamera* camera = GetCurrentEditorCamera();
			if (camera == nullptr)
				return Actor{};

			const auto [origin, direction] = Raycast(camera, mouseX, mouseY);

			auto meshView = m_ActiveScene->GetAllActorsWith<MeshRendererComponent>();
			for (const auto e : meshView)
			{
				Actor actor{ e, m_ActiveScene.Raw() };

				if (!actor.IsActive())
					continue;

				const TransformComponent worldSpaceTransform = m_ActiveScene->GetWorldSpaceTransform(actor);

				const MeshRendererComponent& meshRendererComponent = actor.GetComponent<MeshRendererComponent>();
				if (!meshRendererComponent.Visible)
					continue;

				if (!AssetManager::IsHandleValid(meshRendererComponent.Mesh))
					continue;
			}

			auto staticMeshView = m_ActiveScene->GetAllActorsWith<StaticMeshRendererComponent>();
			for (const auto e : staticMeshView)
			{
				Actor actor{ e, m_ActiveScene.Raw() };

				if (!actor.IsActive())
					continue;

				const TransformComponent worldSpaceTransform = m_ActiveScene->GetWorldSpaceTransform(actor);

				const StaticMeshRendererComponent& staticMeshRendererComponent = actor.GetComponent<StaticMeshRendererComponent>();
				if (!staticMeshRendererComponent.Visible)
					continue;

				if (!AssetManager::IsHandleValid(staticMeshRendererComponent.StaticMesh))
					continue;

				SharedReference<StaticMesh> staticMesh = AssetManager::GetAsset<StaticMesh>(staticMeshRendererComponent.StaticMesh);
				if (staticMesh == nullptr)
					continue;

				const Math::mat4 transform = worldSpaceTransform.GetTransform();
				const std::unordered_map<uint32_t, StaticSubmesh>& submeshes = staticMesh->GetSubmeshes();

				for (const auto& [submeshIndex, submesh] : submeshes)
				{
					Math::Ray ray{
						Math::Inverse(transform) * Math::vec4(origin, 1.0f),
						Math::Inverse((Math::mat3(transform))) * direction
					};

					const Math::AABB& aabb = submesh.GetBoundingBox();

					float t;
					const bool intersects = ray.IntersectsAABB(aabb, t);
					if (!intersects)
						continue;

					const float distance = Math::Distance(camera->GetPosition(), worldSpaceTransform.Translation);
					selectionData.emplace_back(SelectionData{ actor.GetUUID(), distance });
					break;
				}
			}

			if (selectionData.empty())
			{
				SelectionManager::DeselectActor();
				return Actor{};
			}

			const bool anyViewportHovered = (m_SceneViewportHovered && !InPlaySceneState()) || m_SecondViewportHovered;
			if (!anyViewportHovered)
			{
				return Actor{};
			}

			// start with the first option as a default
			SelectionData selectedData = selectionData[0];

			float closest = selectedData.Distance;
			for (const SelectionData& current : selectionData)
			{
				if (current.Distance > closest)
					continue;
				
				closest = current.Distance;
				selectedData.SelectedUUID = current.SelectedUUID;
				selectedData.Distance = current.Distance;
			}

			if (Actor hovered = m_ActiveScene->TryGetActorWithUUID(selectedData.SelectedUUID))
			{
				return hovered;
			}
		}

		return Actor{};
	}

	void EditorLayer::OnSelectGizmoToolSelected()
	{
		m_GizmoType = -1; // Invalid gizmo (a.k.a. the select tool)
	}

	void EditorLayer::OnTranslationGizmoToolSelected()
	{
		if (InPlaySceneState())
		{
			if (!m_SecondViewportHovered)
			{
				return;
			}
		}

		m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
	}

	void EditorLayer::OnRotationGizmoToolSelected()
	{
		if (InPlaySceneState())
		{
			if (m_SceneViewportHovered)
			{
				return;
			}
		}

		m_GizmoType = ImGuizmo::OPERATION::ROTATE;
	}

	void EditorLayer::OnScaleToolGizmoSelected()
	{
		if (InPlaySceneState())
		{
			if (m_SceneViewportHovered)
			{
				return;
			}
		}

		m_GizmoType = ImGuizmo::OPERATION::SCALE;
	}

}
