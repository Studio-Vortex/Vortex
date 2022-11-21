#pragma once

#include <Sparky.h>
#include <Sparky/Renderer/EditorCamera.h>

#include "Panels/ProjectSettingsPanel.h"
#include "Panels/SceneHierarchyPanel.h"
#include "Panels/ContentBrowserPanel.h"
#include "Panels/ScriptRegistryPanel.h"
#include "Panels/MaterialViewerPanel.h"
#include "Panels/BuildSettingsPanel.h"
#include "Panels/AssetManagerPanel.h"
#include "Panels/ShaderEditorPanel.h"
#include "Panels/PerformancePanel.h"
#include "Panels/ConsolePanel.h"
#include "Panels/AboutPanel.h"

namespace Sparky {

	class EditorLayer : public Layer
	{
	public:
		EditorLayer();
		~EditorLayer() override = default;

		void OnAttach() override;
		void OnDetach() override;

		void OnUpdate(TimeStep ts) override;
		void OnGuiRender() override;
		void OnEvent(Event& e) override;

	private:
		void UI_Toolbar();
		void OnOverlayRender();
		bool OnKeyPressedEvent(KeyPressedEvent& e);
		bool OnMouseButtonPressedEvent(MouseButtonPressedEvent& e);
		bool OnMouseButtonReleasedEvent(MouseButtonReleasedEvent& e);

		void CreateStartingEntities();

		// Project

		void CreateNewProject();
		void OpenExistingProject();
		void OpenProject(const std::filesystem::path& path);
		void SaveProject();

		// Scene

		void CreateNewScene();
		void OpenExistingScene();
		void OpenScene(const std::filesystem::path& path);
		void SaveSceneAs();
		void SaveScene();

		void SerializeScene(SharedRef<Scene> scene, const std::filesystem::path& path);

		void OnScenePlay();
		void OnScenePause();
		void OnSceneResume();
		void OnSceneStop();
		void RestartScene();

		void OnSceneSimulate();
		void RestartSceneSimulation();

		// Audio

		void PauseAudioSources();
		void ResumeAudioSources();
		void StopAudioSources();
		void RestartAudioSources();

		// Gizmos

		void OnNoGizmoSelected();
		void OnTranslationToolSelected();
		void OnRotationToolSelected();
		void OnScaleToolSelected();

		// Editor Callbacks

		void OnLaunchRuntime(const std::filesystem::path& path);

		// Helper

		void DuplicateSelectedEntity();

	private:
		EditorCamera m_EditorCamera;
		SharedRef<Framebuffer> m_Framebuffer = nullptr;

		SharedRef<Scene> m_ActiveScene = nullptr;
		SharedRef<Scene> m_EditorScene = nullptr;
		SharedRef<Project> m_ActiveProject = nullptr;
		
		std::filesystem::path m_EditorScenePath;

		Entity m_HoveredEntity;

		Math::vec2 m_ViewportSize{};
		Math::vec2 m_ViewportBounds[2] = { Math::vec2() };
		Math::vec2 m_MousePosLastFrame = Math::vec2();
		Math::vec4 m_Physics3DColliderColor = ColorToVec4(Color::Green);
		Math::vec4 m_Physics2DColliderColor = Math::vec4{ (44.0f / 255.0f), (151.0f / 255.0f), (167.0f / 255.0f), 1.0f };

		float m_EditorCameraFOV = 45.0f;
		float m_EditorCameraFOVLastFrame = 0.0f;
		int32_t m_GizmoType = -1;

		bool m_ShowSceneCreateEntityMenu = false;

		bool m_DrawEditorGrid = true;
		bool m_DrawEditorAxes = true;
		bool m_EditorDebugViewEnabled = false;

		bool m_ShowPhysicsColliders = false;

		bool m_SceneViewportFocused = false;
		bool m_SceneViewportHovered = false;
		bool m_SceneViewportMaximized = false;
		bool m_MaximizeOnPlay = false;

		enum class SceneState
		{
			Edit = 0, Play = 1, Simulate = 2,
		};
		
		// Editor resources

		SharedRef<Texture2D> m_PlayIcon = nullptr;
		SharedRef<Texture2D> m_PauseIcon = nullptr;
		SharedRef<Texture2D> m_StopIcon = nullptr;
		SharedRef<Texture2D> m_SimulateIcon = nullptr;
		SharedRef<Texture2D> m_StepIcon = nullptr;
		SharedRef<Texture2D> m_LocalModeIcon = nullptr;
		SharedRef<Texture2D> m_WorldModeIcon = nullptr;
		SharedRef<Texture2D> m_SelectToolIcon = nullptr;
		SharedRef<Texture2D> m_TranslateToolIcon = nullptr;
		SharedRef<Texture2D> m_RotateToolIcon = nullptr;
		SharedRef<Texture2D> m_ScaleToolIcon = nullptr;

		uint32_t m_TranslationMode = 0; // Local mode

		std::vector<SharedRef<AudioSource>> m_AudioSourcesToResume = std::vector<SharedRef<AudioSource>>();

		// Panels

		ProjectSettingsPanel m_ProjectSettingsPanel;
		SceneHierarchyPanel m_SceneHierarchyPanel;
		ContentBrowserPanel m_ContentBrowserPanel;
		ScriptRegistryPanel m_ScriptRegistryPanel;
		MaterialViewerPanel m_MaterialViewerPanel;
		BuildSettingsPanel m_BuildSettingsPanel;
		AssetManagerPanel m_AssetManagerPanel;
		ShaderEditorPanel m_ShaderEditorPanel;
		PerformancePanel m_PerformancePanel;
		ConsolePanel m_ConsolePanel;
		AboutPanel m_AboutPanel;

		// Settings

		uint32_t m_FrameStepCount = 1;
		bool m_GizmosEnabled = true;
		bool m_GizmoSnapEnabled = true;
		bool m_OrthographicGizmos = false;
		float m_SnapValue = 0.5f;
		float m_RotationSnapValue = 45.0f;
		bool m_DrawGizmoGrid = false;
		float m_GizmoGridSize = 1.0f;

		ProjectProperties m_Settings{
			ProjectProperties::ScriptingProperties{ "Assets", "", false },
			ProjectProperties::PhysicsProperties{ m_Physics3DColliderColor, m_Physics2DColliderColor, m_ShowPhysicsColliders },
			ProjectProperties::EditorProperties{ m_FrameStepCount, m_DrawEditorGrid, m_DrawEditorAxes, m_EditorCameraFOV },
			ProjectProperties::GizmoProperties{ m_GizmosEnabled, m_OrthographicGizmos, m_GizmoSnapEnabled, m_SnapValue, m_RotationSnapValue, m_DrawGizmoGrid, m_GizmoGridSize },
		};

		SceneState m_SceneState = SceneState::Edit;
	};

}
