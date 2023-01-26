#pragma once

#include <Vortex.h>
#include <Vortex/Editor/EditorPanel.h>

namespace Vortex {

	class BuildSettingsPanel : public EditorPanel
	{
	public:
		using LaunchRuntimeFn = std::function<void(const std::filesystem::path&)>;

	public:
		BuildSettingsPanel(const LaunchRuntimeFn& callback);
		~BuildSettingsPanel() override = default;

		void OnGuiRender() override;
		void SetProjectContext(SharedRef<Project> project) override {}
		void SetSceneContext(SharedRef<Scene> scene) override {}
		bool& IsOpen() { return s_ShowPanel; }

	private:
		inline static bool s_ShowPanel = false;

	private:
		std::filesystem::path m_ProjectPath;
		std::filesystem::path m_StartupScene;
		LaunchRuntimeFn m_LaunchRuntimeCallback = nullptr;
	};

}
