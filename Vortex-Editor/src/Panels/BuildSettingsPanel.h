#pragma once

#include <Vortex.h>

#include <Vortex/Editor/EditorPanel.h>

namespace Vortex {

	class BuildSettingsPanel : public EditorPanel
	{
	public:
		using BuildAndRunFn = std::function<void()>;
		using BuildFn = std::function<void()>;

	public:
		BuildSettingsPanel(const BuildAndRunFn& func0, const BuildFn& func1);
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
		BuildAndRunFn m_BuildAndRunFn;
		BuildFn m_BuildFn;

		std::filesystem::path m_ProjectPath;
		std::filesystem::path m_StartupScene;
	};

}
