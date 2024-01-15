#include "vxpch.h"
#include "EditorResources.h"

namespace Vortex {

	void EditorResources::Init()
	{
		SearchIcon = LoadTexture("Resources/Icons/General/Search.png");
		ClearIcon = LoadTexture("Resources/Icons/General/Clear.png");

		PlayIcon = LoadTexture("Resources/Icons/PlayButton.png");
		PauseIcon = LoadTexture("Resources/Icons/PauseButton.png");
		StopIcon = LoadTexture("Resources/Icons/StopButton.png");
		SimulateIcon = LoadTexture("Resources/Icons/SimulateButton.png");
		StepIcon = LoadTexture("Resources/Icons/StepButton.png");

		LocalModeIcon = LoadTexture("Resources/Icons/Scene/LocalMode.png");
		WorldModeIcon = LoadTexture("Resources/Icons/Scene/WorldMode.png");
		SelectToolIcon = LoadTexture("Resources/Icons/Scene/SelectTool.png");
		TranslateToolIcon = LoadTexture("Resources/Icons/Scene/TranslateTool.png");
		RotateToolIcon = LoadTexture("Resources/Icons/Scene/RotateTool.png");
		ScaleToolIcon = LoadTexture("Resources/Icons/Scene/ScaleTool.png");

		ShowGridIcon = LoadTexture("Resources/Icons/Scene/GridIcon.png");
		MaximizeOnPlayIcon = LoadTexture("Resources/Icons/Scene/MaximizeIcon.png");
		TopDownViewIcon = LoadTexture("Resources/Icons/Scene/TopDownCameraIcon.png");
		TwoDViewIcon = LoadTexture("Resources/Icons/Scene/2DViewIcon.png");
		PhysicsCollidersIcon = LoadTexture("Resources/Icons/Scene/PhysicsCollidersIcon.png");
		BoundingBoxesIcon = LoadTexture("Resources/Icons/Scene/BoundingBoxIcon.png");
		DisplaySceneIconsIcon = LoadTexture("Resources/Icons/Scene/SceneIconsIcon.png");
		MuteAudioSourcesIcons = LoadTexture("Resources/Icons/Scene/MuteAudioIcon.png");

		CameraIcon = LoadTexture("Resources/Icons/Scene/CameraIcon.png");
		PointLightIcon = LoadTexture("Resources/Icons/Scene/PointLight.png");
		SpotLightIcon = LoadTexture("Resources/Icons/Scene/SpotLight.png");
		SkyLightIcon = LoadTexture("Resources/Icons/Scene/SkyLightIcon.png");
		AudioSourceIcon = LoadTexture("Resources/Icons/Scene/AudioSourceIcon.png");

		CheckerboardIcon = LoadTexture("Resources/Icons/Inspector/Checkerboard.png");
		SettingsIcon = LoadTexture("Resources/Icons/Inspector/SettingsIcon.png");

		DirectoryIcon = LoadTexture("Resources/Icons/ContentBrowser/DirectoryIcon.png");
		AudioFileIcon = LoadTexture("Resources/Icons/ContentBrowser/AudioFileIcon.png");
		OBJIcon = LoadTexture("Resources/Icons/ContentBrowser/OBJIcon.png");
		FBXIcon = LoadTexture("Resources/Icons/ContentBrowser/FBXIcon.png");
		FontIcon = LoadTexture("Resources/Icons/ContentBrowser/FontIcon.png");
		MaterialIcon = LoadTexture("Resources/Icons/ContentBrowser/MaterialIcon.png");
		PrefabIcon = LoadTexture("Resources/Icons/ContentBrowser/PrefabIcon.png");
		CodeFileIcon = LoadTexture("Resources/Icons/ContentBrowser/CodeFileIcon.png");
		SceneIcon = LoadTexture("Resources/Icons/ContentBrowser/SceneIcon.png");
		FileIcon = LoadTexture("Resources/Icons/ContentBrowser/FileIcon.png");
	}

    void EditorResources::Shutdown()
    {
		SearchIcon.Reset();
		ClearIcon.Reset();

		PlayIcon.Reset();
		PauseIcon.Reset();
		StopIcon.Reset();
		SimulateIcon.Reset();
		StepIcon.Reset();

		LocalModeIcon.Reset();
		WorldModeIcon.Reset();
		SelectToolIcon.Reset();
		TranslateToolIcon.Reset();
		RotateToolIcon.Reset();
		ScaleToolIcon.Reset();

		ShowGridIcon.Reset();
		MaximizeOnPlayIcon.Reset();
		TopDownViewIcon.Reset();
		TwoDViewIcon.Reset();
		PhysicsCollidersIcon.Reset();
		BoundingBoxesIcon.Reset();
		DisplaySceneIconsIcon.Reset();
		MuteAudioSourcesIcons.Reset();
		
		CameraIcon.Reset();
		PointLightIcon.Reset();
		SpotLightIcon.Reset();
		SkyLightIcon.Reset();
		AudioSourceIcon.Reset();
		
		CheckerboardIcon.Reset();
		SettingsIcon.Reset();
		
		DirectoryIcon.Reset();
		AudioFileIcon.Reset();
		OBJIcon.Reset();
		FBXIcon.Reset();
		FontIcon.Reset();
		MaterialIcon.Reset();
		PrefabIcon.Reset();
		CodeFileIcon.Reset();
		SceneIcon.Reset();
		FileIcon.Reset();
    }

	SharedReference<Texture2D> EditorResources::LoadTexture(const std::string& filepath)
	{
		TextureProperties imageProps;
		imageProps.Filepath = filepath;
		imageProps.WrapMode = ImageWrap::Repeat;

		return Texture2D::Create(imageProps);
	}

}
