#include "vxpch.h"
#include "EditorResources.h"

namespace Vortex {

	void EditorResources::Init()
	{
		SearchIcon = CreateEditorImage("Resources/Icons/General/Search.png");
		ClearIcon = CreateEditorImage("Resources/Icons/General/Clear.png");

		PlayIcon = CreateEditorImage("Resources/Icons/PlayButton.png");
		PauseIcon = CreateEditorImage("Resources/Icons/PauseButton.png");
		StopIcon = CreateEditorImage("Resources/Icons/StopButton.png");
		SimulateIcon = CreateEditorImage("Resources/Icons/SimulateButton.png");
		StepIcon = CreateEditorImage("Resources/Icons/StepButton.png");

		LocalModeIcon = CreateEditorImage("Resources/Icons/Scene/LocalMode.png");
		WorldModeIcon = CreateEditorImage("Resources/Icons/Scene/WorldMode.png");
		SelectToolIcon = CreateEditorImage("Resources/Icons/Scene/SelectTool.png");
		TranslateToolIcon = CreateEditorImage("Resources/Icons/Scene/TranslateTool.png");
		RotateToolIcon = CreateEditorImage("Resources/Icons/Scene/RotateTool.png");
		ScaleToolIcon = CreateEditorImage("Resources/Icons/Scene/ScaleTool.png");

		ShowGridIcon = CreateEditorImage("Resources/Icons/Scene/GridIcon.png");
		MaximizeOnPlayIcon = CreateEditorImage("Resources/Icons/Scene/MaximizeIcon.png");
		TopDownViewIcon = CreateEditorImage("Resources/Icons/Scene/TopDownCameraIcon.png");
		TwoDViewIcon = CreateEditorImage("Resources/Icons/Scene/2DViewIcon.png");
		PhysicsCollidersIcon = CreateEditorImage("Resources/Icons/Scene/PhysicsCollidersIcon.png");
		BoundingBoxesIcon = CreateEditorImage("Resources/Icons/Scene/BoundingBoxIcon.png");
		DisplaySceneIconsIcon = CreateEditorImage("Resources/Icons/Scene/SceneIconsIcon.png");
		MuteAudioSourcesIcons = CreateEditorImage("Resources/Icons/Scene/MuteAudioIcon.png");

		CameraIcon = CreateEditorImage("Resources/Icons/Scene/CameraIcon.png");
		PointLightIcon = CreateEditorImage("Resources/Icons/Scene/PointLight.png");
		SpotLightIcon = CreateEditorImage("Resources/Icons/Scene/SpotLight.png");
		SkyLightIcon = CreateEditorImage("Resources/Icons/Scene/SkyLightIcon.png");
		AudioSourceIcon = CreateEditorImage("Resources/Icons/Scene/AudioSourceIcon.png");

		CheckerboardIcon = CreateEditorImage("Resources/Icons/Inspector/Checkerboard.png");
		SettingsIcon = CreateEditorImage("Resources/Icons/Inspector/SettingsIcon.png");

		DirectoryIcon = CreateEditorImage("Resources/Icons/ContentBrowser/DirectoryIcon.png");
		AudioFileIcon = CreateEditorImage("Resources/Icons/ContentBrowser/AudioFileIcon.png");
		OBJIcon = CreateEditorImage("Resources/Icons/ContentBrowser/OBJIcon.png");
		FBXIcon = CreateEditorImage("Resources/Icons/ContentBrowser/FBXIcon.png");
		FontIcon = CreateEditorImage("Resources/Icons/ContentBrowser/FontIcon.png");
		CodeFileIcon = CreateEditorImage("Resources/Icons/ContentBrowser/CodeFileIcon.png");
		SceneIcon = CreateEditorImage("Resources/Icons/ContentBrowser/SceneIcon.png");
		FileIcon = CreateEditorImage("Resources/Icons/ContentBrowser/FileIcon.png");
	}

	SharedRef<Texture2D> EditorResources::CreateEditorImage(const std::string& filepath)
	{
		TextureProperties imageProps;
		imageProps.Filepath = filepath;
		imageProps.WrapMode = ImageWrap::Repeat;

		return Texture2D::Create(imageProps);
	}

}
