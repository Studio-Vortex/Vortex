#include "RuntimeLayer.h"

#include <Sparky/Scene/Scene.h>
#include <Sparky/Scene/SceneSerializer.h>
#include <Sparky/Scripting/ScriptRegistry.h>

namespace Sparky {

	RuntimeLayer::RuntimeLayer()
		: Layer("RuntimeLayer") { }

	void RuntimeLayer::OnAttach()
	{
		const auto& appProps = Application::Get().GetProperties();
		const auto& commandLineArgs = appProps.CommandLineArgs;

		if (commandLineArgs.Count > 1)
		{
			FramebufferProperties framebufferProps{};
			framebufferProps.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::Depth };
			framebufferProps.Width = appProps.WindowWidth;
			framebufferProps.Height = appProps.WindowHeight;

			m_Framebuffer = Framebuffer::Create(framebufferProps);

			m_ViewportSize = Math::vec2((float)appProps.WindowWidth, (float)appProps.WindowHeight);

			m_RuntimeScene = CreateShared<Scene>();
			m_RuntimeScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);

			const auto& sceneFilePath = commandLineArgs[1];

			OpenScene(std::filesystem::path(sceneFilePath));
			RenderCommand::SetClearColor(Math::vec3{ (38.0f / 255.0f), (44.0f / 255.0f), (60.0f / 255.0f) });
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

		if (const char* sceneToBeLoaded = ScriptRegistry::GetSceneToBeLoaded(); strlen(sceneToBeLoaded) != 0)
		{
			OpenScene(std::filesystem::path(std::format("Assets/Scenes/{}.sparky", sceneToBeLoaded)));
			ScriptRegistry::ResetSceneToBeLoaded();
		}

		bool scenePaused = m_RuntimeScene->IsPaused();

		if (scenePaused)
			OnRuntimeScenePaused();
		
		if (!scenePaused && !m_AudioSourcesToResume.empty())
			OnRuntimeSceneResumed();

		m_RuntimeScene->OnUpdateRuntime(delta);

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
		Gui::Begin("Game", nullptr, flags);

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

	bool RuntimeLayer::OpenScene(const std::filesystem::path& filepath)
	{
		if (m_RuntimeScene->IsRunning())
		{
			m_RuntimeScene->OnRuntimeStop();

			SharedRef<Scene> newScene = CreateShared<Scene>();
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
			exit(0);
			return false;
		}

		return false;
	}

}
