#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Core/Math.h"
#include "Vortex/Renderer/Color.h"
#include "Vortex/Physics/3D/PhysXTypes.h"
#include "Vortex/Core/ReferenceCounting/SharedRef.h"

#include <filesystem>
#include <string>

namespace Vortex {

	struct VORTEX_API ProjectProperties
	{
		struct VORTEX_API GeneralProperties {
			std::string Name = "Untitled";
			std::filesystem::path AssetDirectory = "";
			std::filesystem::path StartScene = "";
		} General;

		struct VORTEX_API RendererProperties {
			std::string TriangleCullMode = "";
			float LineWidth = 1.5f;
			float EnvironmentMapResolution = 512.0f;
			float PrefilterMapResolution = 128.0f;
			float ShadowMapResolution = 1024.0f;
			float Exposure = 1.0f;
			float Gamma = 2.2f;
			bool UseVSync = true;
			bool DisplaySceneIconsInEditor = true;
		} RendererProps;

		struct VORTEX_API PhysicsProperties
		{
			Math::vec4 Physics3DColliderColor = ColorToVec4(Color::Green);
			BroadphaseType BroadphaseModel = BroadphaseType::AutomaticBoxPrune;
			FrictionType FrictionModel = FrictionType::Patch;
			Math::vec4 Physics2DColliderColor = Math::vec4{ (44.0f / 255.0f), (151.0f / 255.0f), (167.0f / 255.0f), 1.0f };
			bool ShowColliders = false;
		} PhysicsProps;

		struct VORTEX_API ScriptingProperties
		{
			std::filesystem::path ScriptBinaryPath = "";
			bool EnableMonoDebugging = false;
			bool ReloadAssemblyOnPlay = false;
		} ScriptingProps;

		struct VORTEX_API EditorProperties
		{
			uint32_t FrameStepCount = 1;
			float EditorCameraFOV = 45.0f;
			bool DrawEditorGrid = true;
			bool DrawEditorAxes = true;
			bool MaximizeOnPlay = false;
			bool MuteAudioSources = false;
		} EditorProps;

		struct VORTEX_API GizmoProperties
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

	class VORTEX_API Project : public RefCounted
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
