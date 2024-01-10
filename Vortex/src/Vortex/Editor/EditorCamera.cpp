#include "vxpch.h"
#include "EditorCamera.h"

#include "Vortex/Utils/Time.h"

#include "Vortex/Input/Input.h"

#include "Vortex/Editor/UI/UI.h"

namespace Vortex {

	EditorCamera::EditorCamera(const EditorCameraProperties& params)
		: Camera(params)
	{
		m_Distance = Math::Distance(params.Translation, m_FocalPoint);

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

		const bool isPerspective = IsPerspective();
		const bool isOrthographic = IsOrthographic();

		if (Input::IsKeyDown(KeyCode::LeftAlt))
		{
			if (isPerspective)
				m_CameraMode = CameraMode::ArcBall;

			if (Input::IsMouseButtonDown(MouseButton::Middle))
			{
				DisableMouse();
				if (isPerspective)
					MousePan(delta);
				else if (isOrthographic)
					OrthoPan(delta);
			}
			else if (Input::IsMouseButtonDown(MouseButton::Left))
			{
				if (isPerspective)
				{
					DisableMouse();
					MouseRotate(delta);
				}
			}
			else if (Input::IsMouseButtonDown(MouseButton::Right))
			{
				const float zoom = (delta.x + delta.y) * 0.1f;
				DisableMouse();
				if (isPerspective)
					MouseZoom(zoom);
				else if (isOrthographic)
					OrthoZoom(zoom);
			}
			else
			{
				EnableMouse();
			}
		}
		else if (Input::IsMouseButtonDown(MouseButton::Right))
		{
			m_CameraMode = CameraMode::FlyCam;
			
			if (isPerspective)
			{
				DisableMouse();
			}
			
			const float upSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;
			const Math::vec3 yawSign{ 0.0f, upSign, 0.0f };
			const float speed = GetCameraSpeed();

			const float deltaTime = ts.GetDeltaTimeMs();

			if (Input::IsKeyDown(KeyCode::Q))
				m_PositionDelta -= deltaTime * speed * yawSign;
			else if (Input::IsKeyDown(KeyCode::E))
				m_PositionDelta += deltaTime * speed * yawSign;
			if (Input::IsKeyDown(KeyCode::W))
			{
				if (isOrthographic)
					OrthoZoom(deltaTime * speed * 0.1f);
				else
					m_PositionDelta += deltaTime * speed * m_Direction;
			}
			else if (Input::IsKeyDown(KeyCode::S))
			{
				if (isOrthographic)
					OrthoZoom(-deltaTime * speed * 0.1f);
				else
					m_PositionDelta -= deltaTime * speed * m_Direction;
			}
			if (Input::IsKeyDown(KeyCode::A))
				m_PositionDelta -= deltaTime * speed * m_RightDirection;
			else if (Input::IsKeyDown(KeyCode::D))
				m_PositionDelta += deltaTime * speed * m_RightDirection;

			if (isPerspective)
			{
				constexpr float rotateRate{ 0.12f };
				m_YawDelta += Math::Clamp(upSign * delta.x * RotationSpeed(), -rotateRate, rotateRate);
				m_PitchDelta += Math::Clamp(delta.y * RotationSpeed(), -rotateRate, rotateRate);
			}

			CalculateRotation();
		}
		else
		{
			EnableMouse();
		}

		m_InitialMousePosition = mouse;
		m_Position += m_PositionDelta;

		if (isPerspective)
		{
			m_Yaw += m_YawDelta;
			m_Pitch += m_PitchDelta;
		}
		else if (isOrthographic)
		{
			m_Yaw = m_Pitch = m_Distance = 0.0f;
			m_Position.z = 0.1f;
			m_CameraMode = CameraMode::FlyCam;
		}

		if (m_CameraMode == CameraMode::ArcBall)
		{
			m_Position = CalculatePosition();
		}

		UpdateCameraView();
	}

	void EditorCamera::SetDistance(float distance)
	{
		if (IsPerspective())
		{
			m_Distance = distance;
		}
		else if (IsOrthographic())
		{
			m_OrthographicSize = distance;
			m_IsDirty = true;
		}
	}

	float EditorCamera::GetCameraSpeed() const
	{
		float speed = m_Speed;

		if (Input::IsKeyDown(KeyCode::LeftControl))
		{
			speed /= 2 - Math::Log(m_Speed);
		}
		else if (Input::IsKeyDown(KeyCode::LeftShift))
		{
			speed *= 2 - Math::Log(m_Speed);
		}

		return Math::Clamp(speed, MIN_SPEED, MAX_SPEED);
	}

	void EditorCamera::CalculateRotation()
	{
		const float upSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;
		const Math::vec3 yawSign{ 0.0f, upSign, 0.0f };

		m_RightDirection = Math::Cross(m_Direction, yawSign);

		m_Direction = Math::Rotate(
			Math::Normalize(Math::Cross(Math::AngleAxis(-m_PitchDelta, m_RightDirection),
				Math::AngleAxis(-m_YawDelta, yawSign))),
			m_Direction
		);

		const float distance = Math::Distance(m_FocalPoint, m_Position);
		m_FocalPoint = m_Position + GetForwardDirection() * distance;
		m_Distance = distance;
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
		const float yawDamp = 0.6f;
		const float pitchDamp = 0.6f;
		const float posDamp = 0.6f;

		m_YawDelta *= yawDamp;
		m_PitchDelta *= pitchDamp;
		m_PositionDelta *= posDamp;
	}

	void EditorCamera::Focus(const Math::vec3& focusPoint)
	{
		m_CameraMode = CameraMode::FlyCam;
		m_FocalPoint = focusPoint;

		if (IsOrthographic())
		{
			m_FocalPoint.z = 0.0f;
			m_Position = m_FocalPoint;
			SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
		}
		else if (IsPerspective())
		{
			if (m_Distance > m_MinFocusDistance)
			{
				m_Distance -= m_Distance - m_MinFocusDistance;
				m_Position = m_FocalPoint - GetForwardDirection() * m_Distance;
			}

			m_Position = m_FocalPoint - GetForwardDirection() * m_Distance;
			UpdateCameraView();
		}
	}

	std::pair<float, float> EditorCamera::PanSpeed() const
	{
		const float x = Math::Min(m_ViewportSize.x / 1000.0f, 2.4f); // max = 2.4f
		const float sqrtX = x * x;
		const float xFactor = 0.0366f * sqrtX - 0.1778f * x + 0.3021f;

		const float y = Math::Min(m_ViewportSize.y / 1000.0f, 2.4f); // max = 2.4f
		const float sqrtY = y * y;
		const float yFactor = 0.0366f * sqrtY - 0.1778f * y + 0.3021f;

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

	float EditorCamera::OrthoZoomSpeed() const
	{
		const float dt = Time::GetDeltaTime();
		const float currentSize = m_OrthographicSize;
		const float targetSize = m_OrthographicSize + (dt * 2.0f);
		return Math::Lerp(currentSize, targetSize, dt);
	}

	void EditorCamera::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<MouseScrolledEvent>([this](MouseScrolledEvent& e) { return OnMouseScroll(e); });
	}

	bool EditorCamera::OnMouseScroll(MouseScrolledEvent& e)
	{
		const bool rightMouseButtonDown = Input::IsMouseButtonDown(MouseButton::Right);
		const bool flyCam = m_CameraMode == CameraMode::FlyCam;

		if (flyCam && rightMouseButtonDown)
		{
			m_Speed += e.GetYOffset() * Time::GetDeltaTime() * 50.0f;
			m_Speed = std::clamp(m_Speed, MIN_SPEED, MAX_SPEED);
			return false;
		}
		
		if (IsPerspective())
		{
			MouseZoom(e.GetYOffset() * 0.1f);
			UpdateCameraView();
		}

		if (IsOrthographic())
		{
			OrthoZoom(e.GetYOffset() * 0.1f);
		}

		return false;
	}

	void EditorCamera::MousePan(const Math::vec2& delta)
	{
		auto [xSpeed, ySpeed] = PanSpeed();
		m_FocalPoint -= GetRightDirection() * delta.x * xSpeed * m_Distance;
		m_FocalPoint += GetUpDirection() * delta.y * ySpeed * m_Distance;
	}

	void EditorCamera::OrthoPan(const Math::vec2& delta)
	{
		auto [xSpeed, ySpeed] = PanSpeed();
		m_Position -= GetRightDirection() * delta.x * xSpeed * m_OrthographicSize;
		m_Position += GetUpDirection() * delta.y * ySpeed * m_OrthographicSize;
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

	void EditorCamera::OrthoZoom(float delta)
	{
		m_OrthographicSize -= delta * OrthoZoomSpeed();
		SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
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

}
