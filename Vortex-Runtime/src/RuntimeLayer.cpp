#include "RuntimeLayer.h"

#include <Vortex/Scene/Scene.h>
#include <Vortex/Project/ProjectLoader.h>
#include <Vortex/Serialization/SceneSerializer.h>
#include <Vortex/Scripting/ScriptEngine.h>
#include <Vortex/Scripting/ScriptRegistry.h>

namespace Vortex {

	RuntimeLayer::RuntimeLayer()
		: Layer("RuntimeLayer") { }

	void RuntimeLayer::OnAttach()
	{
		const auto& appProps = Application::Get().GetProperties();
		const auto& commandLineArgs = appProps.CommandLineArgs;

		FramebufferProperties framebufferProps{};
		framebufferProps.Attachments = {
			ImageFormat::RGBA16F,
			ImageFormat::RED_INTEGER,
			ImageFormat::RGBA16F,
			ImageFormat::Depth
		};
		framebufferProps.Width = appProps.WindowWidth;
		framebufferProps.Height = appProps.WindowHeight;

		m_Framebuffer = Framebuffer::Create(framebufferProps);

		m_ViewportSize = Math::vec2((float)appProps.WindowWidth, (float)appProps.WindowHeight);

		m_RuntimeScene = Scene::Create();
		m_RuntimeScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);

		if (commandLineArgs.Count > 1)
		{
			const auto& projectFilepath = commandLineArgs[1];

			OpenProject(std::filesystem::path(projectFilepath));
		}
		else
		{
			// Temp
			OpenProject("Projects/SandboxProject/Sandbox.vxproject");
		}
	}

	void RuntimeLayer::OnDetach() { }

	void RuntimeLayer::OnUpdate(TimeStep delta)
	{
		VX_PROFILE_FUNCTION();

		Renderer::RenderToDepthMap(m_RuntimeScene);

		m_RuntimeScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);

		// Resize
		if (FramebufferProperties props = m_Framebuffer->GetProperties();
			m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f && // zero sized framebuffer is invalid
			(props.Width != m_ViewportSize.x || props.Height != m_ViewportSize.y))
		{
			m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		}

		m_Framebuffer->Bind();
		RenderCommand::Clear();

		// Clear entityID attachment to -1
		m_Framebuffer->ClearAttachment(1, -1);

		m_RuntimeScene->OnUpdateRuntime(delta);

		auto [mx, my] = ImGui::GetMousePos();
		my = m_ViewportSize.y - my;

		int mouseX = (int)mx;
		int mouseY = (int)my;

		if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)m_ViewportSize.x && mouseY < (int)m_ViewportSize.y)
		{
			int pixelData = m_Framebuffer->ReadPixel(1, mouseX, mouseY);
			m_HoveredEntity = pixelData == -1 ? Entity() : Entity{ (entt::entity)pixelData, m_RuntimeScene.Raw() };
			ScriptRegistry::SetHoveredEntity(m_HoveredEntity);
		}

		m_Framebuffer->Unbind();
		
		if (Entity primaryCamera = m_RuntimeScene->GetPrimaryCameraEntity())
		{
			PostProcessProperties postProcessProps{};
			postProcessProps.TargetFramebuffer = m_Framebuffer;

			Math::vec3 cameraPos = primaryCamera.GetTransform().Translation;
			postProcessProps.CameraPosition = cameraPos;
			postProcessProps.ViewportSize = Viewport{ 0, 0, (uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y };
			PostProcessStage stages[] = { PostProcessStage::Bloom };
			postProcessProps.Stages = stages;
			postProcessProps.StageCount = VX_ARRAYCOUNT(stages);
			Renderer::BeginPostProcessingStages(postProcessProps);
		}

		const bool pendingTransition = ScriptRegistry::HasPendingTransitionQueued();
		if (pendingTransition)
		{
			QueueSceneTransition();
		}
	}

	void RuntimeLayer::OnGuiRender()
	{
		VX_PROFILE_FUNCTION();

		ImGuiViewport* viewport = Gui::GetMainViewport();
		Gui::SetNextWindowPos(viewport->WorkPos);
		Gui::SetNextWindowSize(viewport->WorkSize);
		Gui::SetNextWindowViewport(viewport->ID);

		// Set the viewport size
		m_ViewportSize = { viewport->WorkSize.x, viewport->WorkSize.y };

		Gui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		Gui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		Gui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0.0f, 0.0f });

		uint32_t flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBringToFrontOnFocus;
		Gui::Begin("##Game", nullptr, flags);

		uint32_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
		Gui::Image(reinterpret_cast<void*>(textureID), Gui::GetContentRegionAvail(), ImVec2{ 0, 1 }, ImVec2{ 1, 0 }); 
		
		Gui::End();

		Gui::PopStyleVar(3);

		// Update Application Gui
		m_RuntimeScene->OnUpdateEntityGui();
	}

	void RuntimeLayer::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);

		dispatcher.Dispatch<WindowCloseEvent>(VX_BIND_CALLBACK(RuntimeLayer::OnWindowCloseEvent));
	}

	void RuntimeLayer::OnRuntimeScenePlay()
	{
		VX_PROFILE_FUNCTION();
		VX_CORE_ASSERT(m_RuntimeScene->IsRunning(), "Scene must not be running!");

		m_RuntimeScene->OnRuntimeStart();

		ScriptRegistry::SetSceneStartTime(Time::GetTime());
	}

	void RuntimeLayer::OnRuntimeSceneStop()
	{
		VX_PROFILE_FUNCTION();
		VX_CORE_ASSERT(m_RuntimeScene->IsRunning(), "Scene must be running!");

		m_RuntimeScene->OnRuntimeStop();

		SharedReference<Scene> newScene = Scene::Create(m_Framebuffer);
		m_RuntimeScene.Swap(newScene);
		m_RuntimeScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);

		// Reset the mouse cursor in case a script turned it off
		Input::SetCursorMode(CursorMode::Normal);
	}

	bool RuntimeLayer::OnWindowCloseEvent(WindowCloseEvent& e)
	{
		CloseProject();

		return false;
	}

	bool RuntimeLayer::OpenProject(const std::filesystem::path& filepath)
	{
		VX_PROFILE_FUNCTION();

		// TODO this should be changed to LoadRuntimeProject in the future
		const bool success = ProjectLoader::LoadEditorProject(filepath);
		if (!success)
			return false;

		const AssetMetadata& sceneMetadata = Project::GetEditorAssetManager()->GetMetadata(Project::GetActive()->GetProperties().General.StartScene);
		VX_CORE_VERIFY(sceneMetadata.IsValid());

		return OpenScene(sceneMetadata);
	}

	void RuntimeLayer::CloseProject()
	{
		if (m_RuntimeScene->IsRunning())
			OnRuntimeSceneStop();

		ProjectLoader::CloseActiveProject();
	}

	bool RuntimeLayer::OpenScene(const AssetMetadata& sceneMetadata)
	{
		VX_PROFILE_FUNCTION();

		if (m_RuntimeScene->IsRunning())
		{
			OnRuntimeSceneStop();
		}

		if (!sceneMetadata.IsValid() || sceneMetadata.Type != AssetType::SceneAsset || !AssetManager::IsHandleValid(sceneMetadata.Handle))
		{
			VX_CORE_FATAL("Could not load {} - not a scene file", sceneMetadata.Filepath.filename().string());
			Application::Get().Quit();
			return false;
		}

		SceneSerializer serializer(m_RuntimeScene);

		std::string fullyQualifedScenePath = (Project::GetAssetDirectory() / sceneMetadata.Filepath).string();
		if (serializer.Deserialize(fullyQualifedScenePath))
		{
			SetSceneBuildIndexFromMetadata(sceneMetadata);

			OnRuntimeScenePlay();
		}

		return true;
	}

	void RuntimeLayer::SetSceneBuildIndexFromMetadata(const AssetMetadata& sceneMetadata)
	{
		std::string filename = sceneMetadata.Filepath.filename().string();
		std::string sceneName = filename.substr(0, filename.find('.'));

		const BuildIndexMap& buildIndices = Scene::GetScenesInBuild();

		for (const auto& [buildIndex, sceneFilepath] : buildIndices)
		{
			if (sceneFilepath.find(sceneMetadata.Filepath.string()) == std::string::npos)
				continue;

			Scene::SetActiveSceneBuildIndex(buildIndex);

			break;
		}
	}

	void RuntimeLayer::QueueSceneTransition()
	{
		VX_PROFILE_FUNCTION();
		VX_CORE_ASSERT(m_RuntimeScene->IsRunning(), "Scene must be running to queue transition!");

		Application::Get().SubmitToMainThreadQueue([=]()
		{
			const BuildIndexMap& buildIndices = Scene::GetScenesInBuild();
			const uint32_t nextBuildIndex = ScriptRegistry::GetNextBuildIndex();

			std::filesystem::path scenePath = buildIndices.at(nextBuildIndex);
			std::filesystem::path assetDirectory = Project::GetAssetDirectory();
			std::filesystem::path nextSceneFilepath = assetDirectory / scenePath;

			// TODO fix this
			const AssetMetadata& sceneMetadata = Project::GetEditorAssetManager()->GetMetadata(nextSceneFilepath);

			OpenScene(sceneMetadata);

			ScriptRegistry::ResetBuildIndex();
		});
	}

}
