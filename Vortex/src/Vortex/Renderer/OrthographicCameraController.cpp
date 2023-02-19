#include "vxpch.h"
#include "OrthographicCameraController.h"

#include "Vortex/Core/Input.h"
#include "Vortex/Core/KeyCodes.h"

namespace Vortex {

	OrthographicCameraController::OrthographicCameraController(float aspectRatio, bool rotation)
		: m_AspectRatio(aspectRatio), m_Bounds({ -m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel }),
		m_Camera(m_Bounds.Left, m_Bounds.Right, m_Bounds.Bottom, m_Bounds.Top), m_Rotation(rotation) { }

	void OrthographicCameraController::OnUpdate(TimeStep ts)
	{
		VX_PROFILE_FUNCTION();

		if (Input::IsKeyDown(KeyCode::A))
		{
			m_CameraPosition.x -= cos(Math::Deg2Rad(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
			m_CameraPosition.y -= sin(Math::Deg2Rad(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
		}
		else if (Input::IsKeyDown(KeyCode::D))
		{
			m_CameraPosition.x += cos(Math::Deg2Rad(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
			m_CameraPosition.y += sin(Math::Deg2Rad(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
		}

		if (Input::IsKeyDown(KeyCode::W))
		{
			m_CameraPosition.x += -sin(Math::Deg2Rad(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
			m_CameraPosition.y += cos(Math::Deg2Rad(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
		}
		else if (Input::IsKeyDown(KeyCode::S))
		{
			m_CameraPosition.x -= -sin(Math::Deg2Rad(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
			m_CameraPosition.y -= cos(Math::Deg2Rad(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
		}

		if (m_Rotation)
		{
			if (Input::IsKeyDown(KeyCode::Q))
				m_CameraRotation += m_CameraRotationSpeed * ts;
			else if (Input::IsKeyDown(KeyCode::E))
				m_CameraRotation -= m_CameraRotationSpeed * ts;

			if (m_CameraRotation > 180.0f)
				m_CameraRotation -= 360.0f;
			else if (m_CameraRotation <= -180.0f)
				m_CameraRotation += 360.0f;

			if (Input::IsKeyDown(KeyCode::R))
				ResetRotation();

			m_Camera.SetRotation(m_CameraRotation);
		}

		if (Input::IsKeyDown(KeyCode::LeftShift))
			m_CameraTranslationSpeed = m_ZoomLevel * m_ShiftKeyModiferSpeed;
		else
			m_CameraTranslationSpeed = m_ZoomLevel;

		m_Camera.SetPosition(m_CameraPosition);
	}

	void OrthographicCameraController::OnEvent(Event& e)
	{
		VX_PROFILE_FUNCTION();

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(VX_BIND_CALLBACK(OrthographicCameraController::OnMouseScrolledEvent));
		dispatcher.Dispatch<WindowResizeEvent>(VX_BIND_CALLBACK(OrthographicCameraController::OnWindowResizeEvent));
	}

	void OrthographicCameraController::OnResize(const Math::vec2& size)
	{
		m_AspectRatio = size.x / size.y;
		CalculateView();
	}

	void OrthographicCameraController::CalculateView()
	{
		m_Bounds = { -m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel };
		m_Camera.SetProjection(m_Bounds.Left, m_Bounds.Right, m_Bounds.Bottom, m_Bounds.Top);
	}

	bool OrthographicCameraController::OnMouseScrolledEvent(MouseScrolledEvent& e)
	{
		VX_PROFILE_FUNCTION();

		m_ZoomLevel -= e.GetYOffset() * m_ZoomModifer; // nerf the zoom level
		m_ZoomLevel = std::min(m_ZoomLevel, m_MaxZoomOutLevel);
		m_ZoomLevel = std::max(m_ZoomLevel, m_MaxZoomInLevel);
		CalculateView();

		return false;
	}

	bool OrthographicCameraController::OnWindowResizeEvent(WindowResizeEvent& e)
	{
		VX_PROFILE_FUNCTION();

		OnResize({ (float)e.GetWidth(), (float)e.GetHeight() });

		return false;
	}

}