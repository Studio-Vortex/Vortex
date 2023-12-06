#include "BuildSettingsPanel.h"

namespace Vortex {
	
	BuildSettingsPanel::BuildSettingsPanel(const BuildAndRunFn& func0, const BuildFn& func1)
		: m_BuildAndRunFn(func0), m_BuildFn(func1) { }

	void BuildSettingsPanel::OnEditorAttach()
	{
		m_ProjectPath = Project::GetProjectFilepath();
		m_StartupScene = Project::GetActive()->GetProperties().General.StartScene;
	}

	void BuildSettingsPanel::OnEditorDetach() { }

	void BuildSettingsPanel::OnGuiRender()
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];
		auto largeFont = io.Fonts->Fonts[1];

		if (!IsOpen)
			return;

		Gui::Begin(m_PanelName.c_str(), &IsOpen);

		UI::BeginPropertyGrid();

		std::string projectPath = m_ProjectPath.string();
		UI::Property("Project Location", projectPath, true);

		UI::EndPropertyGrid();

		SharedReference<Project> activeProject = Project::GetActive();
		ProjectProperties& projectProps = activeProject->GetProperties();

		if (UI::PropertyGridHeader("Scenes in Build"))
		{
			const auto& buildIndices = Scene::GetScenesInBuild();

			auto contentRegionAvail = Gui::GetContentRegionAvail();
			if (Gui::BeginChild("##ScenesInBuild", { 0, contentRegionAvail.y / 2.0f }))
			{
				contentRegionAvail = Gui::GetContentRegionAvail();

				uint32_t i = 0;

				int32_t buildIndexToRemove = -1;

				for (const auto& [buildIndex, sceneFilePath] : buildIndices)
				{
					contentRegionAvail = Gui::GetContentRegionAvail();

					UI::BeginPropertyGrid();

					size_t lastSlashPos = sceneFilePath.find_last_of("/\\");
					size_t lastDotPos = sceneFilePath.find_last_of('.');
					std::string sceneName = sceneFilePath.substr(lastSlashPos + 1, lastDotPos - (lastSlashPos + 1));

					UI::Property(std::to_string(i).c_str(), sceneName, true);

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
					const Fs::Path filePath = Fs::Path(path);

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

							if (const bool isFirstSceneInBuild = projectProps.BuildProps.BuildIndices.size() == 0)
							{
								projectProps.General.StartScene = relativePath;
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

			if (!projectProps.BuildProps.Window.Maximized)
				UI::Property("Size", projectProps.BuildProps.Window.Size);
			if (UI::Property("Force 16:9 Aspect Ratio", projectProps.BuildProps.Window.ForceSixteenByNine))
			{
				if (projectProps.BuildProps.Window.ForceSixteenByNine)
				{
					FindAndSetBestSize();
				}
			}

			UI::Property("Maximized", projectProps.BuildProps.Window.Maximized);
			UI::Property("Decorated", projectProps.BuildProps.Window.Decorated);
			UI::Property("Resizeable", projectProps.BuildProps.Window.Resizeable);

			UI::EndPropertyGrid();
			UI::EndTreeNode();
		}

		UI::Draw::Underline();
		for (uint32_t i = 0; i < 3; i++)
			Gui::Spacing();

		if (Gui::Button("Build"))
		{
			std::invoke(m_BuildFn);
		}

		Gui::SameLine();

		if (Gui::Button("Build and Run"))
		{
			std::invoke(m_BuildAndRunFn);
		}
		
		Gui::End();
	}

	void BuildSettingsPanel::FindAndSetBestSize()
	{
		SharedReference<Project> activeProject = Project::GetActive();
		ProjectProperties& projectProps = activeProject->GetProperties();

		float width = projectProps.BuildProps.Window.Size.x;
		float height = projectProps.BuildProps.Window.Size.y;

		FindBestWidth(width);
		FindBestHeight(height);

		projectProps.BuildProps.Window.Size = { width, height };
	}

	void BuildSettingsPanel::FindBestWidth(float& width)
	{
		if (width > 1600)
			width = 1600;

		if (width < 1600)
			width = 1600 * 0.75f;

		if (width < (1600 * 0.75f))
			width = 800;
	}

	void BuildSettingsPanel::FindBestHeight(float& height)
	{
		if (height > 900)
			height = 900;

		if (height < 900)
			height = 900 * 0.75f;

		if (height < (900 * 0.75f))
			height = 450;
	}

}
