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
		
		std::filesystem::path m_EditorScenePath;

		Entity m_HoveredEntity;

		Math::vec2 m_ViewportSize{};
		Math::vec2 m_ViewportBounds[2] = { Math::vec2() };
		Math::vec2 m_MousePosLastFrame = Math::vec2();

		float m_EditorCameraFOVLastFrame = 0.0f;
		int32_t m_GizmoType = -1;

		bool m_ShowSceneCreateEntityMenu = false;
		bool m_EditorDebugViewEnabled = false;
		bool m_SceneViewportFocused = false;
		bool m_SceneViewportHovered = false;
		bool m_SceneViewportMaximized = false;
		
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

		SharedRef<ProjectSettingsPanel> m_ProjectSettingsPanel = nullptr;
		SceneHierarchyPanel m_SceneHierarchyPanel;
		SharedRef<ContentBrowserPanel> m_ContentBrowserPanel = nullptr;
		ScriptRegistryPanel m_ScriptRegistryPanel;
		MaterialViewerPanel m_MaterialViewerPanel;
		BuildSettingsPanel m_BuildSettingsPanel;
		AssetManagerPanel m_AssetManagerPanel;
		ShaderEditorPanel m_ShaderEditorPanel;
		PerformancePanel m_PerformancePanel;
		ConsolePanel m_ConsolePanel;
		AboutPanel m_AboutPanel;

		// Settings

		enum class SceneState
		{
			Edit = 0, Play = 1, Simulate = 2,
		};

		SceneState m_SceneState = SceneState::Edit;
	};

}
