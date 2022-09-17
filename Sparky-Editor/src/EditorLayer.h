#pragma once

#include <Sparky.h>
#include <Sparky/Renderer/EditorCamera.h>

#include "Panels/SceneHierarchyPanel.h"
#include "Panels/ContentBrowserPanel.h"
#include "Panels/SettingsPanel.h"
#include "Panels/StatsPanel.h"
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
		bool OnKeyPressedEvent(KeyPressedEvent& e);
		bool OnMouseButtonPressedEvent(MouseButtonPressedEvent& e);
		void OnOverlayRender();

		void CreateNewScene();
		void OpenExistingScene();
		void OpenScene(const std::filesystem::path& path);
		void SaveSceneAs();
		void SaveScene();

		void SerializeScene(SharedRef<Scene> scene, const std::filesystem::path& path);

		void OnScenePlay();
		void OnSceneStop();
		void RestartScene();

		void DuplicateSelectedEntity();

		// Gizmos
		void OnNoGizmoSelected();
		void OnTranslationToolSelected();
		void OnRotationToolSelected();
		void OnScaleToolSelected();

		// Panels

		void UI_Toolbar();

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

		int m_GizmoType = -1;

		bool m_ShowPhysicsColliders = false;

		bool m_PrimaryCamera = true;

		bool m_SceneViewportFocused = false;
		bool m_SceneViewportHovered = false;
		bool m_SceneViewportMaximized = false;

		enum class SceneState
		{
			Edit = 0, Play = 1,
		};

		// Editor resources
		SharedRef<Texture2D> m_PlayIcon;
		SharedRef<Texture2D> m_StopIcon;

		// Panels

		SceneHierarchyPanel m_SceneHierarchyPanel;
		ContentBrowserPanel m_ContentBrowserPanel;
		StatsPanel m_StatsPanel;
		AboutPanel m_AboutPanel;

		// Settings

		SettingsPanel::Settings m_Settings{ m_ShowPhysicsColliders };
		SettingsPanel m_SettingsPanel = SettingsPanel(m_Settings);

		SceneState m_SceneState = SceneState::Edit;
	};

}