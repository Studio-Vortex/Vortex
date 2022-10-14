#pragma once

#include <Sparky.h>

namespace Sparky {

	class SettingsPanel
	{
	public:
		struct Settings
		{
			// Physics
			Math::vec4& Physics2DColliderColor;
			bool& ShowColliders;

			// Editor
			bool& DrawEditorGrid;
			bool& DrawEditorAxes;
			float& EditorCameraFOV;
			Math::vec3& ClearColor;

			struct GizmoSettings
			{
				bool& Enabled;
				bool& IsOrthographic;
				bool& SnapEnabled;
				float& SnapValue;
				float& RotationSnapValue;
				bool& DrawGrid;
				float& GridSize;
			};

			GizmoSettings& Gizmos;

			// Build
			const std::filesystem::path& CurrentEditorScenePath;
			std::function<void()> LaunchRuntimeCallback;
		};

		SettingsPanel() = default;
		SettingsPanel(const Settings& settings) : m_Settings(settings) { }

		void OnGuiRender(bool showDefault = false);
		void ShowPanel() { s_ShowPanel = true; }

	private:
		Settings m_Settings;
		inline static bool s_ShowPanel = false;
	};

}
