#pragma once

#include <Vortex.h>

namespace Vortex {

	class BuildSettingsPanel
	{
	public:
		using LaunchRuntimeFn = std::function<void(const std::filesystem::path&)>;

	public:
		BuildSettingsPanel(const LaunchRuntimeFn& callback);
		~BuildSettingsPanel() = default;

		void OnGuiRender();
		void SetProjectContext(SharedRef<Project> project) {}
		void SetSceneContext(SharedRef<Scene> scene) {}
		bool& IsOpen() { return s_ShowPanel; }

	private:
		inline static bool s_ShowPanel = false;

	private:
		std::filesystem::path m_ProjectPath;
		std::filesystem::path m_StartupScene;
		LaunchRuntimeFn m_LaunchRuntimeCallback = nullptr;
	};

}
