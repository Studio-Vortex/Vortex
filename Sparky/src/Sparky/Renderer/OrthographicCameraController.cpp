#include "sppch.h"
#include "OrthographicCameraController.h"

#include "Sparky/Core/Input.h"
#include "Sparky/Core/KeyCodes.h"

namespace Sparky {

	OrthographicCameraController::OrthographicCameraController(float aspectRatio, bool rotation)
		: m_AspectRatio(aspectRatio), m_Bounds({ -m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel }),
		m_Camera(m_Bounds.Left, m_Bounds.Right, m_Bounds.Bottom, m_Bounds.Top), m_Rotation(rotation)
	{

	}

	void OrthographicCameraController::OnUpdate(TimeStep ts)
	{
		SP_PROFILE_FUNCTION();

		if (Input::IsKeyPressed(SP_KEY_A))
		{
			m_CameraPosition.x -= cos(Math::Deg2Rad(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
			m_CameraPosition.y -= sin(Math::Deg2Rad(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
		}
		else if (Input::IsKeyPressed(SP_KEY_D))
		{
			m_CameraPosition.x += cos(Math::Deg2Rad(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
			m_CameraPosition.y += sin(Math::Deg2Rad(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
		}

		if (Input::IsKeyPressed(SP_KEY_W))
		{
			m_CameraPosition.x += -sin(Math::Deg2Rad(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
			m_CameraPosition.y += cos(Math::Deg2Rad(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
		}
		else if (Input::IsKeyPressed(SP_KEY_S))
		{
			m_CameraPosition.x -= -sin(Math::Deg2Rad(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
			m_CameraPosition.y -= cos(Math::Deg2Rad(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
		}

		if (m_Rotation)
		{
			if (Input::IsKeyPressed(SP_KEY_Q))
				m_CameraRotation += m_CameraRotationSpeed * ts;
			else if (Input::IsKeyPressed(SP_KEY_E))
				m_CameraRotation -= m_CameraRotationSpeed * ts;

			if (m_CameraRotation > 180.0f)
				m_CameraRotation -= 360.0f;
			else if (m_CameraRotation <= -180.0f)
				m_CameraRotation += 360.0f;

			if (Input::IsKeyPressed(SP_KEY_R))
				ResetRotation();

			m_Camera.SetRotation(m_CameraRotation);
		}

		if (Input::IsKeyPressed(SP_KEY_LEFT_SHIFT))
			m_CameraTranslationSpeed = m_ZoomLevel * m_ShiftKeyModiferSpeed;
		else
			m_CameraTranslationSpeed = m_ZoomLevel;

		m_Camera.SetPosition(m_CameraPosition);
	}

	void OrthographicCameraController::OnEvent(Event& e)
	{
		SP_PROFILE_FUNCTION();

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(SP_BIND_CALLBACK(OrthographicCameraController::OnMouseScrolledEvent));
		dispatcher.Dispatch<WindowResizeEvent>(SP_BIND_CALLBACK(OrthographicCameraController::OnWindowResizeEvent));
	}

	bool OrthographicCameraController::OnMouseScrolledEvent(MouseScrolledEvent& e)
	{
		SP_PROFILE_FUNCTION();

		m_ZoomLevel -= e.GetYOffset() * m_ZoomModifer; // nerf the zoom level
		m_ZoomLevel = std::min(m_ZoomLevel, m_MaxZoomOutLevel);
		m_ZoomLevel = std::max(m_ZoomLevel, m_MaxZoomInLevel);
		m_Bounds = { -m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel };
		m_Camera.SetProjection(m_Bounds.Left, m_Bounds.Right, m_Bounds.Bottom, m_Bounds.Top);

		return false;
	}

	bool OrthographicCameraController::OnWindowResizeEvent(WindowResizeEvent& e)
	{
		SP_PROFILE_FUNCTION();

		m_AspectRatio = (float)e.GetWidth() / (float)e.GetHeight();
		m_Bounds = { -m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel };
		m_Camera.SetProjection(m_Bounds.Left, m_Bounds.Right, m_Bounds.Bottom, m_Bounds.Top);

		return false;
	}

}