#include "BuildSettingsPanel.h"

namespace Vortex {

	BuildSettingsPanel::BuildSettingsPanel(const SharedRef<Project>& project, const LaunchRuntimeFn& func)
		: m_ProjectProperties(project->GetProperties()), m_LaunchRuntimeFunc(func)
	{
		m_ProjectPath = Project::GetProjectFilepath();
		m_StartupScene = m_ProjectProperties.General.StartScene;
	}

	void BuildSettingsPanel::OnGuiRender()
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];
		auto largeFont = io.Fonts->Fonts[1];

		if (!s_ShowPanel)
			return;

		Gui::Begin("Build Settings", &s_ShowPanel);

		UI::BeginPropertyGrid();

		std::string projectPath = m_ProjectPath.string();
		UI::Property("Project Location", projectPath, true);

		UI::EndPropertyGrid();

		if (UI::PropertyGridHeader("Scenes in Build"))
		{
			const auto& buildIndices = Scene::GetScenesInBuild();

			if (Gui::BeginChild("##ScenesInBuild"))
			{
				auto contentRegionAvail = Gui::GetContentRegionAvail();

				uint32_t i = 0;

				int32_t buildIndexToRemove = -1;

				for (const auto& [buildIndex, sceneFilePath] : buildIndices)
				{
					auto contentRegionAvail = Gui::GetContentRegionAvail();

					UI::BeginPropertyGrid();

					size_t lastSlashPos = sceneFilePath.find_last_of("/\\");
					size_t lastDotPos = sceneFilePath.find_last_of('.');
					std::string sceneName = sceneFilePath.substr(lastSlashPos + 1, lastDotPos - (lastSlashPos + 1));

					UI::Property(std::to_string(i + 1).c_str(), sceneName, true);

					UI::EndPropertyGrid();

					Gui::SameLine();
					UI::ShiftCursor(-contentRegionAvail.x + (contentRegionAvail.x * 0.05f), 3.0f);
					std::string label = (const char*)VX_ICON_TIMES + std::string("##") + std::to_string(i);
					if (Gui::Button(label.c_str()))
					{
						buildIndexToRemove = i;
					}

					i++;
				}

				if (buildIndexToRemove != -1)
				{
					Scene::RemoveIndexFromBuild(buildIndexToRemove);
				}

				Gui::EndChild();
			}

			// Accept Items from the content browser
			if (Gui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					const wchar_t* path = (const wchar_t*)payload->Data;
					std::filesystem::path filePath = std::filesystem::path(path);

					if (filePath.extension().string() == ".vortex")
					{
						bool isNewScene = true;

						for (auto& [buildIndex, sceneFilepath] : buildIndices)
						{
							if (filePath.string().find(sceneFilepath) != std::string::npos)
							{
								isNewScene = false;
							}
						}

						if (isNewScene)
						{
							auto relativePath = FileSystem::Relative(filePath, Project::GetAssetDirectory());

							if (const bool isFirstSceneInBuild = m_ProjectProperties.BuildProps.BuildIndices.size() == 0)
							{
								m_ProjectProperties.General.StartScene = relativePath;
							}

							Scene::SubmitSceneToBuild(relativePath.string());
						}
					}
				}

				Gui::EndDragDropTarget();
			}

			UI::EndTreeNode();
		}

		if (UI::PropertyGridHeader("Window", false))
		{
			UI::BeginPropertyGrid();

			if (!m_ProjectProperties.BuildProps.Window.Maximized)
				UI::Property("Size", m_ProjectProperties.BuildProps.Window.Size);
			if (UI::Property("Force 16:9 Aspect Ratio", m_ProjectProperties.BuildProps.Window.ForceSixteenByNine))
			{
				if (m_ProjectProperties.BuildProps.Window.ForceSixteenByNine)
				{
					FindAndSetBestSize();
				}
			}

			UI::Property("Maximized", m_ProjectProperties.BuildProps.Window.Maximized);
			UI::Property("Decorated", m_ProjectProperties.BuildProps.Window.Decorated);
			UI::Property("Resizeable", m_ProjectProperties.BuildProps.Window.Resizeable);

			UI::EndPropertyGrid();
			UI::EndTreeNode();
		}

		UI::Draw::Underline();
		for (uint32_t i = 0; i < 3; i++)
			Gui::Spacing();

		if (Gui::Button("Build"))
		{
			// TODO build asset pack here
		}

		Gui::SameLine();

		if (Gui::Button("Build and Run"))
		{
			m_LaunchRuntimeFunc(m_ProjectPath);
		}
		
		Gui::End();
	}

	void BuildSettingsPanel::FindAndSetBestSize()
	{
		float width = m_ProjectProperties.BuildProps.Window.Size.x;
		float height = m_ProjectProperties.BuildProps.Window.Size.y;

		FindBestWidth(width);
		FindBestHeight(height);

		m_ProjectProperties.BuildProps.Window.Size = { width, height };
	}

	void BuildSettingsPanel::FindBestWidth(float& width)
	{
		if (width > 1600)
			width = 1600;

		if (width < 1600)
			width = 1600 * 1.5f;

		if (width < (1600 * 1.5f))
			width = 800;
	}

	void BuildSettingsPanel::FindBestHeight(float& height)
	{
		if (height > 900)
			height = 900;

		if (height < 900)
			height = 900 * 1.5f;

		if (height < (900 * 1.5f))
			height = 450;
	}

}
