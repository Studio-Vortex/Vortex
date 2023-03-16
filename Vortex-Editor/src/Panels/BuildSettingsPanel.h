#pragma once

#include <Vortex.h>

namespace Vortex {

	class BuildSettingsPanel
	{
	public:
		using LaunchRuntimeFn = std::function<void(const std::filesystem::path&)>;

	public:
		BuildSettingsPanel(const SharedRef<Project>& project, const LaunchRuntimeFn& func);
		~BuildSettingsPanel() = default;

		void OnGuiRender();
		void SetProjectContext(SharedRef<Project> project) {}
		void SetSceneContext(const SharedReference<Scene>& scene) {}
		bool& IsOpen() { return s_ShowPanel; }

	private:
		void FindAndSetBestSize();
		void FindBestWidth(float& width);
		void FindBestHeight(float& height);

	private:
		inline static bool s_ShowPanel = false;

	private:
		ProjectProperties& m_ProjectProperties;
		LaunchRuntimeFn m_LaunchRuntimeFunc;

		std::filesystem::path m_ProjectPath;
		std::filesystem::path m_StartupScene;
	};

}
