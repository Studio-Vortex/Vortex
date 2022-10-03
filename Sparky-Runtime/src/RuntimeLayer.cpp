#include "RuntimeLayer.h"

#include <Sparky/Scene/SceneSerializer.h>

namespace Sparky {

	extern const std::filesystem::path g_AssetPath;

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
			m_RuntimeScene = CreateShared<Scene>();
			m_RuntimeScene->OnViewportResize(appProps.WindowWidth, appProps.WindowHeight);

			m_ViewportSize = Math::vec2((float)appProps.WindowWidth, (float)appProps.WindowHeight);

			const auto& sceneFilePath = commandLineArgs[1];

			if (OpenScene(std::filesystem::path(sceneFilePath)))
				m_RuntimeScene->OnRuntimeStart();
		}
	}

	void RuntimeLayer::OnDetach()
	{
		m_RuntimeScene->OnRuntimeStop();
	}

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

		m_RuntimeScene->OnUpdateRuntime(delta);

		m_Framebuffer->Unbind();
	}

	void RuntimeLayer::OnGuiRender()
	{
		static bool open = true;

		Gui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		Gui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);

		Gui::Begin("Game", &open, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings);

		uint32_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
		Gui::Image(reinterpret_cast<void*>(textureID), Gui::GetContentRegionAvail(), ImVec2{0, 1}, ImVec2{1, 0});
		
		Gui::End();
		Gui::PopStyleVar(2);
	}

	void RuntimeLayer::OnEvent(Event& e)
	{
		
	}

	bool RuntimeLayer::OpenScene(const std::filesystem::path& filepath)
	{
		if (std::filesystem::exists(filepath) && filepath.extension().string() == ".sparky")
		{
			SceneSerializer serializer(m_RuntimeScene);

			if (serializer.Deserialize(filepath.string()))
				return true;
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