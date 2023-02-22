#include "LauncherLayer.h"

#include <Vortex/Serialization/SceneSerializer.h>

namespace Vortex {

	LauncherLayer::LauncherLayer()
		: Layer("LauncherLayer") { }

	void LauncherLayer::OnAttach()
	{
		const auto& appProps = Application::Get().GetProperties();

		FramebufferProperties framebufferProps{};
		framebufferProps.Attachments = { ImageFormat::RGBA16F, ImageFormat::Depth };
		framebufferProps.Width = appProps.WindowWidth;
		framebufferProps.Height = appProps.WindowHeight;

		m_Framebuffer = Framebuffer::Create(framebufferProps);

		m_ViewportSize = Math::vec2((float)appProps.WindowWidth, (float)appProps.WindowHeight);

		ImageProperties imageProps;
		imageProps.Filepath = "Resources/Images/VortexLogo.png";
		imageProps.WrapMode = ImageWrap::Repeat;

		m_VortexLogoIcon = Texture2D::Create(imageProps);

		m_Properties.EditorPath = Application::Get().GetEditorBinaryPath();
		m_Properties.WorkingDirectory = Application::Get().GetProperties().WorkingDirectory;
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
			m_ProjectPath = std::filesystem::path();
			selectedProject = 0;
		}

		UI::Draw::Underline();

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
		UI::Draw::Underline();

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

		if (Gui::BeginPopupModal("Create New Project"))
		{
			static const char* options[] = { ProjectTypeToString(ProjectType::e2D), ProjectTypeToString(ProjectType::e3D) };
			int32_t selectedProjectType = (int32_t)m_Properties.Type;

			UI::BeginPropertyGrid();
			if (UI::PropertyDropdown("Project Type", options, VX_ARRAYCOUNT(options), selectedProjectType))
			{
				m_Properties.Type == (ProjectType)selectedProjectType;
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

			bool textBoxesEmpty = strlen(m_Properties.ProjectNameBuffer) == 0 && strlen(m_Properties.ProjectDirectoryBuffer) == 0;

			Gui::SameLine();
			Gui::SetCursorPosX((Gui::GetWindowWidth() * 0.5f) - (buttonSize.x * 0.5f));
			Gui::BeginDisabled(textBoxesEmpty);
			if (Gui::Button("Create Project", buttonSize))
			{
				CreateProject();
			}

			Gui::EndDisabled();

			Gui::EndPopup();
		}
	}

	void LauncherLayer::ResetInputFields()
	{
		memset(m_Properties.ProjectNameBuffer, 0, VX_MAX_PROJECT_NAME_LENGTH);
		memset(m_Properties.ProjectDirectoryBuffer, 0, VX_MAX_PROJECT_DIR_LENGTH);
	}

	void LauncherLayer::CreateProject()
	{
		if (!std::filesystem::exists(m_Properties.ProjectDirectoryBuffer))
			std::filesystem::create_directories(m_Properties.ProjectDirectoryBuffer);

		SharedRef<Project> project = Project::New();
		auto& projectProps = project->GetProperties();
		projectProps.General.Name = std::string(m_Properties.ProjectNameBuffer);
		projectProps.General.AssetDirectory = "Assets";
		projectProps.General.AssetRegistryPath = projectProps.General.AssetDirectory / "AssetRegistry.vxr";
		projectProps.General.StartScene = "Scenes/SampleScene.vortex";
		projectProps.ScriptingProps.ScriptBinaryPath = std::format("Scripts/Binaries/{}.dll", m_Properties.ProjectNameBuffer);
		auto projectFilename = std::format("{}.vxproject", projectProps.General.Name);
		m_ProjectPath = m_Properties.ProjectDirectoryBuffer / std::filesystem::path(projectFilename);
		std::filesystem::path projectDirectoryPath = m_ProjectPath.parent_path();
		std::filesystem::create_directories(projectDirectoryPath / "Assets/Scenes");
		std::filesystem::create_directories(projectDirectoryPath / "Assets/Scripts/Binaries");
		std::filesystem::create_directories(projectDirectoryPath / "Assets/Scripts/Source");
		std::filesystem::copy("Resources/NewProjectTemplate/premake5.lua", (m_Properties.ProjectDirectoryBuffer / std::filesystem::path("Assets/Scripts")).string());
		std::filesystem::copy("Resources/NewProjectTemplate/Win32Gen.bat", (m_Properties.ProjectDirectoryBuffer / std::filesystem::path("Assets/Scripts")).string());

		SharedRef<Scene> startScene = Scene::Create();
		if (m_Properties.Type == ProjectType::e2D)
			Scene::Create2DSampleScene(startScene);
		else
			Scene::Create3DSampleScene(startScene);
		SceneSerializer serializer(startScene);
		serializer.Serialize((m_Properties.ProjectDirectoryBuffer / std::filesystem::path("Assets/Scenes/SampleScene.vortex")).string());

		std::filesystem::path premakeFilepath = (m_Properties.ProjectDirectoryBuffer / projectProps.General.AssetDirectory) / "Scripts/premake5.lua";
		std::ifstream premakeFile(premakeFilepath);
		std::stringstream ss;
		ss << premakeFile.rdbuf();

		std::string str = ss.str();

		ReplaceToken(str, "$PROJECT_NAME$", m_Properties.ProjectNameBuffer);

		std::ofstream fout(premakeFilepath);
		fout << str;
		fout.close();

		Project::SaveActive(m_ProjectPath);

		std::filesystem::current_path("Projects/" + std::string(m_Properties.ProjectNameBuffer) + "/Assets/Scripts");

		FileSystem::LaunchApplication("Win32Gen.bat", "");

		ResetWorkingDirectory();

		// build project dll
		auto projectNameAsFilepath = std::filesystem::path(m_Properties.ProjectNameBuffer);
		projectNameAsFilepath.replace_extension(".sln");
		std::filesystem::path solutionPath = std::filesystem::path("..\\..\\") / "Projects" / m_Properties.ProjectNameBuffer / "Assets\\Scripts" / projectNameAsFilepath;

		std::filesystem::current_path("Resources/HelperScripts");

		FileSystem::LaunchApplication("BuildSolution.bat", solutionPath.string().c_str());

		ResetWorkingDirectory();

		LaunchEditor();

		ResetInputFields();

		Gui::CloseCurrentPopup();
	}

	void LauncherLayer::ReplaceToken(std::string& str, const char* token, const std::string& value)
	{
		size_t pos = 0;

		while ((pos = str.find(token, pos)) != std::string::npos)
		{
			size_t length = strlen(token);
			str.replace(pos, length, value);
			pos += length;
		}
	}

	void LauncherLayer::ResetWorkingDirectory()
	{
		std::filesystem::current_path(m_Properties.WorkingDirectory);
	}

	void LauncherLayer::LaunchEditor()
	{
		std::string projectPath = FileSystem::Relative(m_ProjectPath, m_Properties.WorkingDirectory).string();
		FileSystem::LaunchApplication(m_Properties.EditorPath.c_str(), projectPath.c_str());
	}

	const char* LauncherLayer::ProjectTypeToString(ProjectType type)
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
