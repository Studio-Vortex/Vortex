#include "LauncherLayer.h"

#include <Vortex/Serialization/SceneSerializer.h>
#include <Vortex/Project/ProjectSerializer.h>

namespace Vortex {

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

		m_Properties.EditorAppFilepath = application.GetEditorBinaryPath();
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
		ImGuiViewport* viewport = Gui::GetMainViewport();
		Gui::SetNextWindowPos(viewport->WorkPos);
		Gui::SetNextWindowSize(viewport->WorkSize);
		Gui::SetNextWindowViewport(viewport->ID);

		// Set the viewport size
		m_ViewportSize = { viewport->WorkSize.x, viewport->WorkSize.y };

		UI::ScopedStyle windowRounding(ImGuiStyleVar_WindowRounding, 0.0f);
		UI::ScopedStyle windowBorderSize(ImGuiStyleVar_WindowBorderSize, 1.0f);
		UI::ScopedStyle windowPadding(ImGuiStyleVar_WindowPadding, ImVec2{ 10.0f, 10.0f });

		Gui::Begin("Project Browser", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDecoration);

		Gui::BeginGroup();

		static int selectedProject = 0;
		uint32_t i = 0;

		UI::PushFont("Large");
		Gui::Text("Select A Project");
		UI::PopFont();
		Gui::Spacing();

		Gui::BeginChild("Projects", ImVec2(250, 0), true);

		if (Gui::Selectable("New Project", selectedProject == 0))
		{
			m_Properties.ProjectFilepath = Fs::Path();
			m_SelectedProjectIcon = nullptr;
			selectedProject = 0;
		}

		if (Gui::IsItemFocused() && Gui::IsKeyPressed(ImGuiKey_Enter))
		{
			selectedProject = 0; // 0 is the 'New Project' option
		}

		if (Gui::IsItemHovered() && Gui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
		{
			selectedProject = 0;
			m_IsCreatingNewProject = true;
			Gui::OpenPopup("Create New Project");
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

			if (Gui::Selectable(projectName.c_str(), selectedProject == i) || (Gui::IsItemFocused() && Gui::IsKeyPressed(ImGuiKey_Enter)))
			{
				m_Properties.ProjectFilepath = directoryEntry.path();
				std::string projectIconName = projectName + ".png";
				const Fs::Path projectDirectory = FileSystem::GetParentDirectory(m_Properties.ProjectFilepath);
				
				m_SelectedProjectIcon = nullptr;

				// Find the project image on disk
				for (const auto& projectEntry : std::filesystem::directory_iterator(projectDirectory))
				{
					if (projectEntry.path().string().find(projectIconName) == std::string::npos)
						continue;
					
					TextureProperties imageProps;
					imageProps.Filepath = (projectDirectory / projectIconName).string();
					imageProps.WrapMode = ImageWrap::Repeat;

					m_SelectedProjectIcon = Texture2D::Create(imageProps);

					// we found the texture
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

		UI::PushFont("Huge");
		Gui::TextCentered("Vortex Game Engine", 26.0f);
		UI::PopFont();

		for (uint32_t i = 0; i < 2; i++)
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

		{
			UI::ScopedColor buttonColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.8f, 1.0f));
			UI::ScopedColor buttonActiveColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.2f, 0.8f, 1.0f));
			UI::ScopedColor buttonHoveredColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.2f, 1.0f, 1.0f));

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
					LaunchEditorApp();
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
		}

		Gui::EndChild();
		
		Gui::End();
	}

	void LauncherLayer::OnEvent(Event& e) { }

	void LauncherLayer::DisplayCreateProjectPopup()
	{
		ImVec2 center = Gui::GetMainViewport()->GetCenter();
		Gui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		Gui::SetNextWindowSize(ImVec2(700, 0));
		ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize;

		if (Gui::BeginPopupModal("Create New Project", &m_IsCreatingNewProject, flags))
		{
			UI::BeginPropertyGrid();

			std::string nameBuffer(m_Properties.ProjectNameBuffer);
			if (UI::Property("Project Name", nameBuffer))
			{
				ClearProjectNameBuffer();
				memcpy(m_Properties.ProjectNameBuffer, nameBuffer.data(), nameBuffer.size());
			}

			auto folderClickedFn = [&]()
			{
				std::string filepath = FileDialogue::OpenFolderDialog();
				ClearProjectDirectoryBuffer();
				memcpy(m_Properties.ProjectDirectoryBuffer, filepath.data(), filepath.size());
			};

			std::string directoryBuffer(m_Properties.ProjectDirectoryBuffer);
			if (UI::PropertyWithButton("Directory", (const char*)VX_ICON_FOLDER, directoryBuffer, folderClickedFn))
			{
				ClearProjectDirectoryBuffer();
				memcpy(m_Properties.ProjectDirectoryBuffer, directoryBuffer.data(), directoryBuffer.size());
			}

			static const char* options[] = { Utils::ProjectTypeToString(ProjectType::e2D), Utils::ProjectTypeToString(ProjectType::e3D) };
			int32_t currentProjectType = (int32_t)m_Properties.ProjectType;

			if (UI::PropertyDropdown("Project Type", options, VX_ARRAYSIZE(options), currentProjectType))
			{
				m_Properties.ProjectType = (ProjectType)currentProjectType;
			}

			UI::EndPropertyGrid();

			ImVec2 buttonSize = { Gui::GetContentRegionAvail().x / 4.0f, 50.0f };

			if (Gui::Button("Go Back", buttonSize))
			{
				ResetInputFields();
				Gui::CloseCurrentPopup();
			}

			const bool anyInputEmpty = strlen(m_Properties.ProjectNameBuffer) == 0 || strlen(m_Properties.ProjectDirectoryBuffer) == 0;

			Gui::SameLine();

			UI::ShiftCursorX((Gui::GetWindowWidth() * 0.5f) - (buttonSize.x * 0.5f));
			Gui::BeginDisabled(anyInputEmpty);
			if (Gui::Button("Create Project", buttonSize))
			{
				CreateProject();
				Gui::CloseCurrentPopup();
			}
			Gui::EndDisabled();

			Gui::EndPopup();
		}
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
		auto sleep = [](auto duration) { std::this_thread::sleep_for(duration); };

		GenerateProjectSolution();
		sleep(500ms);

		BuildProjectSolution();
		sleep(500ms);
		SaveProjectToDisk();

		LaunchEditorApp();
		ResetInputFields();
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

		const Fs::Path projectFilename = std::format("{}.vxproject", projectProps.General.Name);
		m_Properties.ProjectFilepath = m_Properties.ProjectDirectoryBuffer / Fs::Path(projectFilename);
		const Fs::Path projectDirectory = FileSystem::GetParentDirectory(m_Properties.ProjectFilepath);

		FileSystem::CreateDirectoriesV(projectDirectory / "Assets/Scripts/Binaries");
		FileSystem::CreateDirectoriesV(projectDirectory / "Assets/Scripts/Source");
		FileSystem::CreateDirectoriesV(projectDirectory / "Assets/Scenes");
		FileSystem::CreateDirectoriesV(projectDirectory / "Assets/Materials");
		const Fs::Path assetDirectory = projectDirectory / "Assets";
		FileSystem::RecursiveDirectoryCopy("Resources/NewProjectTemplate", assetDirectory);
		
		const bool is2DProject = m_Properties.ProjectType == ProjectType::e2D;
		const Fs::Path sceneTemplateFilepath = Fs::Path("Resources/Templates") / (is2DProject ? "SampleScene2DTemplate.vortex" : "SampleScene3DTemplate.vortex");

		const std::string contents = FileSystem::ReadText(sceneTemplateFilepath);

		const Fs::Path startSceneFilepath = projectProps.General.StartScene;
		std::ofstream fout(assetDirectory / startSceneFilepath);
		VX_CORE_ASSERT(fout.is_open(), "Failed to open file!");
		fout.write(contents.data(), contents.size());
		fout.close();
	}

	void LauncherLayer::CreatePremakeBuildScript()
	{
		const Fs::Path assetDir = Project::GetAssetDirectory();
		const Fs::Path buildScriptFilepath = Fs::Path("Projects") / m_Properties.ProjectNameBuffer / assetDir / "Scripts/premake5.lua";

		std::ifstream premakeFile(buildScriptFilepath);
		VX_CORE_ASSERT(premakeFile.is_open(), "Failed to open premake file!");
		std::stringstream ss;
		ss << premakeFile.rdbuf();

		std::string contents = ss.str();

		String::ReplaceToken(contents, "$PROJECT_NAME$", m_Properties.ProjectNameBuffer);

		std::ofstream fout(buildScriptFilepath);
		VX_CORE_ASSERT(fout.is_open(), "Failed to open file!");
		fout.write(contents.data(), contents.size());
		fout.close();
	}

	void LauncherLayer::GenerateProjectSolution()
	{
		const std::string projectName = m_Properties.ProjectNameBuffer;
		FileSystem::SetWorkingDirectory("Projects/" + projectName + "/Assets/Scripts");
		
		Platform::LaunchProcess("Win64-GenProject.bat", "");
		
		ResetWorkingDirectory();
	}

	void LauncherLayer::BuildProjectSolution()
	{
		Fs::Path projectSolutionFilename = Fs::Path(m_Properties.ProjectNameBuffer);
		FileSystem::ReplaceExtension(projectSolutionFilename, ".sln");
		const Fs::Path solutionPath = Fs::Path("..\\..\\") / "Projects" / m_Properties.ProjectNameBuffer / "Assets\\Scripts" / projectSolutionFilename;

		FileSystem::SetWorkingDirectory("Resources/HelperScripts");
		Platform::LaunchProcess("Win64-BuildSolution.bat", solutionPath.string().c_str());

		ResetWorkingDirectory();
	}

	void LauncherLayer::SaveProjectToDisk()
	{
		ProjectSerializer serializer(Project::GetActive());
		const bool success = serializer.Serialize(m_Properties.ProjectFilepath);
		VX_CORE_ASSERT(success, "Failed to save project to disk!");
	}

	void LauncherLayer::LaunchEditorApp()
	{
		const std::string projectPath = FileSystem::Relative(m_Properties.ProjectFilepath, m_Properties.WorkingDirectory).string();
		const std::string editorAppPath = m_Properties.EditorAppFilepath.string();

		Platform::LaunchProcess(editorAppPath.c_str(), projectPath.c_str());
	}

	void LauncherLayer::ResetInputFields()
	{
		ClearProjectNameBuffer();
		ClearProjectDirectoryBuffer();
	}

	void LauncherLayer::ClearProjectNameBuffer()
	{
		memset(m_Properties.ProjectNameBuffer, 0, VX_MAX_PROJECT_NAME_LENGTH);
	}

	void LauncherLayer::ClearProjectDirectoryBuffer()
	{
		memset(m_Properties.ProjectDirectoryBuffer, 0, VX_MAX_PROJECT_DIR_LENGTH);
	}

	void LauncherLayer::ResetWorkingDirectory()
	{
		FileSystem::SetWorkingDirectory(m_Properties.WorkingDirectory);
	}

}
