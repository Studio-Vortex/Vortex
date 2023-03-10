#pragma once

#include "Vortex/Renderer/Texture.h"

namespace Vortex {

	class EditorResources
	{
	public:
		static void Init();

		// General
		inline static SharedRef<Texture2D> SearchIcon = nullptr;
		inline static SharedRef<Texture2D> ClearIcon = nullptr;

		// Scene
		inline static SharedRef<Texture2D> PlayIcon = nullptr;
		inline static SharedRef<Texture2D> PauseIcon = nullptr;
		inline static SharedRef<Texture2D> StopIcon = nullptr;
		inline static SharedRef<Texture2D> SimulateIcon = nullptr;
		inline static SharedRef<Texture2D> StepIcon = nullptr;

		inline static SharedRef<Texture2D> LocalModeIcon = nullptr;
		inline static SharedRef<Texture2D> WorldModeIcon = nullptr;

		inline static SharedRef<Texture2D> SelectToolIcon = nullptr;
		inline static SharedRef<Texture2D> TranslateToolIcon = nullptr;
		inline static SharedRef<Texture2D> RotateToolIcon = nullptr;
		inline static SharedRef<Texture2D> ScaleToolIcon = nullptr;

		inline static SharedRef<Texture2D> ShowGridIcon = nullptr;
		inline static SharedRef<Texture2D> MaximizeOnPlayIcon = nullptr;
		inline static SharedRef<Texture2D> TopDownViewIcon = nullptr;
		inline static SharedRef<Texture2D> TwoDViewIcon = nullptr;
		inline static SharedRef<Texture2D> PhysicsCollidersIcon = nullptr;
		inline static SharedRef<Texture2D> BoundingBoxesIcon = nullptr;
		inline static SharedRef<Texture2D> DisplaySceneIconsIcon = nullptr;
		inline static SharedRef<Texture2D> MuteAudioSourcesIcons = nullptr;

		inline static SharedRef<Texture2D> CameraIcon = nullptr;
		inline static SharedRef<Texture2D> PointLightIcon = nullptr;
		inline static SharedRef<Texture2D> SpotLightIcon = nullptr;
		inline static SharedRef<Texture2D> SkyLightIcon = nullptr;
		inline static SharedRef<Texture2D> AudioSourceIcon = nullptr;

		// Inspector
		inline static SharedRef<Texture2D> CheckerboardIcon = nullptr;
		inline static SharedRef<Texture2D> SettingsIcon = nullptr;

		// Content Browser
		inline static SharedRef<Texture2D> DirectoryIcon = nullptr;
		inline static SharedRef<Texture2D> AudioFileIcon = nullptr;
		inline static SharedRef<Texture2D> OBJIcon = nullptr;
		inline static SharedRef<Texture2D> FBXIcon = nullptr;
		inline static SharedRef<Texture2D> FontIcon = nullptr;
		inline static SharedRef<Texture2D> CodeFileIcon = nullptr;
		inline static SharedRef<Texture2D> SceneIcon = nullptr;
		inline static SharedRef<Texture2D> FileIcon = nullptr;

		static SharedRef<Texture2D> CreateEditorImage(const std::string& filepath);
	};

}
