#include "LauncherLayer.h"

#include <Sparky/Utils/PlatformUtils.h>

namespace Sparky {

	LauncherLayer::LauncherLayer()
		: Layer("LauncherLayer") { }

	void LauncherLayer::OnAttach()
	{
		const auto& appProps = Application::Get().GetProperties();

		FramebufferProperties framebufferProps{};
		framebufferProps.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::Depth };
		framebufferProps.Width = appProps.WindowWidth;
		framebufferProps.Height = appProps.WindowHeight;

		m_Framebuffer = Framebuffer::Create(framebufferProps);

		m_ViewportSize = Math::vec2((float)appProps.WindowWidth, (float)appProps.WindowHeight);
	}

	void LauncherLayer::OnDetach() { }

	void LauncherLayer::OnUpdate(TimeStep delta)
	{
		// Resize
		if (FramebufferProperties props = m_Framebuffer->GetProperties();
			m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f && // zero sized framebuffer is invalid
			(props.Width != m_ViewportSize.x || props.Height != m_ViewportSize.y))
		{
			m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		}

		m_Framebuffer->Bind();
		RenderCommand::Clear();
		m_Framebuffer->Unbind();
	}

	void LauncherLayer::OnGuiRender()
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];
		auto largeFont = io.Fonts->Fonts[1];
		auto hugeFont = io.Fonts->Fonts[2];

		ImGuiViewport* viewport = Gui::GetMainViewport();
		Gui::SetNextWindowPos(viewport->WorkPos);
		Gui::SetNextWindowSize(viewport->WorkSize);
		Gui::SetNextWindowViewport(viewport->ID);

		// Set the viewport size
		m_ViewportSize = { viewport->WorkSize.x, viewport->WorkSize.y };

		Gui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		Gui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		Gui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0.0f, 0.0f });

		Gui::Begin("Project Browser", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDecoration);
		
		static int selectedProject = 0;
		uint32_t i = 0;

		Gui::PushFont(largeFont);
		Gui::Text("Select A Project");
		Gui::PopFont();

		Gui::Spacing();

		Gui::BeginChild("Projects", ImVec2(250, 0), true);

		if (Gui::Selectable("New Project", selectedProject == 0))
		{
			m_ProjectPath = std::filesystem::path();
			selectedProject = 0;
		}

		Gui::Separator();

		i = 1;

		for (auto& directoryEntry : std::filesystem::directory_iterator("Assets\\Scenes"))
		{
			size_t lastBackslashPos = directoryEntry.path().string().find_last_of('\\') + 1;
			size_t dotPos = directoryEntry.path().string().find_last_of('.');
			std::string projectName = directoryEntry.path().string().substr(lastBackslashPos, dotPos - lastBackslashPos);

			if (Gui::Selectable(projectName.c_str(), selectedProject == i))
			{
				m_ProjectPath = directoryEntry.path();
				selectedProject = i;
			}

			i++;
		}

		Gui::EndChild();

		Gui::SameLine();
		
		Gui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.0f, 0.0f, 1.0f));
		Gui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8f, 0.0f, 0.0f, 1.0f));
		Gui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
		
		ImVec2 buttonSize = { Gui::GetContentRegionAvail().x / 4.0f, 50.0f };

		Gui::SetCursorPos({ Gui::GetContentRegionAvail().x - buttonSize.x, Gui::GetContentRegionAvail().y - buttonSize.y });
		if (Gui::Button("Launch Editor", buttonSize))
			LaunchEditor();

		Gui::PopStyleColor(3);

		Gui::End();

		Gui::PopStyleVar(3);
	}

	void LauncherLayer::OnEvent(Event& e) { }

	void LauncherLayer::LaunchEditor()
	{
		if (m_ProjectPath.empty())
			FileSystem::LaunchApplication("build\\Debug\\Sparky-Editor.exe", "");
		else
			FileSystem::LaunchApplication("build\\Debug\\Sparky-Editor.exe", m_ProjectPath.string().c_str());
	}

}
