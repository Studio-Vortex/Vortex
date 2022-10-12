#include "sppch.h"
#include "EditorCamera.h"

#include "Sparky/Core/Input.h"
#include "Sparky/Core/KeyCodes.h"
#include "Sparky/Core/MouseCodes.h"

namespace Sparky {

	EditorCamera::EditorCamera(float fov, float aspectRatio, float nearClip, float farClip)
		: m_FOV(fov), m_AspectRatio(aspectRatio), m_NearClip(nearClip), m_FarClip(farClip), Camera(Math::Perspective(Math::Deg2Rad(fov), aspectRatio, nearClip, farClip))
	{
		UpdateView();
	}

	void EditorCamera::UpdateProjection()
	{
		m_AspectRatio = m_ViewportWidth / m_ViewportHeight;
		m_ProjectionMatrix = Math::Perspective(Math::Deg2Rad(m_FOV), m_AspectRatio, m_NearClip, m_FarClip);
	}

	void EditorCamera::UpdateView()
	{
		if (s_LockEditorCameraRotation)
			m_Yaw = m_Pitch = 0.0f; // Lock the camera's rotation

		m_Position = CalcualtePosition(); 

		Math::quaternion orientation = GetOrientation();
		m_ViewMatrix = Math::Translate(m_Position) * Math::ToMat4(orientation);
		m_ViewMatrix = Math::Inverse(m_ViewMatrix);
	}

	void EditorCamera::ResetCameraPositionToWorldOrigin()
	{
		m_Position = Math::vec3(0.0f);
		m_FocalPoint = Math::vec3(0.0f);
		m_Distance = 10.0f;
		m_Pitch = 0.0f;
		m_Yaw = 0.0f;

		UpdateView();
	}

	std::pair<float, float> EditorCamera::PanSpeed() const
	{
		float x = std::min(m_ViewportWidth / 1000.0f, 2.4f); // max = 2.4f
		float xFactor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

		float y = std::min(m_ViewportHeight / 1000.0f, 2.4f); // max = 2.4f
		float yFactor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;

		return { xFactor, yFactor };
	}

	float EditorCamera::RotationSpeed() const
	{
		return 0.8f;
	}

	float EditorCamera::ZoomSpeed() const
	{
		float distance = m_Distance * 0.2f;
		distance = std::max(distance, 0.0f);
		float speed = distance * distance;
		speed = std::min(speed, 100.0f); // max speed = 100
		return speed;
	}

	void EditorCamera::OnUpdate(TimeStep delta)
	{
		const Math::vec2& mousePosition{ Input::GetMouseX(), Input::GetMouseY() };
		Math::vec2 mouseDelta = (mousePosition - m_InitialMousePosition) * 0.003f;
		m_InitialMousePosition = mousePosition;
		float originalDistance = m_Distance;

		if (Input::IsKeyPressed(Key::LeftAlt) || Input::IsKeyPressed(Key::RightAlt))
		{
			if (Input::IsMouseButtonPressed(Mouse::ButtonLeft))
				MouseRotate(mouseDelta);
			else if (Input::IsMouseButtonPressed(Mouse::ButtonRight))
				MouseZoom(mouseDelta.y);
		}
		else if (Input::IsMouseButtonPressed(Mouse::ButtonRight))
		{
			Math::vec3 moveSpeed = s_MoveSpeed;
			bool shiftPressed = Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift);

			if (m_MouseYDelta)
			{
				m_ShiftModifer += m_MouseXDelta;
				m_MouseYDelta = 0.0f;
			}

			if (shiftPressed)
				moveSpeed *= m_ShiftModifer;

			Math::vec3 cameraVelocity = Math::vec3(0.0f);

			if (Input::IsKeyPressed(Key::W))
				cameraVelocity += GetForwardDirection();
			if (Input::IsKeyPressed(Key::S))
				cameraVelocity -= GetForwardDirection();

			if (Input::IsKeyPressed(Key::A))
				cameraVelocity -= GetRightDirection();
			if (Input::IsKeyPressed(Key::D))
				cameraVelocity += GetRightDirection();

			if (Input::IsKeyPressed(Key::Q))
				cameraVelocity += GetUpDirection();
			if (Input::IsKeyPressed(Key::E))
				cameraVelocity -= GetUpDirection();

			m_Distance = 1.0f;

			m_FocalPoint += cameraVelocity * moveSpeed;
			m_Position = CalcualtePosition();

			MouseRotate(mouseDelta);
			UpdateView();

			return;
		}

		m_Distance = originalDistance;

		if (Input::IsMouseButtonPressed(Mouse::ButtonMiddle))
			MousePan(mouseDelta);

		UpdateView();
	}

	void EditorCamera::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(SP_BIND_CALLBACK(EditorCamera::OnMouseScrolledEvent));
	}

	bool EditorCamera::OnMouseScrolledEvent(MouseScrolledEvent& e)
	{
		m_MouseXDelta = e.GetXOffset();
		m_MouseYDelta = e.GetYOffset();

		float deltaX = m_MouseXDelta * 0.15f;
		float deltaY = m_MouseYDelta * 0.1f;
		
		if (!Input::IsMouseButtonPressed(Mouse::ButtonRight)) // Only scroll if user is not moving or looking around
		{
			// Handle X Scrolling
			MousePanHorizontal(deltaX);
			// Handle Y Scrolling
			MouseZoom(deltaY);
		}

		UpdateView();
		return false;
	}

	void EditorCamera::MousePan(const Math::vec2& delta)
	{
		auto [xSpeed, ySpeed] = PanSpeed();
		m_FocalPoint += -GetRightDirection() * delta.x * xSpeed * m_Distance;
		m_FocalPoint += GetUpDirection() * delta.y * ySpeed * m_Distance;
	}

	void EditorCamera::MousePanHorizontal(float delta)
	{
		if (!Input::IsMouseButtonPressed(Mouse::ButtonLeft)) // user could be clicking on a gizmo
		{
			auto [xSpeed, ySpeed] = PanSpeed();
			m_FocalPoint += -GetRightDirection() * delta * xSpeed * m_Distance;
		}
	}

	void EditorCamera::MouseRotate(const Math::vec2& delta)
	{
		float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;
		m_Yaw += yawSign * delta.x * RotationSpeed();
		m_Pitch += delta.y * RotationSpeed();
	}

	void EditorCamera::MouseZoom(float delta)
	{
		m_Distance -= delta * ZoomSpeed();
		if (m_Distance < 1.0f)
		{
			m_FocalPoint += GetForwardDirection();
			m_Distance = 1.0f;
		}
	}

	Math::vec3 EditorCamera::GetUpDirection() const
	{
		return Math::Rotate(GetOrientation(), Math::vec3(0.0f, 1.0f, 0.0f));
	}

	Math::vec3 EditorCamera::GetRightDirection() const
	{
		return Math::Rotate(GetOrientation(), Math::vec3(1.0f, 0.0f, 0.0f));
	}

	Math::vec3 EditorCamera::GetForwardDirection() const
	{
		return Math::Rotate(GetOrientation(), Math::vec3(0.0f, 0.0f, -1.0f));
	}

	Math::vec3 EditorCamera::CalcualtePosition() const
	{
		return m_FocalPoint - GetForwardDirection() * m_Distance;
	}

	Math::quaternion EditorCamera::GetOrientation() const
	{
		return Math::quaternion(Math::vec3(-m_Pitch, -m_Yaw, 0.0f));
	}

}
