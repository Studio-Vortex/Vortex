#include "sppch.h"
#include "OrthographicCamera.h"

namespace Sparky {

	OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top)
		:
		m_ProjectionMatrix(Math::Ortho(left, right, bottom, top, -1.0f, 1.0f)),
		m_ViewMatrix(Math::Identity()),
		m_ViewProjectionMatrix(m_ProjectionMatrix* m_ViewMatrix),
		m_Position(),
		m_Rotation(0.0f)
	{
	}

	void OrthographicCamera::ReCalculateViewMatrix()
	{
		Math::mat4 transform = Math::Translate(Math::Identity(), m_Position) *
			Math::Rotate(Math::Identity(), m_Rotation, { 0.0f, 0.0f, 1.0f });

		m_ViewMatrix = Math::Inverse(transform);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

}