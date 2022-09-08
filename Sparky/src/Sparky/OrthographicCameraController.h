#pragma once

#include "Sparky/Core/TimeStep.h"
#include "Sparky/Renderer/OrthographicCamera.h"

#include "Sparky/Events/ApplicationEvent.h"
#include "Sparky/Events/MouseEvent.h"
#include "Sparky/Events/KeyEvent.h"

namespace Sparky {

	class OrthographicCameraController
	{
	public:
		OrthographicCameraController(float aspectRatio, bool rotation = false);

		void OnUpdate(TimeStep ts);
		void OnEvent(Event& e);
		
		inline const OrthographicCamera& GetCamera() const { return m_Camera; }
		inline OrthographicCamera& GetCamera() { return m_Camera; }

		inline const Math::vec3& GetCameraPosition() const { return m_CameraPosition; }

		inline void ResetCameraRotation() { m_CameraRotation = 0.0f; }
		inline void SetCameraRotationSpeed(float speed) { m_CameraRotationSpeed = speed; }
		inline float GetCameraRotation() const { return m_CameraRotation; }

	private:
		bool OnMouseScrolledEvent(MouseScrolledEvent& e);
		bool OnWindowResizeEvent(WindowResizeEvent& e);

	private:
		float m_AspectRatio;
		float m_ZoomLevel = 1.0f;
		OrthographicCamera m_Camera;

		bool m_Rotation;

		Math::vec3 m_CameraPosition{};
		float m_CameraRotation{};

		float m_CameraTranslationSpeed = 1.0f;
		float m_CameraRotationSpeed = 180.0f;
	};

}
