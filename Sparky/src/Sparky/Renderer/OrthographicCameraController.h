#pragma once

#include "Sparky/Core/TimeStep.h"
#include "Sparky/Renderer/OrthographicCamera.h"

#include "Sparky/Core/Math.h"
#include "Sparky/Events/ApplicationEvent.h"
#include "Sparky/Events/MouseEvent.h"
#include "Sparky/Events/KeyEvent.h"

namespace Sparky {

	struct OrthographicCameraBounds
	{
		float Left, Right;
		float Bottom, Top;

		inline float GetWidth() const { return Right - Left; }
		inline float GetHeight() const { return Top - Bottom; }
	};
	
	class OrthographicCameraController
	{
	public:
		OrthographicCameraController(float aspectRatio, bool rotation = false);

		void OnUpdate(TimeStep ts);
		void OnEvent(Event& e);
		
		void OnResize(const Math::vec2& size);

		inline const OrthographicCamera& GetCamera() const { return m_Camera; }
		inline OrthographicCamera& GetCamera() { return m_Camera; }

		inline const Math::vec3& GetPosition() const { return m_CameraPosition; }

		inline float GetRotation() const { return m_CameraRotation; }
		inline void SetRotation(float rotation) { m_CameraRotation = rotation; }
		inline void ResetRotation() { m_CameraRotation = 0.0f; }

		inline float GetRotationSpeed() const { return m_CameraRotationSpeed; }
		inline void SetRotationSpeed(float speed) { m_CameraRotationSpeed = speed; }

		inline float GetZoomLevel() const { return m_ZoomLevel; }
		inline void SetZoomLevel(float zoomLevel) { m_ZoomLevel = zoomLevel; CalculateView(); }

		inline const OrthographicCameraBounds& GetBounds() const { return m_Bounds; }

	private:
		void CalculateView();

		bool OnMouseScrolledEvent(MouseScrolledEvent& e);
		bool OnWindowResizeEvent(WindowResizeEvent& e);

	private:
		float m_AspectRatio;
		float m_ZoomLevel = 1.0f;
		float m_ZoomModifer = 0.25f;
		float m_MaxZoomInLevel = 0.25f;
		float m_MaxZoomOutLevel = 500.0f;
		OrthographicCameraBounds m_Bounds;
		OrthographicCamera m_Camera;

		bool m_Rotation;

		Math::vec3 m_CameraPosition{};
		float m_CameraRotation{};

		float m_CameraTranslationSpeed = 1.0f;
		float m_ShiftKeyModiferSpeed = 4.0f;
		float m_CameraRotationSpeed = 180.0f;
	};

}
