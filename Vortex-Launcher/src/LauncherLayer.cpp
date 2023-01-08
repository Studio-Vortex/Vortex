#include "LauncherLayer.h"

#include <Vortex/Utils/PlatformUtils.h>
#include <Vortex/Scene/SceneSerializer.h>

namespace Vortex {

#define VX_MAX_PROJECT_NAME_LENGTH 256
#define VX_MAX_PROJECT_DIR_LENGTH 256

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

		m_VortexLogoIcon = Texture2D::Create("Resources/Images/VortexLogo.png");
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
			if (directoryEntry.path().string().find(".vxproject") == std::string::npos)
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
		Gui::TextCentered("Vortex Game Engine", 24.0f);
		Gui::PopFont();
		Gui::Spacing();
		Gui::Spacing();
		Gui::Separator();

		ImVec2 logoSize = { contentRegionAvail.x / 4.0f, contentRegionAvail.x / 4.0f };
		Gui::SetCursorPos({ contentRegionAvail.x * 0.5f - logoSize.x * 0.5f, 75.0f });
		Gui::Image((void*)m_VortexLogoIcon->GetRendererID(), logoSize, { 0, 1 }, { 1, 0 });

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

		auto ResetInputBoxesFunc = [&](char* projectNameBuffer, char* projectDirectoryBuffer)
		{
			memset(projectNameBuffer, 0, VX_MAX_PROJECT_NAME_LENGTH);
			memset(projectDirectoryBuffer, 0, VX_MAX_PROJECT_DIR_LENGTH);
		};

		if (Gui::BeginPopupModal("Create New Project"))
		{
			static char projectNameBuffer[VX_MAX_PROJECT_NAME_LENGTH]{ 0 };
			ImGuiIO& io = Gui::GetIO();
			auto contextRegionAvail = Gui::GetContentRegionAvail();
			Gui::Columns(2);
			Gui::Text("Project Name");
			Gui::Spacing();
			Gui::Text("Directory");
			Gui::NextColumn();
			Gui::PushItemWidth(-1);
			Gui::InputText("##Project Name", projectNameBuffer, VX_MAX_PROJECT_NAME_LENGTH);

			Gui::PushItemWidth(-1);
			static char projectDirectoryBuffer[VX_MAX_PROJECT_NAME_LENGTH]{ 0 };
			Gui::InputText("##Directory", projectDirectoryBuffer, VX_MAX_PROJECT_NAME_LENGTH);

			Gui::Columns(1);

			Gui::Spacing();
			Gui::Separator();
			Gui::Spacing();

			ImVec2 buttonSize = { Gui::GetContentRegionAvail().x / 4.0f, 50.0f };

			if (Gui::Button("Go Back", buttonSize))
			{
				ResetInputBoxesFunc(projectNameBuffer, projectDirectoryBuffer);
				Gui::CloseCurrentPopup();
			}

			bool textBoxesEmpty = strlen(projectNameBuffer) == 0 && strlen(projectDirectoryBuffer) == 0;

			Gui::SameLine();
			Gui::SetCursorPosX((Gui::GetWindowWidth() * 0.5f) - (buttonSize.x * 0.5f));
			Gui::BeginDisabled(textBoxesEmpty);
			if (Gui::Button("Create Project", buttonSize))
			{
				if (!std::filesystem::exists(projectDirectoryBuffer))
					std::filesystem::create_directories(projectDirectoryBuffer);

				SharedRef<Project> project = Project::New();
				auto& projectProps = project->GetProperties();
				projectProps.General.Name = std::string(projectNameBuffer);
				projectProps.General.AssetDirectory = "Assets";
				projectProps.General.StartScene = "Scenes/Untitled.vortex";
				projectProps.General.ScriptBinaryPath = std::format("Scripts/Binaries/{}.dll", projectNameBuffer);

				auto projectFilename = std::format("{}.vxproject", projectProps.General.Name);
				m_ProjectPath = projectDirectoryBuffer / std::filesystem::path(projectFilename);
				std::filesystem::path projectDirectoryPath = m_ProjectPath.parent_path();
				std::filesystem::create_directories(projectDirectoryPath / "Assets/Scenes");
				std::filesystem::create_directories(projectDirectoryPath / "Assets/Scripts/Binaries");
				std::filesystem::create_directories(projectDirectoryPath / "Assets/Scripts/Source");

				// premake script
				//FileSystem::LaunchApplication("Resources/HelperScripts/GeneratePremakeScript.bat", ((projectDirectoryBuffer / projectProps.General.AssetDirectory) / "Scripts").string().c_str());

				// build project dll
				//FileSystem::LaunchApplication("Resources/HelperScripts/BuildSolution.bat", (projectDirectoryBuffer / projectProps.General.ScriptBinaryPath).string().c_str());

				SharedRef<Scene> startScene = Scene::Create();
				Scene::CreateDefaultEntities(startScene);
				SceneSerializer serializer(startScene);
				serializer.Serialize((projectDirectoryBuffer / std::filesystem::path("Assets/Scenes/Untitled.vortex")).string());

				Project::SaveActive(m_ProjectPath);

				LaunchEditor();

				ResetInputBoxesFunc(projectNameBuffer, projectDirectoryBuffer);
				Gui::CloseCurrentPopup();
			}
			Gui::EndDisabled();

			Gui::EndPopup();
		}
	}

	void LauncherLayer::LaunchEditor()
	{
		std::string editorPath = Application::Get().GetEditorBinaryPath();
		FileSystem::LaunchApplication(editorPath.c_str(), m_ProjectPath.string().c_str());
	}

}
