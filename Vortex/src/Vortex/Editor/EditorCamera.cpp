#include "vxpch.h"
#include "EditorCamera.h"

#include "Vortex/Core/Input.h"
#include "Vortex/Core/KeyCodes.h"
#include "Vortex/Core/MouseCodes.h"
#include "Vortex/UI/UI.h"

namespace Vortex {

	EditorCamera::EditorCamera(const float degFOV, const float width, const float height, const float nearP, const float farP)
		: Camera(Math::PerspectiveFOV(Math::Deg2Rad(degFOV), width, height, farP, nearP), Math::PerspectiveFOV(Math::Deg2Rad(degFOV), width, height, nearP, farP)), m_FocalPoint(0.0f), m_VerticalFOV(Math::Deg2Rad(degFOV)), m_NearClip(nearP), m_FarClip(farP)
	{
		Init();
	}

	void EditorCamera::Init()
	{
		constexpr Math::vec3 position = { -5, 5, 5 };
		m_Distance = Math::Distance(position, m_FocalPoint);

		m_Yaw = 3.0f * Math::PI / 4.0f;
		m_Pitch = Math::PI / 4.0f;

		m_Position = CalculatePosition();
		const Math::quaternion orientation = GetOrientation();
		m_Direction = Math::EulerAngles(orientation) * (180.0f / Math::PI);
		m_ViewMatrix = Math::Translate(m_Position) * Math::ToMat4(orientation);
		m_ViewMatrix = Math::Inverse(m_ViewMatrix);
	}

	static void DisableMouse()
	{
		Input::SetCursorMode(CursorMode::Locked);
		UI::SetInputEnabled(false);
	}

	static void EnableMouse()
	{
		Input::SetCursorMode(CursorMode::Normal);
		UI::SetInputEnabled(true);
	}

	void EditorCamera::OnUpdate(TimeStep ts)
	{
		const Math::vec2& mouse{ Input::GetMouseX(), Input::GetMouseY() };
		const Math::vec2 delta = (mouse - m_InitialMousePosition) * 0.002f;

		if (!m_IsActive)
		{
			if (!UI::IsInputEnabled())
			{
				UI::SetInputEnabled(true);
			}

			return;
		}

		if (Input::IsMouseButtonDown(MouseButton::Right) && !Input::IsKeyDown(KeyCode::LeftAlt))
		{
			m_CameraMode = CameraMode::FlyCam;
			DisableMouse();
			const float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;

			const float speed = GetCameraSpeed();

			if (Input::IsKeyDown(KeyCode::Q))
				m_PositionDelta -= ts.GetDeltaTimeMs() * speed * Math::vec3{ 0.f, yawSign, 0.f };
			if (Input::IsKeyDown(KeyCode::E))
				m_PositionDelta += ts.GetDeltaTimeMs() * speed * Math::vec3{ 0.f, yawSign, 0.f };
			if (Input::IsKeyDown(KeyCode::S))
				m_PositionDelta -= ts.GetDeltaTimeMs() * speed * m_Direction;
			if (Input::IsKeyDown(KeyCode::W))
				m_PositionDelta += ts.GetDeltaTimeMs() * speed * m_Direction;
			if (Input::IsKeyDown(KeyCode::A))
				m_PositionDelta -= ts.GetDeltaTimeMs() * speed * m_RightDirection;
			if (Input::IsKeyDown(KeyCode::D))
				m_PositionDelta += ts.GetDeltaTimeMs() * speed * m_RightDirection;

			constexpr float maxRate{ 0.12f };
			m_YawDelta += Math::Clamp(yawSign * delta.x * RotationSpeed(), -maxRate, maxRate);
			m_PitchDelta += Math::Clamp(delta.y * RotationSpeed(), -maxRate, maxRate);

			m_RightDirection = Math::Cross(m_Direction, Math::vec3{ 0.f, yawSign, 0.f });

			m_Direction = Math::Rotate(
				Math::Normalize(Math::Cross(Math::AngleAxis(-m_PitchDelta, m_RightDirection),
				Math::AngleAxis(-m_YawDelta, Math::vec3{ 0.f, yawSign, 0.f }))), m_Direction
			);

			const float distance = Math::Distance(m_FocalPoint, m_Position);
			m_FocalPoint = m_Position + GetForwardDirection() * distance;
			m_Distance = distance;
		}
		else if (Input::IsKeyDown(KeyCode::LeftAlt))
		{
			m_CameraMode = CameraMode::ArcBall;

			if (Input::IsMouseButtonDown(MouseButton::Middle))
			{
				DisableMouse();
				MousePan(delta);
			}
			else if (Input::IsMouseButtonDown(MouseButton::Left))
			{
				DisableMouse();
				MouseRotate(delta);
			}
			else if (Input::IsMouseButtonDown(MouseButton::Right))
			{
				DisableMouse();
				MouseZoom((delta.x + delta.y) * 0.1f);
			}
			else
			{
				EnableMouse();
			}
		}
		else
		{
			EnableMouse();
		}

		m_InitialMousePosition = mouse;
		m_Position += m_PositionDelta;
		m_Yaw += m_YawDelta;
		m_Pitch += m_PitchDelta;

		if (m_Use2DView)
		{
			m_Yaw = m_Pitch = 0.0f;
		}
		else if (m_UseTopDownView)
		{
			m_Yaw = 0.0f;
			m_Pitch = Math::Deg2Rad(90.0f);
		}

		if (m_CameraMode == CameraMode::ArcBall)
		{
			m_Position = CalculatePosition();
		}

		UpdateCameraView();
	}

	float EditorCamera::GetCameraSpeed() const
	{
		float speed = m_NormalSpeed;

		if (Input::IsKeyDown(KeyCode::LeftControl))
		{
			speed /= 2 - Math::Log(m_NormalSpeed);
		}
		if (Input::IsKeyDown(KeyCode::LeftShift))
		{
			speed *= 2 - Math::Log(m_NormalSpeed);
		}

		return Math::Clamp(speed, MIN_SPEED, MAX_SPEED);
	}

	void EditorCamera::UpdateCameraView()
	{
		const float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;

		// Extra step to handle the problem when the camera direction is the same as the up vector
		const float cosAngle = Math::Dot(GetForwardDirection(), GetUpDirection());
		if (cosAngle * yawSign > 0.99f)
		{
			m_PitchDelta = 0.0f;
		}

		const Math::vec3 lookAt = m_Position + GetForwardDirection();
		m_Direction = Math::Normalize(lookAt - m_Position);
		m_Distance = Math::Distance(m_Position, m_FocalPoint);
		m_ViewMatrix = Math::LookAt(m_Position, lookAt, Math::vec3{ 0.f, yawSign, 0.f });

		//damping for smooth camera
		m_YawDelta *= 0.6f;
		m_PitchDelta *= 0.6f;
		m_PositionDelta *= 0.8f;
	}

	void EditorCamera::Focus(const Math::vec3& focusPoint)
	{
		m_FocalPoint = focusPoint;
		m_CameraMode = CameraMode::FlyCam;

		if (m_Distance > m_MinFocusDistance)
		{
			m_Distance -= m_Distance - m_MinFocusDistance;
			m_Position = m_FocalPoint - GetForwardDirection() * m_Distance;
		}

		m_Position = m_FocalPoint - GetForwardDirection() * m_Distance;

		UpdateCameraView();
	}

	std::pair<float, float> EditorCamera::PanSpeed() const
	{
		const float x = Math::Min(float(m_ViewportWidth) / 1000.0f, 2.4f); // max = 2.4f
		const float xFactor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

		const float y = Math::Min(float(m_ViewportHeight) / 1000.0f, 2.4f); // max = 2.4f
		const float yFactor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;

		return { xFactor, yFactor };
	}

	float EditorCamera::RotationSpeed() const
	{
		return 0.3f;
	}

	float EditorCamera::ZoomSpeed() const
	{
		float distance = m_Distance * 0.2f;
		distance = Math::Max(distance, 0.0f);
		float speed = distance * distance;
		speed = Math::Min(speed, 50.0f); // max speed = 50

		return speed;
	}

	void EditorCamera::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<MouseScrolledEvent>([this](MouseScrolledEvent& e) { return OnMouseScroll(e); });
	}

	bool EditorCamera::OnMouseScroll(MouseScrolledEvent& e)
	{
		if (Input::IsMouseButtonDown(MouseButton::Right))
		{
			m_NormalSpeed += e.GetYOffset() * 0.03f * m_NormalSpeed;
			m_NormalSpeed = std::clamp(m_NormalSpeed, MIN_SPEED, MAX_SPEED);
		}
		else
		{
			MouseZoom(e.GetYOffset() * 0.1f);
			UpdateCameraView();
		}

		return true;
	}

	void EditorCamera::MousePan(const Math::vec2& delta)
	{
		auto [xSpeed, ySpeed] = PanSpeed();
		m_FocalPoint -= GetRightDirection() * delta.x * xSpeed * m_Distance;
		m_FocalPoint += GetUpDirection() * delta.y * ySpeed * m_Distance;
	}

	void EditorCamera::MouseRotate(const Math::vec2& delta)
	{
		const float yawSign = GetUpDirection().y < 0.0f ? -1.0f : 1.0f;
		m_YawDelta += yawSign * delta.x * RotationSpeed();
		m_PitchDelta += delta.y * RotationSpeed();
	}

	void EditorCamera::MouseZoom(float delta)
	{
		m_Distance -= delta * ZoomSpeed();
		const Math::vec3 forwardDir = GetForwardDirection();
		m_Position = m_FocalPoint - forwardDir * m_Distance;

		if (m_Distance < 1.0f)
		{
			m_FocalPoint += forwardDir * m_Distance;
			m_Distance = 1.0f;
		}

		m_PositionDelta += delta * ZoomSpeed() * forwardDir;
	}

	Math::vec3 EditorCamera::GetUpDirection() const
	{
		return Math::Rotate(GetOrientation(), Math::vec3(0.0f, 1.0f, 0.0f));
	}

	Math::vec3 EditorCamera::GetRightDirection() const
	{
		return Math::Rotate(GetOrientation(), Math::vec3(1.f, 0.f, 0.f));
	}

	Math::vec3 EditorCamera::GetForwardDirection() const
	{
		return Math::Rotate(GetOrientation(), Math::vec3(0.0f, 0.0f, -1.0f));
	}

	Math::vec3 EditorCamera::CalculatePosition() const
	{
		return m_FocalPoint - GetForwardDirection() * m_Distance + m_PositionDelta;
	}

	Math::quaternion EditorCamera::GetOrientation() const
	{
		return Math::quaternion(Math::vec3(-m_Pitch - m_PitchDelta, -m_Yaw - m_YawDelta, 0.0f));
	}

	void EditorCamera::SetVerticalFOV(float degFOV)
	{
		m_VerticalFOV = Math::Deg2Rad(degFOV);
		SetPerspectiveProjectionMatrix(m_VerticalFOV, m_ViewportWidth, m_ViewportHeight, m_NearClip, m_FarClip);
	}

}
