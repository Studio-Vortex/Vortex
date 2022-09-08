#include "sppch.h"
#include "OrthographicCameraController.h"

#include "Sparky/Core/Input.h"
#include "Sparky/Core/KeyCodes.h"

namespace Sparky {

	OrthographicCameraController::OrthographicCameraController(float aspectRatio, bool rotation)
		: m_AspectRatio(aspectRatio), m_Camera(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel), m_Rotation(rotation)
	{

	}

	void OrthographicCameraController::OnUpdate(TimeStep ts)
	{
		if (Input::IsKeyPressed(SP_KEY_W))
			m_CameraPosition.y += m_CameraTranslationSpeed * ts;
		else if (Input::IsKeyPressed(SP_KEY_S))
			m_CameraPosition.y -= m_CameraTranslationSpeed * ts;

		if (Input::IsKeyPressed(SP_KEY_A))
			m_CameraPosition.x -= m_CameraTranslationSpeed * ts;
		else if (Input::IsKeyPressed(SP_KEY_D))
			m_CameraPosition.x += m_CameraTranslationSpeed * ts;

		if (m_Rotation)
		{
			if (Input::IsKeyPressed(SP_KEY_Q))
				m_CameraRotation -= m_CameraRotationSpeed * ts;
			else if (Input::IsKeyPressed(SP_KEY_E))
				m_CameraRotation += m_CameraRotationSpeed * ts;

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
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(SP_BIND_CALLBACK(OrthographicCameraController::OnMouseScrolledEvent));
		dispatcher.Dispatch<WindowResizeEvent>(SP_BIND_CALLBACK(OrthographicCameraController::OnWindowResizeEvent));
	}

	bool OrthographicCameraController::OnMouseScrolledEvent(MouseScrolledEvent& e)
	{
		m_ZoomLevel -= e.GetYOffset() * 0.25f;
		m_ZoomLevel = std::min(m_ZoomLevel, 20.0f); // zoom out limit
		m_ZoomLevel = std::max(m_ZoomLevel, 0.25f); // zoom in limit
		m_Camera.SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);

		return false;
	}

	bool OrthographicCameraController::OnWindowResizeEvent(WindowResizeEvent& e)
	{
		m_AspectRatio = (float)e.GetWidth() / (float)e.GetHeight();
		m_Camera.SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);

		return false;
	}

}