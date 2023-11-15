#pragma once

#include <Vortex.h>
#include <Vortex/Scene/SceneRenderer.h>

#include <Vortex/Editor/PanelManager.h>

#include "Panels/PhysicsMaterialEditorPanel.h"
#include "Panels/PhysicsStatisticsPanel.h"
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
		void UIHandleAssetDrop();
		void UIOnPopupRender();
		void OnGizmosRender(EditorCamera* editorCamera, Math::vec2 viewportBounds[2], bool allowInPlayMode = false);
		void OnSecondViewportPanelRender();
		void OnEvent(Event& e) override;

	private:
		void ResizeTargetFramebuffersIfNeeded();

		// UI

		void UI_GizmosToolbar();
		void UI_CentralToolbar();
		void UI_SceneSettingsToolbar();
		void OnOverlayRender(EditorCamera* editorCamera, bool renderInPlayMode);

		// Popups

		void OnCreateScriptPopupRender();
		void OnMeshImportPopupRender();

		// Events

		bool OnWindowDragDropEvent(WindowDragDropEvent& e);
		bool OnKeyPressedEvent(KeyPressedEvent& e);
		bool OnMouseButtonPressedEvent(MouseButtonPressedEvent& e);
		bool OnWindowCloseEvent(WindowCloseEvent& e);

		// Project

		void CreateNewProject();
		bool OpenExistingProject();
		void OpenProject(const std::filesystem::path& filepath);
		void SaveProject();
		void CloseProject();

		// Scene

		void CreateNewScene();
		void OpenExistingScene();
		void OpenScene(const std::filesystem::path& filepath);
		void SaveSceneAs();
		void SaveScene();

		void SerializeScene(SharedReference<Scene>& scene, const std::filesystem::path& filepath);

		void OnScenePlay();
		void OnScenePause();
		void OnSceneResume();
		void OnSceneStop();
		void RestartScene();

		void OnSceneSimulate();
		void RestartSceneSimulation();

		// Gizmos

		void OnNoGizmoSelected();
		void OnTranslationToolSelected();
		void OnRotationToolSelected();
		void OnScaleToolSelected();

		// Overlay

		void OverlayRenderMeshBoundingBox(Entity entity, const Math::mat4& transform, const Math::vec4& boundingBoxColor);
		void OverlayRenderMeshBoundingBoxes(const Math::vec4& boundingBoxColor);
		void OverlayRenderMeshOutline(Entity entity, const Math::mat4& transform, const Math::vec4& outlineColor);
		void OverlayRenderMeshCollider(Entity entity, const Math::mat4& transform, const Math::vec4& colliderColor);
		void OverlayRenderMeshColliders(const Math::vec4& colliderColor);
		void OverlayRenderSpriteCollider(EditorCamera* editorCamera, Entity entity, const Math::mat4& transform, const Math::vec4& colliderColor);
		void OverlayRenderSpriteColliders(EditorCamera* editorCamera, const Math::vec4& colliderColor);
		void OverlayRenderSpriteBoundingBoxes(const Math::vec4& boundingBoxColor);
		void OverlayRenderSpriteOutline(Entity entity, const Math::mat4& transform, const Math::vec4& outlineColor);
		void OverlayRenderGrid(bool drawAxis);

		// Editor Callbacks

		void OnLaunchRuntime(const std::filesystem::path& filepath);
		void QueueSceneTransition();

		// Helper

		void SetWindowTitle(const std::string& sceneName);
		void DuplicateSelectedEntity();
		void SetSceneContext(SharedReference<Scene>& scene);
		void ResetEditorCameras();
		void CaptureFramebufferImageToDisk();
		void ReplaceSceneFileExtensionIfNeeded(std::string& filepath);

		std::pair<float, float> GetMouseViewportSpace(bool mainViewport);
		std::pair<Math::vec3, Math::vec3> CastRay(EditorCamera* editorCamera, float mx, float my);

	private:
		EditorCamera* m_EditorCamera = nullptr;
		EditorCamera* m_SecondEditorCamera = nullptr;
		SharedReference<Framebuffer> m_Framebuffer = nullptr;
		SharedReference<Framebuffer> m_SecondViewportFramebuffer = nullptr;
		SceneRenderer m_SecondViewportRenderer;

		SharedReference<Scene> m_ActiveScene = nullptr;
		SharedReference<Scene> m_EditorScene = nullptr;
		
		std::filesystem::path m_EditorScenePath;

		std::filesystem::path m_StartScenePath;

		Entity m_HoveredEntity;

		std::string m_MeshFilepath = "";
		MeshImportOptions m_ModelImportOptions = MeshImportOptions();
		Entity m_MeshEntityToEdit;

		Math::vec2 m_ViewportSize{};
		Math::vec2 m_ViewportBounds[2] = { Math::vec2() };
		Math::vec2 m_SecondViewportSize{};
		Math::vec2 m_SecondViewportBounds[2] = { Math::vec2() };

		int32_t m_GizmoType = -1;
		uint32_t m_TranslationMode = 0; // Local mode

		bool m_ShowScenePanel = true;
		bool m_ShowSecondViewport = false;
		bool m_ShowSceneCreateEntityMenu = false;
		bool m_SceneViewportFocused = false;
		bool m_SceneViewportHovered = false;
		bool m_SecondViewportFocused = false;
		bool m_SecondViewportHovered = false;
		bool m_SceneViewportMaximized = false;
		bool m_AllowViewportCameraEvents = false;
		bool m_AllowSecondViewportCameraEvents = false;
		bool m_StartedClickInViewport = false;
		bool m_StartedClickInSecondViewport = false;
		bool m_ShowSelectedEntityCollider = false;
		bool m_ShowSelectedEntityOutline = true;
		bool m_CaptureFramebufferToDiskOnSave = false;
		bool m_TransitionedFromStartScene = false;

		// Popups

		bool m_OpenCreateScriptPopup = false;
		bool m_OpenMeshImportPopup = false;

		SharedReference<PanelManager> m_PanelManager = nullptr;

		PhysicsMaterialEditorPanel m_PhysicsMaterialEditorPanel;
		PhysicsStatisticsPanel m_PhysicsStatsPanel;
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

		enum class SceneState { Edit = 0, Play = 1, Simulate = 2 };
		SceneState m_SceneState = SceneState::Edit;

		enum class SelectionMode { Entity, Submesh };
		SelectionMode m_SelectionMode = SelectionMode::Entity;
	};

}
