#pragma once

#include <Sparky.h>

namespace Sparky {

	class SettingsPanel
	{
	public:
		struct Settings
		{
			Math::vec4& ColliderColor;
			float& EditorCameraFOV;
			bool& ShowColliders;
			const std::filesystem::path& CurrentEditorScenePath;
			std::function<void()> LaunchRuntimeCallback;
			Math::vec3 ClearColor = Math::vec3{ 0.1f, 0.1f, 0.1f };
		};

		SettingsPanel() = default;
		SettingsPanel(const Settings& settings) : m_Settings(settings) { }

		void OnGuiRender(bool showDefault = false);
		void ShowPanel() { s_ShowPanel = true; }

	private:
		void RenderSettingOptions(uint32_t selectedSetting);

	private:
		Settings m_Settings;
		inline static bool s_ShowPanel = false;
	};

}
