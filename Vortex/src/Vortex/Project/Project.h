#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Core/Math.h"
#include "Vortex/Renderer/Color.h"

#include <filesystem>
#include <string>

namespace Vortex {

	struct ProjectProperties
	{
		struct {
			std::string Name = "Untitled";
			std::filesystem::path AssetDirectory = "";
			std::filesystem::path ScriptBinaryPath = "";
			std::filesystem::path StartScene = "";
			bool EnableMonoDebugging = false;
		} General;

		struct RendererProperties {
			bool DisplaySceneIconsInEditor = true;
			std::string TriangleCullMode = "";
		} RendererProps;

		struct PhysicsProperties
		{
			Math::vec4 Physics3DColliderColor = ColorToVec4(Color::Green);
			Math::vec4 Physics2DColliderColor = Math::vec4{ (44.0f / 255.0f), (151.0f / 255.0f), (167.0f / 255.0f), 1.0f };
			bool ShowColliders = false;
		} PhysicsProps;

		struct EditorProperties
		{
			uint32_t FrameStepCount = 1;
			bool DrawEditorGrid = true;
			bool DrawEditorAxes = true;
			bool MaximizeOnPlay = false;
			bool MuteAudioSources = false;
			bool ReloadAssemblyOnPlay = false;
			float EditorCameraFOV = 45.0f;
		} EditorProps;

		struct GizmoProperties
		{
			bool Enabled = true;
			bool IsOrthographic = false;
			bool SnapEnabled = true;
			float SnapValue = 0.5f;
			float RotationSnapValue = 45.0f;
			bool DrawGrid = false;
			float GridSize = 1.0f;
		} GizmoProps;
	};

	class Project
	{
	public:
		inline ProjectProperties& GetProperties() { return m_Properties; }
		inline const ProjectProperties& GetProperties() const { return m_Properties; }

		inline static SharedRef<Project> GetActive()
		{
			return s_ActiveProject;
		}

		inline static const std::filesystem::path& GetProjectDirectory()
		{
			VX_CORE_ASSERT(s_ActiveProject, "No active project!");
			return s_ActiveProject->m_ProjectDirectory;
		}

		inline static std::filesystem::path GetAssetDirectory()
		{
			VX_CORE_ASSERT(s_ActiveProject, "No active project!");
			return GetProjectDirectory() / s_ActiveProject->m_Properties.General.AssetDirectory;
		}

		inline static std::filesystem::path GetCacheDirectory()
		{
			VX_CORE_ASSERT(s_ActiveProject, "No active project!");
			return GetProjectDirectory() / "Cache";
		}

		// TODO: move to asset manager
		inline static std::filesystem::path GetAssetFileSystemPath(const std::filesystem::path& path)
		{
			VX_CORE_ASSERT(s_ActiveProject, "No active project!");
			return GetAssetDirectory() / path;
		}

		inline const std::string& GetName() const { return m_Properties.General.Name; }

		static SharedRef<Project> New();
		static SharedRef<Project> Load(const std::filesystem::path& path);
		static bool SaveActive(const std::filesystem::path& path);

	private:
		ProjectProperties m_Properties;
		std::filesystem::path m_ProjectDirectory = "";

		inline static SharedRef<Project> s_ActiveProject = nullptr;
	};

}
