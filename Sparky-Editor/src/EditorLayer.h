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
		SharedRef<Texture2D> m_GridTexture;

		SharedRef<Scene> m_ActiveScene;
		Entity m_SquareEntity;
		Entity m_CameraEntity;
		Entity m_SecondCamera;

		bool m_PrimaryCamera = true;

		Math::vec2 m_ViewportSize{};

		bool m_ViewportFocused = false;
		bool m_ViewportHovered = false;

		float m_RotatedQuadRotation = 0.0f;
		float m_RotatedQuadRotationSpeed = 25.0f;
		Math::vec3 m_RotatedQuadPos{ 3.0f, -3.0f, -0.1f };

		Math::vec4 m_SquareColor;

		Math::vec4 m_GridColor;
		float m_GridScale;

		// Panels

		SceneHierarchyPanel m_SceneHierarchyPanel;
	};

}