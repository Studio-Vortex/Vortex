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
		OrthographicCameraController m_CameraController;

		SharedRef<Framebuffer> m_Framebuffer;

		SharedRef<Scene> m_ActiveScene;
		Entity m_SquareEntity;
		Entity m_CameraEntity;
		Entity m_SecondCamera;

		bool m_PrimaryCamera = true;

		Math::vec2 m_ViewportSize{};

		bool m_ViewportFocused = false;
		bool m_ViewportHovered = false;

		// Panels

		SceneHierarchyPanel m_SceneHierarchyPanel;
	};

}