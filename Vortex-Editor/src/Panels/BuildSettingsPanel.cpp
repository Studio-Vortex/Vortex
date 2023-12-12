#include "BuildSettingsPanel.h"

namespace Vortex {
	
	BuildSettingsPanel::BuildSettingsPanel(const BuildAndRunFn& func0, const BuildFn& func1)
		: m_BuildAndRunFn(func0), m_BuildFn(func1) { }

	void BuildSettingsPanel::OnPanelAttach()
	{
		m_ProjectPath = Project::GetProjectFilepath();
		m_StartupScene = Project::GetActive()->GetProperties().General.StartScene;
	}

	void BuildSettingsPanel::OnPanelDetach() { }

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

		std::string startScenePath = m_StartupScene.string();
		std::vector<std::string> sceneFilepathStrings;
		std::vector<const char*> sceneFilepaths;

		const AssetRegistry& assetRegistry = Project::GetEditorAssetManager()->GetAssetRegistry();
		for (const auto& [assetHandle, metadata] : assetRegistry)
		{
			if (metadata.Type != AssetType::SceneAsset)
				continue;

			std::string sceneFilepath = metadata.Filepath.string();
			sceneFilepathStrings.push_back(sceneFilepath);
			sceneFilepaths.push_back(sceneFilepathStrings.back().c_str());
		}

		if (UI::PropertyDropdownSearch("Start Scene", sceneFilepaths.data(), sceneFilepaths.size(), startScenePath, m_StartSceneSearchTextFilter))
		{
			Project::GetActive()->GetProperties().General.StartScene = m_StartupScene = Fs::Path(startScenePath);
		}

		UI::EndPropertyGrid();

		SharedReference<Project> project = Project::GetActive();
		ProjectProperties& properties = project->GetProperties();

		if (UI::PropertyGridHeader("Window", false))
		{
			UI::BeginPropertyGrid();

			if (!properties.BuildProps.Window.Maximized)
			{
				UI::Property("Size", properties.BuildProps.Window.Size);
			}

			if (UI::Property("Force 16:9 Aspect Ratio", properties.BuildProps.Window.ForceSixteenByNine))
			{
				if (properties.BuildProps.Window.ForceSixteenByNine)
				{
					FindAndSetBestSize();
				}
			}

			UI::Property("Maximized", properties.BuildProps.Window.Maximized);
			UI::Property("Decorated", properties.BuildProps.Window.Decorated);
			UI::Property("Resizeable", properties.BuildProps.Window.Resizeable);

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
