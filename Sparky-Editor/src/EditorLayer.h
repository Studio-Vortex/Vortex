#pragma once

#include <Sparky.h>
#include <Sparky/Renderer/EditorCamera.h>

#include "Panels/SceneHierarchyPanel.h"
#include "Panels/ContentBrowserPanel.h"
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

		void CreateNewScene();
		void OpenExistingScene();
		void SaveSceneAs();

	private:
		EditorCamera m_EditorCamera;
		SharedRef<Framebuffer> m_Framebuffer;

		SharedRef<Scene> m_ActiveScene;
		Entity m_HoveredEntity;
		Entity m_SquareEntity;
		Entity m_CameraEntity;
		Entity m_SecondCamera;

		bool m_PrimaryCamera = true;

		Math::vec2 m_ViewportSize{};
		Math::vec2 m_ViewportBounds[2] = { Math::vec2() };

		bool m_ViewportFocused = false;
		bool m_ViewportHovered = false;

		int m_GizmoType = -1;

		// Panels

		SceneHierarchyPanel m_SceneHierarchyPanel;
		ContentBrowserPanel m_ContentBrowserPanel;
		AboutPanel m_AboutPanel;
	};

}