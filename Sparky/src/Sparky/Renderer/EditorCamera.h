#pragma once

#include "Sparky/Renderer/Camera.h"
#include "Sparky/Core/TimeStep.h"
#include "Sparky/Events/Event.h"
#include "Sparky/Events/MouseEvent.h"

#include "Sparky/Core/Math.h"

namespace Sparky {

	class EditorCamera : public Camera
	{
	public:
		EditorCamera() = default;
		EditorCamera(float fov, float aspectRatio, float nearClip, float farClip);
		~EditorCamera() override = default;

		void OnUpdate(TimeStep delta);
		void OnEvent(Event& e);

		inline float GetDistance() const { return m_Distance; }
		inline void SetDistance(float distance) { m_Distance = distance; }

		inline float GetFOV() const { return m_FOV; }
		inline void SetFOV(float fov) { m_FOV = fov; UpdateProjection(); }

		inline void SetViewportSize(float width, float height) { m_ViewportWidth = width; m_ViewportHeight = height; UpdateProjection(); }

		const Math::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		Math::mat4 GetViewProjection() const { return m_ProjectionMatrix * m_ViewMatrix; }

		Math::vec3 GetUpDirection() const;
		Math::vec3 GetRightDirection() const;
		Math::vec3 GetForwardDirection() const;
		const Math::vec3& GetPosition() const { return m_Position; }
		Math::quaternion GetOrientation() const;

		float GetPitch() const { return m_Pitch; }
		float GetYaw() const { return m_Yaw; }

		void ResetAtOrigin();

	private:
		void UpdateProjection();
		void UpdateView();

		bool OnMouseScrolledEvent(MouseScrolledEvent& e);

		void MousePan(const Math::vec2& delta);
		void MousePanHorizontal(float delta);
		void MouseRotate(const Math::vec2& delta);
		void MouseZoom(float delta);

		Math::vec3 CalcualtePosition() const;

		std::pair<float, float> PanSpeed() const;
		float RotationSpeed() const;
		float ZoomSpeed() const;

	private:
		float m_FOV = 45.0f, m_AspectRatio = 1.778f, m_NearClip = 0.1f, m_FarClip = 1000.0f;

		Math::mat4 m_ViewMatrix;
		Math::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
		Math::vec3 m_FocalPoint = Math::vec3(0.0f);

		Math::vec2 m_InitialMousePosition = Math::vec2(0.0f);

		float m_Distance = 10.0f;
		float m_Pitch = 0.0f, m_Yaw = 0.0f;

		float m_ViewportWidth = 1600.0f, m_ViewportHeight = 900.0f;

	};

}
