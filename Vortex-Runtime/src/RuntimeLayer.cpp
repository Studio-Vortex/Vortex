#include "RuntimeLayer.h"

#include <Vortex/Project/ProjectLoader.h>

#include <Vortex/Serialization/SceneSerializer.h>

#include <Vortex/Scripting/ScriptEngine.h>
#include <Vortex/Scripting/ScriptRegistry.h>

namespace Vortex {

	RuntimeLayer::RuntimeLayer()
		: Layer("RuntimeLayer") { }

	void RuntimeLayer::OnAttach()
	{
		const Application& app = Application::Get();
		const ApplicationProperties& applicationProperties = app.GetProperties();

		FramebufferProperties framebufferProps{};
		framebufferProps.Attachments = {
			ImageFormat::RGBA16F,
			ImageFormat::RED_INTEGER,
			ImageFormat::RGBA16F,
			ImageFormat::Depth
		};
		framebufferProps.Width = applicationProperties.WindowWidth;
		framebufferProps.Height = applicationProperties.WindowHeight;

		m_Framebuffer = Framebuffer::Create(framebufferProps);

		m_ViewportSize = Math::vec2((float)applicationProperties.WindowWidth, (float)applicationProperties.WindowHeight);

		m_RuntimeScene = Scene::Create();
		m_RuntimeScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);

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
			// Temp
			OpenProject("Projects/SandboxProject/Sandbox.vxproject");
		}
	}

	void RuntimeLayer::OnDetach() { }

	void RuntimeLayer::OnUpdate(TimeStep delta)
	{
		VX_PROFILE_FUNCTION();

		// Shadow pass
		if (Actor skyLightEntity = m_RuntimeScene->GetSkyLightActor())
		{
			const LightSourceComponent& lsc = skyLightEntity.GetComponent<LightSourceComponent>();
			if (lsc.Visible && lsc.CastShadows)
			{
				Renderer::RenderToDepthMap(m_RuntimeScene);
			}
		}

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

		// Update Scene
		m_RuntimeScene->OnUpdateRuntime(delta);

		m_Framebuffer->Unbind();
		
		// Bloom pass
		if (Actor primaryCamera = m_RuntimeScene->GetPrimaryCameraActor())
		{
			PostProcessProperties postProcessProps{};
			postProcessProps.TargetFramebuffer = m_Framebuffer;
			Math::vec3 cameraPos = primaryCamera.GetTransform().Translation;
			postProcessProps.CameraPosition = cameraPos;
			postProcessProps.ViewportInfo = Viewport{ 0, 0, (uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y };
			PostProcessStage stages[] = { PostProcessStage::Bloom };
			postProcessProps.Stages = stages;
			postProcessProps.StageCount = VX_ARRAYSIZE(stages);
			Renderer::BeginPostProcessingStages(postProcessProps);
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

		UI::ScopedStyle windowRounding(ImGuiStyleVar_WindowRounding, 0.0f);
		UI::ScopedStyle windowBorderSize(ImGuiStyleVar_WindowBorderSize, 0.0f);
		UI::ScopedStyle windowPadding(ImGuiStyleVar_WindowPadding, ImVec2{ 0.0f, 0.0f });

		uint32_t flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBringToFrontOnFocus;
		Gui::Begin("##ApplicationViewport", nullptr, flags);

		const ImVec2 viewportMinRegion = Gui::GetWindowContentRegionMin();
		const ImVec2 viewportMaxRegion = Gui::GetWindowContentRegionMax();
		const ImVec2 viewportOffset = Gui::GetWindowPos();

		m_ViewportBounds.MinBound = Math::vec2(viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y);
		m_ViewportBounds.MaxBound = Math::vec2(viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y);

		m_RuntimeScene->SetViewportBounds(m_ViewportBounds);

		uint32_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
		Gui::Image(reinterpret_cast<void*>(textureID), Gui::GetContentRegionAvail(), ImVec2{ 0, 1 }, ImVec2{ 1, 0 }); 
		
		Gui::End();

		// Render Application Gui
		m_RuntimeScene->InvokeActorOnGuiRender();
	}

	void RuntimeLayer::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);

		dispatcher.Dispatch<WindowCloseEvent>(VX_BIND_CALLBACK(RuntimeLayer::OnWindowCloseEvent));
	}

	void RuntimeLayer::OnScenePlay()
	{
		VX_PROFILE_FUNCTION();

		VX_CORE_ASSERT(m_RuntimeScene, "Invalid scene!");

		ScriptRegistry::SetSceneStartTime(Time::GetTime());

		m_RuntimeScene->OnRuntimeStart();
	}

	void RuntimeLayer::OnSceneStop()
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
		if (!ProjectLoader::LoadEditorProject(filepath))
		{
			return false;
		}

		const Fs::Path& startScenePath = Project::GetActive()->GetProperties().General.StartScene;
		const AssetMetadata& metadata = Project::GetEditorAssetManager()->GetMetadata(startScenePath);
		VX_CORE_VERIFY(metadata.IsValid());

		return OpenScene(metadata);
	}

	void RuntimeLayer::CloseProject()
	{
		if (m_RuntimeScene->IsRunning())
		{
			// Invoke Actor.OnApplicationQuit
			m_RuntimeScene->GetAllActorsWith<ScriptComponent>().each([=](auto actorID, auto& sc)
			{
				Actor actor{ actorID, m_RuntimeScene.Raw() };

				actor.CallMethod(ScriptMethod::OnApplicationQuit);
			});

			OnSceneStop();
		}

		ScriptEngine::Shutdown();
	}

	bool RuntimeLayer::OpenScene(const AssetMetadata& metadata)
	{
		VX_PROFILE_FUNCTION();

		if (m_RuntimeScene->IsRunning())
		{
			OnSceneStop();
		}

		if (!metadata.IsValid() || metadata.Type != AssetType::SceneAsset || !AssetManager::IsHandleValid(metadata.Handle))
		{
			VX_CORE_FATAL("Could not load {} - not a scene file", metadata.Filepath.filename().string());
			Application::Get().Close();
			return false;
		}

		SceneSerializer serializer(m_RuntimeScene);

		const Fs::Path assetDirectory = Project::GetAssetDirectory();
		std::string fullyQualifedScenePath = (assetDirectory / metadata.Filepath).string();
		if (serializer.Deserialize(fullyQualifedScenePath))
		{
			OnScenePlay();
		}

		return true;
	}

	void RuntimeLayer::QueueSceneTransition()
	{
		VX_PROFILE_FUNCTION();

		VX_CORE_ASSERT(m_RuntimeScene->IsRunning(), "Scene must be running to queue transition!");

		Application::Get().SubmitToPreUpdateFrameMainThreadQueue([=]()
		{
			// TODO
			const Fs::Path assetDirectory = Project::GetAssetDirectory();
			const Fs::Path nextSceneFilepath = assetDirectory / "";

			// TODO this will change eventually
			const AssetMetadata& sceneMetadata = Project::GetEditorAssetManager()->GetMetadata(nextSceneFilepath);

			OpenScene(sceneMetadata);
		});
	}

}
