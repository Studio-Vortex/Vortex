#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Renderer/Texture.h"

#include <vector>

namespace Vortex {

	class VORTEX_API EditorResources
	{
	public:
		static void Init();
		static void Shutdown();

		// General
		inline static SharedReference<Texture2D> SearchIcon = nullptr;
		inline static SharedReference<Texture2D> ClearIcon = nullptr;

		// Scene
		inline static SharedReference<Texture2D> PlayIcon = nullptr;
		inline static SharedReference<Texture2D> PauseIcon = nullptr;
		inline static SharedReference<Texture2D> StopIcon = nullptr;
		inline static SharedReference<Texture2D> SimulateIcon = nullptr;
		inline static SharedReference<Texture2D> StepIcon = nullptr;

		inline static SharedReference<Texture2D> LocalModeIcon = nullptr;
		inline static SharedReference<Texture2D> WorldModeIcon = nullptr;

		inline static SharedReference<Texture2D> SelectToolIcon = nullptr;
		inline static SharedReference<Texture2D> TranslateToolIcon = nullptr;
		inline static SharedReference<Texture2D> RotateToolIcon = nullptr;
		inline static SharedReference<Texture2D> ScaleToolIcon = nullptr;

		inline static SharedReference<Texture2D> ShowGridIcon = nullptr;
		inline static SharedReference<Texture2D> MaximizeOnPlayIcon = nullptr;
		inline static SharedReference<Texture2D> TopDownViewIcon = nullptr;
		inline static SharedReference<Texture2D> TwoDViewIcon = nullptr;
		inline static SharedReference<Texture2D> PhysicsCollidersIcon = nullptr;
		inline static SharedReference<Texture2D> BoundingBoxesIcon = nullptr;
		inline static SharedReference<Texture2D> DisplaySceneIconsIcon = nullptr;
		inline static SharedReference<Texture2D> MuteAudioSourcesIcons = nullptr;

		inline static SharedReference<Texture2D> CameraIcon = nullptr;
		inline static SharedReference<Texture2D> PointLightIcon = nullptr;
		inline static SharedReference<Texture2D> SpotLightIcon = nullptr;
		inline static SharedReference<Texture2D> SkyLightIcon = nullptr;
		inline static SharedReference<Texture2D> AudioSourceIcon = nullptr;

		// Inspector
		inline static SharedReference<Texture2D> CheckerboardIcon = nullptr;
		inline static SharedReference<Texture2D> SettingsIcon = nullptr;

		// Content Browser
		inline static SharedReference<Texture2D> DirectoryIcon = nullptr;
		inline static SharedReference<Texture2D> AudioFileIcon = nullptr;
		inline static SharedReference<Texture2D> OBJIcon = nullptr;
		inline static SharedReference<Texture2D> FBXIcon = nullptr;
		inline static SharedReference<Texture2D> MaterialIcon = nullptr;
		inline static SharedReference<Texture2D> PrefabIcon = nullptr;
		inline static SharedReference<Texture2D> FontIcon = nullptr;
		inline static SharedReference<Texture2D> CodeFileIcon = nullptr;
		inline static SharedReference<Texture2D> SceneIcon = nullptr;
		inline static SharedReference<Texture2D> FileIcon = nullptr;

		static SharedReference<Texture2D> LoadTexture(const std::string& filepath);
	};

}
