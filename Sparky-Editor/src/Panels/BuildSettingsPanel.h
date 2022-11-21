#pragma once

#include <Sparky.h>

namespace Sparky {

	class BuildSettingsPanel
	{
	private:
		using LaunchRuntimeFn = std::function<void(const std::filesystem::path&)>;

	public:
		BuildSettingsPanel() = default;
		
		void SetContext(const LaunchRuntimeFn& callback);

		void OnGuiRender(bool showDefault = false);
		bool& IsOpen() { return s_ShowPanel; }

	private:
		inline static bool s_ShowPanel = false;

	private:
		std::filesystem::path m_ProjectPath;
		LaunchRuntimeFn m_LaunchRuntimeCallback = nullptr;
	};

}
