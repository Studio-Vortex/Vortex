#pragma once

#include <Vortex.h>

#include "Panels/ProjectSettingsPanel.h"
#include "Panels/SceneHierarchyPanel.h"
#include "Panels/ContentBrowserPanel.h"
#include "Panels/ScriptRegistryPanel.h"
#include "Panels/MaterialEditorPanel.h"
#include "Panels/SceneRendererPanel.h"
#include "Panels/AssetRegistryPanel.h"
#include "Panels/BuildSettingsPanel.h"
#include "Panels/ShaderEditorPanel.h"
#include "Panels/PerformancePanel.h"
#include "Panels/ConsolePanel.h"
#include "Panels/AboutPanel.h"

namespace Vortex {

	class EditorCamera;

	class EditorLayer : public Layer
	{
	public:
		EditorLayer();
		~EditorLayer() override = default;

		void OnAttach() override;
		void OnDetach() override;

		void OnUpdate(TimeStep ts) override;
		void OnGuiRender() override;
		void OnMainMenuBarRender();
		void OnScenePanelRender();
		void OnAssetDropped(bool& meshImportPopupOpen);
		void OnMeshImportPopupOpened(bool& meshImportPopupOpen);
		void OnGizmosRender();
		void OnSecondViewportRender();
		void OnEvent(Event& e) override;

	private:
		void UI_GizmosModeToolbar();
		void UI_GizmosToolbar();
		void UI_CentralToolbar();
		void UI_SceneSettingsToolbar();
		void OnOverlayRender();
		bool OnKeyPressedEvent(KeyPressedEvent& e);
		bool OnMouseButtonPressedEvent(MouseButtonPressedEvent& e);

		// Project

		void CreateNewProject();
		bool OpenExistingProject();
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

		// Gizmos

		void OnNoGizmoSelected();
		void OnTranslationToolSelected();
		void OnRotationToolSelected();
		void OnScaleToolSelected();

		// Editor Callbacks

		void OnLaunchRuntime(const std::filesystem::path& filepath);

		// Helper

		void DuplicateSelectedEntity();
		void SetSceneContext(const SharedRef<Scene>& scene);

	private:
		EditorCamera* m_EditorCamera = nullptr;
		SharedRef<Framebuffer> m_Framebuffer = nullptr;

		SharedRef<Scene> m_ActiveScene = nullptr;
		SharedRef<Scene> m_EditorScene = nullptr;
		
		std::filesystem::path m_EditorScenePath;

		Entity m_HoveredEntity;
		Entity m_ModelEntityToEdit;

		Math::vec2 m_ViewportSize{};
		Math::vec2 m_ViewportBounds[2] = { Math::vec2() };
		Math::vec2 m_MousePosLastFrame = Math::vec2();

		int32_t m_GizmoType = -1;

		bool m_ShowScenePanel = true;
		bool m_ShowSecondViewport = false;
		bool m_ShowSceneCreateEntityMenu = false;
		bool m_EditorDebugViewEnabled = false;
		bool m_SceneViewportFocused = false;
		bool m_SceneViewportHovered = false;
		bool m_SceneViewportMaximized = false;

		uint32_t m_TranslationMode = 0; // Local mode

		std::vector<SharedRef<AudioSource>> m_AudioSourcesToResume = std::vector<SharedRef<AudioSource>>();

		std::string m_ModelFilepath = "";
		ModelImportOptions m_ModelImportOptions = ModelImportOptions();

		SharedRef<ProjectSettingsPanel> m_ProjectSettingsPanel = nullptr;
		SceneHierarchyPanel m_SceneHierarchyPanel;
		SharedRef<ContentBrowserPanel> m_ContentBrowserPanel = nullptr;
		ScriptRegistryPanel m_ScriptRegistryPanel;
		MaterialEditorPanel m_MaterialEditorPanel;
		SceneRendererPanel m_SceneRendererPanel;
		AssetRegistryPanel m_AssetRegistryPanel;
		SharedRef<BuildSettingsPanel> m_BuildSettingsPanel = nullptr;
		ShaderEditorPanel m_ShaderEditorPanel;
		PerformancePanel m_PerformancePanel;
		ConsolePanel m_ConsolePanel;
		AboutPanel m_AboutPanel;

		enum class SceneState { Edit = 0, Play = 1, Simulate = 2 };
		SceneState m_SceneState = SceneState::Edit;
	};

}
