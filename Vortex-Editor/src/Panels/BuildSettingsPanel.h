#pragma once

#include <Vortex.h>

#include <Vortex/Editor/EditorPanel.h>

namespace Vortex {

	class BuildSettingsPanel : public EditorPanel
	{
	public:
		using LaunchRuntimeFn = std::function<void(const std::filesystem::path&)>;

	public:
		BuildSettingsPanel(const LaunchRuntimeFn& func);
		~BuildSettingsPanel() override = default;

		void OnEditorAttach() override;
		void OnEditorDetach() override;

		void OnGuiRender() override;

		EDITOR_PANEL_TYPE(BuildSettings)

	private:
		void FindAndSetBestSize();
		void FindBestWidth(float& width);
		void FindBestHeight(float& height);

	private:
		LaunchRuntimeFn m_LaunchRuntimeFunc;

		std::filesystem::path m_ProjectPath;
		std::filesystem::path m_StartupScene;
	};

}
