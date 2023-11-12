#include "LauncherLayer.h"

#include <Vortex/Serialization/SceneSerializer.h>
#include <Vortex/Project/ProjectSerializer.h>

namespace Vortex {

	namespace Utils {

		static const char* ProjectTypeToString(ProjectType type)
		{
			switch (type)
			{
				case ProjectType::e2D: return "2D";
				case ProjectType::e3D: return "3D";
			}

			VX_CORE_ASSERT(false, "Unknown project type!");
			return "";
		}

	}

	LauncherLayer::LauncherLayer()
		: Layer("LauncherLayer") { }

	void LauncherLayer::OnAttach()
	{
		Application& application = Application::Get();
		const auto& appProperties = application.GetProperties();

		FramebufferProperties framebufferProps{};
		framebufferProps.Attachments = { ImageFormat::RGBA16F, ImageFormat::Depth };
		framebufferProps.Width = appProperties.WindowWidth;
		framebufferProps.Height = appProperties.WindowHeight;

		m_Framebuffer = Framebuffer::Create(framebufferProps);

		m_ViewportSize = Math::vec2((float)appProperties.WindowWidth, (float)appProperties.WindowHeight);

		m_Properties.EditorPath = application.GetEditorBinaryPath();
		m_Properties.WorkingDirectory = appProperties.WorkingDirectory;

		ResetInputFields();
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
			m_Properties.ProjectPath = std::filesystem::path();
			m_SelectedProjectIcon = nullptr;
			selectedProject = 0;
		}

		i = 1;

		UI::Draw::Underline();

		for (const auto& directoryEntry : std::filesystem::recursive_directory_iterator("Projects"))
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
				m_Properties.ProjectPath = directoryEntry.path();
				std::string projectIconName = projectName + ".png";
				std::filesystem::path projectDirectory = FileSystem::GetParentDirectory(m_Properties.ProjectPath);
				
				m_SelectedProjectIcon = nullptr;

				for (const auto& projectEntry : std::filesystem::recursive_directory_iterator(projectDirectory))
				{
					if (projectEntry.path().string().find(projectIconName) == std::string::npos)
					{
						continue;
					}
					
					TextureProperties imageProps;
					imageProps.Filepath = (projectDirectory / projectIconName).string();
					imageProps.WrapMode = ImageWrap::Repeat;

					m_SelectedProjectIcon = Texture2D::Create(imageProps);
					break;
				}

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
		Gui::TextCentered("Vortex Game Engine", 26.0f);
		Gui::PopFont();
		Gui::Spacing();
		Gui::Spacing();
		UI::Draw::Underline();

		if (m_SelectedProjectIcon)
		{
			ImVec2 contentRegionAvailable = Gui::GetContentRegionAvail();
			float imageScale = 1.1f;
			ImVec2 textureSize = { 640 * imageScale, 360 * imageScale };
			Gui::SetCursorPos({ contentRegionAvailable.x * 0.5f - textureSize.x * 0.5f, contentRegionAvailable.y * 0.5f - textureSize.y * 0.5f });
			Gui::Image((ImTextureID)m_SelectedProjectIcon->GetRendererID(), textureSize, { 0, 1 }, { 1, 0 });
		}

		Gui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.8f, 1.0f));
		Gui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.2f, 0.8f, 1.0f));
		Gui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.2f, 1.0f, 1.0f));

		ImVec2 buttonSize = { contentRegionAvail.x / 4.0f, 50.0f };
		Gui::SetCursorPos({ contentRegionAvail.x * 0.5f - buttonSize.x * 0.5f, contentRegionAvail.y - buttonSize.y * 1.5f });
		const char* buttonText = selectedProject == 0 ? "Create Project" : "Open Project";
		if (!m_IsCreatingNewProject && Gui::Button(buttonText, buttonSize))
		{
			if (selectedProject == 0) // Create a new project
			{
				m_IsCreatingNewProject = true;
				Gui::OpenPopup("Create New Project");
			}
			else
			{
				LaunchEditor();
			}
		}

		if (Gui::IsPopupOpen("Create New Project"))
		{
			DisplayCreateProjectPopup();
		}
		else
		{
			m_IsCreatingNewProject = false;
		}

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
			static const char* options[] = { Utils::ProjectTypeToString(ProjectType::e2D), Utils::ProjectTypeToString(ProjectType::e3D) };
			int32_t currentProjectType = (int32_t)m_Properties.ProjectType;

			UI::BeginPropertyGrid();
			if (UI::PropertyDropdown("Project Type", options, VX_ARRAYCOUNT(options), currentProjectType))
			{
				m_Properties.ProjectType == (ProjectType)currentProjectType;
			}
			UI::EndPropertyGrid();

			auto contextRegionAvail = Gui::GetContentRegionAvail();
			Gui::Columns(2);
			Gui::Text("Project Name");
			Gui::Spacing();
			Gui::Text("Directory");
			Gui::NextColumn();
			Gui::PushItemWidth(-1);
			Gui::InputText("##Project Name", m_Properties.ProjectNameBuffer, VX_MAX_PROJECT_NAME_LENGTH);
			Gui::PushItemWidth(Gui::GetContentRegionAvail().x * 0.9f);
			Gui::InputText("##Directory", m_Properties.ProjectDirectoryBuffer, VX_MAX_PROJECT_NAME_LENGTH);
			Gui::SameLine();
			if (Gui::Button("..."))
			{
				std::string filepath = FileDialogue::OpenFolderDialog();
				memcpy(m_Properties.ProjectDirectoryBuffer, filepath.data(), VX_MAX_PROJECT_DIR_LENGTH);
			}

			Gui::Columns(1);

			Gui::Spacing();
			UI::Draw::Underline();
			Gui::Spacing();

			ImVec2 buttonSize = { Gui::GetContentRegionAvail().x / 4.0f, 50.0f };

			if (Gui::Button("Go Back", buttonSize))
			{
				ResetInputFields();
				Gui::CloseCurrentPopup();
			}

			const bool inputFieldsEmpty = strlen(m_Properties.ProjectNameBuffer) == 0 && strlen(m_Properties.ProjectDirectoryBuffer) == 0;

			Gui::SameLine();

			UI::ShiftCursorX((Gui::GetWindowWidth() * 0.5f) - (buttonSize.x * 0.5f));
			Gui::BeginDisabled(inputFieldsEmpty);
			if (Gui::Button("Create Project", buttonSize))
			{
				CreateProject();
				Gui::CloseCurrentPopup();
			}
			Gui::EndDisabled();

			Gui::EndPopup();
		}
	}

	void LauncherLayer::CreateProjectFilesAndDirectories()
	{
		Project::New();
		ProjectProperties& projectProps = Project::GetActive()->GetProperties();

		projectProps.General.Name = std::string(m_Properties.ProjectNameBuffer);
		projectProps.General.AssetDirectory = "Assets";
		projectProps.General.AssetRegistryPath = "AssetRegistry.vxr";
		projectProps.General.StartScene = "Scenes/SampleScene.vortex";
		projectProps.ScriptingProps.ScriptBinaryPath = std::format("Scripts/Binaries/{}.dll", m_Properties.ProjectNameBuffer);

		auto projectFilename = std::format("{}.vxproject", projectProps.General.Name);
		m_Properties.ProjectPath = m_Properties.ProjectDirectoryBuffer / std::filesystem::path(projectFilename);
		std::filesystem::path projectDirectoryPath = FileSystem::GetParentDirectory(m_Properties.ProjectPath);

		FileSystem::CreateDirectoriesV(projectDirectoryPath / "Assets/Scripts/Binaries");
		FileSystem::CreateDirectoriesV(projectDirectoryPath / "Assets/Scripts/Source");
		FileSystem::RecursiveDirectoryCopy("Resources/NewProjectTemplate", projectDirectoryPath / std::filesystem::path("Assets"));
	}

	void LauncherLayer::CreatePremakeBuildScript()
	{
		std::filesystem::path premakeFilepath = std::filesystem::path("Projects") / m_Properties.ProjectNameBuffer / Project::GetProjectDirectory() / Project::GetAssetDirectory() / "Scripts/premake5.lua";

		std::ifstream premakeFile(premakeFilepath);
		VX_CORE_ASSERT(premakeFile.is_open(), "Failed to open premake file!");
		std::stringstream ss;
		ss << premakeFile.rdbuf();

		std::string premakeFileStr = ss.str();

		String::ReplaceToken(premakeFileStr, "$PROJECT_NAME$", m_Properties.ProjectNameBuffer);

		std::ofstream fout(premakeFilepath);
		fout << premakeFileStr;
		fout.close();
	}

	void LauncherLayer::GenerateProjectSolution()
	{
		std::string projectName = m_Properties.ProjectNameBuffer;
		FileSystem::SetWorkingDirectory("Projects/" + projectName + "/Assets/Scripts");
		Platform::LaunchProcess("Win64Gen.bat", "");
		ResetWorkingDirectory();
	}

	void LauncherLayer::BuildProjectSolution()
	{
		auto projectSolutionFilename = std::filesystem::path(m_Properties.ProjectNameBuffer);
		FileSystem::ReplaceExtension(projectSolutionFilename, ".sln");
		std::filesystem::path solutionPath = std::filesystem::path("..\\..\\") / "Projects" / m_Properties.ProjectNameBuffer / "Assets\\Scripts" / projectSolutionFilename;

		FileSystem::SetWorkingDirectory("Resources/HelperScripts");
		Platform::LaunchProcess("BuildSolution.bat", solutionPath.string().c_str());
		ResetWorkingDirectory();
	}

	void LauncherLayer::CreateProject()
	{
		if (!FileSystem::Exists(m_Properties.ProjectDirectoryBuffer))
		{
			FileSystem::CreateDirectoriesV(m_Properties.ProjectDirectoryBuffer);
		}

		CreateProjectFilesAndDirectories();
		VX_CORE_ASSERT(Project::GetActive(), "Project wasn't created properly!");
		CreatePremakeBuildScript();
		
		using namespace std::chrono_literals;
		auto sleep = [](auto time) { std::this_thread::sleep_for(time); };

		GenerateProjectSolution();
		sleep(500ms);

		BuildProjectSolution();
		sleep(500ms);
		SaveProjectToDisk();

		LaunchEditor();
		ResetInputFields();
	}

	void LauncherLayer::SaveProjectToDisk()
	{
		ProjectSerializer serializer(Project::GetActive());
		const bool success = serializer.Serialize(m_Properties.ProjectPath);
		VX_CORE_ASSERT(success, "Failed to serialize project!");
	}

	void LauncherLayer::LaunchEditor()
	{
		std::string projectPath = FileSystem::Relative(m_Properties.ProjectPath, m_Properties.WorkingDirectory).string();
		Platform::LaunchProcess(m_Properties.EditorPath.string().c_str(), projectPath.c_str());
	}

	void LauncherLayer::ResetInputFields()
	{
		memset(m_Properties.ProjectNameBuffer, 0, VX_MAX_PROJECT_NAME_LENGTH);
		memset(m_Properties.ProjectDirectoryBuffer, 0, VX_MAX_PROJECT_DIR_LENGTH);
	}

	void LauncherLayer::ResetWorkingDirectory()
	{
		FileSystem::SetWorkingDirectory(m_Properties.WorkingDirectory);
	}

}
