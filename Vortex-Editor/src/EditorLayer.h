#pragma once

#include <Vortex.h>
#include <Vortex/Scene/SceneRenderer.h>

#include <Vortex/Editor/PanelManager.h>

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
		void OnGizmosRender(EditorCamera* editorCamera, const ViewportBounds& viewportBounds, bool allowInPlayMode = false);
		void OnSecondViewportPanelRender();
		void OnEvent(Event& e) override;

	private:
		void ResizeTargetFramebuffersIfNeeded();

		// UI

		void UIGizmosToolbar();
		void UICentralToolbar();
		void UIViewportSettingsToolbar();

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
		bool OpenProject(const std::filesystem::path& filepath);
		void SaveProject();
		void CloseProject();
		void BuildProject();
		void BuildAndRunProject();

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

		void OnOverlayRender(EditorCamera* editorCamera, bool renderInPlayMode);

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

		std::vector<Math::vec4> GetFrustumCornersWorldSpace(const TransformComponent& transform, const SceneCamera& sceneCamera);

		std::pair<float, float> GetEditorCameraMouseViewportSpace(bool mainViewport);
		std::pair<Math::vec3, Math::vec3> CastRay(EditorCamera* editorCamera, float mx, float my);

		Entity GetHoveredMeshEntityFromRaycast();

	private:
		struct SelectionData
		{
			UUID SelectedUUID;
			float Distance;

			SelectionData(UUID uuid = 0, float distance = 0.0f)
				: SelectedUUID(uuid), Distance(distance) { }
		};

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

		struct MeshImportPopupData
		{
			std::string MeshFilepath = "";
			MeshImportOptions ModelImportOptions = MeshImportOptions();
			Entity MeshEntityToEdit;
		};

		MeshImportPopupData m_MeshImportPopupData;

		Math::vec2 m_ViewportPanelSize{};
		Math::vec2 m_SecondViewportPanelSize{};

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

		enum class SceneState { Edit = 0, Play = 1, Simulate = 2 };
		SceneState m_SceneState = SceneState::Edit;

		enum class SelectionMode { Entity, Submesh };
		SelectionMode m_SelectionMode = SelectionMode::Entity;
	};

}
