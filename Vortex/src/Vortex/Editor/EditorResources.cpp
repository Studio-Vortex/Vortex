#include "vxpch.h"
#include "EditorResources.h"

namespace Vortex {

	void EditorResources::Init()
	{
		SearchIcon = Texture2D::Create("Resources/Icons/General/Search.png");
		ClearIcon = Texture2D::Create("Resources/Icons/General/Clear.png");

		PlayIcon = Texture2D::Create("Resources/Icons/PlayButton.png");
		PauseIcon = Texture2D::Create("Resources/Icons/PauseButton.png");
		StopIcon = Texture2D::Create("Resources/Icons/StopButton.png");
		SimulateIcon = Texture2D::Create("Resources/Icons/SimulateButton.png");
		StepIcon = Texture2D::Create("Resources/Icons/StepButton.png");

		LocalModeIcon = Texture2D::Create("Resources/Icons/Scene/LocalMode.png");
		WorldModeIcon = Texture2D::Create("Resources/Icons/Scene/WorldMode.png");
		SelectToolIcon = Texture2D::Create("Resources/Icons/Scene/SelectTool.png");
		TranslateToolIcon = Texture2D::Create("Resources/Icons/Scene/TranslateTool.png");
		RotateToolIcon = Texture2D::Create("Resources/Icons/Scene/RotateTool.png");
		ScaleToolIcon = Texture2D::Create("Resources/Icons/Scene/ScaleTool.png");

		ShowGridIcon = Texture2D::Create("Resources/Icons/Scene/GridIcon.png");
		MaximizeOnPlayIcon = Texture2D::Create("Resources/Icons/Scene/MaximizeIcon.png");
		TopDownViewIcon = Texture2D::Create("Resources/Icons/Scene/TopDownCameraIcon.png");
		TwoDViewIcon = Texture2D::Create("Resources/Icons/Scene/2DViewIcon.png");
		PhysicsCollidersIcon = Texture2D::Create("Resources/Icons/Scene/PhysicsCollidersIcon.png");
		BoundingBoxesIcon = Texture2D::Create("Resources/Icons/Scene/BoundingBoxIcon.png");
		DisplaySceneIconsIcon = Texture2D::Create("Resources/Icons/Scene/SceneIconsIcon.png");
		MuteAudioSourcesIcons = Texture2D::Create("Resources/Icons/Scene/MuteAudioIcon.png");

		CameraIcon = Texture2D::Create("Resources/Icons/Scene/CameraIcon.png");
		PointLightIcon = Texture2D::Create("Resources/Icons/Scene/PointLight.png");
		SpotLightIcon = Texture2D::Create("Resources/Icons/Scene/SpotLight.png");
		SkyLightIcon = Texture2D::Create("Resources/Icons/Scene/SkyLightIcon.png");
		AudioSourceIcon = Texture2D::Create("Resources/Icons/Scene/AudioSourceIcon.png");

		CheckerboardIcon = Texture2D::Create("Resources/Icons/Inspector/Checkerboard.png");
		SettingsIcon = Texture2D::Create("Resources/Icons/Inspector/SettingsIcon.png");
		DotsIcon = Texture2D::Create("Resources/Icons/Inspector/DotsIcon.png");

		DirectoryIcon = Texture2D::Create("Resources/Icons/ContentBrowser/DirectoryIcon.png");
		AudioFileIcon = Texture2D::Create("Resources/Icons/ContentBrowser/AudioFileIcon.png");
		OBJIcon = Texture2D::Create("Resources/Icons/ContentBrowser/OBJIcon.png");
		FBXIcon = Texture2D::Create("Resources/Icons/ContentBrowser/FBXIcon.png");
		FontIcon = Texture2D::Create("Resources/Icons/ContentBrowser/FontIcon.png");
		CodeFileIcon = Texture2D::Create("Resources/Icons/ContentBrowser/CodeFileIcon.png");
		SceneIcon = Texture2D::Create("Resources/Icons/ContentBrowser/SceneIcon.png");
		FileIcon = Texture2D::Create("Resources/Icons/ContentBrowser/FileIcon.png");
	}

}
