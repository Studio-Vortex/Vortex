#pragma once

#include <Vortex.h>
#include <Vortex/Scene/SceneRenderer.h>

#include <Vortex/Editor/PanelManager.h>

namespace Vortex {

	class EditorCamera;

	enum class SceneState { Edit = 0, Play = 1, Simulate = 2 };
	enum class SelectionMode { Actor, Submesh };

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
		void OnGizmosRender(EditorCamera* editorCamera, const ViewportBounds& viewportBounds);
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
		bool OpenProject(const Fs::Path& filepath);
		void SaveProject();
		void CloseProject();
		void BuildProject();
		void BuildAndRunProject();

		// Scene

		void CreateNewScene();
		void OpenExistingScene();
		void OpenScene(const Fs::Path& filepath);
		void SaveSceneAs();
		void SaveScene();

		void SerializeScene(SharedReference<Scene>& scene, const Fs::Path& filepath);

		void OnScenePlay();
		void OnScenePause();
		void OnSceneResume();
		void OnSceneStop();
		void RestartScene();

		void OnSceneSimulate();
		void RestartSceneSimulation();

		// Gizmos

		void OnSelectGizmoToolSelected();
		void OnTranslationGizmoToolSelected();
		void OnRotationGizmoToolSelected();
		void OnScaleToolGizmoSelected();

		// Overlay

		void OnOverlayRender(EditorCamera* editorCamera);

		void OverlayRenderSelectedActorOutline(const Math::vec4& outlineColor);

		void OverlayRenderMeshBoundingBox(Actor actor, const Math::mat4& transform, const Math::vec4& boundingBoxColor);
		void OverlayRenderMeshBoundingBoxes(const Math::vec4& boundingBoxColor);
		void OverlayRenderMeshOutline(Actor actor, const Math::mat4& transform, const Math::vec4& outlineColor);
		void OverlayRenderMeshCollider(Actor actor, const Math::mat4& transform, const Math::vec4& colliderColor);
		void OverlayRenderMeshColliders(const Math::vec4& colliderColor);
		void OverlayRenderSpriteCollider(EditorCamera* editorCamera, Actor actor, const Math::mat4& transform, const Math::vec4& colliderColor);
		void OverlayRenderSpriteColliders(EditorCamera* editorCamera, const Math::vec4& colliderColor);
		void OverlayRenderSpriteBoundingBoxes(const Math::vec4& boundingBoxColor);
		void OverlayRenderSpriteOutline(Actor actor, const Math::mat4& transform, const Math::vec4& outlineColor);
		void OverlayRenderGrid(bool drawAxes);

		// Editor Callbacks

		void LaunchRuntimeApp();
		void QueueSceneTransition();

		// Helper

		void SetWindowTitle(const std::string& sceneName);
		void DuplicateSelectedActor();
		void SetSceneContext(SharedReference<Scene>& scene);
		void ResetEditorCameras();
		void CaptureSceneViewportFramebufferImageToDisk();
		void ReplaceSceneFileExtensionIfNeeded(std::string& filepath);

		void ToggleGrid() const;
		void ToggleSceneViewportMaximized() const;

		void SwitchSceneState(SceneState state);
		bool InEditSceneState() const;
		bool InPlaySceneState() const;
		bool InSimulateSceneState() const;

		EditorCamera* GetCurrentEditorCamera() const;

		std::vector<Math::vec4> GetCameraFrustumCornersWorldSpace(const Camera* camera, const Math::mat4& view);

		std::pair<float, float> GetEditorCameraMouseViewportSpace(bool mainViewport);
		Math::Ray Raycast(EditorCamera* editorCamera, float mx, float my);

		Actor GetHoveredMeshActorFromRaycast();

		struct EditorCameraProperties
		{
			Math::vec2 ViewportSize;
			float FOV;
			float NearPlane;
			float FarPlane;
		};

		EditorCamera* CreateEditorCamera(const EditorCameraProperties& properties);

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
		
		Fs::Path m_EditorSceneFilepath;
		Fs::Path m_StartSceneFilepath;
		Fs::Path m_RuntimeAppFilepath;

		Actor m_HoveredActor;

		struct MeshImportPopupData
		{
			std::string MeshFilepath = "";
			MeshImportOptions ModelImportOptions = MeshImportOptions();
			Actor MeshActorToEdit;
		};

		MeshImportPopupData m_MeshImportPopupData;

		Math::vec2 m_ViewportPanelSize{};
		ViewportBounds m_ViewportBounds;
		Math::vec2 m_SecondViewportPanelSize{};
		ViewportBounds m_SecondViewportBounds;

		int32_t m_GizmoType = -1;
		uint32_t m_TranslationMode = 0; // Local mode

		float m_SceneViewportBorderFadeLengthInSeconds = 3.0f;
		float m_SceneViewportBorderFadeTimer;
		Math::vec4 m_SceneViewportOnPlayBorderColor = ColorToVec4(Color::Red);
		Math::vec4 m_SceneViewportOnSimulateBorderColor = ColorToVec4(Color::Green);

		bool m_SceneViewportPanelOpen = true;
		bool m_SecondViewportPanelOpen = false;
		bool m_ShowViewportCreateActorMenu = false;
		bool m_SceneViewportFocused = false;
		bool m_SceneViewportHovered = false;
		bool m_SecondViewportFocused = false;
		bool m_SecondViewportHovered = false;
		mutable bool m_SceneViewportMaximized = false;
		bool m_AllowViewportCameraEvents = false;
		bool m_AllowSecondViewportCameraEvents = false;
		bool m_StartedClickInViewport = false;
		bool m_StartedClickInSecondViewport = false;
		bool m_ShowSelectedActorOutline = true;
		bool m_CaptureSceneViewportFramebufferToDiskOnSave = false;
		bool m_TransitionedFromStartScene = false;

		// Popups

		bool m_CreateScriptPopupOpen = false;
		bool m_MeshImportPopupOpen = false;

		SharedReference<PanelManager> m_PanelManager = nullptr;

		SceneState m_SceneState = SceneState::Edit;
		SelectionMode m_SelectionMode = SelectionMode::Actor;
	};

}
