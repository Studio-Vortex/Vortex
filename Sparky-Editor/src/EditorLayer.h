#pragma once

#include <Sparky.h>

#include "Panels/SceneHierarchyPanel.h"

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

		void CreateNewScene();
		void OpenExistingScene();
		void SaveSceneAs();

	private:
		SharedRef<Framebuffer> m_Framebuffer;

		SharedRef<Scene> m_ActiveScene;
		Entity m_SquareEntity;
		Entity m_CameraEntity;
		Entity m_SecondCamera;

		bool m_PrimaryCamera = true;

		Math::vec2 m_ViewportSize{};

		bool m_ViewportFocused = false;
		bool m_ViewportHovered = false;

		int m_GizmoType = -1;

		// Panels

		SceneHierarchyPanel m_SceneHierarchyPanel;
	};

}