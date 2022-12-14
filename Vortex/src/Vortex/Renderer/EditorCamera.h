#pragma once

#include "Vortex/Renderer/Camera.h"
#include "Vortex/Core/TimeStep.h"
#include "Vortex/Events/Event.h"
#include "Vortex/Events/MouseEvent.h"

#include "Vortex/Core/Math.h"

namespace Vortex {

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

		inline const Math::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		inline Math::mat4 GetViewProjection() const { return m_ProjectionMatrix * m_ViewMatrix; }

		Math::vec3 GetUpDirection() const;
		Math::vec3 GetRightDirection() const;
		Math::vec3 GetForwardDirection() const;
		inline const Math::vec3& GetPosition() const { return m_Position; }

		inline float GetPitch() const { return m_Pitch; }
		inline float GetYaw() const { return m_Yaw; }

		void ResetCameraPositionToWorldOrigin();
		void MoveToPosition(const Math::vec3& translation);

		inline static bool IsIn2DView() { return s_LockForward; }
		inline static bool IsInTopDownView() { return s_LockDown; }
		inline static void LockTo2DView(bool lockRotation) { s_LockForward = lockRotation; }
		inline static void LockToTopDownView(bool lockRotation) { s_LockDown = lockRotation; }
		inline static Math::vec3 GetMoveSpeed() { return s_MoveSpeed; }
		inline static void SetMoveSpeed(const Math::vec3 moveSpeed) { s_MoveSpeed = moveSpeed; }

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

		float m_MouseXDelta = 0.0f, m_MouseYDelta = 0.0f;
		float m_ShiftModifer = 3.0f;

		Math::mat4 m_ViewMatrix = Math::Identity();
		Math::vec3 m_Position = Math::vec3(0.0f);
		Math::vec3 m_FocalPoint = Math::vec3(0.0f, 5.0f, 10.0f);

		Math::vec2 m_InitialMousePosition = Math::vec2(0.0f);

		float m_Distance = 0.0f;
		float m_Pitch = Math::Deg2Rad(25.0f), m_Yaw = 0.0f;

		float m_ViewportWidth = 1600.0f, m_ViewportHeight = 900.0f;

		inline static bool s_LockForward = false;
		inline static bool s_LockDown = false;
		inline static Math::vec3 s_MoveSpeed = { 5.0f, 5.0f, 5.0f };

	};

}
