#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Core/Math/Math.h"

#include "Vortex/Renderer/Color.h"
#include "Vortex/Physics/3D/PhysXTypes.h"

#include "Vortex/Utils/FileSystem.h"

#include <string>

namespace Vortex {

	struct VORTEX_API ProjectProperties
	{
		struct VORTEX_API GeneralProperties
		{
			std::string Name = "Untitled";
			std::filesystem::path AssetDirectory = "";
			std::filesystem::path AssetRegistryPath = "AssetRegistry.vxr";
			std::filesystem::path StartScene = "";
		} General;

		struct VORTEX_API BuildProperties
		{
			struct VORTEX_API WindowSettings
			{
				Math::vec2 Size = Math::vec2(1600.0f, 900.0f);
				bool ForceSixteenByNine = false;
				bool Maximized = false;
				bool Decorated = true;
				bool Resizeable = true;
			} Window;
		} BuildProps;

		struct VORTEX_API EditorProperties
		{
			uint32_t FrameStepCount = 1;
			float EditorCameraFOV = 45.0f;
			bool DrawEditorGrid = true;
			bool DrawEditorAxes = true;
			bool MaximizeOnPlay = false;
			bool ShowBoundingBoxes = false;
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
			float GizmoSize = 1.0f;
		} GizmoProps;

		struct VORTEX_API RendererProperties
		{
			std::string TriangleCullMode = "";
			float LineWidth = 1.5f;
			float EnvironmentMapResolution = 512.0f;
			float PrefilterMapResolution = 128.0f;
			float ShadowMapResolution = 1024.0f;
			float Exposure = 1.0f;
			float Gamma = 2.2f;
			Math::vec3 BloomThreshold = Math::vec3(0.2126f, 0.7152f, 0.0722f);
			uint32_t BloomSampleSize = 5;
			uint32_t RenderFlags = 0;
			bool UseVSync = true;
			bool DisplaySceneIconsInEditor = true;
		} RendererProps;

		struct VORTEX_API NetworkProperties
		{

		} NetworkProps;

		struct VORTEX_API PhysicsProperties
		{
			BroadphaseType BroadphaseModel = BroadphaseType::AutomaticBoxPrune;
			FrictionType FrictionModel = FrictionType::Patch;
			Math::vec4 Physics3DColliderColor = ColorToVec4(Color::Green);
			Math::vec3 Physics3DGravity = Math::vec3(0.0f, -9.81f, 0.0f);
			uint32_t Physics3DPositionIterations = 8;
			uint32_t Physics3DVelocityIterations = 2;
			Math::vec4 Physics2DColliderColor = Math::vec4{ (44.0f / 255.0f), (151.0f / 255.0f), (167.0f / 255.0f), 1.0f };
			Math::vec2 Physics2DGravity = Math::vec2(0.0f, -9.81f);
			uint32_t Physics2DPositionIterations = 2;
			uint32_t Physics2DVelocityIterations = 6;
			bool ShowColliders = false;
		} PhysicsProps;

		struct VORTEX_API ScriptingProperties
		{
			std::filesystem::path ScriptBinaryPath = "";
			uint32_t DebugListenerPort = 2550;
			bool EnableMonoDebugging = false;
			bool ReloadAssemblyOnPlay = false;
		} ScriptingProps;
	};

}
