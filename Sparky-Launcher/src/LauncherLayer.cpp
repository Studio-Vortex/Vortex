#include "LauncherLayer.h"

#include <Sparky/Utils/PlatformUtils.h>

namespace Sparky {

#define SP_MAX_PROJECT_NAME_LENGTH 256
#define SP_MAX_PROJECT_DIR_LENGTH 256

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

		m_SparkyLogoIcon = Texture2D::Create("Resources/Images/SparkyLogo.jpg");
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
		Gui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
		Gui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 10.0f, 10.0f });

		Gui::Begin("Project Browser", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDecoration);

		Gui::BeginGroup();

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

		for (auto& directoryEntry : std::filesystem::recursive_directory_iterator("Projects"))
		{
			if (directoryEntry.path().string().find(".sproject") == std::string::npos)
			{
				continue;
			}

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
		Gui::EndGroup();
		Gui::SameLine();
		
		ImVec2 contentRegionAvail = Gui::GetContentRegionAvail();

		Gui::BeginChild("Right", contentRegionAvail);

		Gui::PushFont(hugeFont);
		Gui::TextCentered("Sparky Game Engine", 24.0f);
		Gui::PopFont();
		Gui::Spacing();
		Gui::Spacing();
		Gui::Separator();

		ImVec2 logoSize = { contentRegionAvail.x / 4.0f, contentRegionAvail.x / 4.0f };
		Gui::SetCursorPos({ contentRegionAvail.x * 0.5f - logoSize.x * 0.5f, 75.0f });
		Gui::Image((void*)m_SparkyLogoIcon->GetRendererID(), logoSize, { 0, 1 }, { 1, 0 });

		Gui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.8f, 1.0f));
		Gui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.2f, 0.8f, 1.0f));
		Gui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.2f, 1.0f, 1.0f));

		ImVec2 buttonSize = { contentRegionAvail.x / 4.0f, 50.0f };
		Gui::SetCursorPos({ contentRegionAvail.x * 0.5f - buttonSize.x * 0.5f, contentRegionAvail.y - buttonSize.y * 1.5f });
		const char* buttonText = selectedProject == 0 ? "Create Project" : "Open Project";
		if (!m_CreatingNewProject && Gui::Button(buttonText, buttonSize))
		{
			if (selectedProject == 0) // Create a new project
			{
				m_CreatingNewProject = true;
				Gui::OpenPopup("Create New Project");
			}
			else
			{
				LaunchEditor();
			}
		}

		if (Gui::IsPopupOpen("Create New Project"))
			DisplayCreateProjectPopup();
		else
			m_CreatingNewProject = false;

		Gui::PopStyleColor(3);
		Gui::PopStyleVar(3);
		Gui::EndChild();
		
		Gui::End();
	}

	void LauncherLayer::OnEvent(Event& e) { }

	void LauncherLayer::DisplayCreateProjectPopup()
	{
		ImVec2 center = Gui::GetMainViewport()->GetCenter();
		Gui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		Gui::SetNextWindowSize(ImVec2(700, 0));

		if (Gui::BeginPopupModal("Create New Project"))
		{
			static char projectNameBuffer[SP_MAX_PROJECT_NAME_LENGTH]{ 0 };
			ImGuiIO& io = Gui::GetIO();
			auto contextRegionAvail = Gui::GetContentRegionAvail();
			Gui::Columns(2);
			Gui::Text("Project Name");
			Gui::Spacing();
			Gui::Text("Directory");
			Gui::NextColumn();
			Gui::PushItemWidth(-1);
			if (Gui::InputText("##Project Name", projectNameBuffer, SP_MAX_PROJECT_NAME_LENGTH))
			{

			}
			Gui::PushItemWidth(-1);
			static char projectDirectoryBuffer[SP_MAX_PROJECT_NAME_LENGTH]{ 0 };
			if (Gui::InputText("##Directory", projectDirectoryBuffer, SP_MAX_PROJECT_NAME_LENGTH))
			{

			}

			Gui::Columns(1);

			Gui::Spacing();
			Gui::Separator();
			Gui::Spacing();

			ImVec2 buttonSize = { Gui::GetContentRegionAvail().x / 4.0f, 50.0f };

			if (Gui::Button("Go Back", buttonSize))
			{
				Gui::CloseCurrentPopup();
			}

			bool projectReady = strlen(projectNameBuffer) != 0 && std::filesystem::exists(projectDirectoryBuffer);

			Gui::SameLine();
			Gui::SetCursorPosX((Gui::GetWindowWidth() * 0.5f) - (buttonSize.x * 0.5f));
			Gui::BeginDisabled(!projectReady);
			if (Gui::Button("Create Project", buttonSize))
			{

			}
			Gui::EndDisabled();

			Gui::EndPopup();
		}
	}

	void LauncherLayer::LaunchEditor()
	{
		FileSystem::LaunchApplication(Application::Get().GetEditorAppPath().c_str(), m_ProjectPath.string().c_str());
	}

}
