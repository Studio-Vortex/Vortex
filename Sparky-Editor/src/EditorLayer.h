#pragma once

#include <Sparky.h>
#include <Sparky/Renderer/EditorCamera.h>

#include "Panels/SceneHierarchyPanel.h"
#include "Panels/ContentBrowserPanel.h"
#include "Panels/ScriptRegistryPanel.h"
#include "Panels/ShaderEditorPanel.h"
#include "Panels/PerformancePanel.h"
#include "Panels/SettingsPanel.h"
#include "Panels/ConsolePanel.h"
#include "Panels/AboutPanel.h"

namespace Sparky {

	struct Skybox
	{
		SharedRef<Texture2D> Textures[6];
	};

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
		void OnOverlayRender();
		bool OnKeyPressedEvent(KeyPressedEvent& e);
		bool OnMouseButtonPressedEvent(MouseButtonPressedEvent& e);

		void CreateNewScene();
		void OpenExistingScene();
		void OpenScene(const std::filesystem::path& path);
		void SaveSceneAs();
		void SaveScene();

		void SerializeScene(SharedRef<Scene> scene, const std::filesystem::path& path);

		void OnScenePlay();
		void OnSceneStop();
		void RestartScene();

		void OnSceneSimulate();
		void RestartSceneSimulation();

		void AddEmptyEntity();
		void DuplicateSelectedEntity();

		// Gizmos

		void OnNoGizmoSelected();
		void OnTranslationToolSelected();
		void OnRotationToolSelected();
		void OnScaleToolSelected();

		// Panels

		void UI_Toolbar();

		// Editor Callbacks

		void OnLaunchRuntime();

	private:
		EditorCamera m_EditorCamera;
		SharedRef<Framebuffer> m_Framebuffer;

		SharedRef<Scene> m_ActiveScene;
		SharedRef<Scene> m_EditorScene;
		
		std::filesystem::path m_EditorScenePath;

		Entity m_HoveredEntity;

		Math::vec2 m_ViewportSize{};
		Math::vec2 m_ViewportBounds[2] = { Math::vec2() };
		Math::vec2 m_MousePosLastFrame = Math::vec2();
		Math::vec4 m_Physics2DColliderColor = Math::vec4{ (44.0f / 255.0f), (151.0f / 255.0f), (167.0f / 255.0f), 1.0f };
		Math::vec3 m_EditorClearColor = Math::vec3{ (38.0f / 255.0f), (44.0f / 255.0f), (60.0f / 255.0f) };

		float m_EditorCameraFOV = 45.0f;
		float m_EditorCameraFOVLastFrame = 0.0f;
		int32_t m_GizmoType = -1;

		bool m_DrawEditorGrid = true;
		bool m_DrawEditorAxes = true;
		bool m_DrawEditorCubemap = true;
		bool m_EditorDebugViewEnabled = false;

		bool m_ShowPhysicsColliders = false;

		bool m_SceneViewportFocused = false;
		bool m_SceneViewportHovered = false;
		bool m_SceneViewportMaximized = false;

		enum class SceneState
		{
			Edit = 0, Play = 1, Simulate = 2,
		};
		
		// Editor resources

		SharedRef<Texture2D> m_PlayIcon;
		SharedRef<Texture2D> m_StopIcon;
		SharedRef<Texture2D> m_SimulateIcon;

		// Panels

		SceneHierarchyPanel m_SceneHierarchyPanel;
		ContentBrowserPanel m_ContentBrowserPanel;
		ScriptRegistryPanel m_ScriptRegistryPanel;
		ShaderEditorPanel m_ShaderEditorPanel;
		PerformancePanel m_PerformancePanel;
		ConsolePanel m_ConsolePanel;
		AboutPanel m_AboutPanel;

		// Settings

		bool m_GizmosEnabled = true;
		bool m_GizmoSnapEnabled = true;
		bool m_OrthographicGizmos = false;
		float m_SnapValue = 0.5f;
		float m_RotationSnapValue = 45.0f;
		bool m_DrawGizmoGrid = false;
		float m_GizmoGridSize = 1.0f;

		SettingsPanel::Settings::GizmoSettings m_GizmoSettings{
			m_GizmosEnabled, m_OrthographicGizmos, m_GizmoSnapEnabled, m_SnapValue, m_RotationSnapValue, m_DrawGizmoGrid, m_GizmoGridSize
		};

		SettingsPanel::Settings m_Settings{ 
			m_Physics2DColliderColor, m_ShowPhysicsColliders, m_DrawEditorGrid, m_DrawEditorAxes, m_DrawEditorCubemap, m_EditorCameraFOV, m_EditorClearColor, m_GizmoSettings, m_EditorScenePath, SP_BIND_CALLBACK(EditorLayer::OnLaunchRuntime)
		};
		SettingsPanel m_SettingsPanel = SettingsPanel(m_Settings);

		SceneState m_SceneState = SceneState::Edit;
	};

}
