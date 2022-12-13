#include "RuntimeLayer.h"

#include <Sparky/Scene/Scene.h>
#include <Sparky/Scene/SceneSerializer.h>
#include <Sparky/Scripting/ScriptEngine.h>
#include <Sparky/Scripting/ScriptRegistry.h>

namespace Sparky {

	RuntimeLayer::RuntimeLayer()
		: Layer("RuntimeLayer") { }

	void RuntimeLayer::OnAttach()
	{
		const auto& appProps = Application::Get().GetProperties();
		const auto& commandLineArgs = appProps.CommandLineArgs;

		FramebufferProperties framebufferProps{};
		framebufferProps.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth };
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
			OpenProject("Projects/SandboxProject/Sandbox.sproject");
		}
	}

	void RuntimeLayer::OnDetach() { }

	void RuntimeLayer::OnUpdate(TimeStep delta)
	{
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

		if (const char* sceneToBeLoaded = ScriptRegistry::GetSceneToBeLoaded(); strlen(sceneToBeLoaded) != 0)
		{
			auto scenePath = std::format("Assets/Scenes/{}.sparky", sceneToBeLoaded);
			OpenScene(Project::GetProjectDirectory() / scenePath);
			ScriptRegistry::ResetSceneToBeLoaded();
		}

		bool scenePaused = m_RuntimeScene->IsPaused();

		if (scenePaused)
			OnRuntimeScenePaused();
		
		if (!scenePaused && !m_AudioSourcesToResume.empty())
			OnRuntimeSceneResumed();

		m_RuntimeScene->OnUpdateRuntime(delta);

		auto [mx, my] = ImGui::GetMousePos();
		my = m_ViewportSize.y - my;

		int mouseX = (int)mx;
		int mouseY = (int)my;

		if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)m_ViewportSize.x && mouseY < (int)m_ViewportSize.y)
		{
			int pixelData = m_Framebuffer->ReadPixel(1, mouseX, mouseY);
			m_HoveredEntity = pixelData == -1 ? Entity() : Entity{ (entt::entity)pixelData, m_RuntimeScene.get() };
		}

		ScriptRegistry::SetHoveredEntity(m_HoveredEntity);

		m_Framebuffer->Unbind();
	}

	void RuntimeLayer::OnGuiRender()
	{
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

	void RuntimeLayer::OnEvent(Event& e) { }

	void RuntimeLayer::OnRuntimeScenePaused()
	{
		auto view = m_RuntimeScene->GetAllEntitiesWith<AudioSourceComponent>();

		// Pause all audio sources in the scene
		for (auto& e : view)
		{
			Entity entity{ e, m_RuntimeScene.get() };
			SharedRef<AudioSource> audioSource = entity.GetComponent<AudioSourceComponent>().Source;
			if (audioSource->IsPlaying())
			{
				audioSource->Pause();
				m_AudioSourcesToResume.push_back(audioSource);
			}
		}
	}

	void RuntimeLayer::OnRuntimeSceneResumed()
	{
		if (!m_AudioSourcesToResume.empty())
		{
			for (auto& audioSource : m_AudioSourcesToResume)
				audioSource->Play();

			m_AudioSourcesToResume.clear();
		}
	}

	bool RuntimeLayer::OpenProject(const std::filesystem::path& filepath)
	{
		if (filepath.extension().string() != ".sproject")
		{
			SP_WARN("Could not load {} - not a project file", filepath.filename().string());
			return false;
		}

		if (Project::Load(filepath))
		{
			std::string projectName = std::format("{} Project Load Time", filepath.filename().string());
			InstrumentationTimer timer(projectName.c_str());

			ScriptEngine::Init();

			auto startScenePath = Project::GetAssetFileSystemPath(Project::GetActive()->GetProperties().General.StartScene);
			OpenScene(startScenePath.string());

			TagComponent::ResetAddedMarkers();

			return true;
		}

		return false;
	}

	bool RuntimeLayer::OpenScene(const std::filesystem::path& filepath)
	{
		if (m_RuntimeScene->IsRunning())
		{
			m_RuntimeScene->OnRuntimeStop();

			SharedRef<Scene> newScene = Scene::Create();
			m_RuntimeScene.swap(newScene);
			m_RuntimeScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);

			// Reset the mouse cursor in case a script turned it off
			Application::Get().GetWindow().ShowMouseCursor(true);
		}

		if (std::filesystem::exists(filepath) && filepath.extension().string() == ".sparky")
		{
			SceneSerializer serializer(m_RuntimeScene);

			if (serializer.Deserialize(filepath.string()))
			{
				m_RuntimeScene->OnRuntimeStart();
				return true;
			}
		}
		else
		{
			SP_WARN("Could not load {} - not a scene file", filepath.filename().string());
			system("pause");
			Application::Get().Quit();
			return false;
		}

		return false;
	}

}
