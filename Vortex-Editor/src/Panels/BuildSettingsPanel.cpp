#include "BuildSettingsPanel.h"

namespace Vortex {
	
	BuildSettingsPanel::BuildSettingsPanel(const BuildAndRunFn& func0, const BuildFn& func1)
		: m_BuildAndRunFn(func0), m_BuildFn(func1) { }

	void BuildSettingsPanel::OnPanelAttach()
	{
		SharedReference<Project> project = Project::GetActive();
		const ProjectProperties& properties = project->GetProperties();

		m_ProjectPath = project->GetProjectFilepath();
		m_StartupScene = properties.General.StartScene;
	}

	void BuildSettingsPanel::OnPanelDetach() { }

	void BuildSettingsPanel::OnGuiRender()
	{
		if (!IsOpen)
			return;

		Gui::Begin(m_PanelName.c_str(), &IsOpen);

		UI::BeginPropertyGrid();

		std::string projectPath = m_ProjectPath.string();
		UI::Property("Project Location", projectPath, true);
		
		SharedReference<Project> project = Project::GetActive();
		ProjectProperties& properties = project->GetProperties();

		auto OnSceneSelectedFn = [&](const Fs::Path& filepath) {
			properties.General.StartScene = m_StartupScene = filepath;
		};

		auto OnSceneDroppedFn = [&](const Fs::Path& filepath) {
			AssetType type = Project::GetEditorAssetManager()->GetAssetTypeFromFilepath(filepath);
			if (type == AssetType::SceneAsset)
			{
				OnSceneSelectedFn(Project::GetEditorAssetManager()->GetMetadata(filepath).Filepath);
			}
			else
			{
				VX_CONSOLE_LOG_ERROR("[Editor] Trying to set start scene with invalid asset type: {}", Utils::StringFromAssetType(type));
			}
		};

		const std::string startScenePath = m_StartupScene.string();
		AssetHandle selectedHandle = 0;
		if (UI::PropertyAssetReference<Scene>("Start Scene", startScenePath, selectedHandle, OnSceneDroppedFn, Project::GetEditorAssetManager()->GetAssetRegistry()))
		{
			const AssetMetadata& metadata = Project::GetEditorAssetManager()->GetMetadata(selectedHandle);
			if (AssetManager::IsHandleValid(metadata.Handle))
			{
				OnSceneSelectedFn(metadata.Filepath);
			}
		}

		UI::EndPropertyGrid();

		if (UI::PropertyGridHeader("Window", false))
		{
			UI::BeginPropertyGrid();

			bool& maximized = properties.BuildProps.Window.Maximized;
			if (!maximized)
			{
				UI::Property("Size", properties.BuildProps.Window.Size);
			}

			bool& forceSixteenByNine = properties.BuildProps.Window.ForceSixteenByNine;
			if (UI::Property("Force 16:9 Aspect Ratio", forceSixteenByNine))
			{
				if (forceSixteenByNine)
					FindAndSetBestSize();
			}

			bool& decorated = properties.BuildProps.Window.Decorated;
			bool& resizeable = properties.BuildProps.Window.Resizeable;
			UI::Property("Maximized", maximized);
			UI::Property("Decorated", decorated);
			UI::Property("Resizeable", resizeable);

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
		SharedReference<Project> project = Project::GetActive();
		ProjectProperties& properties = project->GetProperties();

		float width = properties.BuildProps.Window.Size.x;
		float height = properties.BuildProps.Window.Size.y;

		FindBestWidth(width);
		FindBestHeight(height);

		properties.BuildProps.Window.Size = { width, height };
	}

	void BuildSettingsPanel::FindBestWidth(float& width)
	{
		if (width > 1600)
		{
			width = 1600;
			return;
		}

		if (width < 1200)
		{
			width = 800;
			return;
		}

		if (width < 1600)
		{
			width = 1200;
			return;
		}
	}

	void BuildSettingsPanel::FindBestHeight(float& height)
	{
		if (height > 900)
		{
			height = 900;
			return;
		}

		if (height < 675)
		{
			height = 450;
			return;
		}

		if (height < 900)
		{
			height = 675;
			return;
		}
	}

}
